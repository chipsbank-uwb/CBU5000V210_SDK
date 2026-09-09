/**
 * @file    AppUwbTdoaResponder.c
 * @brief   TDOA responder: synchronized TX timestamps in a scheduled slot.
 * @details Role slice of the exchange in tdoa_common.h. The responder receives
 *          Poll-DTM, estimates the initiator/responder clock ratio across two
 *          rounds, and transmits Response-DTM in its own slot carrying the
 *          synchronized TX timestamp. Timestamp arithmetic is isolated in
 *          TdoaMath.h so the timing model can be reviewed apart from the state
 *          machine.
 *
 * @note Poll-DTM carries the previous round (m-1) initiator TX timestamp, so the
 *       responder feeds current response timing back in the next round.
 * @author  Chipsbank
 * @date    2026
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbTdoaResponder.h"
#include "AppUwbCalibResponder.h"
#include "tdoa_common.h"
#include "tdoa_config.h"
#include "TdoaMath.h"
#include "CB_system.h"
#include "CB_uwbframework.h"
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
// TDOA: RESPONDER SETUP
//-------------------------------
// (Full four-node sequence diagram lives in tdoa_common.h; below is the
//  responder's own role view.)
//-------------------------------------------------------
//    Initiator                         Responder
//      Idle                               Idle
//       |---------1. POLL_DTM(m) --------->| a
//       |<--------2. RESP_DTM(m) ----------| b  (deferred: POLL SFD + slot ms)
//       |---------3. POLL_DTM(m+1) ------->|
//       |<--------4. RESP_DTM(m+1) --------|
//     Repeat                             Repeat
//
// DEF_TDOA_RESP_SLOT_DELAY_US      : abs timer 0 offset for deferred RESP TX (2)
// DEF_TDOA_RESP_RX_REOPEN_DELAY_US : abs timer 1 offset for RX reopen after TX
// TDOA_WATCHDOG_TIMEOUT_MS         : Timer0 one-shot watchdog per round
//
//   clock ratio = delta(init TX) / delta(resp RX)
//   sync TX     = init TX ref + resp turnaround * ratio + sum T_p bias
//   (sync becomes valid only after two complete rounds of history)
//
//  a: current round local RX timestamp in s_stHistory[0].rxTsu
//  b: current round local TX timestamp in s_stHistory[0].txTsu
//     history[0]=round m, [1]=round m-1 (sync), [2]=round m-2 (clk-ratio delta)
//-------------------------------------------------------
//-------------------------------
// DEFINE SECTION
//-------------------------------
/* Absolute delay (us) from POLL SFD to RX reopen after our RESP TX done. */
#define DEF_TDOA_RESP_RX_REOPEN_DELAY_US     9000u
/* Slot interval: each responder slot is 1 ms apart. */
#define DEF_TDOA_RESP_SLOT_INTERVAL_MS        1u
#define DEF_TDOA_RESP_SLOT_DELAY_US(slot_index) \
    ((uint32_t)(slot_index) * (uint32_t)DEF_TDOA_RESP_SLOT_INTERVAL_MS * 1000u)

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum {
    EN_APP_STATE_RX_WAIT,       /* RX is open and waiting for the initiator Poll-DTM. */
    EN_APP_STATE_TX_SEND,       /* A valid Poll-DTM was received; build and schedule Response-DTM. */
    EN_APP_STATE_WAIT_TX_DONE,  /* Scheduled TX is pending; wait until the radio reports TX done. */
} app_uwbtdoa_state_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct {
    cb_uwbsystem_tx_tsutimestamp_st tsuTxInit;    /* Initiator TX timestamp from Poll-DTM. */
    cb_uwbsystem_rx_tsutimestamp_st rxTsu;         /* Local Poll-DTM RX timestamp. */
    cb_uwbsystem_tx_tsutimestamp_st txTsu;         /* Local Response-DTM TX timestamp. */
    uint8_t  rxValid;       /* Local RX timestamp is valid. */
    uint8_t  txValid;       /* Local TX timestamp is valid. */
    uint8_t  dataComplete;  /* Both local RX and TX timestamps are valid. */
    uint32_t roundIndex;    /* Round index used to validate history continuity. */
} app_uwbtdoa_respround_st;

