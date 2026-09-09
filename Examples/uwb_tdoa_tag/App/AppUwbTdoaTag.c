/**
 * @file    AppUwbTdoaTag.c
 * @brief   TDOA tag: RX callbacks, TDOA computation, and report TX.
 * @details Role slice of the exchange in tdoa_common.h. The tag receives
 *          Poll-DTM and all three Response-DTM frames, computes per-responder
 *          TDOA with its own clock ratio, and broadcasts a Tag-Report carrying
 *          three distance differences. Timestamp arithmetic is isolated in
 *          TdoaMath.h so the timing model can be reviewed apart from the state
 *          machine.
 *
 * @author  Chipsbank
 * @date    2026
 * @note    Poll-DTM and Response-DTM both carry round m-1 timestamps, so the tag
 *          uses history[1] when computing TDOA for the current round m.
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbTdoaTag.h"
#include "tdoa_common.h"
#include "tdoa_config.h"
#include "TdoaMath.h"
#include "CB_uwbframework.h"
#include "CB_system.h"
#include "NonLIB_sharedUtils.h"
#include "APP_common.h"

#define APP_UWB_TDOA_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_TDOA_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_tdoa_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_tdoa_print(...)
#endif

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

static cb_uwbsystem_packetconfig_st s_stUwbPacketConfig = APP_TDOA_TAG_PACKET_CONFIG_DEFAULTS;
static cb_uwbsystem_rx_irqenable_st s_stRxIrqEnable = {.rx0Done = 1};
static cb_uwbsystem_tx_irqenable_st s_stTxIrqEnable = {.txDone  = 1};

/* Abs timer 0: deferred report TX, fires DEF_TDOA_TAG_REPORT_TX_DELAY_US after Responder-3 SFD. */
static cb_uwbframework_trx_scheduledconfig_st s_stReportTxSchedCfg = {
    .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,
    .eventIndex         = EN_UWBEVENT_17_RX0_SFD_DET,
    .absTimer           = EN_UWB_ABSOLUTE_TIMER_0,
    .timeoutValue       = 0,
    .eventCtrlMask      = EN_UWBCTRL_TX_START_MASK,
};

/* Abs timer 1: deferred RX reopen, fires DEF_TDOA_TAG_RX_REOPEN_DELAY_US after report TX_DONE. */
static cb_uwbframework_trx_scheduledconfig_st s_stRxReopenSchedCfg = {
    .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,
    .eventIndex         = EN_UWBEVENT_28_TX_DONE,
    .absTimer           = EN_UWB_ABSOLUTE_TIMER_1,
    .timeoutValue       = 0,
    .eventCtrlMask      = EN_UWBCTRL_RX0_START_MASK,
};

//-------------------------------
// TDOA: TAG SETUP
//-------------------------------
// (Full four-node sequence diagram lives in tdoa_common.h; below is the
//  tag's own role view.)
//-------------------------------------------------------
//    Initiator                Tag                Responder1/2/3
//      Idle                   Idle                   Idle
//       |---1. POLL_DTM(m)---->| p                     |
//       |                      |<--2. RESP_DTM(m,id=1)-| r1  +1ms
//       |                      |<--3. RESP_DTM(m,id=2)-| r2  +2ms
//       |                      |<--4. RESP_DTM(m,id=3)-| r3  +3ms
//       |<--5. TAG_REPORT(m)---|                       |     +4ms
//      Repeat                 Repeat                 Repeat
//
// TDOA_WATCHDOG_TIMEOUT_MS        : 20ms one-shot hardware watchdog (Timer0)
// DEF_TDOA_TAG_REPORT_DELAY_MS    : fixed finalize/report offset from POLL (+4ms)
// DEF_TDOA_TAG_RX_REOPEN_DELAY_US : deferred RX reopen offset from TX_DONE (+5ms)
//
//   TDOA for responder k (round m, using history[1]):
//     tdoa_ticks = (rk - p)*clk_ratio - (rk_txSync - init_txPrev)
//     dist_diff  = tdoa_ticks * TDOA_M_PER_TSU_TICK / TDOA_TSU_FRAC_SCALE
//     clk_ratio  = delta(init TX) / delta(tag RX poll)  [two consecutive POLLs]
//
//  p:  rxPollTsu          r1/r2/r3: rxRespTsu[1..3]
//      history[0]=round m, [1]=round m-1 (TDOA), [2]=round m-2 (clk-ratio delta)
//-------------------------------------------------------
//-------------------------------
// DEFINE SECTION
//-------------------------------
/* Bitmask covering all four nodes (initiator + 3 responders). */
#define DEF_TDOA_TAG_ALL_ANCHOR_MASK             0x0Fu
/* Bits 1-3 must carry a valid sync timestamp for TDOA to be computable. */
#define DEF_TDOA_TAG_RESP_SYNC_MASK              0x0Eu
/* Legacy abs-timer-0 timeout value; the report is now sent immediately at the
 * POLL + N ms deadline, so this only seeds the (otherwise idle) abs-timer-0
 * event-capture config kept alive by app_tdoa_prepare_report_tx_schedule(). */
#define DEF_TDOA_TAG_REPORT_TX_DELAY_US          1000u
/* The round is finalized and the report transmitted at a fixed offset after the
 * POLL was received (POLL + TDOA_MAX_ANCHORS ms), independent of which anchors
 * were heard, so a debug report is sent even on incomplete rounds. */
