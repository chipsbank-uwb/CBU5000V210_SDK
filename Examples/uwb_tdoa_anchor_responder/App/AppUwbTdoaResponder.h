/**
 * @file    AppUwbTdoaResponder.h
 * @brief   TDOA responder private configuration and entry declaration.
 * @details Keeps responder-role defaults, UWB packet settings, slot timing,
 *          and the application entry point in one example-local header.
 * @author  Chipsbank
 * @date    2026
 */

#ifndef __APP_UWB_TDOA_RESPONDER_H
#define __APP_UWB_TDOA_RESPONDER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include "tdoa_config.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
/* Adjust the example anchor layout here when the board placement changes. */
#define DEF_TDOA_RESP_ANCHOR0_X             0.0f
#define DEF_TDOA_RESP_ANCHOR0_Y             0.0f
#define DEF_TDOA_RESP_ANCHOR1_X             0.0f
#define DEF_TDOA_RESP_ANCHOR1_Y             0.3f
#define DEF_TDOA_RESP_ANCHOR2_X             0.3f
#define DEF_TDOA_RESP_ANCHOR2_Y             0.0f
#define DEF_TDOA_RESP_ANCHOR3_X             0.3f
#define DEF_TDOA_RESP_ANCHOR3_Y             0.3f

/* ===================================================================
 * Responder-local UWB packet configuration
 * =================================================================== */
#define DEF_TDOA_RESP_PACKET_CONFIG_DEFAULTS                                \
{                                                                            \
    .prfMode            = EN_PRF_MODE_BPRF_62P4,                             \
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,                            \
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,                        \
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,                        \
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,                   \
    .sfdId              = EN_UWB_SFD_ID_2,                                   \
    .phrRangingBit      = 0x00,                                              \
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,                              \
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,                          \
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,                             \
    .stsKey             = {0x14EB220FUL, 0xF86050A8UL, 0xD1D336AAUL, 0x14148674UL}, \
    .stsVUpper          = {0xD37EC3CAUL, 0xC44FA8FBUL, 0x362EEB34UL},        \
    .stsVCounter        = 0x1F9A3DE4UL,                                      \
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,                             \
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
void app_tdoa_responder(void);

#endif /* __APP_UWB_TDOA_RESPONDER_H */
