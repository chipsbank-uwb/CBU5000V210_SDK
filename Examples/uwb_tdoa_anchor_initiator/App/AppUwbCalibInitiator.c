/**
 * @file    AppUwbCalibInitiator.c
 * @brief   TDOA initiator boot calibration state machine (DS-TWR responder side).
 * @details The TDOA initiator answers as the DS-TWR *responder*: it receives
 *          CAL_POLL, replies with a deferred CAL_RESP, waits for CAL_FINAL, then
 *          sends CAL_DATA carrying its own tround/treply so the driver side can
 *          close cb_framework_uwb_calculate_distance() locally. The frame ladder
 *          and timestamp mapping are in the role diagram below.
 *
 *          Direction-A robustness model: CAL_POLL/CAL_FINAL RX are NON_DEFERRED
 *          (always-listening); only CAL_RESP TX is deferred for DS-TWR turnaround
 *          precision. One abs timer (EN_UWB_ABSOLUTE_TIMER_0) carries it:
 *            CAL_RESP TX : POLL SFD + 700 us -> TX
 *          Recovery is per-state tick guards plus one overall deadline backstop
 *          (no per-sample hardware watchdog). After CAL_END the initiator returns
 *          CAL_END_ACK and lingers briefly to re-ACK any resent CAL_END before
 *          handing the radio back to TDOA.
 * @author  Chipsbank
 * @date    2026
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbCalibInitiator.h"
#include "AppUwbTdoaInitiator.h"
#include "tdoa_common.h"
#include "tdoa_config.h"
#include "CB_uwbframework.h"
#include "NonLIB_sharedUtils.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_TDOA_BOOT_CALIB_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_TDOA_BOOT_CALIB_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_boot_calib_init_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_boot_calib_init_print(...)
#endif

//-------------------------------
// TDOA: CALIB-INITIATOR SETUP
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
//       |---------7. CAL_END ------------->|   (locked_sum_tp_bias_cm; resent every 10 ms)
//       |<--------8. CAL_END_ACK ----------|   (this device acks, then lingers 30 ms)
//     Terminate                         Resume TDOA
//
// All RX windows here are NON_DEFERRED (always-listening); only CAL_RESP TX is
// deferred (POLL SFD + 700 us). No per-sample watchdog: FINAL_RX uses a 5 ms
// tick guard, and one overall deadline backstops the whole session.
//
// DS-TWR initiator: Tround_1 = b - a    Treply_2 = c - b
// DS-TWR responder: Treply_1 = e - d    Tround_2 = f - e
//
//  a: CAL_POLL  TX (remote)     d: stPollRxTsu   (this device)
//  b: CAL_RESP  RX (remote)     e: stRespTxTsu   (this device)
//  c: CAL_FINAL TX (remote)     f: stFinalRxTsu  (this device)
//-------------------------------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------

/* TX-done guard deadlines (milliseconds from tx_start call).
 * Delayed TX (CAL_RESP, CAL_DATA): scheduled offset ~700us rounds up to 1 ms.
 * Non-deferred TX (CAL_ACK): offset 0.
 */
#define CALIB_TXDONE_GUARD_DELAYED_MS   (1u + TDOA_TXDONE_GUARD_MS)   /* = 3 ms */
#define CALIB_TXDONE_GUARD_NODLY_MS     (TDOA_TXDONE_GUARD_MS)        /* = 2 ms */

/* Overall calibration deadline (ms from app_tdoa_calib_start). The initiator is
 * a passive follower: a single per-sample watchdog timeout must NOT abort the
 * session, otherwise the radio is handed back to TDOA mid-window and its
 * POLL_DTM frames corrupt the responder's samples. It recovers each sample and
 * keeps following the responder until CAL_END arrives or this deadline elapses.
 * Covers the responder's 3 s window plus margin. */
#define CALIB_OVERALL_DEADLINE_MS       (TDOA_BOOT_CALIB_DURATION_MS + 1500u)

//-------------------------------
// ENUM SECTION
//-------------------------------