#define DEF_TDOA_TAG_REPORT_DELAY_MS             ((uint32_t)TDOA_MAX_ANCHORS)
/* Delay after the report TX-done event before reopening RX (microseconds).
 * Implemented via abs timer 1 keyed on EN_UWBEVENT_28_TX_DONE so the tag
 * never polls a tick counter in the main loop.
 */
#define DEF_TDOA_TAG_RX_REOPEN_DELAY_US          5000u
/* Distance jump threshold for the per-channel outlier filter (metres). */
#define DEF_TDOA_TAG_DIST_JUMP_THRESHOLD_M       0.20f

/* TX-done guard deadline offset for the tag report (deferred +1 ms slot). */
#define DEF_TDOA_TAG_REPORT_TX_OFFSET_MS         1u

/* Drop reason codes (DEF_TDOA_DROP_*) are shared via tdoa_common.h so the
 * initiator can decode the drop_reason carried in the report payload. */

//-------------------------------
// ENUM SECTION
//-------------------------------

typedef enum {
    EN_APP_STATE_WAIT_POLL = 0, /* Idle; waiting for the initiator Poll-DTM. */
    EN_APP_STATE_WAIT_RESP,     /* Poll received; collecting responder frames. */
    EN_APP_STATE_COMPUTE,       /* All frames in or timeout; compute TDOA. */
    EN_APP_STATE_TX_REPORT,     /* Deferred report TX in progress. */
} app_uwbtdoa_state_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

/* Per-round measurement context stored in the circular history buffer. */
typedef struct {
    volatile uint8_t active;          /* 1 while this round is being collected. */
    uint32_t roundIndex;
    uint8_t  anchorRxMask;          /* Bit set for each received frame (bit 0 = poll, bits 1-3 = resp). */
    uint8_t  syncValidMask;         /* Bit set for each responder whose sync_valid flag was 1. */
    uint8_t  txPrevValid;           /* tx_prev_valid field from the Poll-DTM payload. */
    uint8_t  pendingReason;         /* Drop reason accumulated before finalize. */
    uint16_t overrunBase;           /* s_rxOverrunCnt snapshot at round start; detects slot collisions. */
    cb_uwbsystem_tx_tsutimestamp_st  tsuTxInitPrev;              /* Initiator TX timestamp from Poll-DTM (round m-1). */
    cb_uwbsystem_rx_tsutimestamp_st  rxPollTsu;                  /* Tag local RX timestamp for Poll-DTM. */
    cb_uwbsystem_rx_tsutimestamp_st  rxRespTsu[TDOA_MAX_ANCHORS]; /* Tag local RX timestamps for each Response-DTM. */
    cb_uwbsystem_tx_tsutimestamp_st  tsuTxSync[TDOA_MAX_ANCHORS]; /* Responder synchronized TX timestamps (from round m-1). */
} app_uwbtdoa_taground_st;


/* IRQ-to-main-loop handoff slot for one received frame.
 * The IRQ fills an available slot; the main loop pops and processes it.
 * volatile on 'used' prevents the compiler from caching the flag in a register.
 */
typedef struct {
    uint8_t  payload[TDOA_RX_MSG_MAX_LEN];
    uint8_t  len;
    uint8_t  frameType;
    uint32_t roundIndex;
    volatile uint8_t used;
    cb_uwbsystem_rx_tsutimestamp_st rxTsu;
} app_uwbtdoa_rxmsgslot_st;

/* IRQ-to-main-loop signalling flags. The RX0-done ISR sets Rx0Done; the
 * TX-done ISR sets TxDone. The main loop drains the flags and does all work.
 */
typedef struct {
    volatile uint8_t Rx0Done;
    volatile uint8_t TxDone;
} app_uwbtdoa_irqstatus_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

#define DEF_TDOA_TAG_RX_SLOT_COUNT       TDOA_MAX_ANCHORS
#define DEF_TDOA_TAG_RX_SLOT_POLL_INDEX  0u

/* One RX slot per expected frame type per round (poll + 3 responses). */
static app_uwbtdoa_rxmsgslot_st s_stRxSlots[DEF_TDOA_TAG_RX_SLOT_COUNT];
/* Incremented in the IRQ when a slot is already occupied; checked at round end. */
static uint16_t s_rxOverrunCnt = 0;

static volatile app_uwbtdoa_irqstatus_st s_stIrqStatus = { 0 };

static volatile app_uwbtdoa_state_en s_enAppTdoaState;

/* Failure state recorded by the watchdog IRQ for timeout diagnostic printing. */
static volatile app_uwbtdoa_state_en s_failureState;
/* Set to 1 by the watchdog IRQ; cleared by main loop after handling. */
static volatile uint8_t s_watchdogTimeout = 0;

/* Tick captured when entering TX_REPORT wait; used by the txdone guard. */
static volatile uint32_t s_txStartTickMs = 0;
/* 1 while the deferred report TX is actively in flight. */
static volatile uint8_t  s_reportTxActive = 0;
/* Tick captured when the POLL was received; drives the fixed POLL + N ms
 * round-finalize deadline so a report is sent even on incomplete rounds. */
static uint32_t s_pollRxTick = 0;

/* Circular history buffer — same layout as the responder example.
 * Index 0 is always the current round after advancing the buffer.
 * Index 1 is round m-1 (complete; used for TDOA computation).
 * Index 2 is round m-2 (used for clock-ratio delta).
 */
static app_uwbtdoa_taground_st s_stHistory[TDOA_HISTORY_SIZE];
static uint8_t s_currIdx = 0;

static uint32_t s_statOk   = 0;
static uint32_t s_statLost = 0;

static tdoa_report_payload_t s_stTxReport;

