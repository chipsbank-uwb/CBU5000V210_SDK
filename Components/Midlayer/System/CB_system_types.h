/**
 * @file    CB_system_types.h
 * @brief   Contains cb_system types
 * @details This file contains cb_system structs and enums
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __CB_SYSTEM_TYPES_H
#define __CB_SYSTEM_TYPES_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------


//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief Enumeration defining UWB RX ports.
 */
typedef enum
{
  EN_UWB_RX_0   = 0b001,  // UWB RX port 0
  EN_UWB_RX_1   = 0b010,  // UWB RX port 1
  EN_UWB_RX_2   = 0b100,  // UWB RX port 2
  EN_UWB_RX_02  = 0b101,  // UWB RX port 0 & 2 
  EN_UWB_RX_ALL = 0b111,  // All UWB RX ports
} cb_uwbsystem_rxport_en;

typedef enum 
{
  EN_UWB_Channel_5  = 5,  // Fira3.0 UCI definition
  EN_UWB_Channel_6  = 6,  // Fira3.0 UCI definition
  EN_UWB_Channel_8  = 8,  // Fira3.0 UCI definition
  EN_UWB_Channel_9  = 9,  // Fira3.0 UCI definition (Default)
  EN_UWB_Channel_10 = 10, // Fira3.0 UCI definition
  EN_UWB_Channel_12 = 12, // Fira3.0 UCI definition, not supported yet, temporary set as Channel 9 internally
  EN_UWB_Channel_13 = 13, // Fira3.0 UCI definition, not supported yet, temporary set as Channel 9 internally
  EN_UWB_Channel_14 = 14  // Fira3.0 UCI definition, not supported yet, temporary set as Channel 9 internally
}cb_uwbsystem_channelnum_en;

typedef enum 
{
  EN_UWB_RX_OPERATION_MODE_GENERAL = 0,
  EN_UWB_RX_OPERATION_MODE_COEXIST = 1,
}cb_uwbsystem_rxoperationmode_en;

typedef struct
{  
  double rx0_rx1;     /**< Phase difference between RX0 and RX1 */
  double rx0_rx2;     /**< Phase difference between RX0 and RX2 */
  double rx1_rx2;     /**< Phase difference between RX1 and RX2 */
  uint8_t rxstatus;
}cb_uwbsystem_pdoa_3ddata_st;

typedef struct
{  
  volatile cb_uwbsystem_pdoa_3ddata_st mean;
  volatile cb_uwbsystem_pdoa_3ddata_st median;
  uint8_t stRxstatus;  
}cb_uwbsystem_pdoaresult_st;

typedef enum 
{
  EN_PRF_MODE_BPRF_62P4   = 0, // Fira3.0 UCI definition
  EN_PRF_MODE_HPRF_124P8  = 1, // Fira3.0 UCI definition
  EN_PRF_MODE_HPRF_249P6  = 2, // Fira3.0 UCI definition
  EN_PRF_MODE_LG4A_62P4   = 3  // Fira3.0 RFU : In-house used
}cb_uwbsystem_prfmode_en;

typedef enum 
{
  EN_PSDU_DATA_RATE_6P81 = 0, // Fira3.0 UCI definition
  EN_PSDU_DATA_RATE_7P80 = 1, // Fira3.0 UCI definition
  EN_PSDU_DATA_RATE_27P2 = 2, // Fira3.0 UCI definition
  EN_PSDU_DATA_RATE_31P2 = 3, // Fira3.0 UCI definition
  EN_PSDU_DATA_RATE_0P85 = 4  // Fira3.0 UCI definition
}cb_uwbsystem_psdu_datarate_en;

typedef enum {
  EN_BPRF_PHR_DATA_RATE_0P85 = 0, // Fira3.0 UCI definition
  EN_BPRF_PHR_DATA_RATE_6P81 = 1  // Fira3.0 UCI definition
}cb_uwbsystem_bprf_phr_datarate_en;

/**
 * @brief Enumeration defining UWB Preamble Code Indexes
 * 
 * @note 9 - 24 is used for BPRF, 25-32 is used for HPRF
 */
