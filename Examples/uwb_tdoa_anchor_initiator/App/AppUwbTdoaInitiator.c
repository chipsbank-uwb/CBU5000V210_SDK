/**
 * @file    AppUwbTdoaInitiator.c
 * @brief   TDOA anchor initiator: Poll-DTM broadcast and position solving.
 * @details Role slice of the exchange in tdoa_common.h. The initiator broadcasts
 *          Poll-DTM each round (advertising its previous-round TX timestamp),
 *          receives one Tag-Report, and runs the Chan/Taylor solver in TdoaMath.h.
 *
 *          State machine:
 *            IDLE →(Timer0) TX_SEND → WAIT_TX_DONE →(txdone or guard) RX_ACTIVE
 *            →(TAG_REPORT or Timer0) RX_FINALIZE → IDLE
 *
 *          Timers: Timer0 = 10ms free-run round trigger; Timer1 = 20ms one-shot
 *          watchdog, armed only on the IDLE→TX_SEND edge and disarmed in
 *          RX_FINALIZE / on CAL_REQ hand-off.
 *
 * @author  Chipsbank
 * @date    2026
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbTdoaInitiator.h"
#include "AppUwbCalibInitiator.h"
#include "TdoaMath.h"
#include "tdoa_common.h"
#include "tdoa_config.h"
#include "CB_uwbframework.h"
#include "CB_timer.h"
#include "CB_scr.h"
#include "CB_system.h"
#include "NonLIB_sharedUtils.h"
#include "APP_common.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_TDOA_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_TDOA_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_tdoa_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_tdoa_print(...)
#endif

//-------------------------------
// TDOA: INITIATOR SETUP
//-------------------------------
// (Full four-node sequence diagram lives in tdoa_common.h; below is the
//  initiator's own role view.)
//-------------------------------------------------------
//    Initiator                 Responder Anchors / Tag
//      Idle                               Idle
//     a |---------1. Poll-DTM(m) --------->|
//       |<--------2. Resp-DTM(m, id=1) ----|  (not consumed by initiator)
//       |<--------3. Resp-DTM(m, id=2) ----|  (not consumed by initiator)
//       |<--------4. Resp-DTM(m, id=3) ----|  (not consumed by initiator)
//       |<--------5. Tag-Report(m) --------|
//      Finalize                          Finalize
//       |---------6. Poll-DTM(m+1) ------->|
//      Repeat                             Repeat
//
// DEF_TDOA_INITIATOR_POLL_PERIOD_MS : Timer0 period between rounds (1, 6)
// TDOA_WATCHDOG_TIMEOUT_MS          : Timer1 one-shot per-round watchdog
// TDOA_TXDONE_GUARD_MS              : software guard for POLL TX-done (offset 0)
//
//  a: s_stTxPrev (POLL TX timestamp, fed back in the next round)
//  5: Tag-Report dist_diff[3] + anchor_rx_mask -> Chan/Taylor solve
//-------------------------------------------------------
//-------------------------------
// DEFINE SECTION
//-------------------------------

/* ===================================================================
 * Initiator-local round timing
 * =================================================================== */
#define DEF_TDOA_INITIATOR_POLL_PERIOD_MS   10u

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum {
    EN_APP_STATE_IDLE = 0,      /* Idle; wait for the Timer0 period to start a new round. */
    EN_APP_STATE_TX_SEND,       /* Build and transmit the Poll-DTM frame. */
    EN_APP_STATE_WAIT_TX_DONE,  /* Wait until the Poll-DTM TX-done IRQ fires. */
    EN_APP_STATE_RX_ACTIVE,     /* RX is open for the Tag-Report frame. */
    EN_APP_STATE_RX_FINALIZE,   /* Round ended; close RX and solve the tag position. */
} app_uwbtdoa_state_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct {
    volatile uint8_t Timer0Expired;
    volatile uint8_t TxDone;
    volatile uint8_t Rx0Done;
} app_uwbtdoa_irqstatus_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static cb_uwbsystem_packetconfig_st s_stUwbPacketConfig = DEF_TDOA_INITIATOR_PACKET_CONFIG_DEFAULTS;
static cb_uwbsystem_tx_irqenable_st s_stTxIrqEnable     = { .txDone  = 1 };
static cb_uwbsystem_rx_irqenable_st s_stRxIrqEnable     = { .rx0Done = 1 };