typedef struct {
    volatile uint8_t TxDone;
    volatile uint8_t Rx0Done;
} app_uwbtdoa_irqstatus_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

/* Abs timer 0: schedules the deferred RESP slot TX (event = POLL SFD, abs0). */
static cb_uwbframework_trx_scheduledconfig_st s_stRespTxSchedCfg = {
    .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,
    .eventIndex         = EN_UWBEVENT_17_RX0_SFD_DET,
    .absTimer           = EN_UWB_ABSOLUTE_TIMER_0,
    .timeoutValue       = 0,
    .eventCtrlMask      = EN_UWBCTRL_TX_START_MASK,
};

/*
 * Abs timer 1: reopens RX after our RESP TX done completes (event = same POLL
 * SFD, abs1). Both abs0 and abs1 share the event timestamp captured by the same
 * enable_scheduled_trx call; they use different absolute timers to fire
 * independently. Relatch rule: every TX-done handling re-latches the shared
 * event by doing disable+enable on s_stRespTxSchedCfg BEFORE configuring this
 * abs1 timer.
 */
static cb_uwbframework_trx_scheduledconfig_st s_stRxReopenSchedCfg = {
    .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,
    .eventIndex         = EN_UWBEVENT_17_RX0_SFD_DET,
    .absTimer           = EN_UWB_ABSOLUTE_TIMER_1,
    .timeoutValue       = DEF_TDOA_RESP_RX_REOPEN_DELAY_US,
    .eventCtrlMask      = EN_UWBCTRL_RX0_START_MASK,
};

static cb_uwbsystem_packetconfig_st s_stUwbPacketConfig = DEF_TDOA_RESP_PACKET_CONFIG_DEFAULTS;
static cb_uwbsystem_tx_irqenable_st s_stTxIrqEnable = { .txDone = 1};
static cb_uwbsystem_rx_irqenable_st s_stRxIrqEnable = { .rx0Done = 1};
static volatile app_uwbtdoa_irqstatus_st s_stIrqStatus;

/* Single RX handoff slot.
 * The callback fills this slot once, then the state machine consumes and
 * clears it. If another frame arrives before consumption, RX is restarted and
 * the old slot is preserved instead of being overwritten.
 */
static tdoa_rx_msg_slot_t s_stRxSlot;

/* Circular history buffer.
 * Index 0 is always the current round after advance is called.
 * Index 1 is the previous round (m-1), and index 2 is round (m-2).
 * The clock-ratio and sync calculations need at least two completed rounds.
 */
static app_uwbtdoa_respround_st s_stHistory[TDOA_HISTORY_SIZE];
static uint8_t s_currIdx = 0;

static app_uwbtdoa_state_en s_enAppTdoaState;
static app_uwbtdoa_state_en s_failureState;    /* State saved when watchdog fires. */
static volatile uint8_t     s_watchdogTimeout; /* Set by timer IRQ; cleared on arm. */
static double               s_clkRatio;
static uint8_t              s_clkRatioValid;
static tdoa_ratio_filter_state_st s_clkRatioFilter;
static uint32_t             s_currentRound;
static uint8_t              s_calibDoneHandled;
static uint32_t             s_statTxOk;
static double               s_ticksPerNs;
static double               s_sumTpBias;
static float                s_sumTpBiasCm;

/* TX-done guard: records the tick when tx_start was called and the slot offset
 * (ms) for this TX. Deadline = s_txStartTickMs + s_txSlotOffsetMs + TDOA_TXDONE_GUARD_MS.
 */
