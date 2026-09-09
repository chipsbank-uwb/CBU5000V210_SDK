/**
 * @file    AppUwbCalibResponder.c
 * @brief   TDOA responder boot calibration state machine (DS-TWR driver/initiator side).
 * @details The TDOA responder drives the DS-TWR calibration exchange. It sends
 *          CAL_REQ and waits for CAL_ACK, then runs a DS-TWR session
 *          (POLL → RESP → FINAL → DATA) in a 3-second sampling window. Distance
 *          is computed locally from own tround/treply and the initiator's
 *          tround/treply received in CAL_DATA. After the window closes the
 *          trimmed-mean bias is locked and sent to the initiator in CAL_END.
 *
 *          Direction-A robustness model: all RX windows are NON_DEFERRED
 *          (always-listening) with short per-state tick guards; only the single
 *          CAL_FINAL TX is deferred for DS-TWR turnaround precision. One abs
 *          timer (EN_UWB_ABSOLUTE_TIMER_0) carries that single schedule:
 *            - CAL_FINAL TX: RX0_SFD_DET (RESP) + 700 us → TX start
 *          CAL_END is resent every 10 ms until the initiator returns CAL_END_ACK.
 *
 * @author  Chipsbank
 * @date    2026
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbCalibResponder.h"
#include "AppUwbTdoaResponder.h"
#include "tdoa_common.h"
#include "tdoa_config.h"
#include "TdoaMath.h"
#include "CB_uwbframework.h"
#include "NonLIB_sharedUtils.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_TDOA_BOOT_CALIB_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_TDOA_BOOT_CALIB_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_calib_resp_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_calib_resp_print(...)
#endif

//-------------------------------
// TDOA: CALIB-RESPONDER SETUP
//-------------------------------
//-------------------------------------------------------
//   Calib-Responder                 Calib-Initiator
//   (DS-TWR initiator / driver)      (DS-TWR responder)
//       |---------1. CAL_REQ ------------->|
//       |<--------2. CAL_ACK --------------|
//     a |---------3. CAL_POLL ------------>| d
//     b |<--------4. CAL_RESP -------------| e
//     c |---------5. CAL_FINAL ----------->| f
//       |<--------6. CAL_DATA -------------|   (initiator tround/treply + bias)
//       |     ... repeat 3..6 over a 3 s sampling window ...
//       |---------7. CAL_END ------------->|   (locked_sum_tp_bias_cm)
//       |     ... resend CAL_END every 10 ms until END_ACK (1 s cap) ...
//       |<--------8. CAL_END_ACK ----------|
//   Resume TDOA                        Resume TDOA
//
// All RX windows here are NON_DEFERRED (always-listening); only CAL_FINAL TX is
// deferred (RESP SFD + 700 us). No per-sample watchdog: each RX wait uses a 5 ms
// tick guard that restarts the sample from CAL_REQ on a miss.
//
// DS-TWR initiator: Tround_1 = b - a    Treply_2 = c - b
// DS-TWR responder: Treply_1 = e - d    Tround_2 = f - e
//
//  a: poll_tx_tsu   (this device)     d: CAL_POLL  RX (remote)
//  b: resp_rx_tsu   (this device)     e: CAL_RESP  TX (remote)
//  c: final_tx_tsu  (this device)     f: CAL_FINAL RX (remote)
//-------------------------------------------------------
//-------------------------------
// DEFINE SECTION
//-------------------------------

/* TX-done guard deadlines (ms).  Non-deferred TX: tx_start + 0 + 2ms.
 * Deferred TX (CAL_FINAL, offset = 1ms rounded up): tx_start + 1 + 2ms. */
#define CALIB_TXDONE_GUARD_NONDEFERRED_MS  (TDOA_TXDONE_GUARD_MS)          /* 2 ms */
#define CALIB_TXDONE_GUARD_DEFERRED_MS     (1u + TDOA_TXDONE_GUARD_MS)     /* 3 ms */

//-------------------------------
// ENUM SECTION
//-------------------------------

/**
 * @brief States of the responder calibration state machine.
 * @details This enum replaces the previous responder-as-receiver flow.
 *          The TDOA responder is now the DS-TWR *driver* (initiator side).
 */