typedef enum {
    EN_APP_CALIB_STATE_IDLE = 0,        /* State machine inactive. */
    EN_APP_CALIB_STATE_SEND_ACK,        /* Send CAL_ACK (non-deferred) to confirm entry. */
    EN_APP_CALIB_STATE_WAIT_ACK_TX,     /* Wait until CAL_ACK is transmitted. */
    EN_APP_CALIB_STATE_WAIT_POLL_RX,    /* Open RX for CAL_POLL; arm 50ms watchdog. */
    EN_APP_CALIB_STATE_WAIT_POLL_DONE,  /* CAL_POLL received; send deferred CAL_RESP. */
    EN_APP_CALIB_STATE_SEND_RESP,       /* Issue deferred CAL_RESP TX (POLL SFD + 700 us). */
    EN_APP_CALIB_STATE_WAIT_RESP_TX,    /* Wait for CAL_RESP TX done. */
    EN_APP_CALIB_STATE_WAIT_FINAL_RX,   /* Open deferred RX for CAL_FINAL (RESP done + 500 us). */
    EN_APP_CALIB_STATE_WAIT_FINAL_DONE, /* CAL_FINAL received; send deferred CAL_DATA. */
    EN_APP_CALIB_STATE_SEND_DATA,       /* Issue deferred CAL_DATA TX (FINAL SFD + 700 us). */
    EN_APP_CALIB_STATE_WAIT_DATA_TX,    /* Wait for CAL_DATA TX done. */
    EN_APP_CALIB_STATE_WAIT_END_RX,     /* Waiting for next CAL_POLL or CAL_END. */
    EN_APP_CALIB_STATE_END_ACK_WAIT_TX, /* Wait for CAL_END_ACK TX done. */
    EN_APP_CALIB_STATE_END_ACK_LINGER,  /* Re-ACK any resent CAL_END before resuming TDOA. */
} app_uwbtdoacalib_state_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

typedef struct {
    struct {
        uint8_t                     active;
        uint8_t                     done;
        app_uwbtdoacalib_state_en   state;
    } control;
    struct {
        uint8_t  responderId;
        uint16_t seq;
    } session;
    struct {
        /* DS-TWR responder timestamps: d, e, f. */
        cb_uwbsystem_rx_tsutimestamp_st stPollRxTsu;   /* d: POLL received. */
        cb_uwbsystem_tx_tsutimestamp_st stRespTxTsu;   /* e: RESP sent. */
        cb_uwbsystem_rx_tsutimestamp_st stFinalRxTsu;  /* f: FINAL received. */

        /* TX-done guard state. */
        uint32_t txStartTickMs;         /* Tick when tx_start was called. */
        uint8_t  txIsDelayed;           /* 1 = deferred TX, 0 = non-deferred. */
        uint8_t  txMissedEvent;         /* 1 = resend as non-deferred after missed trigger. */

        /* Overall calibration deadline base, set in app_tdoa_calib_start. Checked
         * unconditionally each tick as the single backstop (no hardware watchdog). */
        uint32_t calibStartMs;
        uint32_t stateTickMs;   /* RX-wait entry tick; per-state tick guard. */
        uint32_t lingerTickMs;  /* END_ACK linger quiet-window reference. */
    } timing;
    struct {
        uint8_t             TxDone;
        uint8_t             watchdogTimeout;
        app_uwbtdoacalib_state_en failureState;
        tdoa_rx_msg_slot_t  stRxSlot;
    } io;
} app_uwbtdoacalib_ctx_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

static cb_uwbsystem_packetconfig_st s_stUwbPacketConfig = DEF_TDOA_INITIATOR_PACKET_CONFIG_DEFAULTS;
static cb_uwbsystem_tx_irqenable_st s_stTxIrqEnable     = { .txDone  = 1 };
static cb_uwbsystem_rx_irqenable_st s_stRxIrqEnable     = { .rx0Done = 1 };

/* CAL_RESP TX: deferred, triggered by POLL SFD + 700 us.
 * This is the only scheduled config: CAL_POLL/CAL_FINAL RX are NON_DEFERRED. */
static cb_uwbframework_trx_scheduledconfig_st s_stRespTxSchedCfg = {
    .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,
    .eventIndex         = EN_UWBEVENT_17_RX0_SFD_DET,
    .absTimer           = EN_UWB_ABSOLUTE_TIMER_0,
    .timeoutValue       = TDOA_BOOT_CALIB_TREPLY_US,    /* 700 us */
    .eventCtrlMask      = EN_UWBCTRL_TX_START_MASK,
};

static app_uwbtdoacalib_ctx_st s_stCtx;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void app_tdoa_calib_reset_link(void);
static void app_tdoa_calib_open_poll_rx(void);
static void app_tdoa_calib_send_ack(void);
static void app_tdoa_calib_send_end_ack(void);
static void app_tdoa_calib_send_resp(uint8_t non_deferred);
static void app_tdoa_calib_send_data(void);
static void app_tdoa_calib_handle_poll_rx(void);
static void app_tdoa_calib_handle_final_rx(void);
static uint8_t app_tdoa_calib_handle_end_frame(void);
static void app_tdoa_calib_timeout_print(void);

