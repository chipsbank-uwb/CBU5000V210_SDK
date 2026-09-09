/**
 * @file    TdoaMath.c
 * @brief   Non-inline TDOA math helper implementations.
 * @author  Chipsbank
 * @date    2026
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "TdoaMath.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static int tdoa_math_solve_chan_3x3(float Ga[3][3], float h[3], float out[3]);
static int tdoa_math_solve_taylor_3x2(float J[3][2], float f[3], float delta[2]);
static uint8_t tdoa_math_is_last_position_valid(const tdoa_math_position_context_st* context,
                                                float xMin,
                                                float xMax,
                                                float yMin,
                                                float yMax);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
static int tdoa_math_solve_chan_3x3(float Ga[3][3], float h[3], float out[3])
{
    float c00 = Ga[1][1] * Ga[2][2] - Ga[1][2] * Ga[2][1];
    float c01 = Ga[1][0] * Ga[2][2] - Ga[1][2] * Ga[2][0];
    float c02 = Ga[1][0] * Ga[2][1] - Ga[1][1] * Ga[2][0];
    float c10 = Ga[0][1] * Ga[2][2] - Ga[0][2] * Ga[2][1];
    float c11 = Ga[0][0] * Ga[2][2] - Ga[0][2] * Ga[2][0];
    float c12 = Ga[0][0] * Ga[2][1] - Ga[0][1] * Ga[2][0];
    float c20 = Ga[0][1] * Ga[1][2] - Ga[0][2] * Ga[1][1];
    float c21 = Ga[0][0] * Ga[1][2] - Ga[0][2] * Ga[1][0];
    float c22 = Ga[0][0] * Ga[1][1] - Ga[0][1] * Ga[1][0];
    float det = Ga[0][0] * c00 - Ga[0][1] * c01 + Ga[0][2] * c02;
    float invDet;
    float inv[3][3];

    if (FLOAT_ABS_DIFF(det, 0.0f) < TDOA_MATH_SINGULAR_EPSILON) {
        return -1;
    }

    invDet = 1.0f / det;
    inv[0][0] =  c00 * invDet;
    inv[0][1] = -c10 * invDet;
    inv[0][2] =  c20 * invDet;
    inv[1][0] = -c01 * invDet;
    inv[1][1] =  c11 * invDet;
    inv[1][2] = -c21 * invDet;
    inv[2][0] =  c02 * invDet;
    inv[2][1] = -c12 * invDet;
    inv[2][2] =  c22 * invDet;

    out[0] = inv[0][0] * h[0] + inv[0][1] * h[1] + inv[0][2] * h[2];
    out[1] = inv[1][0] * h[0] + inv[1][1] * h[1] + inv[1][2] * h[2];
    out[2] = inv[2][0] * h[0] + inv[2][1] * h[1] + inv[2][2] * h[2];

    return 0;
}

static int tdoa_math_solve_taylor_3x2(float J[3][2], float f[3], float delta[2])
{
    float JTJ[2][2] = {0};
    float JTf[2] = {0};
    float det;
    float invDet;

    for (uint8_t i = 0; i < 2u; i++) {
        for (uint8_t j = 0; j < 2u; j++) {
            JTJ[i][j] = J[0][i] * J[0][j] + J[1][i] * J[1][j] + J[2][i] * J[2][j];
        }
    }

    for (uint8_t i = 0; i < 2u; i++) {
        JTf[i] = J[0][i] * f[0] + J[1][i] * f[1] + J[2][i] * f[2];
    }

    det = JTJ[0][0] * JTJ[1][1] - JTJ[0][1] * JTJ[1][0];
    if (FLOAT_ABS_DIFF(det, 0.0f) < TDOA_MATH_SINGULAR_EPSILON) {
        return -1;
    }

    invDet = 1.0f / det;
    delta[0] = ( JTJ[1][1] * JTf[0] - JTJ[0][1] * JTf[1]) * invDet;
    delta[1] = (-JTJ[1][0] * JTf[0] + JTJ[0][0] * JTf[1]) * invDet;

    return 0;
}

static uint8_t tdoa_math_is_last_position_valid(const tdoa_math_position_context_st* context,
                                                float xMin,
                                                float xMax,
                                                float yMin,
                                                float yMax)
{
    if (context == 0 || context->hasLastValid == 0u) {
        return 0u;
    }

    if (context->lastValidX == TDOA_MATH_INVALID_POSITION) {
        return 0u;
    }

    return (context->lastValidX >= xMin - TDOA_MATH_BOX_MARGIN_M &&
            context->lastValidX <= xMax + TDOA_MATH_BOX_MARGIN_M &&
            context->lastValidY >= yMin - TDOA_MATH_BOX_MARGIN_M &&
            context->lastValidY <= yMax + TDOA_MATH_BOX_MARGIN_M) ? 1u : 0u;
}

void tdoa_math_sort_float_ascending(float* values, uint16_t count)
{
    uint16_t i;

    if (values == 0 || count == 0u) {
        return;
    }

    for (i = 1u; i < count; i++) {
        float key = values[i];
        int16_t j = (int16_t)i - 1;

        while (j >= 0 && values[j] > key) {
            values[j + 1] = values[j];
            j--;
        }
        values[j + 1] = key;
    }
}

uint8_t tdoa_math_trimmed_mean(float* values,
                               uint16_t count,
                               uint8_t trimLowPercent,
                               uint8_t trimHighPercent,
                               float* meanOut)
{
    uint16_t trimLow;
    uint16_t trimHigh;
    uint16_t startIdx;
    uint16_t endIdx;
    uint16_t i;
    float sum = 0.0f;

    if (values == 0 || meanOut == 0 || count == 0u) {
        return 0u;
    }

    tdoa_math_sort_float_ascending(values, count);

    trimLow = tdoa_trim_count(count, trimLowPercent);
    trimHigh = tdoa_trim_count(count, trimHighPercent);
    startIdx = trimLow;
    endIdx = (uint16_t)(count - trimHigh);

    if (endIdx <= startIdx) {
        return 0u;
    }

    for (i = startIdx; i < endIdx; i++) {
        sum += values[i];
    }

    *meanOut = sum / (float)(endIdx - startIdx);
    return 1u;
}

uint8_t tdoa_math_calculate_position(tdoa_math_position_context_st* context,
                                     const tdoa_runtime_config_t* config,
                                     const float distDiff[3],
                                     tdoa_math_position_result_st* result)
{
    float ax[4];
    float ay[4];
    float K[4];
    float xMin = 999.0f;
    float xMax = -999.0f;
    float yMin = 999.0f;
    float yMax = -999.0f;
    float x0;
    float y0;
    float K0;
    float Ga[3][3];
    float h[3];
    float za0[3] = {0};
    uint8_t chanOk = 0u;
    uint8_t lastIsValid;
    float xEst;
    float yEst;
    float f[3];
    float J[3][2];
    float delta[2];
    float lastFNorm = 999.0f;
    uint8_t divergeCnt = 0u;
    float finalX;
    float finalY;
    uint8_t taylorIterations = 0u;

    if (context == 0 || config == 0 || distDiff == 0 || result == 0) {
        return 0u;
    }

    result->taylorIterations = 0u;

    if (FLOAT_ABS_DIFF(distDiff[0], 0.0f) > TDOA_MATH_MAX_DIST_DIFF_M ||
        FLOAT_ABS_DIFF(distDiff[1], 0.0f) > TDOA_MATH_MAX_DIST_DIFF_M ||
        FLOAT_ABS_DIFF(distDiff[2], 0.0f) > TDOA_MATH_MAX_DIST_DIFF_M) {
        return 0u;
    }

    for (uint8_t i = 0; i < 4u; i++) {
        ax[i] = config->anchor_x[i];
        ay[i] = config->anchor_y[i];
        K[i] = ax[i] * ax[i] + ay[i] * ay[i];
        if (ax[i] < xMin) { xMin = ax[i]; }
        if (ax[i] > xMax) { xMax = ax[i]; }
        if (ay[i] < yMin) { yMin = ay[i]; }
        if (ay[i] > yMax) { yMax = ay[i]; }
    }

    x0 = ax[0];
    y0 = ay[0];
    K0 = K[0];

    for (uint8_t i = 0; i < 3u; i++) {
        uint8_t k = (uint8_t)(i + 1u);
        float rk0 = distDiff[i];

        Ga[i][0] = ax[k] - x0;
        Ga[i][1] = ay[k] - y0;
        Ga[i][2] = rk0;
        h[i] = (K[k] - K0 - rk0 * rk0) / 2.0f;
    }

    if (tdoa_math_solve_chan_3x3(Ga, h, za0) == 0) {
        float r0Est = za0[2];
        float xChan = za0[0];
        float yChan = za0[1];

        if (r0Est > 0.0f &&
            xChan >= xMin - TDOA_MATH_BOX_MARGIN_M && xChan <= xMax + TDOA_MATH_BOX_MARGIN_M &&
            yChan >= yMin - TDOA_MATH_BOX_MARGIN_M && yChan <= yMax + TDOA_MATH_BOX_MARGIN_M) {
            float resNorm = 0.0f;

            for (uint8_t i = 0; i < 3u; i++) {
                float err = Ga[i][0] * za0[0] + Ga[i][1] * za0[1] + Ga[i][2] * za0[2] - h[i];
                resNorm += err * err;
            }

            if (sqrtf(resNorm) < TDOA_MATH_CHAN_RESIDUAL_LIMIT) {
                chanOk = 1u;
            }
        }
    }

    lastIsValid = tdoa_math_is_last_position_valid(context, xMin, xMax, yMin, yMax);
    if (chanOk != 0u) {
        xEst = za0[0];
        yEst = za0[1];
    } else if (lastIsValid != 0u) {
        xEst = context->lastValidX;
        yEst = context->lastValidY;
    } else {
        xEst = TDOA_MATH_INVALID_POSITION;
        yEst = TDOA_MATH_INVALID_POSITION;
    }

    finalX = xEst;
    finalY = yEst;

    for (uint8_t iter = 0; iter < TDOA_MATH_TAYLOR_MAX_ITERATION; iter++) {
        float r0Est = sqrtf((xEst - x0) * (xEst - x0) + (yEst - y0) * (yEst - y0));
        float fNorm = 0.0f;

        taylorIterations = (uint8_t)(iter + 1u);

        if (r0Est < TDOA_MATH_MIN_RANGE_M) {
            r0Est = TDOA_MATH_MIN_RANGE_M;
        }

        for (uint8_t i = 0; i < 3u; i++) {
            uint8_t k = (uint8_t)(i + 1u);
            float rkEst = sqrtf((xEst - ax[k]) * (xEst - ax[k]) +
                                (yEst - ay[k]) * (yEst - ay[k]));

            if (rkEst < TDOA_MATH_MIN_RANGE_M) {
                rkEst = TDOA_MATH_MIN_RANGE_M;
            }

            f[i] = distDiff[i] - (rkEst - r0Est);
            fNorm += f[i] * f[i];
            J[i][0] = (xEst - ax[k]) / rkEst - (xEst - x0) / r0Est;
            J[i][1] = (yEst - ay[k]) / rkEst - (yEst - y0) / r0Est;
        }

        fNorm = sqrtf(fNorm);
        if (iter > 0u && fNorm > lastFNorm) {
            divergeCnt++;
            if (divergeCnt >= 2u) {
                finalX = (lastIsValid != 0u) ? context->lastValidX : TDOA_MATH_INVALID_POSITION;
                finalY = (lastIsValid != 0u) ? context->lastValidY : TDOA_MATH_INVALID_POSITION;
                break;
            }
        } else {
            divergeCnt = 0u;
        }
        lastFNorm = fNorm;

        if (tdoa_math_solve_taylor_3x2(J, f, delta) != 0) {
            break;
        }

        xEst += delta[0];
        yEst += delta[1];
        finalX = xEst;
        finalY = yEst;

        {
            float extMargin = (xMax - xMin) * 1.5f + (yMax - yMin) * 1.5f;
            if (extMargin < 10.0f) {
                extMargin = 10.0f;
            }
            if (xEst < xMin - extMargin || xEst > xMax + extMargin ||
                yEst < yMin - extMargin || yEst > yMax + extMargin) {
                finalX = (lastIsValid != 0u) ? context->lastValidX : TDOA_MATH_INVALID_POSITION;
                finalY = (lastIsValid != 0u) ? context->lastValidY : TDOA_MATH_INVALID_POSITION;
                break;
            }
        }

        if (sqrtf(delta[0] * delta[0] + delta[1] * delta[1]) < TDOA_MATH_TAYLOR_DELTA_LIMIT_M) {
            break;
        }
    }

    context->lastValidX = finalX;
    context->lastValidY = finalY;
    context->hasLastValid = 1u;

    result->x = finalX;
    result->y = finalY;
    result->taylorIterations = taylorIterations;
    return 1u;
}

/* Clear the sliding window to an empty, unseeded state. */
static void tdoa_math_dist_filter_reset(tdoa_dist_filter_state_st* state)
{
    state->windowSum            = 0.0f;
    state->head                 = 0u;
    state->count                = 0u;
    state->valueValid           = 0u;
    state->abnormalCnt          = 0u;
    state->abnormalStreakActive = 0u;
    state->lastAbnormalDist     = 0.0f;
}