typedef enum {
    EN_APP_CALIB_STATE_SEND_REQ = 0,    /* Broadcast CAL_REQ to the initiator. */
    EN_APP_CALIB_STATE_WAIT_REQ_TX,     /* Wait for CAL_REQ TX done. */
    EN_APP_CALIB_STATE_WAIT_ACK_RX,     /* Wait for CAL_ACK from the initiator. */
    EN_APP_CALIB_STATE_SEND_POLL,       /* Send CAL_POLL (DS-TWR point a). */
    EN_APP_CALIB_STATE_WAIT_POLL_TX,    /* Wait for CAL_POLL TX done; arm resp-rx abs timer. */
    EN_APP_CALIB_STATE_WAIT_RESP_RX,    /* Wait for CAL_RESP (DS-TWR point b); arm final-tx abs timer. */
    EN_APP_CALIB_STATE_WAIT_FINAL_TX,   /* Wait for deferred CAL_FINAL TX done; arm data-rx abs timer. */
    EN_APP_CALIB_STATE_WAIT_DATA_RX,    /* Wait for CAL_DATA; compute and store distance sample. */
    EN_APP_CALIB_STATE_WAIT_NEXT_SAMPLE,/* Idle gap before the next POLL (TDOA_BOOT_CALIB_SAMPLE_GAP_MS). */
    EN_APP_CALIB_STATE_SEND_END,        /* Sampling window expired; broadcast CAL_END. */
    EN_APP_CALIB_STATE_WAIT_END_TX,     /* Wait for CAL_END TX done. */
    EN_APP_CALIB_STATE_WAIT_END_ACK_RX, /* Wait for CAL_END_ACK; resend CAL_END every 10 ms. */
    EN_APP_CALIB_STATE_DONE,            /* Terminal state; normal TDOA may start. */
} app_calib_state_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct {
    struct {
        uint8_t             active;     /* State machine is running. */
        uint8_t             done;       /* State machine has reached a terminal result. */
        uint8_t             locked;     /* Enough samples collected to lock an average. */
        uint8_t             lock_guard; /* Set once compute_lock() ran; prevents re-locking on CAL_END resend. */
        app_calib_state_en  state;      /* Current state. */
    } control;
    struct {
        uint16_t seq;               /* Frame sequence counter. */
        uint8_t  initiator_id;      /* Initiator ID paired with this responder. */
    } session;
    struct {
        uint8_t  window_started;    /* Set on first CAL_ACK receipt. */
        uint32_t calib_start_ms;    /* Tick snapshot for the 3-second window. */
        uint32_t tx_start_tick_ms;  /* Tick at tx_start() call; used for txdone guard. */
        uint32_t sample_gap_start_ms; /* Tick at start of inter-sample idle gap. */
        uint32_t state_tick_ms;     /* Tick at RX-wait entry; used for per-state tick guard. */
        uint32_t end_first_tick_ms; /* Tick at first CAL_END TX; caps the END/END_ACK handshake. */
        /* DS-TWR driver-side timestamps (a=POLL TX, b=RESP RX, c=FINAL TX). */
        cb_uwbsystem_tx_tsutimestamp_st poll_tx_tsu;   /* Point a. */
        cb_uwbsystem_rx_tsutimestamp_st resp_rx_tsu;   /* Point b. */
        cb_uwbsystem_tx_tsutimestamp_st final_tx_tsu;  /* Point c. */
    } timing;
    struct {
        uint8_t            watchdog_timeout;   /* Set by timer IRQ. */
        app_calib_state_en failure_state;      /* State at watchdog expiry. */
        uint8_t            TxDone;             /* TX done from IRQ. */
        tdoa_rx_msg_slot_t stRxSlot;           /* RX snapshot from IRQ. */
    } io;
    struct {
        float                       locked_sum_tp_bias_cm;  /* Filtered result (cm). */
        tdoa_calib_sample_buffer_t  sample_buf;             /* 128-deep ring buffer. */
    } result;
} app_calib_ctx_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static cb_uwbsystem_packetconfig_st s_pkt_cfg = DEF_TDOA_RESP_PACKET_CONFIG_DEFAULTS;
static cb_uwbsystem_tx_irqenable_st s_tx_irq  = { .txDone = 1, .sfdDone = 0 };
static cb_uwbsystem_rx_irqenable_st s_rx_irq  = { .rx0Done = 1 };

/* Treply2: after RESP RX0_SFD_DET (+700 us) → send deferred CAL_FINAL.
 * This is the only scheduled config: CAL_RESP/CAL_DATA RX are NON_DEFERRED. */
static cb_uwbframework_trx_scheduledconfig_st s_treply2_cfg = {
    .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,
    .eventIndex         = EN_UWBEVENT_17_RX0_SFD_DET,
    .absTimer           = EN_UWB_ABSOLUTE_TIMER_0,
    .timeoutValue       = TDOA_BOOT_CALIB_TREPLY_US,        /* 700 us */
    .eventCtrlMask      = EN_UWBCTRL_TX_START_MASK,
};

