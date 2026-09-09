/**
 * @file    TdoaMath.h
 * @brief   TDOA math helpers.
 * @details Shared timing, calibration filtering, and position-solving formulas used
 *          by the TDOA examples. Application files should call these helpers and
 *          keep protocol state handling separate from the math itself.
 * @author  Chipsbank
 * @date    2026
 */

#ifndef __TDOA_MATH_H
#define __TDOA_MATH_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include <math.h>
#include "tdoa_common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define TDOA_MATH_INVALID_POSITION       (-999.0f)
#define TDOA_MATH_MAX_DIST_DIFF_M        100.0f
#define TDOA_MATH_MIN_RANGE_M            1e-4f
#define TDOA_MATH_SINGULAR_EPSILON       1e-6f
#define TDOA_MATH_CHAN_RESIDUAL_LIMIT    0.15f
#define TDOA_MATH_TAYLOR_MAX_ITERATION   10u
#define TDOA_MATH_TAYLOR_DELTA_LIMIT_M   0.01f
#define TDOA_MATH_BOX_MARGIN_M           0.5f
#define TDOA_DIST_SMA_WINDOW             10u    /* Distance moving-average window size (samples). */
#define TDOA_CLK_RATIO_SMA_WINDOW        10u    /* Clock-ratio moving-average window size (samples). */

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
    float lastValidX;
    float lastValidY;
    uint8_t hasLastValid;
} tdoa_math_position_context_st;

typedef struct
{
    float x;
    float y;
    uint8_t taylorIterations;
} tdoa_math_position_result_st;

/* Per-channel sliding-average distance filter state (with spike rejection). */
typedef struct
{
    float   window[TDOA_DIST_SMA_WINDOW]; /* Ring buffer of recent in-range samples. */
    float   windowSum;                    /* Running sum of held samples (O(1) average). */
    uint8_t head;                         /* Next write index into window[]. */
    uint8_t count;                        /* Number of samples held (0..TDOA_DIST_SMA_WINDOW). */
    uint8_t valueValid;                   /* Filter has been seeded at least once. */
    uint8_t abnormalCnt;                  /* Consecutive out-of-range sample count. */
    uint8_t abnormalStreakActive;         /* An out-of-range streak is in progress. */
    float   lastAbnormalDist;             /* Last out-of-range sample (streak coherence check). */
} tdoa_dist_filter_state_st;

/* Clock-ratio sliding-average filter state (moving average with outlier rejection). */
typedef struct
{
    double  window[TDOA_CLK_RATIO_SMA_WINDOW]; /* Ring buffer of recent accepted ratio estimates. */
    double  windowSum;                         /* Running sum of held samples (O(1) average). */
    uint8_t head;                              /* Next write index into window[]. */
    uint8_t count;                             /* Number of samples held (0..TDOA_CLK_RATIO_SMA_WINDOW). */
} tdoa_ratio_filter_state_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static inline void tdoa_math_position_context_reset(tdoa_math_position_context_st* context);
static inline double tdoa_math_ticks_per_ns(void);
static inline double tdoa_math_cm_to_ticks(double distanceCm, double ticksPerNs);
static inline float tdoa_math_ticks_to_cm(double ticks, double ticksPerNs);
static inline double tdoa_math_initial_sum_tp_bias_ticks(double tPropNs,
                                                         double biasCm,
                                                         double ticksPerNs);
static inline uint8_t tdoa_math_calculate_responder_clock_ratio(cb_uwbsystem_tx_tsutimestamp_st initTxNew,
                                                                cb_uwbsystem_tx_tsutimestamp_st initTxOld,
                                                                cb_uwbsystem_rx_tsutimestamp_st respRxNew,
                                                                cb_uwbsystem_rx_tsutimestamp_st respRxOld,
                                                                double* outRatio);
static inline uint8_t tdoa_math_filter_responder_clock_ratio(tdoa_ratio_filter_state_st* state,
                                                             double candidateRatio,
                                                             double maxDiff,
                                                             double* filteredRatio);
static inline double tdoa_math_responder_turnaround_ticks(cb_uwbsystem_tx_tsutimestamp_st txTsu,
                                                          cb_uwbsystem_rx_tsutimestamp_st rxTsu);
static inline double tdoa_math_responder_slot_delay_ticks(uint8_t slot);
static inline void tdoa_math_calculate_responder_sync_tx(cb_uwbsystem_tx_tsutimestamp_st initTxReference,
                                                         double responderTurnaroundTicks,
                                                         double clockRatio,
                                                         double sumTpBiasTicks,
                                                         cb_uwbsystem_tx_tsutimestamp_st* syncOut);
void tdoa_math_sort_float_ascending(float* values, uint16_t count);
uint8_t tdoa_math_trimmed_mean(float* values,
                               uint16_t count,
                               uint8_t trimLowPercent,
                               uint8_t trimHighPercent,
                               float* meanOut);
