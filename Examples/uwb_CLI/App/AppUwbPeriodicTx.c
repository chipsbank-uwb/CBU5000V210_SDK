/**
 * @file    AppUwbPeriodicTx.c
 * @brief   Implementation of UWB-COMMTRX: Periodic TX
 * @details This module provides functions to initialize, transmit, and receive data using the UWB communication transceiver.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbPeriodicTx.h"
#include "CB_commtrx.h"
#include "CB_system.h"
#include "AppSysIrqCallback.h"
#include "APP_common.h"
#define APP_UWB_PERIODICTRX_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_PERIODICTRX_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_periodictrx_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_periodicrx_print(...)
#endif


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
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_periodictx_timestamp_and_payload_printout(void);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static cb_uwbsystem_tx_irqenable_st stTxIrqEnable;
static uint8_t s_stopPeriodicTx = APP_FALSE;
static volatile uint8_t s_prev_tx_done = APP_FALSE;
static volatile uint8_t s_periodic_tx_on_flag = APP_FALSE;

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Stop the periodic tx session with user input.
 * @detail This flips the flag that is checked before starting the TX again.
 */
void APP_UWB_PeriodicTX_Stop(void) {
  s_stopPeriodicTx = APP_TRUE;
}

/**
 * @brief Initializes the Ultra-Wideband (UWB) transceiver for application use.
 * 
 * This function initializes the UWB transceiver for application usage by calling
 * the CB_COMMTRX_UwbInit() function.
 */
void APP_UWB_PeriodicTX_Init(void)
{
  CB_COMMTRX_UwbInit();
}

/**
 * @brief Transmits a packet using the UWB transceiver.
 * 
 * This function transmits a packet using the UWB transceiver with the provided
 * packet configuration, payload, and interrupt enable settings.
 * 
 * @param packetConfig The configuration settings for the UWB packet transmission.
 */
void app_uwb_periodic_tx_transmitpacket(const stUwbPeriodicTxPacketConfig* const packetConfig)
{  
  cb_system_uwb_init();

  const uint32_t cyclesFor500ms = (uint32_t)(0.5f * SystemCoreClock / 1000);
  
  s_stopPeriodicTx = APP_FALSE;
  
  cb_uwbsystem_txpayload_st txPayload;
  uint8_t payload[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
  txPayload.ptrAddress = &payload[0];
  txPayload.payloadSize = sizeof(payload);
  
  stTxIrqEnable.txDone  = CB_TRUE;
  stTxIrqEnable.sfdDone = CB_FALSE;
  
<<<<<<< HEAD
  if (txIrqEnable.txDone == CB_TRUE)  app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ,      APP_UWB_PeriodicTX_TX_DONE_IRQ_Callback);
  if (txIrqEnable.sfdDone == CB_TRUE)  app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_TX_SFD_MARK_APP_IRQ, APP_UWB_PeriodicTX_SFD_MARK_IRQ_Callback);
=======
  if (stTxIrqEnable.txDone == CB_TRUE)  APP_IRQ_RegisterIrqCallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ,      APP_UWB_PeriodicTX_TX_DONE_IRQ_Callback);
  if (stTxIrqEnable.sfdDone == CB_TRUE)  APP_IRQ_RegisterIrqCallback(EN_IRQENTRY_UWB_TX_SFD_MARK_APP_IRQ, APP_UWB_PeriodicTX_SFD_MARK_IRQ_Callback);
>>>>>>> KwanYeu/renameVariable
  
  CB_SYSTEM_UwbConfigureTxHwTimerInterval(packetConfig->timeInterval);
  CB_COMMTRX_UwbTransmit(txPayload, stTxIrqEnable);
  app_uwb_periodictrx_print("Start Periodic TRX with payload size: %d\n", txPayload.payloadSize);

  s_periodic_tx_on_flag = APP_TRUE;
  uint32_t startCycle = DWT->CYCCNT;
	uint32_t elapsedCycles = 0;
  
  while (s_periodic_tx_on_flag) {
    if (s_prev_tx_done) {
			uint32_t currentCycle = DWT->CYCCNT;
      elapsedCycles = (currentCycle < startCycle) ? (0xFFFFFFFF - startCycle + currentCycle + 1) :
                                                     (currentCycle - startCycle);
			if (elapsedCycles >= cyclesFor500ms) {
				app_uwb_periodictrx_print(".");
				startCycle = currentCycle;
      }
      s_prev_tx_done = APP_FALSE;
    }
  }
}

/**
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered.
 */

void APP_UWB_PeriodicTX_TX_DONE_IRQ_Callback(void)
{ 
  cb_system_uwb_configure_tx_irq(stTxIrqEnable); // TX_DONE IRQ need to be cleared/disabled, else it will keep triggering and enter IRQ endlessly.
  if (s_stopPeriodicTx == APP_TRUE) {
    app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ, APP_UWB_PeriodicTX_TX_DONE_IRQ_Callback);
    CB_SYSTEM_TxOff();
    s_periodic_tx_on_flag = APP_FALSE;
    return;
  }
  cb_system_uwb_tx_restart();
  s_prev_tx_done = APP_TRUE;
}

/**
 * @brief Callback function for the UWB TX SFD Mark IRQ.
 * 
 * This function is called when the UWB TX SFD Mark IRQ is triggered. It fetches
 * the timestamp related to the SFD Mark event and prints it via UART.
 */
void APP_UWB_PeriodicTX_SFD_MARK_IRQ_Callback(void)
{ 
  app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_TX_SFD_MARK_APP_IRQ, APP_UWB_PeriodicTX_SFD_MARK_IRQ_Callback);
}