//-------------------------------
// INTERNAL HELPERS
//-------------------------------

/**
 * @brief Disable all scheduled TRX configs and tear down TX/RX state.
 */
static void app_tdoa_calib_reset_link(void)
{
    cb_framework_uwb_disable_scheduled_trx(s_stRespTxSchedCfg);
    cb_framework_uwb_tx_end();
    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
    s_stCtx.io.TxDone = 0u;
}

/**
 * @brief Open an immediate non-deferred RX window for CAL_POLL or CAL_END.
 *        Arms the 50ms watchdog.
 */
static void app_tdoa_calib_open_poll_rx(void)
{
    cb_framework_uwb_tsu_clear();

    memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                              &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
    s_stCtx.control.state = EN_APP_CALIB_STATE_WAIT_POLL_RX;
}

/**
 * @brief Build and send CAL_ACK (non-deferred).
 */
static void app_tdoa_calib_send_ack(void)
{
    tdoa_calib_ctrl_payload_t ack;
    cb_uwbsystem_txpayload_st tx_payload;

    ack.frame_type  = TDOA_FRAME_TYPE_CAL_ACK;
    ack.initiator_id = g_tdoa_config.device_id;
    ack.responder_id = s_stCtx.session.responderId;
    ack.seq          = s_stCtx.session.seq;
    ack.status       = TDOA_BOOT_CALIB_STATUS_ACTIVE;

    tx_payload.ptrAddress  = (uint8_t*)&ack;
    tx_payload.payloadSize = sizeof(ack);

    cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &tx_payload,
                              &s_stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
    s_stCtx.timing.txStartTickMs = cb_hal_get_time_ms();
    s_stCtx.timing.txIsDelayed   = 0u;
    s_stCtx.timing.txMissedEvent = 0u;
    s_stCtx.control.state = EN_APP_CALIB_STATE_WAIT_ACK_TX;
}

/**
 * @brief Build and send CAL_END_ACK (non-deferred) to acknowledge a CAL_END.
 */
static void app_tdoa_calib_send_end_ack(void)
{
    tdoa_calib_ctrl_payload_t end_ack;
    cb_uwbsystem_txpayload_st tx_payload;

    end_ack.frame_type   = TDOA_FRAME_TYPE_CAL_END_ACK;
    end_ack.initiator_id = g_tdoa_config.device_id;
    end_ack.responder_id = s_stCtx.session.responderId;
    end_ack.seq          = s_stCtx.session.seq;
    end_ack.status       = TDOA_BOOT_CALIB_STATUS_DONE;

    tx_payload.ptrAddress  = (uint8_t*)&end_ack;
    tx_payload.payloadSize = sizeof(end_ack);

    cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &tx_payload,
                              &s_stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
    s_stCtx.timing.txStartTickMs = cb_hal_get_time_ms();
    s_stCtx.timing.txIsDelayed   = 0u;
    s_stCtx.timing.txMissedEvent = 0u;
}

/**
 * @brief Build and send CAL_RESP.
 * @param non_deferred  1 = send immediately (guard-expiry retry that missed trigger),
 *                      0 = send deferred (POLL SFD + 700 us, normal path).
 *
 * Normal path:
 *   Call cb_framework_uwb_enable_scheduled_trx(s_stRespTxSchedCfg) BEFORE
 *   the POLL SFD arrives (done in app_tdoa_calib_open_poll_rx), then
 *   call cb_framework_uwb_configure_scheduled_trx after POLL RX done, then
 *   call tx_start(DEFERRED).
 */
static void app_tdoa_calib_send_resp(uint8_t non_deferred)
{
    tdoa_calib_ctrl_payload_t resp;
    cb_uwbsystem_txpayload_st tx_payload;
    cb_uwbsystem_trx_startmode_en mode;

    resp.frame_type   = TDOA_FRAME_TYPE_CAL_RESP;
    resp.initiator_id = g_tdoa_config.device_id;
    resp.responder_id = s_stCtx.session.responderId;
    resp.seq          = s_stCtx.session.seq;
    resp.status       = TDOA_BOOT_CALIB_STATUS_ACTIVE;

    tx_payload.ptrAddress  = (uint8_t*)&resp;
    tx_payload.payloadSize = sizeof(resp);

    if (non_deferred) {
        mode = EN_TRX_START_NON_DEFERRED;
        s_stCtx.timing.txIsDelayed   = 0u;
        s_stCtx.timing.txMissedEvent = 0u;
    } else {
        /* Configure the abs-timer schedule now that the POLL SFD timestamp is latched. */
        cb_framework_uwb_configure_scheduled_trx(s_stRespTxSchedCfg);
        mode = EN_TRX_START_DEFERRED;
        s_stCtx.timing.txIsDelayed   = 1u;
        s_stCtx.timing.txMissedEvent = 0u;
    }

    cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &tx_payload,
                              &s_stTxIrqEnable, mode);
    s_stCtx.timing.txStartTickMs = cb_hal_get_time_ms();
    s_stCtx.control.state        = EN_APP_CALIB_STATE_WAIT_RESP_TX;
}