/* Push one sample into the ring buffer, evicting the oldest once full (O(1) sum). */
static void tdoa_math_dist_filter_push(tdoa_dist_filter_state_st* state, float value)
{
    if (state->count >= TDOA_DIST_SMA_WINDOW) {
        state->windowSum -= state->window[state->head];
    } else {
        state->count++;
    }
    state->window[state->head] = value;
    state->windowSum          += value;
    state->head                = (uint8_t)((state->head + 1u) % TDOA_DIST_SMA_WINDOW);
}

void tdoa_math_tag_update_distance_filter(tdoa_dist_filter_state_st* state,
                                          float newDist,
                                          uint8_t ratioValid,
                                          float* filteredDist,
                                          float jumpThresholdM,
                                          uint8_t abnormalMaxRetry)
{
    float diffFromOutput;

    if (state == 0 || filteredDist == 0) {
        return;
    }

    /* Seed (or re-seed) the window when uninitialised or the clock ratio is unusable. */
    if ((state->valueValid == 0u) || (ratioValid == 0u)) {
        tdoa_math_dist_filter_reset(state);
        tdoa_math_dist_filter_push(state, newDist);
        state->valueValid = 1u;
        *filteredDist     = newDist;
        return;
    }

    diffFromOutput = FLOAT_ABS_DIFF(newDist, *filteredDist);

    if (diffFromOutput <= jumpThresholdM) {
        /* In-range sample: append to the sliding window and output its average. */
        tdoa_math_dist_filter_push(state, newDist);
        *filteredDist               = state->windowSum / (float)state->count;
        state->abnormalCnt          = 0u;
        state->abnormalStreakActive = 0u;
        return;
    }

    /* Out-of-range sample: hold the output and require confirmation before accepting. */
    if (state->abnormalStreakActive != 0u) {
        float diffFromLast = FLOAT_ABS_DIFF(newDist, state->lastAbnormalDist);
        if (diffFromLast <= jumpThresholdM) {
            if (state->abnormalCnt < 0xFFu) {
                state->abnormalCnt++;
            }
        } else {
            state->abnormalCnt = 1u;
        }
    } else {
        state->abnormalCnt = 1u;
    }

    state->abnormalStreakActive = 1u;
    state->lastAbnormalDist     = newDist;

    if (state->abnormalCnt > abnormalMaxRetry) {
        /* Confirmed relocation: flush the window and snap to the new value (low latency). */
        tdoa_math_dist_filter_reset(state);
        tdoa_math_dist_filter_push(state, newDist);
        state->valueValid = 1u;
        *filteredDist     = newDist;
    }
}
