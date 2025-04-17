/**
 * @file    AppUwbPeriodicRx.c
 * @brief   Implementation of UWB-COMMTRX: Periodic RX
 * @details This module provides functions to initialize and receive data periodically using the UWB.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "AppUwbPeriodicRx.h"
#include "AppSysIrqCallback.h"
#include "CB_system_types.h"
#include <string.h>
#include "CB_uwbframework.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_PERIODICRX_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_PERIODICRX_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_periodicrx_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_periodicrx_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief Structure used for logging.
 */
typedef struct 
{
  cb_uwbsystem_rxport_en enRxPort; /**< RX Port being checked */
  uint8_t logOpt;     /**< Logging level */
  uint32_t cycleIdx;  /**< Id of the current packet */
  uint32_t rxOkCnt;   /**< Number of packets received ok */
} stLogSettings;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void app_uwb_periodicrx_log(stLogSettings* const LogSettings);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint32_t s_num_receive = 0;
static uint8_t s_rx_done = APP_FALSE;        // Flipped by rx done handlers
static volatile stLogSettings logSettings;

/* Default Rx packet configuration.*/
static cb_uwbsystem_packetconfig_st Rxpacketconfig = 
{
  .prfMode            = EN_PRF_MODE_BPRF,                 // PRF mode selection
  .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
  .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
  .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
  .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
  .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
  .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
  .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
  .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
  .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
  .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
  .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
  .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
  .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
};

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void app_periodic_rx(void)
{
  stUwbPeriodicRxPacketConfig periodicRxConfig = 
  {
    .enRxPort = EN_UWB_RX_0,
    .packetsToRcv = 2000, //num of packets to receive Typically 10000
    .logOpt = 0, // Default to 0
  };
  app_uwb_periodicrx_receive_packet(&periodicRxConfig);
}
/**
 * @brief Puts device into receiving mode for configured duration.
 * @param packetConfig configuration of the rx session.
 * @see stUwbPeriodicRxPacketConfig
 * @detail Besides starting the rx session, this function also: configures the rx session; enable the needed interrupts;
 * start timer for specified duration; call the logging helper.
 */
void app_uwb_periodicrx_receive_packet(const stUwbPeriodicRxPacketConfig *const PacketConfig) 
{
  cb_framework_uwb_init();
    
  cb_framework_uwb_qmode_trx_enable();    
    
  s_num_receive = PacketConfig->packetsToRcv;
  s_rx_done = APP_FALSE;

  cb_uwbsystem_rx_irqenable_st stRxIrqEnable;
  stRxIrqEnable.rx0Done       = CB_TRUE;
  stRxIrqEnable.rx0PdDone     = CB_FALSE;
  stRxIrqEnable.rx0SfdDetDone = CB_FALSE; 
    
  switch (PacketConfig->enRxPort)
  {
    case EN_UWB_RX_0:   { stRxIrqEnable.rx0Done = APP_TRUE; break; }
    case EN_UWB_RX_1:   { stRxIrqEnable.rx1Done = APP_TRUE; break; }
    case EN_UWB_RX_2:   { stRxIrqEnable.rx2Done = APP_TRUE; break; }
    case EN_UWB_RX_ALL: { break; }
    default: { break; }
  }

  logSettings.enRxPort = PacketConfig->enRxPort;
  logSettings.logOpt = PacketConfig->logOpt;
  logSettings.cycleIdx = 0;
  logSettings.rxOkCnt = 0;
  
  while (1) 
  {
    cb_framework_uwb_rx_start(EN_UWB_RX_0, &Rxpacketconfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED); // RX START
    while (s_rx_done != APP_TRUE);
    if(logSettings.cycleIdx >= s_num_receive) 
    { 
       break;
    }
    app_uwb_periodicrx_log(&logSettings);
    cb_framework_uwb_rx_end(EN_UWB_RX_0);                  // RX END
    s_rx_done = APP_FALSE;
  }
  
  app_uwb_periodicrx_print("Packet Received: %d\n", logSettings.cycleIdx);
  app_uwb_periodicrx_print("Packet Received OK: %d\n", logSettings.rxOkCnt);
}

/**
 * @brief Handler for rx0 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void cb_uwbapp_rx0_done_irqcb(void) 
{ 
  s_rx_done = APP_TRUE;
}

/**
 * @brief Handler for rx1 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void cb_uwbapp_rx1_done_irqhandler(void) 
{ 
  s_rx_done = APP_TRUE; 
}

/**
 * @brief Handler for rx2 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void cb_uwbapp_rx2_done_irqcb(void) 
{ 
  s_rx_done = APP_TRUE; 
}

/**
 * @brief Helper function to log received packet info.
 * @details
 * Log Option 0: Includes cycle's count, and rx done interrupt status (Simple)
 * Log Option 1: option 0 + cir_i + cir_q (Simple + Cir)
 * Log Option 2: option1 + expanded rx interrupt status (every bit)
 */