typedef enum
{
  EN_UWB_PREAMBLE_CODE_IDX_9  = 9,  // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_10 = 10, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_11 = 11, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_12 = 12, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_13 = 13, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_14 = 14, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_15 = 15, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_16 = 16, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_17 = 17, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_18 = 18, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_19 = 19, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_20 = 20, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_21 = 21, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_22 = 22, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_23 = 23, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_24 = 24, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_25 = 25, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_26 = 26, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_27 = 27, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_28 = 28, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_29 = 29, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_30 = 30, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_31 = 31, // Fira3.0 UCI definition
  EN_UWB_PREAMBLE_CODE_IDX_32 = 32, // Fira3.0 UCI definition  
}cb_uwbsystem_preamblecodeidx_en;

typedef enum
{
  EN_PREAMBLE_DURATION_32_SYMBOLS   = 0, // Fira3.0 UCI definition
  EN_PREAMBLE_DURATION_64_SYMBOLS   = 1, // Fira3.0 UCI definition
  EN_PREAMBLE_DURATION_16_SYMBOLS   = 2, // Fira3.0 RFU : In-house used
  EN_PREAMBLE_DURATION_24_SYMBOLS   = 3, // Fira3.0 RFU : In-house used
  EN_PREAMBLE_DURATION_48_SYMBOLS   = 4, // Fira3.0 RFU : In-house used
  EN_PREAMBLE_DURATION_96_SYMBOLS   = 5, // Fira3.0 RFU : In-house used
  EN_PREAMBLE_DURATION_128_SYMBOLS  = 6, // Fira3.0 RFU : In-house used
  EN_PREAMBLE_DURATION_256_SYMBOLS  = 7, // Fira3.0 RFU : In-house used
  EN_PREAMBLE_DURATION_1024_SYMBOLS = 8, // Fira3.0 RFU : In-house used
  EN_PREAMBLE_DURATION_4096_SYMBOLS = 9  // Fira3.0 RFU : In-house used
}cb_uwbsystem_preambleduration_en;

typedef enum
{
  EN_UWB_SFD_ID_0  = 0, // Fira3.0 UCI definition  
  EN_UWB_SFD_ID_1  = 1, // Fira3.0 UCI definition  
  EN_UWB_SFD_ID_2  = 2, // Fira3.0 UCI definition  
  EN_UWB_SFD_ID_3  = 3, // Fira3.0 UCI definition  
  EN_UWB_SFD_ID_4  = 4, // Fira3.0 UCI definition  
}cb_uwbsystem_sdf_id_en;

typedef enum
{
  EN_RFRAME_CONFIG_SP0 = 0, // Fira3.0 UCI definition  
  EN_RFRAME_CONFIG_SP1 = 1, // Fira3.0 UCI definition  
  EN_RFRAME_CONFIG_SP3 = 3  // Fira3.0 UCI definition  
}cb_uwbsystem_rframeconfig_en;

typedef enum
{
  EN_STS_LENGTH_32_SYMBOLS  = 0, // Fira3.0 UCI definition  
  EN_STS_LENGTH_64_SYMBOLS  = 1, // Fira3.0 UCI definition  
  EN_STS_LENGTH_128_SYMBOLS = 2  // Fira3.0 UCI definition  
}cb_uwbsystem_stslength_en;

typedef enum
{
  EN_NUM_STS_SEGMENTS_0 = 0, // Fira3.0 UCI definition  
  EN_NUM_STS_SEGMENTS_1 = 1, // Fira3.0 UCI definition  
  EN_NUM_STS_SEGMENTS_2 = 2, // Fira3.0 UCI definition  
  EN_NUM_STS_SEGMENTS_3 = 3, // Fira3.0 UCI definition  
  EN_NUM_STS_SEGMENTS_4 = 4  // Fira3.0 UCI definition  
}cb_uwbsystem_num_stssegments_en;

