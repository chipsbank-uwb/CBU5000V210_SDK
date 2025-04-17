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
#include "CB_commtrx.h"
#include <string.h>

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
typedef struct {
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
static volatile uint8_t s_stopPeriodicRx = APP_FALSE;

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Puts device into receiving mode for configured duration.
 * @param packetConfig configuration of the rx session.
 * @see stUwbPeriodicRxPacketConfig
 * @detail Besides starting the rx session, this function also: configures the rx session; enable the needed interrupts;
 * start timer for specified duration; call the logging helper.
 */
void app_uwb_periodicrx_receive_packet(const stUwbPeriodicRxPacketConfig *const PacketConfig) 
{
  cb_system_uwb_init();
  s_num_receive = PacketConfig->packetsToRcv;
  s_rx_done = APP_FALSE;
  s_stopPeriodicRx = APP_FALSE;

  cb_uwbsystem_rx_irqenable_st stRxIrqEnable;
  memset(&stRxIrqEnable, 0x0, sizeof(cb_uwbsystem_rx_irqenable_st));
  switch (PacketConfig->enRxPort) {
    case EN_UWB_RX_0: { stRxIrqEnable.rx0Done = APP_TRUE; break; }
    case EN_UWB_RX_1: { stRxIrqEnable.rx1Done = APP_TRUE; break; }
    case EN_UWB_RX_2: { stRxIrqEnable.rx2Done = APP_TRUE; break; }
    case EN_UWB_RX_ALL: { break; }
    default: { break; }
  }

<<<<<<< HEAD
  if (rxIrqEnable.rx0Done == APP_TRUE)
    app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX0_DONE_IRQ_Callback);
  
  if (rxIrqEnable.rx1Done == APP_TRUE)
    app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX1_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX1_DONE_IRQ_Callback);
  
  if (rxIrqEnable.rx2Done == APP_TRUE)
    app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX2_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX2_DONE_IRQ_Callback);
=======
  if (stRxIrqEnable.rx0Done == APP_TRUE)
    APP_IRQ_RegisterIrqCallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX0_DONE_IRQ_Callback);
  
  if (stRxIrqEnable.rx1Done == APP_TRUE)
    APP_IRQ_RegisterIrqCallback(EN_IRQENTRY_UWB_RX1_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX1_DONE_IRQ_Callback);
  
  if (stRxIrqEnable.rx2Done == APP_TRUE)
    APP_IRQ_RegisterIrqCallback(EN_IRQENTRY_UWB_RX2_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX2_DONE_IRQ_Callback);
>>>>>>> KwanYeu/renameVariable
  
  logSettings.enRxPort = PacketConfig->enRxPort;
  logSettings.logOpt = PacketConfig->logOpt;
  logSettings.cycleIdx = 0;
  logSettings.rxOkCnt = 0;
  while (1) {
    CB_COMMTRX_UwbReceive(PacketConfig->enRxPort, stRxIrqEnable);
    while (s_rx_done != APP_TRUE && s_stopPeriodicRx != APP_TRUE) {}
    if (logSettings.cycleIdx >= s_num_receive || s_stopPeriodicRx == APP_TRUE) {break;}
    app_uwb_periodicrx_log(&logSettings);
    s_rx_done = APP_FALSE;
  }
  
  app_uwb_periodicrx_print("Packet Received: %d\n", logSettings.cycleIdx);
  app_uwb_periodicrx_print("Packet Received OK: %d\n", logSettings.rxOkCnt);

<<<<<<< HEAD
  if (rxIrqEnable.rx0Done == CB_TRUE)
      app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX0_DONE_IRQ_Callback);
    
  if (rxIrqEnable.rx1Done == CB_TRUE)
      app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX1_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX1_DONE_IRQ_Callback);
    
  if (rxIrqEnable.rx2Done == CB_TRUE)
      app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX2_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX2_DONE_IRQ_Callback);
=======
  if (stRxIrqEnable.rx0Done == CB_TRUE)
      APP_IRQ_DeregisterIrqCallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX0_DONE_IRQ_Callback);
    
  if (stRxIrqEnable.rx1Done == CB_TRUE)
      APP_IRQ_DeregisterIrqCallback(EN_IRQENTRY_UWB_RX1_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX1_DONE_IRQ_Callback);
    
  if (stRxIrqEnable.rx2Done == CB_TRUE)
      APP_IRQ_DeregisterIrqCallback(EN_IRQENTRY_UWB_RX2_DONE_APP_IRQ, APP_UWB_PeriodicRx_RX2_DONE_IRQ_Callback);
>>>>>>> KwanYeu/renameVariable
    
  if (s_stopPeriodicRx) {
    CB_SYSTEM_RxOff(logSettings.enRxPort);
  }
}

/**
 * @brief Handler for rx0 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void APP_UWB_PeriodicRx_RX0_DONE_IRQ_Callback(void) 
{ 
  s_rx_done = APP_TRUE;
}

/**
 * @brief Handler for rx1 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void APP_UWB_PeriodicRx_RX1_DONE_IRQ_Callback(void) 
{ 
  s_rx_done = APP_TRUE; 
}

/**
 * @brief Handler for rx2 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void APP_UWB_PeriodicRx_RX2_DONE_IRQ_Callback(void) 
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
  cb_uwbsystem_rxstatus_un statusRegister = cb_system_uwb_get_rx_status();

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
    case   EN_UWB_RX_ALL:  break;
    default:  break;
  }
  
  uint8_t regStatOk = (rx_ok == APP_TRUE) && (sfd_det == APP_TRUE) && (pd_det == APP_TRUE);
  if (regStatOk) {
    ++LogSettings->rxOkCnt;
  } else {
    app_uwb_periodicrx_print("Cycle_idx: %d, RX%d_ok: %d\n", LogSettings->cycleIdx, rxPortNum, regStatOk);
  }
  ++LogSettings->cycleIdx;
  
  if (LogSettings->logOpt >= 1) {
    cb_uwbsystem_rx_cir_iqdata_st cirRegisterData[256];
    
    cb_system_uwb_store_rx_cir_register(cirRegisterData, EN_UWB_RX_0, 0, 256);
    app_uwb_periodicrx_print("I: ");
    for (uint32_t i = 0; i < 256; ++i) {
      if (i) {
        app_uwb_periodicrx_print(", %d", cirRegisterData[i].I_data);
      } else {
        app_uwb_periodicrx_print("%d", cirRegisterData[i].I_data);
      }
    }
    app_uwb_periodicrx_print("\nQ: ");
    for (uint32_t i = 0; i < 256; ++i) {
      if (i) {
        app_uwb_periodicrx_print(", %d", cirRegisterData[i].Q_data);
      } else {
        app_uwb_periodicrx_print("%d", cirRegisterData[i].Q_data);
      }
    }
    app_uwb_periodicrx_print("\n");
  }
  
  if (LogSettings->logOpt >= 2) 
  {
    cb_uwbsystem_rx_etc_statusregister_st etcStatusRegister;
    cb_system_uwb_get_rx_etc_status_register(&etcStatusRegister);
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

/**
 * @brief Stop the Periodic RX and flip the flag
 */
void APP_UWB_PeriodicRX_Stop(void) 
{
  s_stopPeriodicRx = APP_TRUE;
}