static uint32_t s_txStartTickMs;
static uint8_t  s_txSlotOffsetMs;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void tdoa_role_apply_default_config(tdoa_runtime_config_t* config);
static void app_tdoa_start_poll_rx(void);
static uint8_t app_tdoa_calculate_clk_ratio(double* out_ratio);
static uint8_t app_tdoa_calculate_tx_sync(cb_uwbsystem_tx_tsutimestamp_st* sync_out);
void cb_uwbapp_tx_done_irqhandler(void);
void cb_uwbapp_rx0_done_irqcb(void);
void cb_timer_0_app_irq_callback(void);
void app_tdoa_responder(void);
static void app_tdoa_reset_link(void);
static void app_tdoa_rx_timeout_handler(void);
static void app_tdoa_init_state(void);
static void app_tdoa_process_rx0_done(void);
static uint8_t app_tdoa_handle_poll_received(void);
static void app_tdoa_start_resp_tx(void);
static void app_tdoa_handle_tx_done(void);
static void app_tdoa_timeout_error_message_print(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

void tdoa_role_apply_default_config(tdoa_runtime_config_t* config)
{
    if (config == 0) {
        return;
    }
    config->device_id    = TDOA_DEFAULT_RESPONDER1_ID;
    config->slot         = APP_TDOA_RESPONDER1_SLOT;
    config->t_prop       = TDOA_DEFAULT_T_PROP_NS;
    config->bias         = TDOA_DEFAULT_BIAS_CM;
    config->calib_source = 0;
}

/**
 * @brief Stop any active UWB work and clear transient link state.
 */
static void app_tdoa_reset_link(void)
{
    cb_framework_uwb_disable_scheduled_trx(s_stRespTxSchedCfg);
    cb_framework_uwb_disable_scheduled_trx(s_stRxReopenSchedCfg);
    cb_framework_uwb_tx_end();
    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    memset(&s_stRxSlot, 0, sizeof(s_stRxSlot));
    s_stIrqStatus.TxDone = 0;
}

/**
 * @brief Recover from a missing or invalid Poll-DTM window.
 * @details Performs a full disable+enable+rx_start sequence so that the
 *          absolute timer event capture is rebuilt from scratch.
 */
static void app_tdoa_rx_timeout_handler(void)
{
    app_tdoa_reset_link();
    /* Full relatch after link reset: disable clears the event, enable re-captures SFD, re-arm both abs timers. */
    cb_framework_uwb_enable_scheduled_trx(s_stRespTxSchedCfg);
    cb_framework_uwb_enable_scheduled_trx(s_stRxReopenSchedCfg);
    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                              &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
    app_uwb_tdoa_print("[RESP] timeout, reset link\r\n");
}

/**
 * @brief Open RX for the next Poll-DTM and prepare the scheduled TX offset.
 * @details Called only at startup and from watchdog recovery. Normal per-round
 *          RX reopen uses abs timer 1 (9ms from POLL SFD). The enable here
 *          establishes the initial event capture for abs0/abs1.
 */
static void app_tdoa_start_poll_rx(void)
{
    s_stRespTxSchedCfg.timeoutValue =
        DEF_TDOA_RESP_SLOT_DELAY_US(g_tdoa_config.slot);

    cb_framework_uwb_disable_scheduled_trx(s_stRespTxSchedCfg);
    cb_framework_uwb_enable_scheduled_trx(s_stRespTxSchedCfg);
    /* Turn abs timer 1 on once here so the per-round +9ms RX reopen fires (configure() never calls abs_timer_on). */
    cb_framework_uwb_enable_scheduled_trx(s_stRxReopenSchedCfg);
    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                              &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
}

/**
 * @brief Estimate the initiator/responder clock ratio from two complete rounds.
 */
static uint8_t app_tdoa_calculate_clk_ratio(double* out_ratio)
{
    app_uwbtdoa_respround_st* rd_m1 = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)];
    app_uwbtdoa_respround_st* rd_m2 = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 2)];

    if (!rd_m1->dataComplete || !rd_m2->dataComplete) {
        return 0u;
    }
    if (rd_m1->roundIndex == 0u || rd_m2->roundIndex == 0u) {
        return 0u;
    }
    if (rd_m1->roundIndex != (rd_m2->roundIndex + 1u)) {
        return 0u;
    }

    return tdoa_math_calculate_responder_clock_ratio(rd_m1->tsuTxInit,
                                                     rd_m2->tsuTxInit,
                                                     rd_m1->rxTsu,
                                                     rd_m2->rxTsu,
                                                     out_ratio);
}