static app_calib_ctx_st s_ctx;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void    app_calib_reset_link             (void);
static void    app_calib_restart_sample         (void);
static uint8_t app_calib_window_elapsed         (void);
static void    app_calib_push_sample            (float sample_cm);
static uint16_t app_calib_sample_count          (void);
static void    app_calib_export_recent_samples  (float* output, uint16_t count);
static void    app_calib_compute_lock           (void);
static uint8_t app_calib_validate_ctrl          (uint8_t frame_type);
static void    app_calib_timeout_print          (void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/**
 * @brief Stop all UWB hardware and clear transient flags for a clean retry.
 */
static void app_calib_reset_link(void)
{
    cb_framework_uwb_disable_scheduled_trx(s_treply2_cfg);
    cb_framework_uwb_tx_end();
    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    s_ctx.io.TxDone = 0u;
    memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));
}

/**
 * @brief Tear down the current sample and restart the handshake from CAL_REQ.
 * @details Called from per-state tick-guard expiries. The initiator follows the
 *          fresh CAL_REQ and re-ACKs, so the two sides resynchronise quickly.
 */
static void app_calib_restart_sample(void)
{
    app_calib_reset_link();
    s_ctx.session.seq++;
    s_ctx.control.state = EN_APP_CALIB_STATE_SEND_REQ;
}

/**
 * @brief Return 1 once the 3-second sampling window has elapsed.
 */
static uint8_t app_calib_window_elapsed(void)
{
    if (s_ctx.timing.window_started == 0u) {
        return 0u;
    }
    return (cb_hal_is_time_elapsed_ms(s_ctx.timing.calib_start_ms,
                                       TDOA_BOOT_CALIB_DURATION_MS) == CB_PASS) ? 1u : 0u;
}

/**
 * @brief Push one distance sample (cm) into the ring buffer.
 */
static void app_calib_push_sample(float sample_cm)
{
    tdoa_calib_sample_buffer_t* buf = &s_ctx.result.sample_buf;

    buf->values[buf->write_idx] = sample_cm;
    buf->write_idx = (uint16_t)((buf->write_idx + 1u) & TDOA_BOOT_CALIB_SAMPLE_INDEX_MASK);
    if (buf->valid_count < TDOA_BOOT_CALIB_MAX_SAMPLES) {
        buf->valid_count++;
    }
}

/**
 * @brief Return the number of valid samples in the ring buffer.
 */
static uint16_t app_calib_sample_count(void)
{
    return s_ctx.result.sample_buf.valid_count;
}

/**
 * @brief Copy the most-recent `count` samples from the ring buffer to output.
 */
static void app_calib_export_recent_samples(float* output, uint16_t count)
{
    uint16_t start_idx;
    uint16_t i;

    if (count == 0u) {
        return;
    }
    start_idx = (uint16_t)((s_ctx.result.sample_buf.write_idx - count) &
                            TDOA_BOOT_CALIB_SAMPLE_INDEX_MASK);
    for (i = 0u; i < count; i++) {
        output[i] = s_ctx.result.sample_buf.values[(start_idx + i) &
                                                    TDOA_BOOT_CALIB_SAMPLE_INDEX_MASK];
    }
}

/**
 * @brief Validate a received frame against the expected type and this device ID.
 *
 * @param frame_type Expected type byte (e.g. TDOA_FRAME_TYPE_CAL_ACK).
 * @return 1 when the snapshot is ready and passes all checks.
 */
static uint8_t app_calib_validate_ctrl(uint8_t frame_type)
{
    tdoa_calib_ctrl_payload_t* ctrl;

    if (s_ctx.io.stRxSlot.used == 0u) {
        return 0u;
    }
    if (s_ctx.io.stRxSlot.len < sizeof(tdoa_calib_ctrl_payload_t)) {
        return 0u;
    }
    ctrl = (tdoa_calib_ctrl_payload_t*)s_ctx.io.stRxSlot.payload;
    if (ctrl->frame_type != frame_type) {
        return 0u;
    }
    if (ctrl->responder_id != g_tdoa_config.device_id) {
        return 0u;
    }
    return 1u;
}

/**
 * @brief Apply the trimmed-mean filter and lock the bias (runs at most once).
 * @details Computes the result that CAL_END carries. It does NOT terminate the
 *          state machine: the terminal active=0/done=1 is set only after the
 *          initiator acknowledges CAL_END (or the END handshake cap elapses),
 *          so a CAL_END resend never recomputes or reprints the lock.
 */