static void app_uwb_periodicrx_log(stLogSettings* const LogSettings) 
{
  cb_uwbsystem_rxstatus_un statusRegister = cb_framework_uwb_get_rx_status();

  uint16_t rx_ok     = 0;
  uint16_t sfd_det   = 0;
  uint16_t pd_det    = 0;
  
  uint8_t rxPortNum = 0;
  
  switch(LogSettings->enRxPort)
  {
    case EN_UWB_RX_0:
      rx_ok   = statusRegister.rx0_ok;
      sfd_det = statusRegister.sfd0_det;
      pd_det  = statusRegister.pd0_det;
      rxPortNum = 0;
      break;
    
    case EN_UWB_RX_1:
      rx_ok   = statusRegister.rx1_ok;
      sfd_det = statusRegister.sfd1_det;
      pd_det  = statusRegister.pd1_det;
      rxPortNum = 1;
      break;   
    
    case EN_UWB_RX_2:
      rx_ok   = statusRegister.rx2_ok;
      sfd_det = statusRegister.sfd2_det;
      pd_det  = statusRegister.pd2_det;
      rxPortNum = 2;
      break;
    
    case EN_UWB_RX_ALL:
      break;
    
    default:  break;
  }
  
  uint8_t regStatOk = (rx_ok == APP_TRUE) && (sfd_det == APP_TRUE) && (pd_det == APP_TRUE);
  if (regStatOk)
  {
    ++LogSettings->rxOkCnt;
  } 
  else
  {
    app_uwb_periodicrx_print("Cycle_idx: %d, RX%d_ok: %d\n", LogSettings->cycleIdx, rxPortNum, regStatOk);
  }
  ++LogSettings->cycleIdx;
  
  if (LogSettings->logOpt >= 1)   
  {
    cb_uwbsystem_rx_cir_iqdata_st cirRegisterData[256];
    
    cb_framework_uwb_store_rx_cir_register(cirRegisterData, EN_UWB_RX_0, 0, 256);
    
    app_uwb_periodicrx_print("I: ");
    for (uint32_t i = 0; i < 256; ++i) 
    {
      if (i) 
      {
        app_uwb_periodicrx_print(", %d", cirRegisterData[i].I_data);
      } 
      else 
      {
        app_uwb_periodicrx_print("%d", cirRegisterData[i].I_data);
      }
    }
    app_uwb_periodicrx_print("\nQ: ");
    for (uint32_t i = 0; i < 256; ++i) 
    {
      if (i)
      {
        app_uwb_periodicrx_print(", %d", cirRegisterData[i].Q_data);
      } 
      else 
      {
        app_uwb_periodicrx_print("%d", cirRegisterData[i].Q_data);
      }
    }
    app_uwb_periodicrx_print("\n");
  }
  
  if (LogSettings->logOpt >= 2) 
  {
    cb_uwbsystem_rx_etc_statusregister_st etcStatusRegister;
    cb_framework_uwb_get_rx_etc_status_register(&etcStatusRegister);
    app_uwb_periodicrx_print("dsr_ovf, crc_fail, phr_sec, phr_ded, no2_signal, no1_signal, no0_signal, sfd2_det, sfd1_det, sfd0_det, pr2_det, pr1_det");
    app_uwb_periodicrx_print(", pr0_det, rx2_ok, rx1_ok, rx0_ok, bit-flipped PHR content, Estimated DC I, Estimated DC Q, gain_idx, cfo, ref_sync_idx, cir_sync_idx, rf_pll_lock, bb_pll_lock\n");
    app_uwb_periodicrx_print("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", statusRegister.dsr_ovf, statusRegister.crc_fail, statusRegister.phr_sec, statusRegister.phr_ded, statusRegister.no2_signal,
    statusRegister.no1_signal, statusRegister.no0_signal, statusRegister.sfd2_det, statusRegister.sfd1_det, statusRegister.sfd0_det, statusRegister.pd2_det, statusRegister.pd1_det,
    statusRegister.pd0_det, statusRegister.rx2_ok, statusRegister.rx1_ok, statusRegister.rx0_ok, etcStatusRegister.bitFlippedPhrContent, etcStatusRegister.estimatedDcIvalue, etcStatusRegister.estimatedDcQvalue, 
    etcStatusRegister.gainRx0, etcStatusRegister.cfoEstimatedValue, etcStatusRegister.refSyncIdx, etcStatusRegister.cirSyncIdx, etcStatusRegister.rfPllLock, etcStatusRegister.bbPllLock);
  }
  s_rx_done = APP_TRUE; 
  return; 
}