/**
 * @brief Calculate the synchronized Response-DTM TX timestamp for the payload.
 * @param sync_out Output: synchronized TX timestamp written by math helper.
 * @return 1 if sync is valid, 0 if not enough history.
 */
static uint8_t app_tdoa_calculate_tx_sync(cb_uwbsystem_tx_tsutimestamp_st* sync_out)
{
    app_uwbtdoa_respround_st* rd_m1 = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)];
    app_uwbtdoa_respround_st* rd_m2 = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 2)];

    if (!s_clkRatioValid) {
        return 0u;
    }
    if (rd_m1->roundIndex != (s_currentRound - 1u)) {
        return 0u;
    }
    if (!rd_m1->rxValid) {
        return 0u;
    }

    double delta_trx_raw;

    if (rd_m1->txValid) {
        /* Prefer the actual measured turnaround from the previous round. */
        delta_trx_raw = tdoa_math_responder_turnaround_ticks(rd_m1->txTsu,
                                                             rd_m1->rxTsu);
    } else if (rd_m2->txValid && rd_m2->rxValid) {
        /* During startup, an older complete round beats using only the nominal slot value. */
        delta_trx_raw = tdoa_math_responder_turnaround_ticks(rd_m2->txTsu,
                                                             rd_m2->rxTsu);
    } else {
        /* First rounds lack measured TX history: use the configured slot delay to keep the frame consistent. */
        delta_trx_raw = tdoa_math_responder_slot_delay_ticks(g_tdoa_config.slot);
    }

    tdoa_math_calculate_responder_sync_tx(rd_m1->tsuTxInit,
                                          delta_trx_raw,
                                          s_clkRatio,
                                          s_sumTpBias,
                                          sync_out);

    return 1u;
}

/**
 * @brief TX done callback used by the UWB framework.
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
 * @brief RX0-done ISR: minimal — just sets the flag, all work deferred to main loop.
 */
void cb_uwbapp_rx0_done_irqcb(void)
{
    /* Keep the ISR minimal: validate status and raise a flag; parsing is deferred to main-loop context. */
    cb_uwbsystem_rxstatus_un rx_status = cb_framework_uwb_get_rx_status();
    if (rx_status.rx0_ok) {
        s_stIrqStatus.Rx0Done = 1;
    }
}

/**
 * @brief Timer0 IRQ callback: watchdog for the current round.
 * @details Routes to the calibration watchdog when calibration owns the radio;
 *          otherwise captures the failure state and sets s_watchdogTimeout for
 *          the main loop to handle.
 */
void cb_timer_0_app_irq_callback(void)
{
    if (app_tdoa_calib_is_active()) {
        app_tdoa_calib_timer_irqcb();
        return;
    }
    s_failureState    = s_enAppTdoaState;
    s_watchdogTimeout = 1u;
}

/**
 * @brief Print the timeout state name for diagnostics.
 */
static void app_tdoa_timeout_error_message_print(void)
{
    switch (s_failureState)
    {
        case EN_APP_STATE_RX_WAIT:
            app_uwb_tdoa_print("[RESP] Timeout:RX_WAIT\r\n");
            break;
        case EN_APP_STATE_TX_SEND:
            app_uwb_tdoa_print("[RESP] Timeout:TX_SEND\r\n");
            break;
        case EN_APP_STATE_WAIT_TX_DONE:
            app_uwb_tdoa_print("[RESP] Timeout:WAIT_TX_DONE\r\n");
            break;
        default:
            app_uwb_tdoa_print("[RESP] Timeout:UNKNOWN\r\n");
            break;
    }
}