static void app_calib_compute_lock(void)
{
    uint16_t count;

    if (s_ctx.control.lock_guard != 0u) {
        return;
    }
    s_ctx.control.lock_guard = 1u;

    count = app_calib_sample_count();
    if (count >= TDOA_BOOT_CALIB_MIN_SAMPLES) {
        float  sorted[TDOA_BOOT_CALIB_MAX_SAMPLES];
        float  filtered_mean;

        app_calib_export_recent_samples(sorted, count);
        if (tdoa_math_trimmed_mean(sorted,
                                   count,
                                   TDOA_BOOT_CALIB_TRIM_LOW_PERCENT,
                                   TDOA_BOOT_CALIB_TRIM_HIGH_PERCENT,
                                   &filtered_mean) != 0u) {
            s_ctx.result.locked_sum_tp_bias_cm = filtered_mean;
            s_ctx.control.locked = 1u;
        }
    }

    if (s_ctx.control.locked != 0u) {
        app_calib_resp_print("[CALIB RESP] locked tp_bias_cm=%.2f sample=%u\r\n",
                             s_ctx.result.locked_sum_tp_bias_cm,
                             (unsigned)app_calib_sample_count());
    } else {
        app_calib_resp_print("[CALIB RESP] fallback default tp_bias_cm=%.2f sample=%u\r\n",
                             s_ctx.result.locked_sum_tp_bias_cm,
                             (unsigned)app_calib_sample_count());
    }
}

/**
 * @brief Print a human-readable timeout message based on the failure state.
 */
