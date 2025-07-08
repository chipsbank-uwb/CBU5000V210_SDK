/*! ----------------------------------------------------------------------------
 * @file    CB_uwbpackettemplate.c
 * @brief   Configuration template file for UWB packet format.
 *
 * This file contains configuration options for both Base PRF (BPRF) and 
 * High PRF (HPRF) packet sets used in UWB communications.
 *
 * @details The file defines multiple configuration options:
 *          - BPRF sets (B01-B04): Base Pulse Repetition Frequency configurations
 *          - HPRF sets (H01-H31): High Pulse Repetition Frequency configurations
 *          - Default configuration for demo purposes
 *
 * Each configuration option can be enabled by uncommenting its corresponding
 * define directive.
 *
 * @author  Chipsbank
 * @date    2024
 * ----------------------------------------------------------------------------
 */
#include "CB_uwbpackettemplate.h"

#ifdef CONFIG_OPTION_B01
/* Configuration option BPRF set# 01.
 * - Base PRF (BPRF) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 9
 * - SFD ID 0
 * - Basic frame (SP0) with no STS
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_0,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_0,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_B02
/* Configuration option BPRF set# 02.
 * - Base PRF (BPRF) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 9  
 * - SFD ID 2
 * - Basic frame (SP0) with no STS
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_0,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_B03
/* Configuration option BPRF set# 03.
 * - Base PRF (BPRF) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 9
 * - SFD ID 2
 * - Basic frame (SP1) with 1 STS segment
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_B04
/* Configuration option BPRF set# 04.
 * - Base PRF (BPRF) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 9
 * - SFD ID 2 
 * - Extended frame (SP3) with 1 STS segment
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H01
/* Configuration option HPRF set# 01.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP0) with no STS
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_0,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H02
/* Configuration option HPRF set# 02.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP0) with no STS
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_0,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H03
/* Configuration option HPRF set# 03.
 * - High PRF (249.6 MHz) mode with 27.2 Mbps data rate
 * - 32 symbol preamble with code index 9
 * - SFD ID 2
 * - Basic frame (SP0) with no STS
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_249P6,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_27P2,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_0,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H04
/* Configuration option HPRF set# 04.
 * - High PRF (249.6 MHz) mode with 27.2 Mbps data rate
 * - 32 symbol preamble with code index 9
 * - SFD ID 1
 * - Basic frame (SP0) with no STS
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_249P6,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_27P2,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_1,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_0,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H05
/* Configuration option HPRF set# 05.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP0) with 2 STS segments of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H06
/* Configuration option HPRF set# 06.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 1 STS segment of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H07
/* Configuration option HPRF set# 07.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 2 STS segments of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H08
/* Configuration option HPRF set# 08.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 1 STS segment of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H09
/* Configuration option HPRF set# 09.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 2 STS segments of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H10
/* Configuration option HPRF set# 10.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 1 STS segment of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H11
/* Configuration option HPRF set# 11.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 2 STS segments of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H12
/* Configuration option HPRF set# 12.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2 
 * - Basic frame (SP1) with 1 STS segment of 128 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_128_SYMBOLS,        // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H13
/* Configuration option HPRF set# 13.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 2 STS segments of 128 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_128_SYMBOLS,        // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H14
/* Configuration option HPRF set# 14.
 * - High PRF (249.6 MHz) mode with 27.2 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 1 STS segment of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_249P6,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_27P2,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H15
/* Configuration option HPRF set# 15.
 * - High PRF (249.6 MHz) mode with 27.2 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 2 STS segments of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_249P6,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_27P2,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H16
/* Configuration option HPRF set# 16.
 * - High PRF (249.6 MHz) mode with 27.2 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 1 STS segment of 32 symbols
 */

cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_249P6,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_27P2,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H17
/* Configuration option HPRF set# 17.
 * - High PRF (249.6 MHz) mode with 27.2 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Basic frame (SP1) with 2 STS segments of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_249P6,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_27P2,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H18
/* Configuration option HPRF set# 18.
 * - High PRF (249.6 MHz) mode with 27.2 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 1
 * - Basic frame (SP1) with 1 STS segment of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_249P6,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_27P2,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_1,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H19
/* Configuration option HPRF set# 19.
 * - High PRF (249.6 MHz) mode with 27.2 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 1
 * - Basic frame (SP1) with 2 STS segments of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_249P6,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_27P2,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_1,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H20
/* Configuration option HPRF set# 20.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 3
 * - Extended frame (SP3) with 1 STS segment of 128 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_3,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_128_SYMBOLS,        // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H21
/* Configuration option HPRF set# 21.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 3 
 * - Extended frame (SP3) with 2 STS segments of 128 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_3,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_128_SYMBOLS,        // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H22
/* Configuration option HPRF set# 22.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Extended frame (SP3) with 1 STS segment of 128 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_128_SYMBOLS,        // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H23
/* Configuration option HPRF set# 22.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Extended frame (SP3) with 1 STS segment of 128 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_128_SYMBOLS,        // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H24
/* Configuration option HPRF set# 24.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Extended frame (SP3) with 1 STS segment of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H25
/* Configuration option HPRF set# 25.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 25
 * - SFD ID 2
 * - Extended frame (SP3) with 2 STS segments of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H26
/* Configuration option HPRF set# 26.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Extended frame (SP3) with 1 STS segment of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H27
/* Configuration option HPRF set# 27.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Extended frame (SP3) with 2 STS segments of 64 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},                   // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif


#ifdef CONFIG_OPTION_H28
/* Configuration option HPRF set# 28.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Extended frame (SP3) with 1 STS segment of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H29
/* Configuration option HPRF set# 29.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 2
 * - Extended frame (SP3) with 2 STS segments of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H30
/* Configuration option HPRF set# 30.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 1
 * - Extended frame (SP3) with 1 STS segment of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_1,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_H31
/* Configuration option HPRF set# 31.
 * - High PRF (124.8 MHz) mode with 6.81 Mbps data rate
 * - 32 symbol preamble with code index 25
 * - SFD ID 1
 * - Extended frame (SP3) with 2 STS segments of 32 symbols
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_HPRF_124P8,           // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_25,      // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_32_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_1,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_32_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_2,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif

#ifdef CONFIG_OPTION_DEFAULT
/* Configuration option for Demo.
 * - Base PRF (BPRF) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 9
 * - SFD ID 2
 * - Basic frame (SP0) with no STS
 */
cb_uwbsystem_packetconfig_st config_options = {
    .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_0,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},                   // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };

/* Configuration option SP0.
 * - Base PRF (BPRF) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 9
 * - SFD ID 2
 * - Basic frame (SP0) with no STS
 */
cb_uwbsystem_packetconfig_st config_option_sp0 = {
    .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_0,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},                   // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };

/* Configuration option SP1.
 * - Base PRF (BPRF) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 9
 * - SFD ID 2
 * - Basic frame (SP1) with 1 STS segment
 */
cb_uwbsystem_packetconfig_st config_option_sp1 = {
    .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP1,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},                   // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };

/* Configuration option SP3.
 * - Base PRF (BPRF) mode with 6.81 Mbps data rate
 * - 64 symbol preamble with code index 9
 * - SFD ID 2
 * - Extended frame (SP3) with 1 STS segment
 */
cb_uwbsystem_packetconfig_st config_option_sp3 = {
    .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
    .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
    .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
    .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
    .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
    .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
    .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
    .rframeConfig       = EN_RFRAME_CONFIG_SP3,             // SP0, SP1, SP3
    .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
    .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
    .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
    .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},                   // PhyHrpUwbStsVUpper96
    .stsVCounter        = 0x1F9A3DE4UL,                                                           // PhyHrpUwbStsVCounter
    .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
    };
#endif
