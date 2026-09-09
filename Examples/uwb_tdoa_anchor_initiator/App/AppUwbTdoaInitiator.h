/**
 * @file    AppUwbTdoaInitiator.h
 * @brief   TDOA initiator private configuration and entry declaration.
 * @details Keeps initiator-role defaults, UWB packet settings, Timer0 period,
 *          and the application entry point in one example-local header.
 * @author  Chipsbank
 * @date    2026
 */

#ifndef __APP_UWB_TDOA_INITIATOR_H
#define __APP_UWB_TDOA_INITIATOR_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include "tdoa_config.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_TDOA_INITIATOR_SLOT                  0u

/* ===================================================================
 * Initiator-local UWB packet configuration
 * =================================================================== */
#define DEF_TDOA_INITIATOR_PACKET_CONFIG_DEFAULTS                             \
{                                                                             \
    .prfMode           = EN_PRF_MODE_BPRF_62P4,                               \
    .psduDataRate      = EN_PSDU_DATA_RATE_6P81,                              \
    .bprfPhrDataRate   = EN_BPRF_PHR_DATA_RATE_0P85,                          \
    .preambleCodeIndex = EN_UWB_PREAMBLE_CODE_IDX_9,                          \
    .preambleDuration  = EN_PREAMBLE_DURATION_64_SYMBOLS,                     \
    .sfdId             = EN_UWB_SFD_ID_2,                                     \
    .phrRangingBit     = 0x00,                                                \
    .rframeConfig      = EN_RFRAME_CONFIG_SP0,                                \
    .stsLength         = EN_STS_LENGTH_64_SYMBOLS,                            \
    .numStsSegments    = EN_NUM_STS_SEGMENTS_1,                               \
    .stsKey            = {0x14EB220FUL, 0xF86050A8UL, 0xD1D336AAUL, 0x14148674UL}, \
    .stsVUpper         = {0xD37EC3CAUL, 0xC44FA8FBUL, 0x362EEB34UL},          \
    .stsVCounter       = 0x1F9A3DE4UL,                                        \
    .macFcsType        = EN_MAC_FCS_TYPE_CRC16,                               \
}

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
void tdoa_role_apply_default_config(tdoa_runtime_config_t* config);
void app_tdoa_initiator(void);
void app_tdoa_reset(void);
void app_tdoa_timer_init(uint16_t timeoutMs);
void app_tdoa_timer_off(void);

#endif /* __APP_UWB_TDOA_INITIATOR_H */