static volatile app_uwbtdoa_state_en s_enAppTdoaState = EN_APP_STATE_IDLE;

/* IRQ-to-main-loop flags.
 * The IRQ only sets compact flags here. Protocol parsing and radio restarts
 * stay in the process context to keep ISR work bounded and deterministic.
 */
static volatile app_uwbtdoa_irqstatus_st s_stIrqStatus = { 0 };

/* Round-level state shared by the periodic TDOA exchange. */
static uint32_t s_roundIndex = 0;

/* Previous-round Poll-DTM TX timestamp, returned in the next Poll-DTM so
 * responders and the tag can solve the cross-round timing.
 */
static cb_uwbsystem_tx_tsutimestamp_st s_stTxPrev;
static uint8_t s_txPrevValid = 0;

/* Per-round anchor-RX mask received from the Tag-Report payload. */
static uint8_t s_rxMaskPending = 0;

/* Tag-side drop diagnostics from the Tag-Report payload. drop_reason == NONE
 * means the report carries a valid result; otherwise it is debug-only. */
static uint8_t  s_reportDropReason = DEF_TDOA_DROP_NONE;
static uint16_t s_reportOverrun    = 0;

/* Tag-Report data used as the input to the position solver in TdoaMath.h. */
static tdoa_math_position_context_st s_stPositionContext = {0};
static float   s_reportDistDiff[3] = {0.0f};
static uint8_t s_reportReceived = 0;