uint8_t tdoa_math_calculate_position(tdoa_math_position_context_st* context,
                                     const tdoa_runtime_config_t* config,
                                     const float distDiff[3],
                                     tdoa_math_position_result_st* result);
static inline double tdoa_math_tag_ticks_to_dist_diff_m(double tdoaTicks);
static inline double tdoa_math_tag_calculate_tdoa_ticks(cb_uwbsystem_rx_tsutimestamp_st tagRxResp,
                                                        cb_uwbsystem_rx_tsutimestamp_st tagRxPoll,
                                                        cb_uwbsystem_tx_tsutimestamp_st respTxSync,
                                                        cb_uwbsystem_tx_tsutimestamp_st initTxPrev,
                                                        double clockRatio);
static inline uint8_t tdoa_math_tag_calculate_clock_ratio(cb_uwbsystem_tx_tsutimestamp_st initTxNew,
                                                          cb_uwbsystem_tx_tsutimestamp_st initTxOld,
                                                          cb_uwbsystem_rx_tsutimestamp_st tagRxPollNew,
                                                          cb_uwbsystem_rx_tsutimestamp_st tagRxPollOld,
                                                          double* outRatio);
void tdoa_math_tag_update_distance_filter(tdoa_dist_filter_state_st* state,
                                          float newDist,
                                          uint8_t ratioValid,
                                          float* filteredDist,
                                          float jumpThresholdM,
                                          uint8_t abnormalMaxRetry);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
static inline void tdoa_math_position_context_reset(tdoa_math_position_context_st* context)
{
    if (context == 0) {
        return;
    }

    context->lastValidX = 0.0f;
    context->lastValidY = 0.0f;
    context->hasLastValid = 0u;
}

static inline double tdoa_math_ticks_per_ns(void)
{
    return (double)TDOA_TSU_FREQ_HZ * (double)TDOA_TSU_FRAC_SCALE / 1e9;
}

static inline double tdoa_math_cm_to_ticks(double distanceCm, double ticksPerNs)
{
    return (distanceCm / TDOA_SPEED_OF_LIGHT_CM_NS) * ticksPerNs;
}

static inline float tdoa_math_ticks_to_cm(double ticks, double ticksPerNs)
{
    if (ticksPerNs == 0.0) {
        return 0.0f;
    }

    return (float)((ticks / ticksPerNs) * TDOA_SPEED_OF_LIGHT_CM_NS);
}

static inline double tdoa_math_initial_sum_tp_bias_ticks(double tPropNs,
                                                         double biasCm,
                                                         double ticksPerNs)
{
    return tPropNs * ticksPerNs + tdoa_math_cm_to_ticks(biasCm, ticksPerNs);
}

static inline uint8_t tdoa_math_calculate_responder_clock_ratio(cb_uwbsystem_tx_tsutimestamp_st initTxNew,
                                                                cb_uwbsystem_tx_tsutimestamp_st initTxOld,
                                                                cb_uwbsystem_rx_tsutimestamp_st respRxNew,
                                                                cb_uwbsystem_rx_tsutimestamp_st respRxOld,
                                                                double* outRatio)
{
    int64_t deltaTx;
    int64_t deltaRx;

    if (outRatio == 0) {
        return 0u;
    }

    deltaTx = tdoa_tsu_raw_diff(tdoa_tx_tsu_to_raw(initTxNew),
                                tdoa_tx_tsu_to_raw(initTxOld));
    deltaRx = tdoa_tsu_raw_diff(tdoa_rx_tsu_to_raw(respRxNew),
                                tdoa_rx_tsu_to_raw(respRxOld));

    if (deltaRx == 0 || deltaTx <= 0) {
        return 0u;
    }

    *outRatio = (double)deltaTx / (double)deltaRx;
    return 1u;
}

static inline uint8_t tdoa_math_filter_responder_clock_ratio(tdoa_ratio_filter_state_st* state,
                                                             double candidateRatio,
                                                             double maxDiff,
                                                             double* filteredRatio)
{
    if (state == 0 || filteredRatio == 0) {
        return 0u;
    }
    if (state->count != 0u) {
        double currentAvg = state->windowSum / (double)state->count;
        if (FLOAT_ABS_DIFF(candidateRatio, currentAvg) > maxDiff) {
            return 0u;
        }
    }
    if (state->count >= TDOA_CLK_RATIO_SMA_WINDOW) {
        state->windowSum -= state->window[state->head];
    } else {
        state->count++;
    }
    state->window[state->head] = candidateRatio;
    state->windowSum          += candidateRatio;
    state->head                = (uint8_t)((state->head + 1u) % TDOA_CLK_RATIO_SMA_WINDOW);

    *filteredRatio = state->windowSum / (double)state->count;
    return 1u;
}

static inline double tdoa_math_responder_turnaround_ticks(cb_uwbsystem_tx_tsutimestamp_st txTsu,
                                                          cb_uwbsystem_rx_tsutimestamp_st rxTsu)
{
    return (double)tdoa_tsu_raw_diff(tdoa_tx_tsu_to_raw(txTsu),
                                     tdoa_rx_tsu_to_raw(rxTsu));
}