/* Tag clock ratio and its filter state — updated each round from Poll-DTM history. */
static double  s_tagClkRatio       = 0.0;
static uint8_t s_tagClkRatioValid = 0;
static tdoa_ratio_filter_state_st s_tagClkRatioFilter = {0};

/* Per-channel filtered distance differences (metres) — the values sent in the report. */
static float s_distanceDiff[TDOA_MAX_RESPONDERS];
static tdoa_dist_filter_state_st s_stDistFilter[TDOA_MAX_RESPONDERS];

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void tdoa_role_apply_default_config(tdoa_runtime_config_t* config);
static uint8_t app_tdoa_resolve_rx_slot_index(const uint8_t* payload, uint16_t pkt_size);
static uint8_t app_tdoa_pop_rx_slot(app_uwbtdoa_rxmsgslot_st* slot_snapshot);
static void app_tdoa_prepare_report_tx_schedule(void);
static void app_tdoa_process_rx_slots(void);
void cb_uwbapp_rx0_done_irqcb(void);
void cb_uwbapp_tx_done_irqhandler(void);
void cb_timer_0_app_irq_callback(void);
static void app_tdoa_process_rx0_done(void);
static void app_tdoa_process_tx_done(void);
static void app_tdoa_start_round(const app_uwbtdoa_rxmsgslot_st* slot);
static void app_tdoa_store_response(const app_uwbtdoa_rxmsgslot_st* slot);
static void app_tdoa_process_frame(const app_uwbtdoa_rxmsgslot_st* slot);
static void app_tdoa_send_report(uint32_t round_index, uint8_t anchor_mask, uint8_t reason);
static uint8_t app_tdoa_compute_result(double* out_ratio);
static void app_tdoa_finalize_round(void);
static void app_tdoa_reset(void);
void app_tdoa_tag(void);
static void app_tdoa_reset_link(void);
static void app_tdoa_timeout_error_message_print(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

void tdoa_role_apply_default_config(tdoa_runtime_config_t* config)
{
    if (config == 0) {
        return;
    }
    config->device_id    = APP_TDOA_TAG_DEVICE_ID;
    config->slot         = APP_TDOA_TAG_SLOT;
    config->t_prop       = TDOA_DEFAULT_T_PROP_NS;
    config->bias         = TDOA_DEFAULT_BIAS_CM;
    config->calib_source = 0;
}

/**
 * @brief Watchdog timer IRQ callback — record the failure state and set timeout flag.
 *
 * Runs in IRQ context. Only captures state and signals the main loop; all
 * recovery work (link reset, re-arm) is done in the main loop.
 */
void cb_timer_0_app_irq_callback(void)
{
    s_failureState    = s_enAppTdoaState;
    s_watchdogTimeout = 1u;
}

/**
 * @brief Print the timeout state name to help diagnose which stage stalled.
 */
static void app_tdoa_timeout_error_message_print(void)
{
    switch (s_failureState) {
    case EN_APP_STATE_WAIT_POLL:
        app_uwb_tdoa_print("[TAG] Timeout:WAIT_POLL\r\n");
        break;
    case EN_APP_STATE_WAIT_RESP:
        app_uwb_tdoa_print("[TAG] Timeout:WAIT_RESP\r\n");
        break;
    case EN_APP_STATE_COMPUTE:
        app_uwb_tdoa_print("[TAG] Timeout:COMPUTE\r\n");
        break;
    case EN_APP_STATE_TX_REPORT:
        app_uwb_tdoa_print("[TAG] Timeout:TX_REPORT\r\n");
        break;
    default:
        app_uwb_tdoa_print("[TAG] Timeout:UNKNOWN\r\n");
        break;
    }
}

/**
 * @brief Stop all active UWB work and clear transient state.
 *
 * Called before restarting RX from a clean state (watchdog recovery or init).
 */
static void app_tdoa_reset_link(void)
{
    cb_framework_uwb_disable_scheduled_trx(s_stReportTxSchedCfg);
    cb_framework_uwb_disable_scheduled_trx(s_stRxReopenSchedCfg);
    cb_framework_uwb_tx_end();
    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    memset(&s_stRxSlots, 0, sizeof(s_stRxSlots));
    s_reportTxActive = 0u;
}

/**
 * @brief Map a received frame to its RX slot index.
 *
 * Poll-DTM always maps to slot 0. Response-DTM maps to the responder device_id
 * (1-3). Any other frame type or out-of-range device_id returns
 * DEF_TDOA_TAG_RX_SLOT_COUNT to signal "discard".
 */
static uint8_t app_tdoa_resolve_rx_slot_index(const uint8_t* payload, uint16_t pkt_size)
{
    uint8_t device_id;

    if ((payload == 0) || (pkt_size < 1u)) {
        return DEF_TDOA_TAG_RX_SLOT_COUNT;
    }

    if (payload[0] == TDOA_FRAME_TYPE_POLL_DTM) {
        return DEF_TDOA_TAG_RX_SLOT_POLL_INDEX;
    }

    if (payload[0] != TDOA_FRAME_TYPE_RESP_DTM) {
        return DEF_TDOA_TAG_RX_SLOT_COUNT;
    }

    if (pkt_size < 2u) {
        return DEF_TDOA_TAG_RX_SLOT_COUNT;
    }

    device_id = payload[1];
    if ((device_id == 0u) || (device_id >= DEF_TDOA_TAG_RX_SLOT_COUNT)) {
        return DEF_TDOA_TAG_RX_SLOT_COUNT;
    }

    return device_id;
}

/**
 * @brief Pop the oldest pending RX slot into a caller-owned snapshot.
 *
 * Disables IRQs briefly to atomically read and clear the slot. Returns 1 if a
 * frame was available, 0 if all slots were empty.
 */
static uint8_t app_tdoa_pop_rx_slot(app_uwbtdoa_rxmsgslot_st* slot_snapshot)
{
    uint8_t slot_idx;

    if (slot_snapshot == 0) {
        return 0u;
    }

    __disable_irq();
    for (slot_idx = 0u; slot_idx < DEF_TDOA_TAG_RX_SLOT_COUNT; slot_idx++) {
        if (s_stRxSlots[slot_idx].used != 0u) {
            memcpy(slot_snapshot->payload,
                   s_stRxSlots[slot_idx].payload,
                   sizeof(slot_snapshot->payload));
            slot_snapshot->len         = s_stRxSlots[slot_idx].len;
            slot_snapshot->frameType  = s_stRxSlots[slot_idx].frameType;
            slot_snapshot->roundIndex = s_stRxSlots[slot_idx].roundIndex;
            slot_snapshot->rxTsu      = s_stRxSlots[slot_idx].rxTsu;
            s_stRxSlots[slot_idx].used  = 0u;
            __enable_irq();
            return 1u;
        }
    }
    __enable_irq();
    return 0u;
}

/**
 * @brief Arm the deferred report TX schedule for the current round.
 *
 * The schedule fires DEF_TDOA_TAG_REPORT_TX_DELAY_US after the Responder-3 SFD
 * event. It must be re-armed each round because the trigger event is consumed
 * after each firing.
 */
static void app_tdoa_prepare_report_tx_schedule(void)
{
    s_stReportTxSchedCfg.timeoutValue = DEF_TDOA_TAG_REPORT_TX_DELAY_US;
    cb_framework_uwb_disable_scheduled_trx(s_stReportTxSchedCfg);
    cb_framework_uwb_enable_scheduled_trx(s_stReportTxSchedCfg);
}

/**
 * @brief RX0 done callback — raise flag for main-loop processing.
 *
 * Runs in IRQ context. Sets Rx0Done so the main loop calls
 * app_tdoa_process_rx0_done() to do the actual payload work.
 */
void cb_uwbapp_rx0_done_irqcb(void)
{
    cb_uwbsystem_rxstatus_un rx_status = cb_framework_uwb_get_rx_status();
    if (rx_status.rx0_ok) {
        s_stIrqStatus.Rx0Done = 1;
    }
}

/**
 * @brief TX done callback — set TxDone flag for main-loop handling.
 *
 * Runs in IRQ context. Setting TxDone lets the main loop handle the
 * normal TX-done path (configure RX reopen abs timer, arm deferred RX)
 * and lets the txdone guard in the main loop detect that TX completed.
 */
void cb_uwbapp_tx_done_irqhandler(void)
{
    if ((s_enAppTdoaState == EN_APP_STATE_TX_REPORT) && (s_reportTxActive != 0u)) {
        s_stIrqStatus.TxDone = 1u;
    }
}

/**
 * @brief Deferred RX0-done processor running in main-loop context.
 *
 * Reads payload and TSU from hardware registers, dispatches frame into the
 * matching per-round RX slot, and reopens RX unless in TX_REPORT window.
 */
static void app_tdoa_process_rx0_done(void)
{
    cb_uwbsystem_rxstatus_un rx_status;

    s_stIrqStatus.Rx0Done = 0;

    rx_status = cb_framework_uwb_get_rx_status();
    if (rx_status.rx0_ok) {
        uint8_t  payload[TDOA_RX_MSG_MAX_LEN];
        uint8_t  slot_idx;
        uint16_t pkt_size = cb_framework_uwb_get_rx_packet_size(&s_stUwbPacketConfig);

        if (pkt_size > TDOA_RX_MSG_MAX_LEN) {
            pkt_size = TDOA_RX_MSG_MAX_LEN;
        }

        if (pkt_size >= 1u) {
            cb_framework_uwb_get_rx_payload(payload, pkt_size);
            slot_idx = app_tdoa_resolve_rx_slot_index(payload, pkt_size);

            if (slot_idx < DEF_TDOA_TAG_RX_SLOT_COUNT) {
                if (s_stRxSlots[slot_idx].used != 0u) {
                    s_rxOverrunCnt++;
                } else {
                    memcpy(s_stRxSlots[slot_idx].payload, payload, pkt_size);
                    s_stRxSlots[slot_idx].len        = (uint8_t)pkt_size;
                    s_stRxSlots[slot_idx].frameType = payload[0];
                    s_stRxSlots[slot_idx].roundIndex = 0u;

                    if ((payload[0] == TDOA_FRAME_TYPE_POLL_DTM) &&
                        (pkt_size >= sizeof(tdoa_poll_payload_t))) {
                        const tdoa_poll_payload_t* poll = (const tdoa_poll_payload_t*)payload;
                        s_stRxSlots[slot_idx].roundIndex = poll->round_index;
                    } else if ((payload[0] == TDOA_FRAME_TYPE_RESP_DTM) &&
                               (pkt_size >= sizeof(tdoa_resp_payload_t))) {
                        const tdoa_resp_payload_t* resp = (const tdoa_resp_payload_t*)payload;
                        s_stRxSlots[slot_idx].roundIndex = resp->round_index;
                    }

                    // cb_framework_uwb_get_rx_tsu_timestamp(&s_stRxSlots[slot_idx].rxTsu, EN_UWB_RX_0);
                    cb_framework_uwb_get_rx_tsu_timestamp_lemm(&s_stRxSlots[slot_idx].rxTsu, EN_UWB_RX_0, 18, 0, 2);
                    s_stRxSlots[slot_idx].used = 1u;
                }
            }
        }
    }

    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    if (s_enAppTdoaState != EN_APP_STATE_TX_REPORT) {
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                                  &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
    }
}

/**
 * @brief Handle a confirmed report TX_DONE event from the main loop.
 *
 * Configures abs timer 1 to reopen RX 5ms after TX_DONE via deferred start.
 * The abs timer fires on EN_UWBEVENT_28_TX_DONE + 5000us and auto-starts RX0.
 */
static void app_tdoa_process_tx_done(void)
{
    s_stIrqStatus.TxDone = 0;
    s_reportTxActive     = 0u;

    cb_framework_uwb_disable_scheduled_trx(s_stReportTxSchedCfg);

    /* Arm the RX reopen abs timer 1: event = TX_DONE, timeout = 5ms, action = RX0 start. */
    s_stRxReopenSchedCfg.timeoutValue = DEF_TDOA_TAG_RX_REOPEN_DELAY_US;
    cb_framework_uwb_configure_scheduled_trx(s_stRxReopenSchedCfg);
    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                              &s_stRxIrqEnable, EN_TRX_START_DEFERRED);

    /* Disarm watchdog: one round has ended (TX_DONE = success). */
    tdoa_watchdog_timer_off(EN_TIMER_0);

    s_enAppTdoaState = EN_APP_STATE_WAIT_POLL;

    /* Re-arm watchdog for the next round (covers WAIT_POLL interval). */
    s_watchdogTimeout = 0u;
    tdoa_watchdog_timer_init(EN_TIMER_0, TDOA_WATCHDOG_TIMEOUT_MS);
}