/**
 * @brief Deferred RX0-done processor running in main-loop context.
 *
 * The IRQ only sets the Rx0Done flag; this function performs the actual work:
 * routes to boot calibration when active, otherwise validates the captured
 * frame, stores it into the single RX hand-off slot, and decides whether to
 * reopen the radio for the next Poll-DTM. Hardware payload and TSU registers
 * remain valid until the next RX completes, which leaves enough time for the
 * non-blocking main loop to consume them.
 */
static void app_tdoa_process_rx0_done(void)
{
    s_stIrqStatus.Rx0Done = 0;

    if (app_tdoa_calib_is_active()) {
        app_tdoa_calib_rx_done_irqcb();
        return;
    }

    cb_uwbsystem_rxstatus_un rx_status = cb_framework_uwb_get_rx_status();

    if (rx_status.rx0_ok) {
        if (s_stRxSlot.used) {
            /* Previous frame not yet consumed: keep it intact and restart RX (never overwrite its snapshot). */
            cb_framework_uwb_rx_end(EN_UWB_RX_0);
            app_tdoa_start_poll_rx();
            return;
        }

        uint16_t pkt_size = cb_framework_uwb_get_rx_packet_size(&s_stUwbPacketConfig);
        if (pkt_size > TDOA_RX_MSG_MAX_LEN)
            pkt_size = TDOA_RX_MSG_MAX_LEN;

        cb_framework_uwb_get_rx_payload(s_stRxSlot.payload, pkt_size);
        s_stRxSlot.len        = (uint8_t)pkt_size;
        s_stRxSlot.frame_type = s_stRxSlot.payload[0];
        /* Store the round index early so diagnostics can identify the frame before the payload is consumed. */
        if ((s_stRxSlot.frame_type == TDOA_FRAME_TYPE_POLL_DTM) &&
            (pkt_size >= sizeof(tdoa_poll_payload_t))) {
            const tdoa_poll_payload_t* poll = (const tdoa_poll_payload_t*)s_stRxSlot.payload;
            s_stRxSlot.round_index = poll->round_index;
        } else {
            s_stRxSlot.round_index = 0u;
        }

        // cb_framework_uwb_get_rx_tsu_timestamp(&s_stRxSlot.rx_tsu, EN_UWB_RX_0);
        cb_framework_uwb_get_rx_tsu_timestamp_lemm(&s_stRxSlot.rx_tsu, EN_UWB_RX_0, 18, 0, 2);
        s_stRxSlot.used = 1;
    }

    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    if (!s_stRxSlot.used) {
        app_tdoa_start_poll_rx();
    }
}

/**
 * @brief Initialize responder state, timing constants, and the first RX phase.
 */
static void app_tdoa_init_state(void)
{
    memset((void *)&s_stIrqStatus, 0, sizeof(s_stIrqStatus));
    memset(&s_stRxSlot, 0, sizeof(s_stRxSlot));
    memset(s_stHistory, 0, sizeof(s_stHistory));
    s_currIdx              = 0;
    s_enAppTdoaState       = EN_APP_STATE_RX_WAIT;
    s_failureState         = EN_APP_STATE_RX_WAIT;
    s_watchdogTimeout      = 0u;
    s_clkRatio             = 0.0;
    s_clkRatioValid        = 0;
    memset(&s_clkRatioFilter, 0, sizeof(s_clkRatioFilter));
    s_currentRound         = 0;
    s_statTxOk             = 0;
    s_calibDoneHandled     = 0;
    s_txStartTickMs        = 0u;
    s_txSlotOffsetMs       = 0u;

    cb_framework_uwb_tsu_clear();

    /* Convert config values into TSU ticks once at startup (the native time base of RX/TX timestamps). */
    s_ticksPerNs = tdoa_math_ticks_per_ns();
    s_sumTpBias = tdoa_math_initial_sum_tp_bias_ticks(g_tdoa_config.t_prop,
                                                      g_tdoa_config.bias,
                                                      s_ticksPerNs);
    s_sumTpBiasCm = tdoa_math_ticks_to_cm(s_sumTpBias, s_ticksPerNs);

    app_tdoa_calib_init(s_sumTpBiasCm);
    app_uwb_tdoa_print("[RESP] init, id=%u slot=%u, boot calib started\r\n",
                       g_tdoa_config.device_id, g_tdoa_config.slot);
}

