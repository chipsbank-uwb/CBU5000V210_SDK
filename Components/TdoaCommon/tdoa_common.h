/**
 * @file    tdoa_common.h
 * @brief   Shared TDOA definitions used by all role examples.
 * @details The initiator, responder, and tag example projects include this
 *          header for the common frame layouts, scalar constants, helper
 *          macros, and lightweight inline conversions on the SDK TSU timestamp
 *          types. On-air timestamps embed the SDK structs directly; the raw
 *          tick helpers expand them to 64-bit tick counts for the math layer.
 *
 *          Only protocol-level definitions and shared multi-statement helpers
 *          (watchdog timer init/off, lack-anchor print) live here. Role-specific
 *          state machines, timing math, and position-solving helpers stay in
 *          their own modules (see AppUwbTdoaResponder.c and TdoaMath.h).
 * @author  Chipsbank
 * @date    2026
 */

/* ===================================================================
 * Protocol overview (single source of truth for all three roles)
 * ===================================================================
 *   Initiator          Responder 1/2/3            Tag
 *     IDLE                 RX_WAIT              WAIT_POLL
 *       |                     |                     |
 *    1  |--- POLL_DTM ------->|-------------------->|   t = 0 ms
 *       |                  2  |--- RESP_DTM(1) ---->|   t +1 ms
 *       |                  2  |--- RESP_DTM(2) ---->|   t +2 ms
 *       |                  2  |--- RESP_DTM(3) ---->|   t +3 ms
 *       |<-------------- 3. TAG_REPORT -------------|   t +4 ms
 *    4  | Chan + Taylor solve |                     |
 *     Repeat               Repeat               Repeat   (next round, +10 ms)
 *
 *   1. POLL_DTM   round=N, previous-round initiator TX timestamp, tx_prev_valid
 *   2. RESP_DTM   id=1..3, tsu_tx_sync (clock-synchronized TX timestamp)
 *   3. TAG_REPORT round=N, dist_diff[3] (m), anchor_rx_mask
 *   4. Position solve runs on the initiator only (Chan + Taylor).
 *
 * Each role's own .c file documents only its slice of this exchange and its
 * local state machine; this block is the only full sequence diagram.
 */

#ifndef __TDOA_COMMON_H
#define __TDOA_COMMON_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include <math.h>
#include "CB_system_types.h"
#include "CB_timer.h"   /* enTimer, stTimerSetUp, cb_timer_* (also pulls in the IRQn enum + CMSIS NVIC). */
#include "CB_scr.h"     /* cb_scr_timerX_module_on/off. */

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define FLOAT_ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

/* ===================================================================
 * History buffer macros
 * =================================================================== */

#define TDOA_HISTORY_SIZE 3

/**
 * @brief Resolve the circular history index for a round offset from current.
 * @param curr_idx Current write index.
 * @param offset   0 = current round, 1 = previous round, 2 = two rounds ago.
 * @return Array index into a TDOA_HISTORY_SIZE-deep buffer.
 */
#define TDOA_HISTORY_GET(curr_idx, offset) \
    (((curr_idx) + TDOA_HISTORY_SIZE - ((offset) >= TDOA_HISTORY_SIZE ? TDOA_HISTORY_SIZE - 1 : (offset))) % TDOA_HISTORY_SIZE)

/**
 * @brief Advance the circular history write index to the next round slot.
 * @param curr_idx Current write index.
 * @return Next array index into a TDOA_HISTORY_SIZE-deep buffer.
 */
#define TDOA_HISTORY_ADVANCE(curr_idx) \
    (((curr_idx) + 1) % TDOA_HISTORY_SIZE)

/* ---- Physical constants ---- */
#define TDOA_SPEED_OF_LIGHT_M_S     299792458u     /* Speed of light (m/s). */
#define TDOA_SPEED_OF_LIGHT_CM_NS   29.97924580    /* Speed of light (cm/ns). */
#define TDOA_TSU_FREQ_HZ            124800000u     /* TSU integer-part frequency (Hz) = 124.8 MHz. */
#define TDOA_TSU_FRAC_SCALE         512u           /* TSU fractional-part denominator (9 valid bits). */
#define TDOA_M_PER_TSU_TICK         (TDOA_SPEED_OF_LIGHT_M_S / TDOA_TSU_FREQ_HZ)  /* ~2.4022 m per raw TSU integer tick. */
#define TDOA_CLK_RATIO_MAX_DELTA    5e-8           /* Max allowed clock ratio change per round before the filter snaps to the new value. */