/**
 * @brief Start a new measurement round on receipt of a valid Poll-DTM.
 *
 * Advances the history buffer, records the Poll-DTM RX timestamp, and stores
 * the initiator TX timestamp from the payload into history[1] so it is
 * available for TDOA computation in the next round.
 *
 * tsu_tx_init_prev is copied as a whole struct to avoid non-aligned field
 * access from the packed payload buffer.
 */
static void app_tdoa_start_round(const app_uwbtdoa_rxmsgslot_st* slot)
{
    const tdoa_poll_payload_t* poll = (const tdoa_poll_payload_t*)slot->payload;
    app_uwbtdoa_taground_st* curr;
    app_uwbtdoa_taground_st* prev;

    /* Advance circular buffer and zero the new slot. */
    s_currIdx = TDOA_HISTORY_ADVANCE(s_currIdx);
    memset(&s_stHistory[s_currIdx], 0, sizeof(app_uwbtdoa_taground_st));
    curr = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 0)];

    curr->active        = 1u;
    curr->roundIndex   = poll->round_index;
    curr->anchorRxMask = 0x01u;  /* bit 0 = initiator/poll received */
    curr->txPrevValid  = poll->tx_prev_valid;
    curr->overrunBase  = s_rxOverrunCnt;
    curr->rxPollTsu    = slot->rxTsu;

    /* Poll-DTM carries round m-1 initiator TX ts; attach to history[1] (whole-struct copy, unaligned). */
    prev = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)];
    if (prev->roundIndex == (curr->roundIndex - 1u) || prev->roundIndex == 0u) {
        prev->tsuTxInitPrev = poll->tsu_tx_init_prev;
    }

    app_tdoa_prepare_report_tx_schedule();
    /* Start the fixed POLL + TDOA_MAX_ANCHORS ms finalize/report deadline. */
    s_pollRxTick     = cb_hal_get_time_ms();
    s_enAppTdoaState = EN_APP_STATE_WAIT_RESP;
}