/**
 * @brief Consume the latest Poll-DTM out of the IRQ hand-off slot and update history.
 *
 * @return 1 when a valid Poll-DTM was consumed (caller should advance to TX_SEND),
 *         0 when the frame was ignored and RX has already been restarted.
 */
static uint8_t app_tdoa_handle_poll_received(void)
{
    cb_framework_uwb_rx_end(EN_UWB_RX_0);

    /* Copy locally so the IRQ handoff slot can be released before protocol processing (simple ownership). */
    tdoa_rx_msg_slot_t local;
    memcpy(&local, &s_stRxSlot, sizeof(local));
    s_stRxSlot.used = 0;

    if (local.frame_type != TDOA_FRAME_TYPE_POLL_DTM ||
        local.len < sizeof(tdoa_poll_payload_t))
    {
        /* Ignore unrelated frames and keep waiting for the next valid Poll-DTM (useful in shared RF). */
        app_tdoa_start_poll_rx();
        return 0u;
    }

    const tdoa_poll_payload_t *poll = (const tdoa_poll_payload_t *)local.payload;

    s_currentRound = poll->round_index;

    /* A valid Poll-DTM starts a new round: store RX timestamp now; TX timestamp is filled after TX completes. */
    s_currIdx = TDOA_HISTORY_ADVANCE(s_currIdx);
    memset(&s_stHistory[s_currIdx], 0, sizeof(app_uwbtdoa_respround_st));

    app_uwbtdoa_respround_st* curr = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 0)];
    curr->rxTsu      = local.rx_tsu;
    curr->rxValid    = 1;
    curr->roundIndex = s_currentRound;

    app_uwbtdoa_respround_st* prev = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)];
    if (prev->roundIndex == (s_currentRound - 1u) || prev->roundIndex == 0u) {
        /* Poll-DTM carries the initiator's previous-round TX; attach to history[1] only if tx_prev_valid set. */
        if (poll->tx_prev_valid) {
            /* Copy embedded SDK TX timestamp whole; never dereference its fields from the packed buffer. */
            prev->tsuTxInit = poll->tsu_tx_init_prev;
        }
    }

    if (s_currentRound >= 2u) {
        app_uwbtdoa_respround_st* rd_m2 = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 2)];
        app_uwbtdoa_respround_st* rd_m1 = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)];

        if (rd_m2->dataComplete && rd_m1->dataComplete) {
            double new_clk_ratio = 1.0;
            if (app_tdoa_calculate_clk_ratio(&new_clk_ratio)) {
                /* Filter tiny clock-ratio movement to reduce timestamp jitter while still tracking drift. */
                if (tdoa_math_filter_responder_clock_ratio(&s_clkRatioFilter,
                                                           new_clk_ratio,
                                                           TDOA_CLK_RATIO_MAX_DELTA,
                                                           &s_clkRatio)) {
                    s_clkRatioValid = 1;
                }
            }
        }
    }

    return 1u;
}

/**
 * @brief Build Response-DTM with the synchronized TX timestamp and kick the deferred TX.
 */