static void app_calib_timeout_print(void)
{
    switch (s_ctx.io.failure_state) {
    case EN_APP_CALIB_STATE_SEND_REQ:
        break;
    case EN_APP_CALIB_STATE_WAIT_REQ_TX:
        app_calib_resp_print("[CALIB RESP] Timeout:WAIT_REQ_TX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_ACK_RX:
        app_calib_resp_print("[CALIB RESP] Timeout:WAIT_ACK_RX\r\n");
        break;
    case EN_APP_CALIB_STATE_SEND_POLL:
        break;
    case EN_APP_CALIB_STATE_WAIT_POLL_TX:
        app_calib_resp_print("[CALIB RESP] Timeout:WAIT_POLL_TX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_RESP_RX:
        app_calib_resp_print("[CALIB RESP] Timeout:WAIT_RESP_RX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_FINAL_TX:
        app_calib_resp_print("[CALIB RESP] Timeout:WAIT_FINAL_TX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_DATA_RX:
        app_calib_resp_print("[CALIB RESP] Timeout:WAIT_DATA_RX\r\n");
        break;
    case EN_APP_CALIB_STATE_SEND_END:
        break;
    case EN_APP_CALIB_STATE_WAIT_END_TX:
        app_calib_resp_print("[CALIB RESP] Timeout:WAIT_END_TX\r\n");
        break;
    default:
        break;
    }
}

//-------------------------------
// PUBLIC API
//-------------------------------

/**
 * @brief Initialize the responder boot calibration state machine.
 *
 * @param default_sum_tp_bias_cm Fallback bias (cm) when the window yields too
 *                               few samples to lock.
 */
void app_tdoa_calib_init(float default_sum_tp_bias_cm)
{
    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.control.active               = 1u;
    s_ctx.result.locked_sum_tp_bias_cm = default_sum_tp_bias_cm;
    s_ctx.control.state                = EN_APP_CALIB_STATE_SEND_REQ;

    app_calib_reset_link();

    app_calib_resp_print("[CALIB RESP] start, dev=%u\r\n",
                         (unsigned)g_tdoa_config.device_id);
}

/**
 * @brief Non-blocking step of the responder calibration state machine.
 * @details Call continuously from the main loop while `app_tdoa_calib_is_active()`
 *          returns 1.
 */
void app_tdoa_calib_process(void)
{
    cb_uwbsystem_txpayload_st       tx_payload;
    tdoa_calib_req_payload_t        req;
    tdoa_calib_ctrl_payload_t       ctrl;
    tdoa_calib_end_payload_t        end_frame;

    if (s_ctx.control.active == 0u || s_ctx.control.done != 0u) {
        return;
    }

    switch (s_ctx.control.state)
    {
    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_SEND_REQ:
        /* Broadcast CAL_REQ asking the initiator to pause TDOA and respond. */
        req.frame_type   = TDOA_FRAME_TYPE_CAL_REQ;
        req.responder_id = g_tdoa_config.device_id;
        req.seq          = s_ctx.session.seq;
        req.reserved     = 0u;

        tx_payload.ptrAddress  = (uint8_t*)&req;
        tx_payload.payloadSize = sizeof(req);
        cb_framework_uwb_tx_start(&s_pkt_cfg, &tx_payload, &s_tx_irq, EN_TRX_START_NON_DEFERRED);
        s_ctx.timing.tx_start_tick_ms = cb_hal_get_time_ms();
        s_ctx.control.state           = EN_APP_CALIB_STATE_WAIT_REQ_TX;
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_REQ_TX:
        if (s_ctx.io.TxDone != 0u) {
            s_ctx.io.TxDone = 0u;
            cb_framework_uwb_tx_end();
            /* Open non-deferred RX to listen for CAL_ACK. */
            cb_framework_uwb_rx_end(EN_UWB_RX_0);
            memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_pkt_cfg, &s_rx_irq, EN_TRX_START_NON_DEFERRED);
            s_ctx.timing.state_tick_ms = cb_hal_get_time_ms();
            s_ctx.control.state = EN_APP_CALIB_STATE_WAIT_ACK_RX;
        } else if (cb_hal_is_time_elapsed_ms(s_ctx.timing.tx_start_tick_ms,
                                              CALIB_TXDONE_GUARD_NONDEFERRED_MS) == CB_PASS) {
            /* TX-done guard: treat as lost, abandon and retry. */
            cb_framework_uwb_tx_end();
            s_ctx.io.TxDone = 0u;
            s_ctx.session.seq++;
            s_ctx.control.state = EN_APP_CALIB_STATE_SEND_REQ;
        }
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_ACK_RX:
        if (app_calib_validate_ctrl(TDOA_FRAME_TYPE_CAL_ACK) != 0u) {
            tdoa_calib_ctrl_payload_t* ack = (tdoa_calib_ctrl_payload_t*)s_ctx.io.stRxSlot.payload;
            s_ctx.session.initiator_id = ack->initiator_id;
            s_ctx.session.seq          = ack->seq;

            /* Record the 3-second sampling window start (tick-timer based) on the first ACK. */
            if (s_ctx.timing.window_started == 0u) {
                s_ctx.timing.window_started  = 1u;
                s_ctx.timing.calib_start_ms  = cb_hal_get_time_ms();
            }

            cb_framework_uwb_rx_end(EN_UWB_RX_0);
            memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));

            /* Go straight to END if the window already elapsed before the first ACK (unlikely but safe). */
            if (app_calib_window_elapsed() != 0u) {
                s_ctx.control.state = EN_APP_CALIB_STATE_SEND_END;
            } else {
                s_ctx.control.state = EN_APP_CALIB_STATE_SEND_POLL;
            }
        } else if (cb_hal_is_time_elapsed_ms(s_ctx.timing.state_tick_ms,
                                             TDOA_BOOT_CALIB_RX_GUARD_MS) == CB_PASS) {
            /* No ACK in time: print and resend CAL_REQ (seq incremented). */
            s_ctx.io.failure_state = EN_APP_CALIB_STATE_WAIT_ACK_RX;
            app_calib_timeout_print();
            app_calib_restart_sample();
        }
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_SEND_POLL:
        /* Per-sample reset of TX/RX + scheduled config + TSU clear so each sample's TX timestamps start fresh. */
        app_calib_reset_link();
        cb_framework_uwb_tsu_clear();

        ctrl.frame_type   = TDOA_FRAME_TYPE_CAL_POLL;
        ctrl.initiator_id = s_ctx.session.initiator_id;
        ctrl.responder_id = g_tdoa_config.device_id;
        ctrl.seq          = s_ctx.session.seq;
        ctrl.status       = TDOA_BOOT_CALIB_STATUS_ACTIVE;

        tx_payload.ptrAddress  = (uint8_t*)&ctrl;
        tx_payload.payloadSize = sizeof(ctrl);
        cb_framework_uwb_tx_start(&s_pkt_cfg, &tx_payload, &s_tx_irq, EN_TRX_START_NON_DEFERRED);
        s_ctx.timing.tx_start_tick_ms = cb_hal_get_time_ms();
        s_ctx.control.state           = EN_APP_CALIB_STATE_WAIT_POLL_TX;
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_POLL_TX:
        if (s_ctx.io.TxDone != 0u) {
            s_ctx.io.TxDone = 0u;
            /* Capture the POLL TX timestamp (DS-TWR point a). */
            cb_framework_uwb_get_tx_tsu_timestamp(&s_ctx.timing.poll_tx_tsu);
            cb_framework_uwb_tx_end();

            /* Arm abs timer 0 (treply2) to latch the upcoming RESP SFD, then open NON_DEFERRED RX for CAL_RESP. */
            cb_framework_uwb_enable_scheduled_trx(s_treply2_cfg);
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_pkt_cfg, &s_rx_irq, EN_TRX_START_NON_DEFERRED);

            s_ctx.timing.state_tick_ms = cb_hal_get_time_ms();
            s_ctx.control.state = EN_APP_CALIB_STATE_WAIT_RESP_RX;
        } else if (cb_hal_is_time_elapsed_ms(s_ctx.timing.tx_start_tick_ms,
                                              CALIB_TXDONE_GUARD_NONDEFERRED_MS) == CB_PASS) {
            /* TX-done guard expired: abandon sample (clear TxDone). */
            cb_framework_uwb_tx_end();
            s_ctx.io.TxDone = 0u;
            app_calib_restart_sample();
        }
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_RESP_RX:
        if (s_ctx.io.stRxSlot.used != 0u) {
            /* Validate CAL_RESP. */
            if (app_calib_validate_ctrl(TDOA_FRAME_TYPE_CAL_RESP) != 0u) {
                /* Capture RESP RX timestamp (DS-TWR point b). */
                s_ctx.timing.resp_rx_tsu = s_ctx.io.stRxSlot.rx_tsu;
                cb_framework_uwb_rx_end(EN_UWB_RX_0);
                memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));

                /* CAL_FINAL deferred: abs timer 0 (latched RESP SFD in WAIT_POLL_TX) fires at RESP SFD + 700 us. */
                ctrl.frame_type   = TDOA_FRAME_TYPE_CAL_FINAL;
                ctrl.initiator_id = s_ctx.session.initiator_id;
                ctrl.responder_id = g_tdoa_config.device_id;
                ctrl.seq          = s_ctx.session.seq;
                ctrl.status       = TDOA_BOOT_CALIB_STATUS_ACTIVE;

                tx_payload.ptrAddress  = (uint8_t*)&ctrl;
                tx_payload.payloadSize = sizeof(ctrl);
                cb_framework_uwb_configure_scheduled_trx(s_treply2_cfg);
                cb_framework_uwb_tx_start(&s_pkt_cfg, &tx_payload, &s_tx_irq, EN_TRX_START_DEFERRED);
                s_ctx.timing.tx_start_tick_ms = cb_hal_get_time_ms();
                s_ctx.control.state           = EN_APP_CALIB_STATE_WAIT_FINAL_TX;
            } else {
                /* Wrong frame: discard and keep waiting within the tick guard. */
                memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));
            }
        } else if (cb_hal_is_time_elapsed_ms(s_ctx.timing.state_tick_ms,
                                             TDOA_BOOT_CALIB_RX_GUARD_MS) == CB_PASS) {
            /* No CAL_RESP in time: print and restart the sample from CAL_REQ. */
            s_ctx.io.failure_state = EN_APP_CALIB_STATE_WAIT_RESP_RX;
            app_calib_timeout_print();
            app_calib_restart_sample();
        }
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_FINAL_TX:
        if (s_ctx.io.TxDone != 0u) {
            s_ctx.io.TxDone = 0u;
            /* Capture FINAL TX timestamp (DS-TWR point c). */
            cb_framework_uwb_get_tx_tsu_timestamp(&s_ctx.timing.final_tx_tsu);
            cb_framework_uwb_tx_end();
            cb_framework_uwb_disable_scheduled_trx(s_treply2_cfg);

            /* Open a NON_DEFERRED (always-listening) RX for CAL_DATA. */
            memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_pkt_cfg, &s_rx_irq, EN_TRX_START_NON_DEFERRED);

            s_ctx.timing.state_tick_ms = cb_hal_get_time_ms();
            s_ctx.control.state = EN_APP_CALIB_STATE_WAIT_DATA_RX;
        } else if (cb_hal_is_time_elapsed_ms(s_ctx.timing.tx_start_tick_ms,
                                              CALIB_TXDONE_GUARD_DEFERRED_MS) == CB_PASS) {
            /* Deferred TX-done guard (offset=1ms + 2ms guard). Abandon sample. */
            cb_framework_uwb_tx_end();
            s_ctx.io.TxDone = 0u;
            app_calib_restart_sample();
        }
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_DATA_RX:
    {
        if (s_ctx.io.stRxSlot.used != 0u) {
            if (s_ctx.io.stRxSlot.len >= sizeof(tdoa_calib_data_payload_t)) {
                tdoa_calib_data_payload_t data_frame;
                memcpy(&data_frame, s_ctx.io.stRxSlot.payload, sizeof(data_frame));

                if (data_frame.frame_type  == TDOA_FRAME_TYPE_CAL_DATA &&
                    data_frame.responder_id == g_tdoa_config.device_id &&
                    data_frame.initiator_id == s_ctx.session.initiator_id &&
                    data_frame.seq          == s_ctx.session.seq) {
                    /* Compute distance locally; we are the DS-TWR initiator side. */
                    cb_uwbframework_rangingdatacontainer_st initiator_data;
                    cb_uwbframework_rangingdatacontainer_st responder_data;
                    double distance_cm;

                    memset(&initiator_data, 0, sizeof(initiator_data));
                    cb_framework_uwb_calculate_initiator_tround_treply(
                        &initiator_data,
                        s_ctx.timing.poll_tx_tsu,   /* a: POLL TX */
                        s_ctx.timing.final_tx_tsu,  /* c: FINAL TX */
                        s_ctx.timing.resp_rx_tsu);  /* b: RESP RX */

                    memset(&responder_data, 0, sizeof(responder_data));
                    responder_data.dstwrTroundTreply = data_frame.twr_responder_tround_treply;
                    responder_data.dstwrRangingBias  = data_frame.twr_responder_bias_cm;

                    /* Return value is already in centimeters (the header "meters" doc is wrong). */
                    distance_cm = cb_framework_uwb_calculate_distance(initiator_data, responder_data);

                    /* Store sample in cm, adding the local hardware bias offset. */
                    app_calib_push_sample((float)distance_cm + (float)g_tdoa_config.bias);
                }
            }

            cb_framework_uwb_rx_end(EN_UWB_RX_0);
            memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));

            /* Check window expiry or continue sampling. */
            if (app_calib_window_elapsed() != 0u) {
                s_ctx.control.state = EN_APP_CALIB_STATE_SEND_END;
            } else {
                /* Idle for TDOA_BOOT_CALIB_SAMPLE_GAP_MS before the next POLL. */
                s_ctx.session.seq++;
                s_ctx.timing.sample_gap_start_ms = cb_hal_get_time_ms();
                s_ctx.control.state = EN_APP_CALIB_STATE_WAIT_NEXT_SAMPLE;
            }
        } else if (cb_hal_is_time_elapsed_ms(s_ctx.timing.state_tick_ms,
                                             TDOA_BOOT_CALIB_RX_GUARD_MS) == CB_PASS) {
            /* No CAL_DATA in time: close out with CAL_END if the window is over, else restart from CAL_REQ. */
            s_ctx.io.failure_state = EN_APP_CALIB_STATE_WAIT_DATA_RX;
            app_calib_timeout_print();
            if (app_calib_window_elapsed() != 0u) {
                app_calib_reset_link();
                s_ctx.control.state = EN_APP_CALIB_STATE_SEND_END;
            } else {
                app_calib_restart_sample();
            }
        }
        break;
    }

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_NEXT_SAMPLE:
        /* Pure tick wait between samples; no UWB hardware armed here. */
        if (cb_hal_is_time_elapsed_ms(s_ctx.timing.sample_gap_start_ms,
                                      TDOA_BOOT_CALIB_SAMPLE_GAP_MS) == CB_PASS) {
            s_ctx.control.state = EN_APP_CALIB_STATE_SEND_POLL;
        }
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_SEND_END:
    {
        /* Compute the locked bias once (guarded); CAL_END carries it and resends reuse the same value. */
        uint8_t first_end = (s_ctx.control.lock_guard == 0u);
        app_calib_compute_lock();
        if (first_end != 0u) {
            s_ctx.timing.end_first_tick_ms = cb_hal_get_time_ms();
        }

        /* Tear down any open RX so this TX is clean (resend path re-enters here). */
        app_calib_reset_link();

        end_frame.frame_type             = TDOA_FRAME_TYPE_CAL_END;
        end_frame.initiator_id           = s_ctx.session.initiator_id;
        end_frame.responder_id           = g_tdoa_config.device_id;
        end_frame.seq                    = s_ctx.session.seq;
        end_frame.status                 = (s_ctx.control.locked != 0u) ?
                                           TDOA_BOOT_CALIB_STATUS_DONE :
                                           TDOA_BOOT_CALIB_STATUS_FAIL;
        end_frame.locked_sum_tp_bias_cm  = s_ctx.result.locked_sum_tp_bias_cm;

        tx_payload.ptrAddress  = (uint8_t*)&end_frame;
        tx_payload.payloadSize = sizeof(end_frame);
        cb_framework_uwb_tx_start(&s_pkt_cfg, &tx_payload, &s_tx_irq, EN_TRX_START_NON_DEFERRED);
        s_ctx.timing.tx_start_tick_ms = cb_hal_get_time_ms();
        s_ctx.control.state           = EN_APP_CALIB_STATE_WAIT_END_TX;
        break;
    }

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_END_TX:
        /* After CAL_END is on air, open a NON_DEFERRED RX for CAL_END_ACK. */
        if (s_ctx.io.TxDone != 0u ||
            cb_hal_is_time_elapsed_ms(s_ctx.timing.tx_start_tick_ms,
                                      CALIB_TXDONE_GUARD_NONDEFERRED_MS) == CB_PASS) {
            s_ctx.io.TxDone = 0u;
            cb_framework_uwb_tx_end();
            cb_framework_uwb_rx_end(EN_UWB_RX_0);
            memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_pkt_cfg, &s_rx_irq, EN_TRX_START_NON_DEFERRED);
            s_ctx.control.state = EN_APP_CALIB_STATE_WAIT_END_ACK_RX;
        }
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_WAIT_END_ACK_RX:
        /* Resend CAL_END every END_RESEND_MS until CAL_END_ACK, capped by END_MAX_MS. */
        if (app_calib_validate_ctrl(TDOA_FRAME_TYPE_CAL_END_ACK) != 0u) {
            cb_framework_uwb_rx_end(EN_UWB_RX_0);
            memset(&s_ctx.io.stRxSlot, 0, sizeof(s_ctx.io.stRxSlot));
            app_calib_resp_print("[CALIB RESP] END_ACK rx, done\r\n");
            s_ctx.control.active = 0u;
            s_ctx.control.done   = 1u;
            s_ctx.control.state  = EN_APP_CALIB_STATE_DONE;
        } else if (cb_hal_is_time_elapsed_ms(s_ctx.timing.end_first_tick_ms,
                                             TDOA_BOOT_CALIB_END_MAX_MS) == CB_PASS) {
            /* Handshake cap reached: terminate anyway so TDOA can start. */
            app_calib_resp_print("[CALIB RESP] END_ACK timeout, done\r\n");
            app_calib_reset_link();
            s_ctx.control.active = 0u;
            s_ctx.control.done   = 1u;
            s_ctx.control.state  = EN_APP_CALIB_STATE_DONE;
        } else if (cb_hal_is_time_elapsed_ms(s_ctx.timing.tx_start_tick_ms,
                                             TDOA_BOOT_CALIB_END_RESEND_MS) == CB_PASS) {
            /* No ACK yet: resend CAL_END (compute_lock is guarded, no recompute). */
            s_ctx.control.state = EN_APP_CALIB_STATE_SEND_END;
        }
        break;

    /* -------------------------------------------------------- */
    case EN_APP_CALIB_STATE_DONE:
    default:
        break;
    }
}