/**
 * @brief Store one Response-DTM into the current round context.
 *
 * Records the tag local RX timestamp and the responder's synchronized TX
 * timestamp (from round m-1) into history[1]. Once all anchors have been
 * heard the state machine advances to COMPUTE.
 *
 * tsu_tx_sync is copied as a whole struct to avoid non-aligned access.
 */
static void app_tdoa_store_response(const app_uwbtdoa_rxmsgslot_st* slot)
{
    const tdoa_resp_payload_t* resp = (const tdoa_resp_payload_t*)slot->payload;
    app_uwbtdoa_taground_st* curr;
    app_uwbtdoa_taground_st* prev;
    uint8_t bit_mask;
    cb_uwbsystem_tx_tsutimestamp_st tsu_copy;

    if ((resp->device_id == 0u) || (resp->device_id >= TDOA_MAX_ANCHORS)) {
        return;
    }

    curr = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 0)];

    if (resp->round_index != curr->roundIndex) {
        if (curr->pendingReason == DEF_TDOA_DROP_NONE) {
            curr->pendingReason = DEF_TDOA_DROP_ROUND_MISMATCH;
        }
        return;
    }

    bit_mask = (uint8_t)(1u << resp->device_id);
    curr->anchorRxMask             |= bit_mask;
    curr->rxRespTsu[resp->device_id] = slot->rxTsu;

    /* Store tsuTxSync into history[1]; the tag computes TDOA one round later (whole-struct copy, unaligned). */
    tsu_copy = resp->tsu_tx_sync;
    prev = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)];
    if (prev->roundIndex == (curr->roundIndex - 1u) || prev->roundIndex == 0u) {
        prev->tsuTxSync[resp->device_id] = tsu_copy;
        if (resp->sync_valid) {
            prev->syncValidMask |= bit_mask;
        }
    }

    if ((curr->anchorRxMask & DEF_TDOA_TAG_ALL_ANCHOR_MASK) == DEF_TDOA_TAG_ALL_ANCHOR_MASK) {
        s_enAppTdoaState = EN_APP_STATE_COMPUTE;
    }
}