/* ---- System sizing constants ---- */
#define TDOA_MAX_ANCHORS            4               /* 1 initiator + 3 responders. */
#define TDOA_MAX_RESPONDERS         3               /* Number of responder slots. */
#define TDOA_POLL_PERIOD_MS         10              /* Round period (ms). */
#define TDOA_ABNORMAL_MAX_RETRY     10              /* Max consecutive outliers before the filter snaps to the new value. */

/* ---- Hardware watchdog / TX-done guard timeouts (ms) ---- */
#define TDOA_WATCHDOG_TIMEOUT_MS        20u         /* Per-round one-shot watchdog for TDOA roles (Timer0/Timer1). */
#define TDOA_CALIB_WATCHDOG_TIMEOUT_MS  50u         /* Per-sample one-shot watchdog for boot-calibration roles. */
#define TDOA_TXDONE_GUARD_MS            2u          /* TX-done software-guard margin added after the scheduled TX instant. */

/* ---- Frame type identifiers ---- */
#define TDOA_FRAME_TYPE_POLL_DTM    0x01
#define TDOA_FRAME_TYPE_RESP_DTM    0x02
#define TDOA_FRAME_TYPE_TAG_REPORT  0x03
#define TDOA_FRAME_TYPE_CAL_REQ     0x11
#define TDOA_FRAME_TYPE_CAL_ACK     0x12
#define TDOA_FRAME_TYPE_CAL_POLL    0x13
#define TDOA_FRAME_TYPE_CAL_RESP    0x14
#define TDOA_FRAME_TYPE_CAL_FINAL   0x15
#define TDOA_FRAME_TYPE_CAL_DATA    0x16
#define TDOA_FRAME_TYPE_CAL_END     0x17
#define TDOA_FRAME_TYPE_CAL_END_ACK 0x18    /* Initiator -> Responder: acknowledges CAL_END. */

/* ---- Anchor RX-mask bit layout (Tag REPORT anchor_rx_mask field) ---- */
#define TDOA_ANCHOR_RX_MASK_INITIATOR        (1u << 0)   /* bit0 = initiator / POLL received. */
#define TDOA_ANCHOR_RX_MASK_RESP(id)        (1u << (id))   /* id 1..3: bit1..3 = responder 1..3 received. */
#define TDOA_ANCHOR_RX_MASK_ALL              0x0Fu       /* All four anchors present. */

/* ---- Tag round drop-reason codes (REPORT drop_reason field) ----
 * Shared by the tag (writer) and the initiator (reader) so both agree on the
 * code values. drop_reason == DEF_TDOA_DROP_NONE means the report carries a
 * valid TDOA result; any other value means a debug-only report. */
#define DEF_TDOA_DROP_NONE            0u
#define DEF_TDOA_DROP_TIMEOUT         1u
#define DEF_TDOA_DROP_ROUND_MISMATCH  2u
#define DEF_TDOA_DROP_SYNC_INVALID    3u
#define DEF_TDOA_DROP_OVERRUN         4u

/* ---- Default calibration constants ---- */
#define TDOA_DEFAULT_T_PROP_NS      6.667f          /* Default propagation delay (ns). */
#define TDOA_DEFAULT_BIAS_CM        0               /* Default distance bias (cm). */

/* ---- Default device IDs ---- */
#define TDOA_DEFAULT_INITIATOR_ID   0
#define TDOA_DEFAULT_RESPONDER1_ID  1
#define TDOA_DEFAULT_RESPONDER2_ID  2
#define TDOA_DEFAULT_RESPONDER3_ID  3