typedef enum
{
  EN_MAC_FCS_TYPE_CRC16 = 0, // Fira3.0 UCI definition  
  EN_MAC_FCS_TYPE_CRC32 = 1  // Fira3.0 UCI definition  
}cb_uwbsystem_mac_fcstype_en;

/**
 * @brief Enumeration defining UWB TX RX modes.
 */
typedef enum
{
  EN_UWB_CONFIG_TX = 0,
  EN_UWB_CONFIG_RX = 1,
}cb_uwbsystem_configmodule_selection_en;

typedef enum
{
  EN_UWB_CFO_GAIN_SET   = 0,
  EN_UWB_CFO_GAIN_RESET = 1,
}cb_uwbsystem_rxconfig_cfo_gain_en;

typedef struct
{
  volatile uint32_t T_round_int;
  volatile int16_t  T_round_frac;
  volatile uint32_t T_reply_int;
  volatile int16_t  T_reply_frac;
  volatile uint8_t  success;
}cb_uwbsystem_rangingtroundtreply_st;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief Structure for system configuration settings.
 * 
 * This structure holds configuration parameters for the system, including
 * transmission power, packet mode, and antenna settings.
 */
typedef struct
{
  cb_uwbsystem_channelnum_en channelNum;                   /**< @brief Channel Number               (default 9)    */
  uint8_t bbpllFreqOffest_rf;                              /**< @brief Baseband Frequency Offset Rf (default 127)  */
  uint8_t powerCode_tx;                                    /**< @brief Transmission Power Code                     */  
  cb_uwbsystem_rxoperationmode_en operationMode_rx;        /**< @brief Operation Mode (RX)                         */
  /* Reserved */
  //stUwbSystemConfigExtended extendedArgs; /**< @brief Vendor Specific*/
}__attribute__((aligned(4))) cb_uwbsystem_systemconfig_st;

/**
 * @brief Configuration structure that holds all parameters.
 */
typedef struct
{
  /* PRF and Data Rate Settings */    
  cb_uwbsystem_prfmode_en prfMode;                      /**< @brief PRF mode selection  */
  cb_uwbsystem_psdu_datarate_en psduDataRate;           /**< @brief PSDU data rate  */
  cb_uwbsystem_bprf_phr_datarate_en bprfPhrDataRate;    /**< @brief BPRF PHR data rate  */
  /* SYNC Configuration */
  cb_uwbsystem_preamblecodeidx_en preambleCodeIndex;    /**< @brief Preamble code index (9-32)  */
  cb_uwbsystem_preambleduration_en preambleDuration;    /**< @brief Preamble duration  */
  /* SFD Configuration */
  cb_uwbsystem_sdf_id_en sfdId;                         /**< @brief SFD identifier (0-4)  */
  /* PHR Configuration */
  uint8_t phrRangingBit;                                /**< @brief PHR Ranging Bit: 0 or 1 */
  /* STS Configuration */
  cb_uwbsystem_rframeconfig_en rframeConfig;            /**< @brief Frame configuration  */
  cb_uwbsystem_stslength_en stsLength;                  /**< @brief Length of STS sequence  */
  cb_uwbsystem_num_stssegments_en numStsSegments;       /**< @brief Number of STS segments (0-4)  */
  uint32_t stsKey[4];                                   /**< @brief PhyHrpUwbStsKey  */
  uint32_t stsVUpper[3];                                /**< @brief PhyHrpUwbStsVUpper96  */
  uint32_t stsVCounter;                                 /**< @brief PhyHrpUwbStsVCounter  */
  /* CRC Configuration */
  cb_uwbsystem_mac_fcstype_en macFcsType;               /**< @brief MAC FCS type (CRC16/CRC32)  */
}__attribute__((aligned(4))) cb_uwbsystem_packetconfig_st;

/**
 * @brief Structure representing DCOC i and q
 */
typedef struct
{
  int16_t DC_q;
  int16_t DC_i;
}__attribute__((aligned(4))) cb_uwbsystem_rx_dcoc_st;

typedef struct
{
  float real;
  float imag;
} stRadarCIR_IQ_Complex_t;

/**
 * @brief Enumeration for supported FFT sizes.
 */