/**
 * @brief Dispatch one received frame to the appropriate handler.
 *
 * A new Poll-DTM while already collecting responses finalizes the current
 * incomplete round before starting the new one.
 */
static void app_tdoa_process_frame(const app_uwbtdoa_rxmsgslot_st* slot)
{
    app_uwbtdoa_taground_st* curr;

    if (s_enAppTdoaState == EN_APP_STATE_TX_REPORT) {
        return;
    }

    if (slot->frameType == TDOA_FRAME_TYPE_POLL_DTM) {
        if (slot->len < sizeof(tdoa_poll_payload_t)) {
            return;
        }

        /* New POLL with previous round still open: deadline missed, account as lost without TX, start fresh. */
        curr = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 0)];
        if (curr->active && (s_enAppTdoaState == EN_APP_STATE_WAIT_RESP)) {
            s_statLost++;
        }

        app_tdoa_start_round(slot);
        return;
    }

    if (slot->frameType == TDOA_FRAME_TYPE_RESP_DTM) {
        curr = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 0)];
        if (!curr->active || (s_enAppTdoaState != EN_APP_STATE_WAIT_RESP)) {
            return;
        }
        if (slot->len < sizeof(tdoa_resp_payload_t)) {
            return;
        }
        app_tdoa_store_response(slot);
    }
}

/**
 * @brief Drain all pending RX slots through the frame dispatcher.
 *
 * Stops early if the state advances to COMPUTE or TX_REPORT so that the
 * main loop can handle those transitions without processing stale frames.
 */
static void app_tdoa_process_rx_slots(void)
{
    while (1) {
        app_uwbtdoa_rxmsgslot_st slot_snapshot;

        if (app_tdoa_pop_rx_slot(&slot_snapshot) == 0u) {
            break;
        }

        app_tdoa_process_frame(&slot_snapshot);

        if ((s_enAppTdoaState == EN_APP_STATE_COMPUTE) ||
            (s_enAppTdoaState == EN_APP_STATE_TX_REPORT)) {
            break;
        }
    }
}

/**
 * @brief Build the tag report and transmit it immediately (NON_DEFERRED).
 *
 * Sent once per round at the fixed POLL + TDOA_MAX_ANCHORS ms instant for both
 * valid and dropped rounds. When reason == DEF_TDOA_DROP_NONE the caller has
 * already filled s_distanceDiff with the computed result; otherwise the report
 * is debug-only (distances zeroed) and carries the drop reason + overrun count
 * so the initiator prints the same diagnostics. The RX-reopen abs timer is
 * armed on TX_DONE before the TX so RX restarts 5 ms after the report completes.
 */
static void app_tdoa_send_report(uint32_t round_index, uint8_t anchor_mask,
                                 uint8_t reason)
{
    cb_uwbsystem_txpayload_st tx_payload;

    cb_framework_uwb_rx_end(EN_UWB_RX_0);

    memset(&s_stTxReport, 0, sizeof(s_stTxReport));
    s_stTxReport.frame_type  = TDOA_FRAME_TYPE_TAG_REPORT;
    s_stTxReport.tag_id      = g_tdoa_config.device_id;
    s_stTxReport.round_index = round_index;
    if (reason == DEF_TDOA_DROP_NONE) {
        s_stTxReport.tdoa_dist_diff[0] = s_distanceDiff[0];
        s_stTxReport.tdoa_dist_diff[1] = s_distanceDiff[1];
        s_stTxReport.tdoa_dist_diff[2] = s_distanceDiff[2];
    }
    /* anchor_rx_mask is from the current round: which anchors the tag actually heard. */
    s_stTxReport.anchor_rx_mask = anchor_mask;
    s_stTxReport.drop_reason    = reason;
    s_stTxReport.overrun        = (uint16_t)s_rxOverrunCnt;

    tx_payload.ptrAddress  = (uint8_t*)&s_stTxReport;
    tx_payload.payloadSize = sizeof(tdoa_report_payload_t);

    /* Arm abs timer 1 (RX reopen) before TX so TX_DONE is captured when the report TX completes. */
    s_stRxReopenSchedCfg.timeoutValue = DEF_TDOA_TAG_RX_REOPEN_DELAY_US;
    cb_framework_uwb_enable_scheduled_trx(s_stRxReopenSchedCfg);

    s_enAppTdoaState = EN_APP_STATE_TX_REPORT;
    s_reportTxActive = 1u;
    /* Record tick for the txdone guard in the main loop. */
    s_txStartTickMs = cb_hal_get_time_ms();
    cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &tx_payload,
                              &s_stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
}

/**
 * @brief Compute the TDOA result for the previous round into s_distanceDiff.
 *
 * Uses history[1] (the previous complete round) for all timestamp inputs and
 * history[2] for the clock-ratio delta. The tag clock ratio is estimated from
 * two consecutive Poll-DTM rounds and IIR-filtered; each per-channel distance
 * difference passes through an outlier filter. Returns 1 and writes *out_ratio
 * on success, or 0 when the previous round is unusable (not complete or its
 * responder sync timestamps invalid) so the caller emits a debug report.
 */