static void app_tdoa_start_resp_tx(void)
{
    static tdoa_resp_payload_t resp;
    memset(&resp, 0, sizeof(resp));
    resp.frame_type  = TDOA_FRAME_TYPE_RESP_DTM;
    resp.device_id   = g_tdoa_config.device_id;
    resp.round_index = s_currentRound;

    /* sync_valid may be 0 during startup; the initiator still receives the RESP and builds history. */
    cb_uwbsystem_tx_tsutimestamp_st sync_tsu;
    memset(&sync_tsu, 0, sizeof(sync_tsu));
    if (app_tdoa_calculate_tx_sync(&sync_tsu)) {
        resp.tsu_tx_sync = sync_tsu;
        resp.sync_valid  = 1;
    } else {
        resp.sync_valid = 0;
    }

    /* Re-apply the slot delay before each TX so a runtime change to g_tdoa_config.slot is honoured. */
    s_stRespTxSchedCfg.timeoutValue =
        DEF_TDOA_RESP_SLOT_DELAY_US(g_tdoa_config.slot);
    cb_framework_uwb_configure_scheduled_trx(s_stRespTxSchedCfg);

    cb_uwbsystem_txpayload_st tx_payload;
    tx_payload.ptrAddress  = (uint8_t *)&resp;
    tx_payload.payloadSize = sizeof(resp);

    /* Record guard start time and slot offset; deadline = tx_start + slot + 2ms. */
    s_txStartTickMs  = cb_hal_get_time_ms();
    s_txSlotOffsetMs = g_tdoa_config.slot;

    /* Deferred TX fires at the configured offset from the current Poll-DTM's RX0 SFD event. */
    cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &tx_payload,
                              &s_stTxIrqEnable, EN_TRX_START_DEFERRED);
}

/**
 * @brief Latch the Response-DTM TX timestamp and update completion bookkeeping.
 * @details Also performs the relatch: disable+enable on s_stRespTxSchedCfg
 *          re-captures the shared POLL SFD event timestamp for both abs0 (next
 *          slot TX) and abs1 (9ms RX reopen). This is the sole relatch site per
 *          round, executing at ~+slot ms, well before the next +10ms POLL.
 *          Then configures and arms abs1 for the 9ms deferred RX reopen.
 */
static void app_tdoa_handle_tx_done(void)
{
    s_stIrqStatus.TxDone = 0;

    app_uwbtdoa_respround_st* curr = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 0)];

    /* TX timestamp is valid only after TX done; store it in the current round for later clk-ratio/sync. */
    cb_framework_uwb_get_tx_tsu_timestamp(&curr->txTsu);
    cb_framework_uwb_tx_end();
    curr->txValid = 1;

    if (curr->rxValid && curr->txValid) {
        curr->dataComplete = 1;
    }

    s_statTxOk++;

    /* Watchdog: one round done — disarm and re-arm for the next round (starts when RX reopens for POLL). */
    tdoa_watchdog_timer_off(EN_TIMER_0);
    s_watchdogTimeout = 0u;
    tdoa_watchdog_timer_init(EN_TIMER_0, TDOA_WATCHDOG_TIMEOUT_MS);

    /* Arm abs timer 1 = POLL SFD + 9ms; mask 0 still holds this round's POLL SFD, so no disable/enable relatch. */
    cb_framework_uwb_configure_scheduled_trx(s_stRxReopenSchedCfg);
    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                              &s_stRxIrqEnable, EN_TRX_START_DEFERRED);

    /* Example diagnostic. */
    if (cb_hal_get_time_ms() > 1000) {
        app_uwb_tdoa_print("[RESP R:%lu] clk=%.8f sync=%u  tp_bias_cm=%.2f tx_ok=%lu\r\n",
                        (unsigned long)s_currentRound,
                        s_clkRatioValid ? s_clkRatio : 0.0,
                        (&s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)])->dataComplete,
                        s_sumTpBiasCm,
                        s_statTxOk);
    }
}