/* ---- Boot calibration constants ---- */
#define TDOA_BOOT_CALIB_DURATION_MS           3000u     /* Fixed sampling window, started on the first CAL_ACK (tick-timed). */
#define TDOA_BOOT_CALIB_TREPLY_US             700u
#define TDOA_BOOT_CALIB_FINAL_DELAY_US        500u
#define TDOA_BOOT_CALIB_SAMPLE_GAP_MS         10u       /* Idle gap between consecutive DS-TWR samples. */
#define TDOA_BOOT_CALIB_MAX_SAMPLES           128u
#define TDOA_BOOT_CALIB_SAMPLE_INDEX_MASK     (TDOA_BOOT_CALIB_MAX_SAMPLES - 1u)
#define TDOA_BOOT_CALIB_MIN_SAMPLES           8u
#define TDOA_BOOT_CALIB_TRIM_LOW_PERCENT      5u
#define TDOA_BOOT_CALIB_TRIM_HIGH_PERCENT     20u
#define TDOA_BOOT_CALIB_RX_GUARD_MS           5u        /* Per-state RX-wait tick guard (driver side fast retry). */
#define TDOA_BOOT_CALIB_END_RESEND_MS         10u       /* Responder CAL_END resend period until END_ACK. */
#define TDOA_BOOT_CALIB_END_MAX_MS            1000u     /* Cap on the CAL_END/END_ACK handshake, avoids hang. */
#define TDOA_BOOT_CALIB_END_LINGER_MS         30u       /* Initiator quiet window before resuming TDOA (> resend period). */

/* ---- Boot calibration status codes ---- */
#define TDOA_BOOT_CALIB_STATUS_IDLE           0u
#define TDOA_BOOT_CALIB_STATUS_ACTIVE         1u
#define TDOA_BOOT_CALIB_STATUS_DONE           2u
#define TDOA_BOOT_CALIB_STATUS_FAIL           3u

/* ===================================================================
 * Responder role defaults
 * =================================================================== */

#define APP_TDOA_RESPONDER1_SLOT                 1u
#define APP_TDOA_RESPONDER2_SLOT                 2u
#define APP_TDOA_RESPONDER3_SLOT                 3u

/* ===================================================================
 * Basic on-air types
 * =================================================================== */

/**
 * @brief IRQ-to-main-loop RX snapshot buffer.
 * @details The RX0-done callback fills one of these entries; the application
 *          state machine consumes the payload and timestamp in a bounded step.
 */
#define TDOA_RX_MSG_MAX_LEN  64

typedef struct {
    uint8_t  payload[TDOA_RX_MSG_MAX_LEN];
    uint8_t  len;
    uint8_t  frame_type;
    uint32_t round_index;
    uint8_t  used;      /* 0 = empty, 1 = snapshot is valid. */
    cb_uwbsystem_rx_tsutimestamp_st rx_tsu;  /* SDK RX TSU timestamp captured for this frame. */
} tdoa_rx_msg_slot_t;

/* ===================================================================
 * Protocol payload layouts
 * =================================================================== */

/**
 * @brief Poll-DTM broadcast payload (15 bytes).
 * @details Carries only the previous-round (m-1) initiator TX timestamp, which
 *          is all the responders and tag need to close the cross-round clock
 *          ratio. Per-responder RX feedback was removed when the real-time
 *          tp-bias computation was dropped (bias now comes solely from boot
 *          calibration).
 * @note The embedded SDK timestamp is aligned(4) but lands at packed offset 6
 *       (non-4-aligned). Copy it whole (x = poll->tsu_tx_init_prev); never
 *       dereference its fields directly from a packed buffer (D1).
 */
typedef struct __attribute__((packed)) {
    uint8_t                         frame_type;         /* 0x01 */
    uint8_t                         device_id;          /* Initiator ID. */
    uint32_t                        round_index;        /* Current round index. */
    cb_uwbsystem_tx_tsutimestamp_st tsu_tx_init_prev;   /* Previous-round initiator TX timestamp (8 bytes). */
    uint8_t                         tx_prev_valid;      /* 1 = tsu_tx_init_prev is valid, 0 = first round / unknown. */
} tdoa_poll_payload_t;

/**
 * @brief Response-DTM broadcast payload (16 bytes).
 * @details Each responder replies in its own scheduled slot with a
 *          synchronized TX timestamp expressed on the initiator time base.
 * @note tsu_tx_sync is an SDK aligned(4) struct landing at packed offset 6.
 *       Copy it whole; never dereference its fields directly (D1).
 */