static uint8_t app_tdoa_compute_result(double* out_ratio)
{
    app_uwbtdoa_taground_st* rd_prev = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)];
    app_uwbtdoa_taground_st* rd_m2   = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 2)];
    uint8_t anchor_id;
    double  ratio;

    /* Require all anchors heard and all responder sync timestamps valid. */
    if (!rd_prev->active ||
        ((rd_prev->anchorRxMask & DEF_TDOA_TAG_ALL_ANCHOR_MASK) != DEF_TDOA_TAG_ALL_ANCHOR_MASK) ||
        ((rd_prev->syncValidMask & DEF_TDOA_TAG_RESP_SYNC_MASK) != DEF_TDOA_TAG_RESP_SYNC_MASK)) {
        return 0u;
    }

    /* Update tag clock ratio from two consecutive Poll-DTM rounds. */
    if (rd_m2->active &&
        rd_prev->roundIndex == (rd_m2->roundIndex + 1u)) {
        double new_ratio = 1.0;
        if (tdoa_math_tag_calculate_clock_ratio(rd_prev->tsuTxInitPrev,
                                                rd_m2->tsuTxInitPrev,
                                                rd_prev->rxPollTsu,
                                                rd_m2->rxPollTsu,
                                                &new_ratio)) {
            if (tdoa_math_filter_responder_clock_ratio(&s_tagClkRatioFilter,
                                                       new_ratio,
                                                       TDOA_CLK_RATIO_MAX_DELTA,
                                                       &s_tagClkRatio)) {
                s_tagClkRatioValid = 1u;
            }
        }
    }

    ratio = s_tagClkRatioValid ? s_tagClkRatio : 0.0;

    for (anchor_id = 1u; anchor_id < TDOA_MAX_ANCHORS; anchor_id++) {
        uint8_t resp_idx = anchor_id - 1u;
        double tdoa_ticks = tdoa_math_tag_calculate_tdoa_ticks(
                                rd_prev->rxRespTsu[anchor_id],
                                rd_prev->rxPollTsu,
                                rd_prev->tsuTxSync[anchor_id],
                                rd_prev->tsuTxInitPrev,
                                ratio);

        float dist_m = (float)tdoa_math_tag_ticks_to_dist_diff_m(tdoa_ticks);

        {
            tdoa_dist_filter_state_st* f = &s_stDistFilter[resp_idx];
            tdoa_math_tag_update_distance_filter(f,
                                                 dist_m,
                                                 s_tagClkRatioValid,
                                                 &s_distanceDiff[resp_idx],
                                                 DEF_TDOA_TAG_DIST_JUMP_THRESHOLD_M,
                                                 TDOA_ABNORMAL_MAX_RETRY);
        }
    }

    *out_ratio = ratio;
    return 1u;
}

/**
 * @brief Finalize the round at the fixed POLL + TDOA_MAX_ANCHORS ms deadline.
 *
 * Classifies the current round. If it is complete and the previous round yields
 * a valid TDOA solution, a result report is sent; otherwise a debug report (drop
 * reason + overrun + anchor mask) is sent so the initiator always learns why a
 * round was dropped. Both paths transmit a TAG_REPORT via app_tdoa_send_report.
 */
static void app_tdoa_finalize_round(void)
{
    app_uwbtdoa_taground_st* curr    = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 0)];
    app_uwbtdoa_taground_st* rd_prev = &s_stHistory[TDOA_HISTORY_GET(s_currIdx, 1)];
    uint8_t reason = DEF_TDOA_DROP_NONE;
    double  ratio  = 0.0;

    if (!curr->active) {
        s_enAppTdoaState = EN_APP_STATE_WAIT_POLL;
        return;
    }

    if (s_rxOverrunCnt != curr->overrunBase) {
        reason = DEF_TDOA_DROP_OVERRUN;
    } else if ((curr->anchorRxMask & DEF_TDOA_TAG_ALL_ANCHOR_MASK) != DEF_TDOA_TAG_ALL_ANCHOR_MASK) {
        reason = (curr->pendingReason != DEF_TDOA_DROP_NONE)
               ? curr->pendingReason
               : DEF_TDOA_DROP_TIMEOUT;
    } else if (curr->txPrevValid == 0u) {
        reason = DEF_TDOA_DROP_SYNC_INVALID;
    } else if (!app_tdoa_compute_result(&ratio)) {
        /* Previous round (the one-round-late TDOA source) unusable -> emit debug instead of a position. */
        reason = DEF_TDOA_DROP_SYNC_INVALID;
    }

    if (reason == DEF_TDOA_DROP_NONE) {
        s_statOk++;
        app_tdoa_send_report(rd_prev->roundIndex, curr->anchorRxMask, DEF_TDOA_DROP_NONE);
        app_uwb_tdoa_print("[R:%lu] DIS01:%.3fcm,DIS02:%.3fcm,DIS03:%.3fcm RATIO:%.8f STAT:ok=%lu,lost=%lu\r\n",
                           (unsigned long)rd_prev->roundIndex,
                           s_distanceDiff[0] * 100.0f,
                           s_distanceDiff[1] * 100.0f,
                           s_distanceDiff[2] * 100.0f,
                           ratio,
                           (unsigned long)s_statOk,
                           (unsigned long)s_statLost);
    } else {
        s_statLost++;
        app_tdoa_send_report(curr->roundIndex, curr->anchorRxMask, reason);
        app_uwb_tdoa_print("[R:%lu] DBG:REASON=%s,OVR=%u ",
                           (unsigned long)curr->roundIndex,
                           tdoa_drop_reason_to_str(reason),
                           (unsigned)s_rxOverrunCnt);
#if (APP_UWB_TDOA_UARTPRINT_ENABLE == APP_TRUE)
        tdoa_print_lack_anchors(curr->anchorRxMask, app_uart_printf);
#endif
        app_uwb_tdoa_print("STAT:ok=%lu,lost=%lu\r\n",
                           (unsigned long)s_statOk,
                           (unsigned long)s_statLost);
    }
}