void app_tdoa_responder(void)
{
    cb_framework_uwb_init();
    app_tdoa_init_state();

    while (1)
    {
        /* Drain the RX0 IRQ flag first so calib overlay and TDOA state machine both see fresh frames. */
        if (s_stIrqStatus.Rx0Done) {
            app_tdoa_process_rx0_done();
        }

        if (app_tdoa_calib_is_active()) {
            /* Boot calibration shares the UWB hardware; let it finish before opening normal TDOA RX. */
            app_tdoa_calib_process();
            continue;
        }

        if (!s_calibDoneHandled && app_tdoa_calib_is_done()) {
            s_calibDoneHandled = 1;
            if (app_tdoa_calib_is_locked()) {
                s_sumTpBiasCm = app_tdoa_calib_get_sum_tp_bias_cm();
                s_sumTpBias   = tdoa_math_cm_to_ticks(s_sumTpBiasCm, s_ticksPerNs);
            }
            /* Calib produced the sync bias; start listening for Poll-DTM and arm the watchdog for round 1. */
            s_watchdogTimeout = 0u;
            tdoa_watchdog_timer_init(EN_TIMER_0, TDOA_WATCHDOG_TIMEOUT_MS);
            app_tdoa_start_poll_rx();
            app_uwb_tdoa_print("[RESP] boot calib done, tp_bias_cm=%.2f\r\n",
                               s_sumTpBiasCm);
        }

        /* Handle watchdog timeout: print state, full disable+enable+rx_start to rebuild event capture, re-arm. */
        if (s_watchdogTimeout) {
            s_watchdogTimeout = 0u;
            app_tdoa_timeout_error_message_print();
            app_tdoa_reset_link();
            /* Full relatch: disable+enable re-establishes event capture and re-arms both abs timers. */
            cb_framework_uwb_enable_scheduled_trx(s_stRespTxSchedCfg);
            cb_framework_uwb_enable_scheduled_trx(s_stRxReopenSchedCfg);
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                                      &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
            /* Re-arm watchdog for the next round. */
            tdoa_watchdog_timer_init(EN_TIMER_0, TDOA_WATCHDOG_TIMEOUT_MS);
            s_enAppTdoaState = EN_APP_STATE_RX_WAIT;
            continue;
        }

        switch (s_enAppTdoaState)
        {
            case EN_APP_STATE_RX_WAIT:
                if (s_stRxSlot.used) {
                    if (app_tdoa_handle_poll_received()) {
                        s_enAppTdoaState = EN_APP_STATE_TX_SEND;
                    }
                }
                break;

            case EN_APP_STATE_TX_SEND:
                app_tdoa_start_resp_tx();
                s_enAppTdoaState = EN_APP_STATE_WAIT_TX_DONE;
                break;

            case EN_APP_STATE_WAIT_TX_DONE:
                if (s_stIrqStatus.TxDone) {
                    app_tdoa_handle_tx_done();
                    /* TX done: abs timer 1 armed for 9ms RX reopen; return to RX_WAIT (hardware timer handles delay). */
                    s_enAppTdoaState = EN_APP_STATE_RX_WAIT;
                } else {
                    /* TX-done guard: deadline = tx_start + slot + TDOA_TXDONE_GUARD_MS. */
                    if (cb_hal_is_time_elapsed_ms(s_txStartTickMs,
                            (uint32_t)s_txSlotOffsetMs + (uint32_t)TDOA_TXDONE_GUARD_MS)
                        == CB_PASS)
                    {
                        /* Guard expired: force TX end, clear residual TxDone, abandon this round's RESP (txValid=0). */
                        cb_framework_uwb_tx_end();
                        s_stIrqStatus.TxDone = 0;   /* Clear any late-arriving TxDone. */

                        app_uwb_tdoa_print("[RESP R:%lu] TX guard expired, abandon RESP\r\n",
                                           (unsigned long)s_currentRound);

                        /* Still arm the 9ms RX reopen (as in the normal TX-done path) so the next POLL is not missed. */
                        cb_framework_uwb_configure_scheduled_trx(s_stRxReopenSchedCfg);
                        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                                                  &s_stRxIrqEnable, EN_TRX_START_DEFERRED);

                        /* Abandon path is a round-end event; reset watchdog. */
                        tdoa_watchdog_timer_off(EN_TIMER_0);
                        s_watchdogTimeout = 0u;
                        tdoa_watchdog_timer_init(EN_TIMER_0, TDOA_WATCHDOG_TIMEOUT_MS);

                        s_enAppTdoaState = EN_APP_STATE_RX_WAIT;
                    }
                }
                break;

            default:
                s_enAppTdoaState = EN_APP_STATE_RX_WAIT;
                break;
        }
    }
}