static inline double tdoa_math_responder_slot_delay_ticks(uint8_t slot)
{
    return (double)slot * (double)TDOA_TSU_FREQ_HZ *
           (double)TDOA_TSU_FRAC_SCALE / 1000.0;
}

static inline void tdoa_math_calculate_responder_sync_tx(cb_uwbsystem_tx_tsutimestamp_st initTxReference,
                                                         double responderTurnaroundTicks,
                                                         double clockRatio,
                                                         double sumTpBiasTicks,
                                                         cb_uwbsystem_tx_tsutimestamp_st* syncOut)
{
    uint64_t initTxRaw;
    uint64_t syncRawU;
    double syncRaw;

    if (syncOut == 0) {
        return;
    }

    initTxRaw = tdoa_tx_tsu_to_raw(initTxReference);
    syncRaw = (double)initTxRaw +
              responderTurnaroundTicks * clockRatio +
              sumTpBiasTicks;
    syncRawU = (uint64_t)syncRaw;

    syncOut->txTsuInt = (uint32_t)(syncRawU >> 9);
    syncOut->txTsuFrac = (uint16_t)(syncRawU & 0x1FF);
}

/**
 * @brief Convert a raw TDOA tick difference to a signed distance difference in metres.
 *
 * The TSU timestamp has a fractional part of TDOA_TSU_FRAC_SCALE bits, so the
 * physical tick period is (1 / TSU_FREQ) / FRAC_SCALE seconds. Multiplying by
 * the speed of light gives metres per raw tick.
 */
static inline double tdoa_math_tag_ticks_to_dist_diff_m(double tdoaTicks)
{
    return tdoaTicks * TDOA_M_PER_TSU_TICK / TDOA_TSU_FRAC_SCALE;
}

/**
 * @brief Compute the TDOA tick difference between one responder and the initiator.
 *
 * The tag observes:
 *   diff_rx = tag_RX(resp_k) - tag_RX(poll)   (tag local clock, raw ticks)
 *   diff_tx = resp_k TX_sync - init TX_prev    (initiator clock, raw ticks)
 *
 * Applying the tag clock ratio converts diff_rx to the initiator time base so
 * that the subtraction is coherent:
 *   tdoa_ticks = diff_rx * ratio - diff_tx
 *
 * A positive result means the tag is closer to the initiator than to responder k.
 */
static inline double tdoa_math_tag_calculate_tdoa_ticks(cb_uwbsystem_rx_tsutimestamp_st tagRxResp,
                                                        cb_uwbsystem_rx_tsutimestamp_st tagRxPoll,
                                                        cb_uwbsystem_tx_tsutimestamp_st respTxSync,
                                                        cb_uwbsystem_tx_tsutimestamp_st initTxPrev,
                                                        double clockRatio)
{
    int64_t diffRx = tdoa_tsu_raw_diff(tdoa_rx_tsu_to_raw(tagRxResp),
                                       tdoa_rx_tsu_to_raw(tagRxPoll));
    int64_t diffTx = tdoa_tsu_raw_diff(tdoa_tx_tsu_to_raw(respTxSync),
                                       tdoa_tx_tsu_to_raw(initTxPrev));
    return ((double)diffRx * clockRatio) - (double)diffTx;
}

/**
 * @brief Estimate the initiator/tag clock ratio from two consecutive Poll-DTM rounds.
 *
 * Uses the same cross-round method as the responder: the numerator is the
 * initiator TX interval carried inside Poll-DTM payloads, and the denominator
 * is the tag's local Poll-DTM RX interval.
 *
 *   ratio = delta(init TX) / delta(tag RX poll)
 *
 * Returns 1 on success, 0 if inputs are invalid or the delta is zero.
 */
static inline uint8_t tdoa_math_tag_calculate_clock_ratio(cb_uwbsystem_tx_tsutimestamp_st initTxNew,
                                                          cb_uwbsystem_tx_tsutimestamp_st initTxOld,
                                                          cb_uwbsystem_rx_tsutimestamp_st tagRxPollNew,
                                                          cb_uwbsystem_rx_tsutimestamp_st tagRxPollOld,
                                                          double* outRatio)
{
    int64_t deltaTx;
    int64_t deltaRx;

    if (outRatio == 0) {
        return 0u;
    }

    deltaTx = tdoa_tsu_raw_diff(tdoa_tx_tsu_to_raw(initTxNew),
                                tdoa_tx_tsu_to_raw(initTxOld));
    deltaRx = tdoa_tsu_raw_diff(tdoa_rx_tsu_to_raw(tagRxPollNew),
                                tdoa_rx_tsu_to_raw(tagRxPollOld));

    if (deltaRx == 0 || deltaTx <= 0) {
        return 0u;
    }

    *outRatio = (double)deltaTx / (double)deltaRx;
    return 1u;
}

#endif // __TDOA_MATH_H