/**
 * @brief Build and send CAL_DATA (non-deferred) carrying this device's
 *        DS-TWR-responder tround/treply.
 *
 * This device is the DS-TWR *responder* side:
 *   cb_framework_uwb_calculate_responder_tround_treply(
 *       &container, stRespTxTsu, stPollRxTsu, stFinalRxTsu)
 */
static void app_tdoa_calib_send_data(void)
{
    tdoa_calib_data_payload_t data_payload;
    cb_uwbsystem_txpayload_st tx_payload;
    cb_uwbframework_rangingdatacontainer_st responder_container;

    memset(&responder_container, 0, sizeof(responder_container));
    responder_container.dstwrRangingBias = (int32_t)g_tdoa_config.bias;

    /* DS-TWR responder-side tround/treply: Treply1 = e-d, Tround2 = f-e. */
    cb_framework_uwb_calculate_responder_tround_treply(
        &responder_container,
        s_stCtx.timing.stRespTxTsu,   /* TX timestamp (e) */
        s_stCtx.timing.stPollRxTsu,   /* First RX timestamp (d) */
        s_stCtx.timing.stFinalRxTsu   /* Second RX timestamp (f) */
    );

    data_payload.frame_type             = TDOA_FRAME_TYPE_CAL_DATA;
    data_payload.initiator_id           = g_tdoa_config.device_id;
    data_payload.responder_id           = s_stCtx.session.responderId;
    data_payload.seq                    = s_stCtx.session.seq;
    data_payload.twr_responder_tround_treply = responder_container.dstwrTroundTreply;
    data_payload.twr_responder_bias_cm  = responder_container.dstwrRangingBias;

    tx_payload.ptrAddress  = (uint8_t*)&data_payload;
    tx_payload.payloadSize = sizeof(data_payload);

    /* CAL_DATA carries data only (not a DS-TWR timing point): send non-deferred. */
    s_stCtx.timing.txIsDelayed   = 0u;
    s_stCtx.timing.txMissedEvent = 0u;
    cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &tx_payload,
                              &s_stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
    s_stCtx.timing.txStartTickMs = cb_hal_get_time_ms();
    s_stCtx.control.state        = EN_APP_CALIB_STATE_WAIT_DATA_TX;
}

/**
 * @brief Validate and consume a CAL_POLL frame from the RX mailbox.
 *        Saves the POLL RX timestamp (point d) and sets state to SEND_RESP.
 */
static void app_tdoa_calib_handle_poll_rx(void)
{
    tdoa_calib_ctrl_payload_t poll_local;

    if (s_stCtx.io.stRxSlot.used == 0u ||
        s_stCtx.io.stRxSlot.len < sizeof(tdoa_calib_ctrl_payload_t)) {
        return;
    }

    /* Copy the packed payload into a local struct before field access. */
    memcpy(&poll_local, s_stCtx.io.stRxSlot.payload, sizeof(tdoa_calib_ctrl_payload_t));

    if (poll_local.frame_type  != TDOA_FRAME_TYPE_CAL_POLL ||
        poll_local.initiator_id != g_tdoa_config.device_id  ||
        poll_local.responder_id != s_stCtx.session.responderId) {
        memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
        return;
    }

    /* Update seq to match the sender's latest value. */
    s_stCtx.session.seq = poll_local.seq;

    /* Capture the POLL RX timestamp (point d). */
    s_stCtx.timing.stPollRxTsu = s_stCtx.io.stRxSlot.rx_tsu;

    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
    s_stCtx.control.state = EN_APP_CALIB_STATE_SEND_RESP;
}

/**
 * @brief Validate and consume a CAL_FINAL frame from the RX mailbox.
 *        Saves the FINAL RX timestamp (point f) and sets state to SEND_DATA.
 */