typedef enum
{
  EN_FFT_LEN16   = 0, /**<   16-point FFT */
  EN_FFT_LEN32   = 1, /**<   32-point FFT */
  EN_FFT_LEN64   = 2, /**<   64-point FFT */
  EN_FFT_LEN128  = 3, /**<  128-point FFT */
  EN_FFT_LEN256  = 4, /**<  256-point FFT */
  EN_FFT_LEN512  = 5, /**<  512-point FFT */
  EN_FFT_LEN1024 = 6, /**< 1024-point FFT */
  EN_FFT_LEN2048 = 7, /**< 2048-point FFT */
  EN_FFT_LEN4096 = 8  /**< 4096-point FFT */
} cb_uwbradar_en;

/**
 * @brief Structure that contains rssi, cfo, dcoc and gain
 */
typedef struct
{
  uint32_t cfoEst;
  cb_uwbsystem_rx_dcoc_st dcocRx;
	int16_t  rssiRx;
	uint8_t  gainIdx;
}__attribute__((aligned(4))) cb_uwbsystem_rx_signalinfo_st;

/**
 * @brief Structure representing phr status
 */
typedef struct
{
	uint8_t phrDed;
	uint8_t phrSec;
	uint8_t rx0Ok;
}cb_uwbsystem_rx_phrstatus_st;

/**
 * @brief Struct representing etc status registers.
 */
typedef struct stEtcStatusRegister_
{
  uint32_t bitFlippedPhrContent;
  uint32_t estimatedDcIvalue;
  uint32_t estimatedDcQvalue;
  uint32_t gainRx0;
  uint32_t cfoEstimatedValue;
  uint32_t refSyncIdx;
  uint32_t cirSyncIdx;
  uint32_t rfPllLock; /** rf_pll_lock */
  uint32_t bbPllLock; /** bb_pll_lock */
}cb_uwbsystem_rx_etc_statusregister_st;

/**
 * @brief Union representing status register bits.
 */
typedef union
{
  struct
  {
    uint16_t rx0_ok       : 1;   // [0]  < RX0 OK 
    uint16_t rx1_ok       : 1;   // [1]  < RX1 OK 
    uint16_t rx2_ok       : 1;   // [2]  < RX2 OK 
    uint16_t pd0_det      : 1;   // [3]  < PD0 detection
    uint16_t pd1_det      : 1;   // [4]  < PD1 detection
    uint16_t pd2_det      : 1;   // [5]  < PD2 detection
    uint16_t sfd0_det     : 1;   // [6]  < SFD0 detection
    uint16_t sfd1_det     : 1;   // [7]  < SFD1 detection
    uint16_t sfd2_det     : 1;   // [8]  < SFD2 detection
    uint16_t no0_signal   : 1;   // [9]  < no0_signal
    uint16_t no1_signal   : 1;   // [10] < no1_signal
    uint16_t no2_signal   : 1;   // [11] < no2_signal
    uint16_t phr_ded      : 1;   // [12] < PHR (Preamble Header) detected
    uint16_t phr_sec      : 1;   // [13] < PHR (Preamble Header) second
    uint16_t crc_fail     : 1;   // [14] < CRC (Cyclic Redundancy Check) fail
    uint16_t dsr_ovf      : 1;   // [15] < DSR (Data Sampling Rate) overflow
  };
  uint16_t value;                 /**< Status register value */
} __attribute__((aligned(4))) cb_uwbsystem_rxstatus_un;

/**
 * @brief Structure representing UWB transmission IRQ enable flags.
 */
typedef struct
{
  uint8_t txDone;    /**< Flag for transmission done IRQ */
  uint8_t sfdDone;  /**< Flag for SFD IRQ */
} cb_uwbsystem_tx_irqenable_st;

/**
 * @brief Structure representing UWB RX IRQ enable flags.
 */