/**
 * @brief Initialise tag state, clear all buffers, and open the first RX window.
 */
static void app_tdoa_reset(void)
{
    memset(s_stRxSlots,    0, sizeof(s_stRxSlots));
    memset(s_stHistory,     0, sizeof(s_stHistory));
    memset(s_stDistFilter, 0, sizeof(s_stDistFilter));
    memset(s_distanceDiff, 0, sizeof(s_distanceDiff));
    memset((void *)&s_stIrqStatus, 0, sizeof(s_stIrqStatus));

    s_currIdx           = 0;
    s_rxOverrunCnt     = 0;
    s_txStartTickMs    = 0;
    s_reportTxActive   = 0u;
    s_statOk            = 0;
    s_statLost          = 0;
    s_tagClkRatio      = 0.0;
    s_tagClkRatioValid = 0u;
    memset(&s_tagClkRatioFilter, 0, sizeof(s_tagClkRatioFilter));
    s_watchdogTimeout  = 0u;

    cb_framework_uwb_disable_scheduled_trx(s_stReportTxSchedCfg);
    cb_framework_uwb_disable_scheduled_trx(s_stRxReopenSchedCfg);

    s_enAppTdoaState = EN_APP_STATE_WAIT_POLL;

    /* Full enable+rx_start to establish the abs timer event capture chain. */
    cb_framework_uwb_enable_scheduled_trx(s_stReportTxSchedCfg);
    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                              &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);

    /* Arm watchdog for the first round. */
    tdoa_watchdog_timer_init(EN_TIMER_0, TDOA_WATCHDOG_TIMEOUT_MS);

    app_uwb_tdoa_print("[TAG] init, RX started\r\n");
}

void app_tdoa_tag(void)
{
    cb_framework_uwb_init();

    app_tdoa_reset();

    while (1) {
        /* Handle watchdog timeout first — highest priority event. */
        if (s_watchdogTimeout != 0u) {
            s_watchdogTimeout = 0u;
            app_tdoa_timeout_error_message_print();

            /* Full link reset + disable/enable/rx_start to rebuild the abs timer event capture chain. */
            app_tdoa_reset_link();
            cb_framework_uwb_enable_scheduled_trx(s_stReportTxSchedCfg);
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                                      &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);

            s_enAppTdoaState = EN_APP_STATE_WAIT_POLL;

            /* Re-arm watchdog (clear flag before init). */
            s_watchdogTimeout = 0u;
            tdoa_watchdog_timer_init(EN_TIMER_0, TDOA_WATCHDOG_TIMEOUT_MS);
            continue;
        }

        /* Drain the RX0 IRQ flag. */
        if (s_stIrqStatus.Rx0Done) {
            app_tdoa_process_rx0_done();
        }

        switch (s_enAppTdoaState)
        {
        case EN_APP_STATE_WAIT_POLL:
            app_tdoa_process_rx_slots();
            break;

        case EN_APP_STATE_WAIT_RESP:
        case EN_APP_STATE_COMPUTE:
            /* Collect responses, then finalize at the fixed POLL + N ms deadline so a report is always sent. */
            app_tdoa_process_rx_slots();
            if (((s_enAppTdoaState == EN_APP_STATE_WAIT_RESP) ||
                 (s_enAppTdoaState == EN_APP_STATE_COMPUTE)) &&
                (cb_hal_is_time_elapsed_ms(s_pollRxTick,
                                           DEF_TDOA_TAG_REPORT_DELAY_MS) == CB_PASS)) {
                app_tdoa_finalize_round();
            }
            break;

        case EN_APP_STATE_TX_REPORT:
            if (s_stIrqStatus.TxDone) {
                /* Normal path: TX completed; configure deferred RX reopen. */
                app_tdoa_process_tx_done();
            } else {
                /* txdone guard: deadline = s_txStartTickMs + report_offset(1ms) + 2ms. */
                if (cb_hal_is_time_elapsed_ms(s_txStartTickMs,
                    DEF_TDOA_TAG_REPORT_TX_OFFSET_MS + TDOA_TXDONE_GUARD_MS) == CB_PASS) {
                    /* Guard expired — abort this round's report. */
                    cb_framework_uwb_tx_end();
                    /* Explicitly clear TxDone in case IRQ latched after guard check. */
                    s_stIrqStatus.TxDone = 0u;
                    s_reportTxActive     = 0u;
                    cb_framework_uwb_disable_scheduled_trx(s_stRxReopenSchedCfg);
                    cb_framework_uwb_disable_scheduled_trx(s_stReportTxSchedCfg);

                    /* Watchdog: off + re-arm (abandon path is a round-end event). */
                    tdoa_watchdog_timer_off(EN_TIMER_0);
                    s_watchdogTimeout = 0u;
                    tdoa_watchdog_timer_init(EN_TIMER_0, TDOA_WATCHDOG_TIMEOUT_MS);

                    /* Abs timer 1 won't fire (no TX_DONE); do immediate non-deferred RX reopen, return to WAIT_POLL. */
                    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                                              &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
                    s_enAppTdoaState = EN_APP_STATE_WAIT_POLL;
                }
            }
            break;

        default:
            s_enAppTdoaState = EN_APP_STATE_WAIT_POLL;
            break;
        }
    }
}