static void app_tdoa_calib_handle_final_rx(void)
{
    tdoa_calib_ctrl_payload_t final_local;

    if (s_stCtx.io.stRxSlot.used == 0u ||
        s_stCtx.io.stRxSlot.len < sizeof(tdoa_calib_ctrl_payload_t)) {
        return;
    }

    /* Copy the packed payload into a local struct. */
    memcpy(&final_local, s_stCtx.io.stRxSlot.payload, sizeof(tdoa_calib_ctrl_payload_t));

    if (final_local.frame_type  != TDOA_FRAME_TYPE_CAL_FINAL ||
        final_local.initiator_id != g_tdoa_config.device_id   ||
        final_local.responder_id != s_stCtx.session.responderId) {
        memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
        return;
    }

    /* Capture the FINAL RX timestamp (point f). */
    s_stCtx.timing.stFinalRxTsu = s_stCtx.io.stRxSlot.rx_tsu;

    cb_framework_uwb_rx_end(EN_UWB_RX_0);
    memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
    s_stCtx.control.state = EN_APP_CALIB_STATE_SEND_DATA;
}

/**
 * @brief Check the RX mailbox for a CAL_END frame.
 * @return 1 if a valid CAL_END for this session was consumed, 0 otherwise.
 */
static uint8_t app_tdoa_calib_handle_end_frame(void)
{
    tdoa_calib_end_payload_t end_local;

    if (s_stCtx.io.stRxSlot.used == 0u ||
        s_stCtx.io.stRxSlot.len < sizeof(tdoa_calib_end_payload_t)) {
        return 0u;
    }

    if (s_stCtx.io.stRxSlot.payload[0] != TDOA_FRAME_TYPE_CAL_END) {
        return 0u;
    }

    /* Copy the packed payload. */
    memcpy(&end_local, s_stCtx.io.stRxSlot.payload, sizeof(tdoa_calib_end_payload_t));

    if (end_local.initiator_id != g_tdoa_config.device_id ||
        end_local.responder_id != s_stCtx.session.responderId) {
        memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
        return 0u;
    }

    /* CAL_END received: print bias and ACK; do NOT mark done yet, linger to re-ACK resent CAL_END. */
    app_boot_calib_init_print(
        "[BOOTCAL INIT] CAL_END responder=%u lockedSumTpBiasCm=%.4f\r\n",
        (unsigned)s_stCtx.session.responderId,
        (double)end_local.locked_sum_tp_bias_cm);

    s_stCtx.session.seq = end_local.seq;
    app_tdoa_calib_reset_link();          /* disable RESP sched, end TX/RX, clear slot */
    app_tdoa_calib_send_end_ack();
    s_stCtx.control.state = EN_APP_CALIB_STATE_END_ACK_WAIT_TX;
    return 1u;
}

/**
 * @brief Print the watchdog timeout state label.
 */
