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

#include "AppSysIrqCallback.h"
#include "APP_common.h"
#include "CB_uwbframework.h"
#define APP_UWB_PERIODICTRX_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_PERIODICTRX_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_periodictrx_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_periodictrx_print(...)
#endif

/* Default Rx packet configuration.*/
static cb_uwbsystem_packetconfig_st Txpacketconfig = {
  .prfMode            = EN_PRF_MODE_BPRF_62P4,                 // PRF mode selection
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
static volatile uint8_t s_prev_tx_done = APP_FALSE;
static volatile uint8_t s_periodic_tx_on_flag = APP_FALSE;
static uint32_t b_send_packet_total = 0;
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void app_periodic_tx(void)
{
  app_uwb_periodic_tx_transmitpacket();
}



/**
 * @brief Transmits a packet using the UWB transceiver.
 * 
 * This function transmits a packet using the UWB transceiver with the provided
 * packet configuration, payload, and interrupt enable settings.
 * 
 * @param void
 */
void app_uwb_periodic_tx_transmitpacket(void)
{  
  cb_framework_uwb_init();

  cb_uwbsystem_txpayload_st txPayload;
  uint8_t payload[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
  txPayload.ptrAddress = &payload[0];
  txPayload.payloadSize = sizeof(payload);
  
  stTxIrqEnable.txDone  = CB_TRUE;
  stTxIrqEnable.sfdDone = CB_FALSE;
  
  cb_framework_uwb_tx_start(&Txpacketconfig, &txPayload, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);  // TX START
  app_uwb_periodictrx_print("Start Periodic TRX with payload size: %d\n", txPayload.payloadSize);
  
  s_periodic_tx_on_flag = APP_TRUE;
  b_send_packet_total = 0;
  
  while(s_periodic_tx_on_flag) 
  {
    if(s_prev_tx_done)
    {
      cb_framework_uwb_tx_end();                        // TX END
      b_send_packet_total++;
      app_uwb_periodictrx_print("packet index:%d\n",b_send_packet_total);
      cb_framework_uwb_tx_start(&Txpacketconfig, &txPayload, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);  // TX RESTART
      s_prev_tx_done = APP_FALSE;
    }
  }
}

/**
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered.
 */

void cb_uwbapp_tx_done_irqhandler(void)
{ 
  s_prev_tx_done = APP_TRUE;
}