typedef struct
{
  uint8_t rx0Done;        // UWB_RX0_DONE_IRQn            < RX0 done IRQ enable flag 
  uint8_t rx0PdDone;      // UWB_RX0_PD_DONE_IRQn         < RX0 PD done IRQ enable flag 
  uint8_t rx0SfdDetDone;  // UWB_RX0_SFD_DET_DONE_IRQn    < RX0 SFD detection done IRQ enable flag 
  uint8_t rx1Done;        // UWB_RX1_DONE_IRQn            < RX1 done IRQ enable flag 
  uint8_t rx1PdDone;      // UWB_RX1_PD_DONE_IRQn         < RX1 PD done IRQ enable flag 
  uint8_t rx1SfdDetDone;  // UWB_RX1_SFD_DET_DONE_IRQn    < RX1 SFD detection done IRQ enable flag 
  uint8_t rx2Done;        // UWB_RX2_DONE_IRQn            < RX2 done IRQ enable flag 
  uint8_t rx2PdDone;      // UWB_RX2_PD_DONE_IRQn         < RX2 PD done IRQ enable flag 
  uint8_t rx2SfdDetDone;  // UWB_RX2_SFD_DET_DONE_IRQn    < RX2 SFD detection done IRQ enable flag 
  uint8_t rxStsCirEnd;    // UWB_RX_STS_CIR_END_IRQn      < RX STS CIR end IRQ enable flag 
  uint8_t rxPhrDetected;  // UWB_RX_PHR_DETECTED_IRQn     < RX PHR detected IRQ enable flag 
  uint8_t rxDone;         // UWB_RX_DONE_IRQn             < RX done IRQ enable flag 
} cb_uwbsystem_rx_irqenable_st;

/**
 * @brief Structure representing TX Timestamps.
 */
typedef struct
{  
  uint32_t   txStart;      /**< TX start timestamp */
  uint32_t   sfdMark;      /**< SFD mark timestamp */
  uint32_t   sts1Mark;     /**< STS 1 mark timestamp */
  uint32_t   sts2Mark;     /**< STS 2 mark timestamp */
  uint32_t   txDone;       /**< TX done timestamp */
} cb_uwbsystem_tx_timestamp_st;

/**
 * @brief Structure representing TX Time Stamp Unit timstamp.
 */
typedef struct stTxTsuTimestamp_
{
  uint32_t txTsuInt;      /**< RX TSU integer part:    1 count = 1/124.8MHz     (~8ns)    */
  uint16_t txTsuFrac;     /**< RX TSU fractional part: 1 count = 1/124.8MHz/512 (~15.6ps) */
} __attribute__((aligned(4))) cb_uwbsystem_tx_tsutimestamp_st;

/**
 * @brief Structure representing RX Time Stamp Unit (TSU) timestamp.
 */
typedef struct
{
  uint32_t rxTsuInt;      /**< RX TSU integer part:    1 count = 1/124.8MHz     (~8ns)    */
  uint16_t rxTsuFrac;     /**< RX TSU fractional part: 1 count = 1/124.8MHz/512 (~15.6ps) */
  double rxTsu;
} __attribute__((aligned(4))) cb_uwbsystem_rx_tsutimestamp_st;

/**
 * @brief Structure representing RX Timestamps.
 */
typedef struct stRxTimestamp_
{
  uint32_t startCnt;
  uint32_t startCap;
  uint32_t preambleDetectionCnt;
  uint32_t preambleDetectionCap;
  uint32_t sfdDetectionCnt;
  uint32_t sfdDetectionCap;
  uint32_t stsSegment0Cnt;
  uint32_t stsSegment0Cap;
  uint32_t stsSegment1Cnt;
  uint32_t stsSegment1Cap;
  uint32_t stsCirCnt;
  uint32_t stsCirCap;
  uint32_t phrDoneCnt;
  uint32_t phrDoneCap;
  uint32_t doneCnt;
  uint32_t doneCap;
} cb_uwbsystem_rx_timestamp_st;

/**
 * @brief Structure representing parameters for RX ranging.
 */
typedef struct
{
  uint16_t peak_idx_b4intrpl;    /**< Peak index before interpolation */
  int32_t  peak_b4intrpl;        /**< Peak value before interpolation */
  double   peak_idx_offset;      /**< Peak index offset */
} cb_uwbsystem_rx_rangingparam_st;