typedef struct __attribute__((packed)) {
    uint8_t                         frame_type;     /* 0x02 */
    uint8_t                         device_id;      /* Responder ID (1-3). */
    uint32_t                        round_index;    /* Matching round index. */
    cb_uwbsystem_tx_tsutimestamp_st tsu_tx_sync;    /* Synchronized TX timestamp, initiator time base (8 bytes). */
    uint8_t                         sync_valid;     /* 0 = synchronization not ready, 1 = valid. */
    uint8_t                         reserved;       /* Padding reserved for future use. */
} tdoa_resp_payload_t;

/**
 * @brief Tag Report payload (22 bytes).
 * @details Broadcast by the tag once per round (POLL + TDOA_MAX_ANCHORS ms),
 *          even when the round was incomplete. When drop_reason == DEF_TDOA_DROP_NONE
 *          the three distance differences are valid and feed the initiator's
 *          position solver. When drop_reason != DEF_TDOA_DROP_NONE the report is
 *          debug-only: tdoa_dist_diff is meaningless and the initiator prints the
 *          drop diagnostics (reason, overrun, missing anchors) instead of solving.
 *          anchor_rx_mask reports which anchors the tag actually heard this round.
 */
typedef struct __attribute__((packed)) {
    uint8_t         frame_type;         /* 0x03 */
    uint8_t         tag_id;             /* Tag ID. */
    uint32_t        round_index;        /* Matching round index. */
    float           tdoa_dist_diff[3];  /* Distance differences to responders 1, 2, 3 (metres). */
    uint8_t         anchor_rx_mask;     /* bit0 = initiator/POLL, bit1..3 = responder 1..3. */
    uint8_t         drop_reason;        /* DEF_TDOA_DROP_*; NONE = valid result, else debug-only. */
    uint16_t        overrun;            /* RX overrun counter snapshot (diagnostics). */
} tdoa_report_payload_t;

/**
 * @brief Boot calibration request frame.
 * @details Sent by a responder to ask the initiator to enter calibration mode.
 */
typedef struct __attribute__((packed)) {
    uint8_t frame_type;
    uint8_t responder_id;
    uint16_t seq;
    uint8_t reserved;
} tdoa_calib_req_payload_t;

/**
 * @brief Boot calibration control frame.
 * @details Used for the CAL_ACK / CAL_POLL / CAL_RESP / CAL_FINAL exchanges.
 */
typedef struct __attribute__((packed)) {
    uint8_t frame_type;
    uint8_t initiator_id;
    uint8_t responder_id;
    uint16_t seq;
    uint8_t status;
} tdoa_calib_ctrl_payload_t;

/**
 * @brief Boot calibration data frame (DS-TWR direction reversed: 25 bytes).
 * @details Sent by the TDOA initiator (which acts as the DS-TWR *responder*)
 *          after CAL_FINAL. It forwards that side's own tround/treply and bias
 *          so the TDOA responder (the DS-TWR initiator/driver) can locally
 *          close cb_framework_uwb_calculate_distance(). Field names reflect the
 *          DS-TWR-responder-side semantic of the values they carry.
 */
typedef struct __attribute__((packed)) {
    uint8_t                                frame_type;            /* 0x15 */
    uint8_t                                initiator_id;          /* TDOA initiator ID (DS-TWR responder side). */
    uint8_t                                responder_id;          /* TDOA responder ID (DS-TWR initiator/driver side). */
    uint16_t                               seq;                   /* Sample sequence number. */
    cb_uwbsystem_rangingtroundtreply_st    twr_responder_tround_treply;  /* DS-TWR responder-side (TDOA initiator) tround/treply. */
    int32_t                                twr_responder_bias_cm;        /* DS-TWR responder-side (TDOA initiator) bias (cm). */
} tdoa_calib_data_payload_t;

/**
 * @brief Boot calibration end frame (10 bytes).
 * @details Sent by the TDOA responder once the sampling window closes. It
 *          carries the final trimmed-mean result (locked_sum_tp_bias_cm) so the
 *          TDOA initiator can print it. Single-shot, no retry: if lost, the
 *          initiator falls back to its 50ms watchdog and both sides converge.
 */