/**
 * @brief Return whether responder calibration is currently active.
 */
uint8_t app_tdoa_calib_is_active(void)
{
    return s_ctx.control.active;
}

/**
 * @brief Return whether calibration has reached a terminal state.
 */
uint8_t app_tdoa_calib_is_done(void)
{
    return s_ctx.control.done;
}

/**
 * @brief Return whether a stable sum T_p bias was locked.
 */
uint8_t app_tdoa_calib_is_locked(void)
{
    return s_ctx.control.locked;
}

/**
 * @brief Return the locked or fallback sum T_p bias in centimetres.
 */
float app_tdoa_calib_get_sum_tp_bias_cm(void)
{
    return s_ctx.result.locked_sum_tp_bias_cm;
}

/**
 * @brief TX done IRQ callback routed here when calibration is active.
 * @details Keeps the ISR minimal; defer all state work to the main loop.
 */
void app_tdoa_calib_tx_done_irqcb(void)
{
    s_ctx.io.TxDone = 1u;
}

/**
 * @brief RX done IRQ callback routed here when calibration is active.
 * @details Captures payload and timestamp into the single-slot buffer. The main
 *          loop performs validation, sequence checking, and state transitions.
 */
void app_tdoa_calib_rx_done_irqcb(void)
{
    cb_uwbsystem_rxstatus_un rx_status = cb_framework_uwb_get_rx_status();
    uint16_t pkt_size;

    if (s_ctx.control.active == 0u || s_ctx.control.done != 0u) {
        return;
    }
    if (rx_status.rx0_ok == 0u || s_ctx.io.stRxSlot.used != 0u) {
        return;
    }

    pkt_size = cb_framework_uwb_get_rx_packet_size(&s_pkt_cfg);
    if (pkt_size == 0u || pkt_size > TDOA_RX_MSG_MAX_LEN) {
        return;
    }

    cb_framework_uwb_get_rx_payload(s_ctx.io.stRxSlot.payload, pkt_size);
    s_ctx.io.stRxSlot.len        = (uint8_t)pkt_size;
    s_ctx.io.stRxSlot.frame_type = s_ctx.io.stRxSlot.payload[0];
    s_ctx.io.stRxSlot.round_index = 0u;
    // cb_framework_uwb_get_rx_tsu_timestamp(&s_ctx.io.stRxSlot.rx_tsu, EN_UWB_RX_0);
    cb_framework_uwb_get_rx_tsu_timestamp_lemm(&s_ctx.io.stRxSlot.rx_tsu, EN_UWB_RX_0, 18, 0, 2);
    s_ctx.io.stRxSlot.used = 1u;
}

/**
 * @brief Timer 0 IRQ callback routed here when calibration is active.
 * @details Sets the watchdog flag and records the failure state so the main loop
 *          can print a descriptive timeout message and restart the sample.
 */
void app_tdoa_calib_timer_irqcb(void)
{
    s_ctx.io.failure_state    = s_ctx.control.state;
    s_ctx.io.watchdog_timeout = 1u;
}