static void app_tdoa_calib_timeout_print(void)
{
    switch (s_stCtx.io.failureState) {
    case EN_APP_CALIB_STATE_WAIT_ACK_TX:
        app_boot_calib_init_print("[BOOTCAL INIT] Timeout:WAIT_ACK_TX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_POLL_RX:
        app_boot_calib_init_print("[BOOTCAL INIT] Timeout:WAIT_POLL_RX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_POLL_DONE:
        app_boot_calib_init_print("[BOOTCAL INIT] Timeout:WAIT_POLL_DONE\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_RESP_TX:
        app_boot_calib_init_print("[BOOTCAL INIT] Timeout:WAIT_RESP_TX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_FINAL_RX:
        app_boot_calib_init_print("[BOOTCAL INIT] Timeout:WAIT_FINAL_RX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_FINAL_DONE:
        app_boot_calib_init_print("[BOOTCAL INIT] Timeout:WAIT_FINAL_DONE\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_DATA_TX:
        app_boot_calib_init_print("[BOOTCAL INIT] Timeout:WAIT_DATA_TX\r\n");
        break;
    case EN_APP_CALIB_STATE_WAIT_END_RX:
        app_boot_calib_init_print("[BOOTCAL INIT] Timeout:WAIT_END_RX\r\n");
        break;
    case EN_APP_CALIB_STATE_IDLE:
        break;
    case EN_APP_CALIB_STATE_SEND_ACK:
        break;
    case EN_APP_CALIB_STATE_SEND_RESP:
        break;
    case EN_APP_CALIB_STATE_SEND_DATA:
        break;
    default:
        break;
    }
}

//-------------------------------
// PUBLIC INTERFACE
//-------------------------------

/**
 * @brief Reset calibration context. Called once during application init.
 */
void app_tdoa_calib_init(void)
{
    memset(&s_stCtx, 0, sizeof(s_stCtx));
}

/**
 * @brief Start a calibration session when CAL_REQ is received.
 *
 * The TDOA main file calls this after receiving a CAL_REQ.  We transition
 * directly to SEND_ACK so the first process() call dispatches the CAL_ACK.
 *
 * @param responder_id  TDOA responder ID from the CAL_REQ.
 * @param seq           Sequence number from the CAL_REQ.
 */
void app_tdoa_calib_start(uint8_t responder_id, uint8_t seq)
{
    app_tdoa_calib_reset_link();
    s_stCtx.control.active    = 1u;
    s_stCtx.control.done      = 0u;
    s_stCtx.session.responderId = responder_id;
    s_stCtx.session.seq         = seq;
    s_stCtx.timing.calibStartMs = cb_hal_get_time_ms();
    s_stCtx.control.state       = EN_APP_CALIB_STATE_SEND_ACK;

    app_boot_calib_init_print("[BOOTCAL INIT] start responder=%u seq=%u\r\n",
                              (unsigned)responder_id, (unsigned)seq);
}

/**
 * @brief Abort calibration and release all radio resources.
 */
void app_tdoa_calib_stop(void)
{
    tdoa_watchdog_timer_off(EN_TIMER_1);
    app_tdoa_calib_reset_link();
    memset(&s_stCtx, 0, sizeof(s_stCtx));
}

/**
 * @brief Non-blocking calibration state-machine tick. Call while is_active() != 0.
 */
void app_tdoa_calib_process(void)
{
    if (s_stCtx.control.active == 0u) {
        return;
    }

    /* Overall-deadline backstop (no per-sample watchdog): ends calibration if the responder vanishes. */
    if (cb_hal_is_time_elapsed_ms(s_stCtx.timing.calibStartMs,
                                  CALIB_OVERALL_DEADLINE_MS) == CB_PASS) {
        app_boot_calib_init_print("[BOOTCAL INIT] overall deadline, resume TDOA\r\n");
        app_tdoa_calib_reset_link();
        s_stCtx.control.done   = 1u;
        s_stCtx.control.active = 0u;
        return;
    }

    switch (s_stCtx.control.state)
    {
        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_SEND_ACK:
        {
            /* Send CAL_ACK non-deferred to confirm calibration entry. */
            app_tdoa_calib_send_ack();
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_WAIT_ACK_TX:
        {
            uint32_t guard = CALIB_TXDONE_GUARD_NODLY_MS;

            if (s_stCtx.io.TxDone) {
                s_stCtx.io.TxDone = 0u;
                cb_framework_uwb_tx_end();
                /* Enable RESP TX abs-timer before opening POLL RX so event-capture is armed pre-POLL-SFD. */
                cb_framework_uwb_enable_scheduled_trx(s_stRespTxSchedCfg);
                app_tdoa_calib_open_poll_rx();    /* arms watchdog + opens RX */
            } else if (cb_hal_is_time_elapsed_ms(s_stCtx.timing.txStartTickMs,
                                                  guard) == CB_PASS) {
                /* Guard expiry: tx_end, clear TxDone, retry non-deferred. */
                cb_framework_uwb_tx_end();
                s_stCtx.io.TxDone = 0u;
                app_tdoa_calib_send_ack();
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_WAIT_POLL_RX:
        {
            /* Waiting for CAL_POLL; also accept CAL_END (finished) and CAL_REQ (restart -> re-ACK to resync). */
            if (s_stCtx.io.stRxSlot.used) {
                uint8_t frameType = s_stCtx.io.stRxSlot.payload[0];

                if (frameType == TDOA_FRAME_TYPE_CAL_END) {
                    app_tdoa_calib_handle_end_frame();
                    break;
                }
                if (frameType == TDOA_FRAME_TYPE_CAL_REQ) {
                    /* Responder re-issued CAL_REQ after a timeout: adopt its seq and re-ACK to resync. */
                    if (s_stCtx.io.stRxSlot.len >= sizeof(tdoa_calib_req_payload_t)) {
                        tdoa_calib_req_payload_t reqLocal;
                        memcpy(&reqLocal, s_stCtx.io.stRxSlot.payload,
                               sizeof(tdoa_calib_req_payload_t));
                        s_stCtx.session.seq = reqLocal.seq;
                    }
                    cb_framework_uwb_rx_end(EN_UWB_RX_0);
                    memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
                    s_stCtx.control.state = EN_APP_CALIB_STATE_SEND_ACK;
                    break;
                }
                app_tdoa_calib_handle_poll_rx();
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_WAIT_POLL_DONE:
        {
            /* Transition state — handle_poll_rx sets us to SEND_RESP directly. */
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_SEND_RESP:
        {
            /* Normal path: configure abs-timer from the latched POLL SFD timestamp and issue deferred TX. */
            app_tdoa_calib_send_resp(s_stCtx.timing.txMissedEvent);
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_WAIT_RESP_TX:
        {
            uint32_t guard = s_stCtx.timing.txIsDelayed
                                 ? CALIB_TXDONE_GUARD_DELAYED_MS
                                 : CALIB_TXDONE_GUARD_NODLY_MS;

            if (s_stCtx.io.TxDone) {
                s_stCtx.io.TxDone = 0u;
                /* Capture the RESP TX timestamp (point e). */
                cb_framework_uwb_get_tx_tsu_timestamp(&s_stCtx.timing.stRespTxTsu);
                cb_framework_uwb_disable_scheduled_trx(s_stRespTxSchedCfg);
                cb_framework_uwb_tx_end();
                /* Open a NON_DEFERRED (always-listening) RX for CAL_FINAL. */
                memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
                cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                                          &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
                s_stCtx.timing.stateTickMs = cb_hal_get_time_ms();
                s_stCtx.control.state = EN_APP_CALIB_STATE_WAIT_FINAL_RX;
            } else if (cb_hal_is_time_elapsed_ms(s_stCtx.timing.txStartTickMs,
                                                  guard) == CB_PASS) {
                /* Guard expiry: tx_end, clear TxDone; if deferred TX missed the trigger, retry non-deferred. */
                cb_framework_uwb_disable_scheduled_trx(s_stRespTxSchedCfg);
                cb_framework_uwb_tx_end();
                s_stCtx.io.TxDone = 0u;
                s_stCtx.timing.txMissedEvent = s_stCtx.timing.txIsDelayed;
                s_stCtx.control.state = EN_APP_CALIB_STATE_SEND_RESP;
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_WAIT_FINAL_RX:
        {
            /* Waiting for CAL_FINAL on an always-listening RX. Also check CAL_END. */
            if (s_stCtx.io.stRxSlot.used) {
                if (s_stCtx.io.stRxSlot.payload[0] == TDOA_FRAME_TYPE_CAL_END) {
                    app_tdoa_calib_handle_end_frame();
                    break;
                }
                app_tdoa_calib_handle_final_rx();
            } else if (cb_hal_is_time_elapsed_ms(s_stCtx.timing.stateTickMs,
                                                 TDOA_BOOT_CALIB_RX_GUARD_MS) == CB_PASS) {
                /* No CAL_FINAL in time: reopen a fresh POLL RX window (WAIT_POLL_RX re-ACKs to resync). */
                s_stCtx.io.failureState = EN_APP_CALIB_STATE_WAIT_FINAL_RX;
                app_tdoa_calib_timeout_print();
                app_tdoa_calib_reset_link();
                cb_framework_uwb_enable_scheduled_trx(s_stRespTxSchedCfg);
                app_tdoa_calib_open_poll_rx();
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_WAIT_FINAL_DONE:
        {
            /* Transition state — handle_final_rx sets us to SEND_DATA directly. */
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_SEND_DATA:
        {
            app_tdoa_calib_send_data();
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_WAIT_DATA_TX:
        {
            if (s_stCtx.io.TxDone) {
                s_stCtx.io.TxDone = 0u;
                cb_framework_uwb_tx_end();
                /* One sample delivered. Re-arm RESP TX schedule, reopen POLL RX. */
                cb_framework_uwb_enable_scheduled_trx(s_stRespTxSchedCfg);
                app_tdoa_calib_open_poll_rx();
            } else if (cb_hal_is_time_elapsed_ms(s_stCtx.timing.txStartTickMs,
                                                  CALIB_TXDONE_GUARD_NODLY_MS) == CB_PASS) {
                /* Guard expiry: tx_end, clear TxDone, retry. */
                cb_framework_uwb_tx_end();
                s_stCtx.io.TxDone = 0u;
                s_stCtx.control.state = EN_APP_CALIB_STATE_SEND_DATA;
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_WAIT_END_RX:
        {
            /* Reached only if set explicitly from outside (currently unused; loop-back uses open_poll_rx). */
            if (s_stCtx.io.stRxSlot.used) {
                app_tdoa_calib_handle_end_frame();
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_END_ACK_WAIT_TX:
        {
            /* CAL_END_ACK on air (or guard elapsed): open NON_DEFERRED RX and linger for resent CAL_END. */
            if (s_stCtx.io.TxDone ||
                cb_hal_is_time_elapsed_ms(s_stCtx.timing.txStartTickMs,
                                          CALIB_TXDONE_GUARD_NODLY_MS) == CB_PASS) {
                s_stCtx.io.TxDone = 0u;
                cb_framework_uwb_tx_end();
                memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
                cb_framework_uwb_rx_end(EN_UWB_RX_0);
                cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig,
                                          &s_stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
                s_stCtx.timing.lingerTickMs = cb_hal_get_time_ms();
                s_stCtx.control.state = EN_APP_CALIB_STATE_END_ACK_LINGER;
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case EN_APP_CALIB_STATE_END_ACK_LINGER:
        {
            /* Re-ACK each resent CAL_END; resume TDOA after a quiet window with no further CAL_END. */
            if (s_stCtx.io.stRxSlot.used) {
                if (s_stCtx.io.stRxSlot.payload[0] == TDOA_FRAME_TYPE_CAL_END) {
                    app_tdoa_calib_reset_link();      /* consume frame, end RX */
                    app_tdoa_calib_send_end_ack();
                    s_stCtx.control.state = EN_APP_CALIB_STATE_END_ACK_WAIT_TX;
                } else {
                    memset(&s_stCtx.io.stRxSlot, 0, sizeof(s_stCtx.io.stRxSlot));
                }
            } else if (cb_hal_is_time_elapsed_ms(s_stCtx.timing.lingerTickMs,
                                                 TDOA_BOOT_CALIB_END_LINGER_MS) == CB_PASS) {
                app_tdoa_calib_reset_link();
                s_stCtx.control.done   = 1u;
                s_stCtx.control.active = 0u;
            }
            break;
        }

        case EN_APP_CALIB_STATE_IDLE:
        default:
            break;
    }
}

/**
 * @brief Return non-zero while the calibration state machine owns the radio.
 */
uint8_t app_tdoa_calib_is_active(void)
{
    return s_stCtx.control.active;
}

/**
 * @brief Return non-zero when calibration completed (CAL_END received or watchdog exit).
 */
uint8_t app_tdoa_calib_is_done(void)
{
    return s_stCtx.control.done;
}

/**
 * @brief UWB TX-done IRQ.  Sets the TxDone flag in the context.
 */
void app_tdoa_calib_tx_done_irqcb(void)
{
    s_stCtx.io.TxDone = 1u;
}

/**
 * @brief UWB RX0-done IRQ.  Captures the frame snapshot into the RX mailbox.
 *
 * Frame validation is performed by the process() state machine.  Frames
 * arriving when the mailbox is already occupied are silently dropped.
 */
void app_tdoa_calib_rx_done_irqcb(void)
{
    cb_uwbsystem_rxstatus_un rx_status;
    uint16_t pkt_size;

    if (s_stCtx.control.active == 0u) {
        return;
    }

    rx_status = cb_framework_uwb_get_rx_status();
    if (!rx_status.rx0_ok || s_stCtx.io.stRxSlot.used != 0u) {
        return;
    }

    pkt_size = cb_framework_uwb_get_rx_packet_size(&s_stUwbPacketConfig);
    if (pkt_size == 0u || pkt_size > TDOA_RX_MSG_MAX_LEN) {
        return;
    }

    cb_framework_uwb_get_rx_payload(s_stCtx.io.stRxSlot.payload, pkt_size);
    s_stCtx.io.stRxSlot.len        = (uint8_t)pkt_size;
    s_stCtx.io.stRxSlot.frame_type = s_stCtx.io.stRxSlot.payload[0];
    s_stCtx.io.stRxSlot.round_index = 0u;
    // cb_framework_uwb_get_rx_tsu_timestamp(&s_stCtx.io.stRxSlot.rx_tsu, EN_UWB_RX_0);
    cb_framework_uwb_get_rx_tsu_timestamp_lemm(&s_stCtx.io.stRxSlot.rx_tsu, EN_UWB_RX_0, 18, 0, 2);
    s_stCtx.io.stRxSlot.used = 1u;
}

/**
 * @brief Timer1 watchdog IRQ routed here when calibration is active.
 *
 * Records the failure state and raises the timeout flag.  The flag is
 * consumed by the next process() call, which aborts the sample and resumes
 * the TDOA loop via the done flag.
 */
void app_tdoa_calib_initiator_timer_irqcb(void)
{
    s_stCtx.io.failureState    = s_stCtx.control.state;
    s_stCtx.io.watchdogTimeout = 1u;
}