/**
 * @brief Structure representing RX Time Stamp Unit (TSU) status.
 */
typedef struct
{  
  uint32_t  rx_sfd_tsu_int;     /**< RX SFD TSU integer part */
  uint32_t  rx_sfd_smp_offs;    /**< RX SFD sample offset */
  uint32_t  rx_sfd_smp_sbuf;    /**< RX SFD sample buffer */
  uint32_t  ref_sync_idx;       /**< Reference sync index */
  uint32_t  cir_sync_idx;       /**< CIR sync index */
} cb_uwbsystem_rx_tsustatus_st;

/**
 * @brief Union representing RX Time Stamp Unit (TSU) data.
 */
typedef union
{
  struct
  {
    uint32_t CapSampleCnt:32;       /**< Captured sample count */
    uint32_t CapSampleSbuf:8;       /**< Captured sample buffer (bits [7:0]) */
    uint32_t CapSampleSmpOffs:8;    /**< Captured sample offset (bits [15:8]) */
    uint32_t CapSampleCorr:16;      /**< Captured sample correction (bits [31:16]) */ 
  };
  uint32_t data[2];                  /**< Data array */
} cb_uwbsystem_rx_tsu_un;


/**
 * @brief Structure representing RX Timestamps.
 */
typedef struct
{  
  cb_uwbsystem_rx_tsu_un   rx0StartEvent;              /**< RX0 start event timestamp */
  cb_uwbsystem_rx_tsu_un   rx1StartEvent;              /**< RX1 start event timestamp */
  cb_uwbsystem_rx_tsu_un   rx2StartEvent;              /**< RX2 start event timestamp */
  cb_uwbsystem_rx_tsu_un   rx0PreambleDetectionEvent;  /**< RX0 preamble detection event timestamp */
  cb_uwbsystem_rx_tsu_un   rx0SfdDetectionEvent;       /**< RX0 SFD detection event timestamp */
  cb_uwbsystem_rx_tsu_un   rx0StsSegment0Event;        /**< RX0 STS segment 0 event timestamp */
  cb_uwbsystem_rx_tsu_un   rx0StsSegment1Event;        /**< RX0 STS segment 1 event timestamp */
  cb_uwbsystem_rx_tsu_un   rx0StsCirEndEvent;          /**< RX0 STS CIR end event timestamp */
  cb_uwbsystem_rx_tsu_un   rx1SfdDetectionEvent;       /**< RX1 SFD detection event timestamp */
  cb_uwbsystem_rx_tsu_un   rx2SfdDetectionEvent;       /**< RX2 SFD detection event timestamp */
  cb_uwbsystem_rx_tsu_un   phrDone;                    /**< PHR done timestamp */
  cb_uwbsystem_rx_tsu_un   rxDone;                     /**< RX done timestamp */
} cb_uwbsystem_rx_tsu_st;

/**
 * @brief Structure representing transmission payload for UWB.
 */
typedef struct
{
  uint8_t*        ptrAddress;    /**< Pointer to the payload address */
  uint16_t        payloadSize;  /**< Payload size */
}__attribute__((aligned(4))) cb_uwbsystem_txpayload_st;

typedef struct {
  int16_t Q_data; // CIR_Q_data
  int16_t I_data; // CIR_I_data
} __attribute__((aligned(4))) cb_uwbsystem_rx_cir_iqdata_st;

typedef struct {
  uint8_t  enableBypass;
  uint32_t  gainValue;
} cb_uwbsystem_rx_dbb_gain_st; 

typedef struct {
  uint8_t  enableBypass;
  uint32_t  cfoValue;
} cb_uwbsystem_rx_dbb_cfo_st; 

typedef struct {
  cb_uwbsystem_rx_dbb_gain_st stRxGain;
  cb_uwbsystem_rx_dbb_cfo_st  stRxCfo;
} cb_uwbsystem_rx_dbb_config_st;


//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

#endif /*__CB_SYSTEM_TYPES_H*/