/* Watchdog / txdone-guard state. */
static volatile app_uwbtdoa_state_en s_failureState     = EN_APP_STATE_IDLE;
static volatile uint8_t              s_watchdogTimeout  = 0;
static uint32_t                      s_txStartTickMs    = 0;  /* Recorded when entering WAIT_TX_DONE. */

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void    tdoa_role_apply_default_config(tdoa_runtime_config_t* config);
static void    app_tdoa_timeout_error_message_print(void);
static uint8_t app_tdoa_handle_boot_calib(void);
static void    app_tdoa_start_poll_tx(void);
static void    app_tdoa_handle_tx_done(void);
static void    app_tdoa_process_rx0_done(void);
static void    app_tdoa_finalize_round(uint8_t report_received);
void    cb_timer_0_app_irq_callback(void);
void    cb_timer_1_app_irq_callback(void);
void    cb_uwbapp_tx_done_irqhandler(void);
void    cb_uwbapp_rx0_done_irqcb(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void tdoa_role_apply_default_config(tdoa_runtime_config_t* config)
{
    if (config == 0) {
        return;
    }

    config->device_id    = TDOA_DEFAULT_INITIATOR_ID;
    config->slot         = DEF_TDOA_INITIATOR_SLOT;
    config->t_prop       = TDOA_DEFAULT_T_PROP_NS;
    config->bias         = TDOA_DEFAULT_BIAS_CM;
    config->calib_source = 0;
}

void app_tdoa_initiator(void)
{
    cb_framework_uwb_init();
    app_tdoa_reset();
    app_tdoa_timer_init(DEF_TDOA_INITIATOR_POLL_PERIOD_MS);
    app_tdoa_calib_init();
    s_enAppTdoaState = EN_APP_STATE_IDLE;

    app_uwb_tdoa_print("[INIT] Initiator ready, Timer0=%ums\r\n",
                       DEF_TDOA_INITIATOR_POLL_PERIOD_MS);

    while (1)
    {
        /* Drain the watchdog-timeout flag first so recovery happens before other processing. */
        if (s_watchdogTimeout) {
            s_watchdogTimeout = 0;
            app_tdoa_timeout_error_message_print();

            /* Clear stale IRQ flags accumulated during the failed round (incl. a latched Timer0). */
            s_stIrqStatus.TxDone        = 0;
            s_stIrqStatus.Rx0Done       = 0;
            s_stIrqStatus.Timer0Expired = 0;

            /* Reset link hardware and go IDLE so the next Timer0 edge restarts cleanly. */
            cb_framework_uwb_rx_end(EN_UWB_RX_0);
            cb_framework_uwb_tx_end();
            s_enAppTdoaState = EN_APP_STATE_IDLE;
            continue;
        }

        /* Drain the RX0 IRQ flag so both boot-calib overlay and normal TDOA consume from one path. */
        if (s_stIrqStatus.Rx0Done) {
            app_tdoa_process_rx0_done();
        }

        if (app_tdoa_handle_boot_calib()) {
            continue;
        }

        switch (s_enAppTdoaState)
        {
            case EN_APP_STATE_IDLE:
                if (s_stIrqStatus.Timer0Expired) {
                    /* Arm watchdog on the IDLE→TX_SEND edge only; clear flag first to avoid a timer_off/arm race. */
                    s_watchdogTimeout = 0;
                    tdoa_watchdog_timer_init(EN_TIMER_1, TDOA_WATCHDOG_TIMEOUT_MS);
                    s_enAppTdoaState = EN_APP_STATE_TX_SEND;
                }
                break;

            case EN_APP_STATE_TX_SEND:
                app_tdoa_start_poll_tx();
                s_enAppTdoaState = EN_APP_STATE_WAIT_TX_DONE;
                break;

            case EN_APP_STATE_WAIT_TX_DONE:
                if (s_stIrqStatus.TxDone) {
                    app_tdoa_handle_tx_done();
                    s_enAppTdoaState = EN_APP_STATE_RX_ACTIVE;
                } else {
                    if (cb_hal_is_time_elapsed_ms(s_txStartTickMs, (uint32_t)TDOA_TXDONE_GUARD_MS) == CB_PASS) {
                        /* Guard expired — TxDone IRQ lost: end TX, clear late TxDone, resend the same POLL. */
                        cb_framework_uwb_tx_end();
                        s_stIrqStatus.TxDone   = 0;
                        s_stIrqStatus.Rx0Done  = 0;
                        s_enAppTdoaState = EN_APP_STATE_TX_SEND;
                    }
                }
                break;

            case EN_APP_STATE_RX_ACTIVE:
                if (s_reportReceived) {
                    /* Tag report arrived — finalize immediately. */
                    s_enAppTdoaState = EN_APP_STATE_RX_FINALIZE;
                } else if (s_stIrqStatus.Timer0Expired) {
                    /* Round period elapsed without a Tag-Report; close the round. */
                    s_enAppTdoaState = EN_APP_STATE_RX_FINALIZE;
                }
                break;

            case EN_APP_STATE_RX_FINALIZE:
                /* Always go IDLE first so the watchdog arm edge is hit next round (no RX_FINALIZE→TX_SEND jump). */
                app_tdoa_finalize_round(s_reportReceived);
                tdoa_watchdog_timer_off(EN_TIMER_1);
                s_enAppTdoaState = EN_APP_STATE_IDLE;
                break;

            default:
                s_enAppTdoaState = EN_APP_STATE_IDLE;
                break;
        }
    }
}

/**
 * @brief Reset initiator round state before (re)entering the main loop.
 */
void app_tdoa_reset(void)
{
    memset((void *)&s_stIrqStatus, 0, sizeof(s_stIrqStatus));
    s_roundIndex     = 0;
    s_txStartTickMs  = 0;
    s_txPrevValid    = 0;
    s_rxMaskPending  = 0;
    s_reportReceived = 0;
    s_watchdogTimeout = 0;
    s_failureState   = EN_APP_STATE_IDLE;
    memset(s_reportDistDiff, 0, sizeof(s_reportDistDiff));
    tdoa_math_position_context_reset(&s_stPositionContext);
}

/**
 * @brief Configure Timer0 as the periodic trigger for starting a new TDOA round.
 *
 * @param timeoutMs Round period in milliseconds.
 */
void app_tdoa_timer_init(uint16_t timeoutMs)
{
    stTimerSetUp timer_cfg;

    cb_timer_struct_init(&timer_cfg);
    timer_cfg.Timer = EN_TIMER_0;
    timer_cfg.TimerMode = EN_TIMER_MODE_FREERUN;
    timer_cfg.stTimeOut.TimerTimeoutEvent = DEF_TIMER_TIMEOUT_EVENT_0;
    timer_cfg.stTimeOut.timeoutVal[0] = timeoutMs;
    timer_cfg.TimeUnit = EN_TIMER_MS;

    cb_scr_timer0_module_on();
    cb_timer_init(EN_TIMER_0);
    cb_timer_configure_timer(&timer_cfg);
    cb_timer_enable(EN_TIMER_0);
}

/**
 * @brief Stop Timer0 and disable its IRQ line.
 */
void app_tdoa_timer_off(void)
{
    cb_timer_disable_timer(EN_TIMER_0);
    cb_scr_timer0_module_off();
    NVIC_DisableIRQ(TIMER_0_IRQn);
}

/**
 * @brief Print a human-readable timeout message showing which state the watchdog fired in.
 */
static void app_tdoa_timeout_error_message_print(void)
{
    switch (s_failureState)
    {
        case EN_APP_STATE_IDLE:
            app_uwb_tdoa_print("[INIT] Timeout:IDLE\r\n");
            break;
        case EN_APP_STATE_TX_SEND:
            app_uwb_tdoa_print("[INIT] Timeout:TX_SEND\r\n");
            break;
        case EN_APP_STATE_WAIT_TX_DONE:
            app_uwb_tdoa_print("[INIT] Timeout:WAIT_TX_DONE\r\n");
            break;
        case EN_APP_STATE_RX_ACTIVE:
            app_uwb_tdoa_print("[INIT] Timeout:RX_ACTIVE\r\n");
            break;
        case EN_APP_STATE_RX_FINALIZE:
            app_uwb_tdoa_print("[INIT] Timeout:RX_FINALIZE\r\n");
            break;
        default:
            app_uwb_tdoa_print("[INIT] Timeout:UNKNOWN\r\n");
            break;
    }
}

/**
 * @brief Pause normal TDOA while boot calibration owns the radio.
 *
 * @return 1 when calibration consumed the tick (caller should skip TDOA processing), 0 otherwise.
 */
static uint8_t app_tdoa_handle_boot_calib(void)
{
    /* Boot calib is an overlay: while active the POLL/RX loop pauses and radio events go to the calib module. */
    if (app_tdoa_calib_is_done()) {
        app_tdoa_calib_stop();
        s_rxMaskPending  = 0;
        s_txPrevValid    = 0;
        s_txStartTickMs  = 0;
        s_reportReceived = 0;
        s_stIrqStatus.TxDone        = 0;
        s_stIrqStatus.Rx0Done       = 0;
        s_stIrqStatus.Timer0Expired = 0;
        s_enAppTdoaState = EN_APP_STATE_IDLE;
        app_tdoa_timer_init(DEF_TDOA_INITIATOR_POLL_PERIOD_MS);   /* Re-arm the round trigger stopped at CAL_REQ. */
        app_uwb_tdoa_print("[INIT] boot calib done, resume TDOA\r\n");
        return 1u;
    }

    if (app_tdoa_calib_is_active()) {
        app_tdoa_calib_process();
        return 1u;
    }

    return 0u;
}

/**
 * @brief Build and transmit Poll-DTM, advertising the previous round's TX timestamp.
 */
static void app_tdoa_start_poll_tx(void)
{
    tdoa_poll_payload_t poll;
    cb_uwbsystem_txpayload_st tx_payload;

    memset(&poll, 0, sizeof(poll));

    /* Clear the Timer0 flag before sending so IDLE does not see a stale period trigger. */
    s_stIrqStatus.Timer0Expired = 0;
    s_roundIndex++;

    poll.frame_type    = TDOA_FRAME_TYPE_POLL_DTM;
    poll.device_id     = g_tdoa_config.device_id;
    poll.round_index   = s_roundIndex;
    poll.tx_prev_valid = s_txPrevValid;

    if (s_txPrevValid) {
        /* Copy the SDK timestamp struct whole; never dereference its fields from a packed buffer. */
        poll.tsu_tx_init_prev = s_stTxPrev;
    }

    tx_payload.ptrAddress  = (uint8_t*)&poll;
    tx_payload.payloadSize = sizeof(tdoa_poll_payload_t);

    /* Record the tick before tx_start so the guard deadline is relative to the TX request, not the IRQ. */
    s_txStartTickMs = cb_hal_get_time_ms();

    cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &tx_payload, &s_stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
}

/**
 * @brief Latch the POLL TX timestamp and open RX for the Tag-Report frame.
 */
static void app_tdoa_handle_tx_done(void)
{
    cb_uwbsystem_tx_tsutimestamp_st tx_tsu;

    s_stIrqStatus.TxDone = 0;

    /* POLL TX timestamp is valid only after TX-done; store it for the next round's feedback payload. */
    cb_framework_uwb_get_tx_tsu_timestamp(&tx_tsu);
    cb_framework_uwb_tx_end();

    s_stTxPrev    = tx_tsu;
    s_txPrevValid = 1;

    s_rxMaskPending  = 0;
    s_reportReceived = 0;

    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
}

/**
 * @brief Deferred RX0-done processor running in main-loop context.
 *
 * Handles TAG_REPORT and CAL_REQ; RESP_DTM frames are ignored (req 6).
 */
static void app_tdoa_process_rx0_done(void)
{
    cb_uwbsystem_rxstatus_un rx_status;
    uint16_t pkt_size;
    uint8_t  payload[TDOA_RX_MSG_MAX_LEN];
    uint8_t  frame_type;

    s_stIrqStatus.Rx0Done = 0;

    rx_status = cb_framework_uwb_get_rx_status();
    if (!rx_status.rx0_ok) {
        return;
    }

    if (app_tdoa_calib_is_active()) {
        /* Calibration owns the radio while active. */
        app_tdoa_calib_rx_done_irqcb();
        return;
    }

    pkt_size = cb_framework_uwb_get_rx_packet_size(&s_stUwbPacketConfig);
    if (pkt_size == 0u || pkt_size > TDOA_RX_MSG_MAX_LEN) {
        /* Bad packet — restart RX so we remain open for the rest of the round. */
        cb_framework_uwb_rx_end(EN_UWB_RX_0);
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        return;
    }

    cb_framework_uwb_get_rx_payload(payload, pkt_size);
    frame_type = payload[0];

    if (frame_type == TDOA_FRAME_TYPE_CAL_REQ && pkt_size >= sizeof(tdoa_calib_req_payload_t)) {
        const tdoa_calib_req_payload_t* req = (const tdoa_calib_req_payload_t*)payload;

        /* A Responder requested boot calibration: disarm the watchdog, abort the round, hand off to calib. */
        tdoa_watchdog_timer_off(EN_TIMER_1);
        app_tdoa_timer_off();              /* Stop the 10ms round trigger during calibration. */

        cb_framework_uwb_rx_end(EN_UWB_RX_0);
        cb_framework_uwb_tx_end();

        s_stIrqStatus.TxDone        = 0;
        s_stIrqStatus.Rx0Done       = 0;
        s_stIrqStatus.Timer0Expired = 0;
        s_enAppTdoaState            = EN_APP_STATE_IDLE;

        app_tdoa_calib_start(req->responder_id, req->seq);
        return;
    }

    if (frame_type == TDOA_FRAME_TYPE_TAG_REPORT && pkt_size >= sizeof(tdoa_report_payload_t)) {
        const tdoa_report_payload_t* report = (const tdoa_report_payload_t*)payload;

        s_reportDistDiff[0] = report->tdoa_dist_diff[0];
        s_reportDistDiff[1] = report->tdoa_dist_diff[1];
        s_reportDistDiff[2] = report->tdoa_dist_diff[2];
        s_rxMaskPending     = report->anchor_rx_mask;
        s_reportDropReason  = report->drop_reason;
        s_reportOverrun     = report->overrun;
        s_reportReceived    = 1;
        return;
    }

    /* Any other frame type (e.g. stray RESP_DTM) — restart RX to keep listening. */
    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
}

/**
 * @brief Close the round, print diagnostics, and optionally solve the tag position.
 *
 * @param report_received  1 if a TAG_REPORT was received this round, 0 otherwise.
 */
static void app_tdoa_finalize_round(uint8_t report_received)
{
    cb_framework_uwb_rx_end(EN_UWB_RX_0);

    if (report_received) {
        if (s_reportDropReason != DEF_TDOA_DROP_NONE) {
            /* Debug-only report: mirror the tag's drop diagnostics; no solve. */
            app_uwb_tdoa_print("[R:%lu] TAG_DBG:REASON=%s,OVR=%u ",
                               (unsigned long)(s_roundIndex - 1u),
                               tdoa_drop_reason_to_str(s_reportDropReason),
                               (unsigned)s_reportOverrun);
            tdoa_print_lack_anchors(s_rxMaskPending, app_uart_printf);
            app_uwb_tdoa_print("\r\n");
        } else {
            tdoa_math_position_result_st position;

            if (tdoa_math_calculate_position(&s_stPositionContext,
                                             &g_tdoa_config,
                                             s_reportDistDiff,
                                             &position) != 0u) {
                app_uwb_tdoa_print(
                    "[R:%lu] DIS01: %.3f cm,DIS02: %.3f cm,DIS03: %.3f cm | "
                    "POS_CALC POS:(%.3f, %.3f) ITER:%u\r\n",
                    (unsigned long)(s_roundIndex - 1u),
                    (double)(s_reportDistDiff[0] * 100.0f),
                    (double)(s_reportDistDiff[1] * 100.0f),
                    (double)(s_reportDistDiff[2] * 100.0f),
                    (double)position.x,
                    (double)position.y,
                    (unsigned)position.taylorIterations);
            }
        }

        s_reportReceived = 0;
    } else {
        app_uwb_tdoa_print("NO TAG REPORT!\r\n");
    }

    s_rxMaskPending    = 0;
    s_reportDropReason = DEF_TDOA_DROP_NONE;
    s_reportOverrun    = 0;
}

/**
 * @brief Timer0 periodic trigger callback.
 *
 * Fires at DEF_TDOA_INITIATOR_POLL_PERIOD_MS boundaries. The flag is consumed
 * in EN_APP_STATE_IDLE to start the next Poll-DTM round and in EN_APP_STATE_RX_ACTIVE
 * to close a round that produced no Tag-Report.
 */
void cb_timer_0_app_irq_callback(void)
{
    s_stIrqStatus.Timer0Expired = 1;
}

/**
 * @brief Timer1 one-shot watchdog callback.
 *
 * Fires 20ms after the round starts (IDLE→TX_SEND arm edge). Records the
 * failure state and raises the timeout flag for the main loop to process.
 * When boot calibration is active, the calib module owns Timer1 and this
 * path routes to the calib timeout handler.
 */
void cb_timer_1_app_irq_callback(void)
{
    if (app_tdoa_calib_is_active()) {
        app_tdoa_calib_initiator_timer_irqcb();
        return;
    }
    s_failureState    = s_enAppTdoaState;
    s_watchdogTimeout = 1;
}

/**
 * @brief UWB TX-done IRQ. Routes to calibration when active, otherwise raises a flag.
 */
void cb_uwbapp_tx_done_irqhandler(void)
{
    if (app_tdoa_calib_is_active()) {
        app_tdoa_calib_tx_done_irqcb();
        return;
    }
    s_stIrqStatus.TxDone = 1;
}

/**
 * @brief UWB RX0-done IRQ. Raises a flag for main-loop processing.
 */
void cb_uwbapp_rx0_done_irqcb(void)
{
    /* Keep the ISR minimal: validate status and raise a flag; parsing is deferred to main-loop context. */
    cb_uwbsystem_rxstatus_un rx_status = cb_framework_uwb_get_rx_status();
    if (rx_status.rx0_ok) {
        s_stIrqStatus.Rx0Done = 1;
    }
}