typedef struct __attribute__((packed)) {
    uint8_t frame_type;             /* 0x17 */
    uint8_t initiator_id;
    uint8_t responder_id;
    uint16_t seq;
    uint8_t status;                 /* TDOA_BOOT_CALIB_STATUS_* */
    float   locked_sum_tp_bias_cm;  /* Final locked sum-tp-bias result (cm). */
} tdoa_calib_end_payload_t;

/**
 * @brief Ring buffer of recent calibration distance samples.
 */
typedef struct {
    float    values[TDOA_BOOT_CALIB_MAX_SAMPLES];
    uint16_t write_idx;    /* Next write position (bit-masked wrap). */
    uint16_t valid_count;  /* Number of valid samples, capped at TDOA_BOOT_CALIB_MAX_SAMPLES. */
} tdoa_calib_sample_buffer_t;

/* ===================================================================
 * Runtime configuration structure
 * =================================================================== */

typedef struct {
    /* Identity. */
    uint8_t  device_id;         /* Local device ID. */
    uint8_t  slot;              /* Transmit slot index (1-3 for responders). */

    /* Calibration constants. */
    double   t_prop;            /* Propagation delay (ns). */
    double   bias;              /* Distance bias (cm). */
    uint8_t  calib_source;      /* 0 = DEFAULT, 1 = NVM, 2 = CAL. */

    /* Anchor layout (metres). */
    float    anchor_x[TDOA_MAX_ANCHORS];
    float    anchor_y[TDOA_MAX_ANCHORS];
    uint8_t  anchor_count;      /* Number of configured anchors. */
} tdoa_runtime_config_t;

/* ===================================================================
 * Compile-time size checks (C99/C11/C++ compatible)
 * =================================================================== */

#if defined(__cplusplus)
#define TDOA_STATIC_ASSERT(cond, name) static_assert((cond), #name)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define TDOA_STATIC_ASSERT(cond, name) _Static_assert((cond), #name)
#else
#define TDOA_STATIC_ASSERT(cond, name) typedef char name[(cond) ? 1 : -1]
#endif

TDOA_STATIC_ASSERT(sizeof(tdoa_poll_payload_t) == 15, TDOA_ASSERT_poll_payload_size);
TDOA_STATIC_ASSERT(sizeof(tdoa_resp_payload_t) == 16, TDOA_ASSERT_resp_payload_size);
TDOA_STATIC_ASSERT(sizeof(tdoa_report_payload_t) == 22, TDOA_ASSERT_report_payload_size);
TDOA_STATIC_ASSERT(sizeof(tdoa_calib_req_payload_t) == 5, TDOA_ASSERT_calib_req_payload_size);
TDOA_STATIC_ASSERT(sizeof(tdoa_calib_ctrl_payload_t) == 6, TDOA_ASSERT_calib_ctrl_payload_size);
TDOA_STATIC_ASSERT(sizeof(tdoa_calib_data_payload_t) == 25, TDOA_ASSERT_calib_data_payload_size);
TDOA_STATIC_ASSERT(sizeof(tdoa_calib_end_payload_t) == 10, TDOA_ASSERT_calib_end_payload_size);
TDOA_STATIC_ASSERT((TDOA_BOOT_CALIB_MAX_SAMPLES != 0u) &&
                   ((TDOA_BOOT_CALIB_MAX_SAMPLES & (TDOA_BOOT_CALIB_MAX_SAMPLES - 1u)) == 0u),
                   TDOA_ASSERT_boot_calib_samples_power_of_two);

static inline uint16_t tdoa_trim_count(uint16_t count, uint8_t percent)
{
    return (uint16_t)(((uint32_t)count * (uint32_t)percent) / 100u);
}

/* ===================================================================
 * Raw TSU tick converters
 * =================================================================== *
 * One raw tick = 1 / (TDOA_TSU_FREQ_HZ * TDOA_TSU_FRAC_SCALE). The 9-bit
 * fractional part is masked because only the low 9 bits of the SDK frac field
 * are meaningful (D1). Pass the SDK structs by value (whole-struct copy) so the
 * packed-buffer alignment caveat never applies inside these helpers. */

/**
 * @brief Expand an SDK TX TSU timestamp into a 64-bit raw tick count.
 * @return Tick count in units of 1/(512 * 124.8 MHz).
 */
static inline uint64_t tdoa_tx_tsu_to_raw(cb_uwbsystem_tx_tsutimestamp_st tsu)
{
    return ((uint64_t)tsu.txTsuInt << 9) | (uint64_t)(tsu.txTsuFrac & 0x1FF);
}

/**
 * @brief Expand an SDK RX TSU timestamp into a 64-bit raw tick count.
 * @return Tick count in units of 1/(512 * 124.8 MHz).
 */
static inline uint64_t tdoa_rx_tsu_to_raw(cb_uwbsystem_rx_tsutimestamp_st tsu)
{
    return ((uint64_t)tsu.rxTsuInt << 9) | (uint64_t)(tsu.rxTsuFrac & 0x1FF);
}

/**
 * @brief Signed difference between two raw tick counts (t2 - t1).
 * @return Delta in raw ticks.
 */
static inline int64_t tdoa_tsu_raw_diff(uint64_t t2_raw, uint64_t t1_raw)
{
    return (int64_t)t2_raw - (int64_t)t1_raw;
}

/* ===================================================================
 * ToF and distance helpers
 * =================================================================== */

/**
 * @brief Convert a DS-TWR round-trip tick count to distance in centimetres.
 * @param round_trip_ticks Round-trip delta expressed in raw TSU ticks.
 * @param t_prop_ns        Propagation delay correction (ns).
 * @return Distance (cm).
 */
static inline double tdoa_calc_distance_cm(int64_t round_trip_ticks,
                                            double t_prop_ns)
{
    /* One raw tick = 1/(124.8 MHz * 512) = 15.625 ps. */
    double round_trip_ns = (double)round_trip_ticks * 15.625e-3;
    double one_way_ns = (round_trip_ns - t_prop_ns) / 2.0;
    return one_way_ns * TDOA_SPEED_OF_LIGHT_CM_NS;
}

/* ===================================================================
 * Round-robin slot helper
 * =================================================================== */

/**
 * @brief Compute the nominal TX start tick for a given slot in a round.
 * @param round_index Current round counter.
 * @param slot_index  Slot number (1-3).
 * @return Slot start offset expressed in milliseconds from the round start.
 */
static inline uint32_t tdoa_slot_start_tick(uint32_t round_index, uint8_t slot_index)
{
    (void)round_index;  /* Currently unused; reserved for future scheduling policies. */
    return (uint32_t)(slot_index);  /* One slot per ms. */
}

/* ===================================================================
 * Generic helpers
 * =================================================================== */

/**
 * @brief Euclidean distance between two 2D points.
 */
static inline float tdoa_distance_2d(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

/**
 * @brief Convert distance (metres) to time-of-flight (nanoseconds).
 */
static inline double tdoa_distance_to_tof_ns(double dist_m)
{
    return (dist_m / TDOA_SPEED_OF_LIGHT_M_S) * 1e9;
}

/**
 * @brief Convert time-of-flight (nanoseconds) to distance (centimetres).
 */
static inline double tdoa_tof_ns_to_distance_cm(double tof_ns)
{
    return tof_ns * TDOA_SPEED_OF_LIGHT_CM_NS;
}

/* ===================================================================
 * Shared multi-statement helpers (moved here per req 9)
 * =================================================================== */

/**
 * @brief Arm a one-shot millisecond watchdog timer for the calling role.
 * @details Turns on the SCR module for the given timer, enables its NVIC IRQ,
 *          and configures EVENT0 as a one-shot timeout. The timer auto-starts
 *          on configure. Pass EN_TIMER_0 for Responder/Tag/Calib-Responder and
 *          EN_TIMER_1 for Initiator/Calib-Initiator.
 * @param timer      Timer instance (EN_TIMER_0 or EN_TIMER_1).
 * @param timeout_ms One-shot timeout in milliseconds.
 */
static inline void tdoa_watchdog_timer_init(enTimer timer, uint16_t timeout_ms)
{
    stTimerSetUp timer_cfg;

    if (timer == EN_TIMER_0)
    {
        cb_scr_timer0_module_on();
    }
    else
    {
        cb_scr_timer1_module_on();
    }

    cb_timer_irq_config(timer, EN_TIMER_TIMEOUT_EVENT_0, CB_FALSE);
    NVIC_EnableIRQ((timer == EN_TIMER_0) ? TIMER_0_IRQn : TIMER_1_IRQn);

    timer_cfg.Timer                        = timer;
    timer_cfg.TimerMode                    = EN_TIMER_MODE_ONE_SHOT;
    timer_cfg.TimeUnit                     = EN_TIMER_MS;
    timer_cfg.stTimeOut.timeoutVal[0]      = (uint32_t)timeout_ms;
    timer_cfg.stTimeOut.timeoutVal[1]      = 0u;
    timer_cfg.stTimeOut.timeoutVal[2]      = 0u;
    timer_cfg.stTimeOut.timeoutVal[3]      = 0u;
    timer_cfg.stTimeOut.TimerTimeoutEvent  = DEF_TIMER_TIMEOUT_EVENT_0;
    timer_cfg.TimerEvtComMode              = EN_TIMER_EVTCOM_MODE_00;
    timer_cfg.AutoStartTimer               = EN_START_TIMER_ENABLE;
    timer_cfg.TimerEvtComEnable            = EN_TIMER_EVTCOM_DISABLE;
    timer_cfg.TimerInterrupt               = EN_TIMER_INTERUPT_ENABLE;
    timer_cfg.stPulseWidth.NumberOfCycles  = 0u;
    timer_cfg.stPulseWidth.TimerPulseWidth = EN_TIMER_PULSEWIDTH_DISABLE;

    cb_timer_configure_timer(&timer_cfg);
}

/**
 * @brief Disarm the one-shot watchdog timer for the calling role.
 * @details Stops the timer, powers down its SCR module, disables its NVIC IRQ
 *          and clears any pending IRQ so a close/expire race cannot leak a
 *          stale timeout into the next round (P9).
 * @param timer Timer instance (EN_TIMER_0 or EN_TIMER_1).
 */
static inline void tdoa_watchdog_timer_off(enTimer timer)
{
    IRQn_Type irqn = (timer == EN_TIMER_0) ? TIMER_0_IRQn : TIMER_1_IRQn;

    cb_timer_disable_timer(timer);

    if (timer == EN_TIMER_0)
    {
        cb_scr_timer0_module_off();
    }
    else
    {
        cb_scr_timer1_module_off();
    }

    NVIC_DisableIRQ(irqn);
    NVIC_ClearPendingIRQ(irqn);
}

/**
 * @brief Print "lack anchor x" for every anchor missing from anchor_rx_mask.
 * @details bit0 = anchor 0 (initiator/POLL), bit1..3 = responders 1..3.
 * @param anchor_rx_mask Bitmask of anchors actually received this round.
 * @param print_fn       printf-style sink (e.g. app_uart_printf).
 */
static inline void tdoa_print_lack_anchors(uint8_t anchor_rx_mask,
                                           void (*print_fn)(const char* fmt, ...))
{
    uint8_t id;

    if (print_fn == 0)
    {
        return;
    }

    for (id = 0u; id < TDOA_MAX_ANCHORS; id++)
    {
        if ((anchor_rx_mask & (uint8_t)(1u << id)) == 0u)
        {
            print_fn("lack anchor%u,", (unsigned)id);
        }
    }
}

/**
 * @brief Map a DEF_TDOA_DROP_* code to its human-readable name.
 * @details Shared by the tag (local debug print) and the initiator (mirrored
 *          debug print) so both render identical reason strings.
 * @param reason DEF_TDOA_DROP_* code.
 * @return Static string for the reason (never NULL).
 */
static inline const char* tdoa_drop_reason_to_str(uint8_t reason)
{
    switch (reason)
    {
    case DEF_TDOA_DROP_TIMEOUT:        return "TIMEOUT";
    case DEF_TDOA_DROP_ROUND_MISMATCH: return "ROUND_MISMATCH";
    case DEF_TDOA_DROP_SYNC_INVALID:   return "SYNC_INVALID";
    case DEF_TDOA_DROP_OVERRUN:        return "OVERRUN";
    default:                           return "NONE";
    }
}

#endif /* __TDOA_COMMON_H */
