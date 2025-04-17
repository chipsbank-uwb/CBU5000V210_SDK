/**
 * @file    AppUwbCommTrx.c
 * @brief   [UWB] Communication Transceiver (UWB) Module
 * @details This module provides functions to initialize, transmit, and receive data using the UWB communication transceiver.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbCommTrx.h"
#include "CB_uwbframework.h"
#include "AppSysIrqCallback.h"
#include "AppUwbTRXMemoryPool.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_COMMTRX_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_COMMTRX_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_commtrx_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_commtrx_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum{
  EN_APP_STATE_IDLE = 0,
  EN_APP_STATE_TRANSMIT,
  EN_APP_STATE_WAIT_TX_DONE,
} app_uwbcomtx_state_en;

typedef enum{
  EN_APP_STATE_RECEIVE,
  EN_APP_STATE_WAIT_RX_DONE,
} app_uwbcomtrx_state_en;
//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_commtrx_print_tx_timestamp(void);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint8_t s_simpleTxDoneFlag           = APP_FALSE;
static app_uwbcomtx_state_en s_appCommTxState = EN_APP_STATE_IDLE;
static uint8_t s_simpleRxDoneFlag = APP_FALSE;
static app_uwbcomtrx_state_en s_appCommRxState = EN_APP_STATE_RECEIVE;

/* Default Tx packet configuration.*/
static cb_uwbsystem_packetconfig_st Txpacketconfig = 
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
/**
 * @brief Transmits a UWB packet using the appropriate mode.
 * 
 * This function configures and transmits a UWB packet based on the current packet mode 
 * (either BPRF or HPRF). It initializes the packet configuration structure, sets the 
 * payload data accordingly, and transmits the packet via the UWB transceiver. 
 */
void app_uwb_commtx_main(void)
{  
  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();
  
  //--------------------------------
  // Configure payload
  //--------------------------------
  cb_uwbsystem_txpayload_st txPayload = {0};
  
  if (Txpacketconfig.prfMode == EN_PRF_MODE_BPRF || Txpacketconfig.prfMode == EN_PRF_MODE_LG4A_0P85)
  {
    static uint8_t bprf_payload[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
    txPayload.ptrAddress = &bprf_payload[0];
    txPayload.payloadSize = sizeof(bprf_payload);     
  }
  else if (Txpacketconfig.prfMode == EN_PRF_MODE_HPRF_124P8 || Txpacketconfig.prfMode == EN_PRF_MODE_HPRF_249P6)
  {
    #define DEF_HPRF_TX_SIZE 2000
    static uint8_t hprf_payload[DEF_HPRF_TX_SIZE] = {0x00};
    memset(hprf_payload, 0x01, sizeof(hprf_payload));
    txPayload.ptrAddress = &hprf_payload[0];
    txPayload.payloadSize = sizeof(hprf_payload);
  }
  
  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // TX IRQ
  cb_uwbsystem_tx_irqenable_st stTxIrqEnable;
  stTxIrqEnable.txDone  = CB_TRUE;
  stTxIrqEnable.sfdDone = CB_FALSE;  
  
  if (stTxIrqEnable.txDone  == CB_TRUE)  app_irq_register_irqcallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ,      app_uwb_commtrx_tx_done_irq_callback);
  if (stTxIrqEnable.sfdDone == CB_TRUE)  app_irq_register_irqcallback(EN_IRQENTRY_UWB_TX_SFD_MARK_APP_IRQ,  app_uwb_commtrx_tx_sfd_mark_irq_callback);

  //--------------------------------
  // TRX Quick Switch Mode - Start
  //--------------------------------
  cb_framework_uwb_qmode_trx_enable();  
  
  s_appCommTxState = EN_APP_STATE_TRANSMIT;
  uint32_t startTime = 0;
  
  uint8_t appCommTxOnceDone = APP_FALSE;
  
  while(!appCommTxOnceDone)
  {

    switch (s_appCommTxState)
    {
      case EN_APP_STATE_IDLE:
        s_appCommTxState = EN_APP_STATE_TRANSMIT;
        break;
      case EN_APP_STATE_TRANSMIT:
        cb_framework_uwb_qmode_tx_start(&Txpacketconfig, &txPayload, &stTxIrqEnable);  // TX START
        s_appCommTxState = EN_APP_STATE_WAIT_TX_DONE;
        break;
      case EN_APP_STATE_WAIT_TX_DONE:
        if (s_simpleTxDoneFlag)
        {
          s_simpleTxDoneFlag = APP_FALSE;
          app_commtrx_print_tx_timestamp();
          cb_framework_uwb_qmode_tx_end();                        // TX END
          s_appCommTxState = EN_APP_STATE_IDLE;
          appCommTxOnceDone = APP_TRUE;
        }
        break;
      default:
        break;
    }
  }
  //--------------------------------
  // TRX Quick Switch Mode - End
  //--------------------------------
  //cb_framework_uwb_qmode_trx_disable();
}

void app_commtrx_print_tx_timestamp(void)
{
  cb_uwbsystem_tx_timestamp_st txTimestamp;  
  cb_framework_uwb_get_tx_raw_timestamp(&txTimestamp);
  
  app_uwb_commtrx_print("app_uwb_commtrx_tx_done_irq_callback(): TX timestamp\n");
  app_uwb_commtrx_print(" > txStart  - %uns\n", txTimestamp.txStart);
  app_uwb_commtrx_print(" > sfdMark  - %uns\n", txTimestamp.sfdMark);
  app_uwb_commtrx_print(" > sts1Mark - %uns\n", txTimestamp.sts1Mark);
  app_uwb_commtrx_print(" > sts2Mark - %uns\n", txTimestamp.sts2Mark);
  app_uwb_commtrx_print(" > txDone   - %uns\n", txTimestamp.txDone);
  
  app_irq_deregister_irqcallback (EN_IRQENTRY_UWB_TX_DONE_APP_IRQ, app_uwb_commtrx_tx_done_irq_callback);  
}
/**
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered. It fetches
 * the timestamp related to the TX Done event and prints it via UART.
 */
void app_uwb_commtrx_tx_done_irq_callback(void)
{ 
  s_simpleTxDoneFlag = APP_TRUE;
}

/**
 * @brief Callback function for the UWB TX SFD Mark IRQ.
 * 
 * This function is called when the UWB TX SFD Mark IRQ is triggered. It fetches
 * the timestamp related to the SFD Mark event and prints it via UART.
 */
void app_uwb_commtrx_tx_sfd_mark_irq_callback(void)
{ 
  cb_uwbsystem_tx_timestamp_st txTimestamp;
  cb_framework_uwb_get_tx_raw_timestamp(&txTimestamp);
  
  app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_TX_SFD_MARK_APP_IRQ, app_uwb_commtrx_tx_sfd_mark_irq_callback);
  app_uwb_commtrx_print("app_uwb_commtrx_tx_sfd_mark_irq_callback(timestamp - %u)\n", txTimestamp.sfdMark);
}

/**
 * @brief Receives a UWB packet using the appropriate mode.
 * 
 * This function initializes the UWB transceiver for reception and listens for an 
 * incoming UWB packet. It configures the packet settings, sets the receive port, 
 * and waits until the reception is complete.
 */
void app_uwb_commrx_main(void)
{
  cb_uwbsystem_rxport_en enRxPort        	= EN_UWB_RX_0;
  
  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // RX IRQ
  cb_uwbsystem_rx_irqenable_st stRxIrqEnable;  
  stRxIrqEnable.rx0Done       = CB_TRUE;
  stRxIrqEnable.rx0PdDone     = CB_FALSE;
  stRxIrqEnable.rx0SfdDetDone = CB_FALSE; 
  
  if (stRxIrqEnable.rx0Done       == CB_TRUE) app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ         , app_uwb_commtrx_rx0_done_irq_callback         );
  if (stRxIrqEnable.rx0PdDone     == CB_TRUE) app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_PD_DONE_APP_IRQ      , app_uwb_commtrx_rx0_pd_done_irq_callback      );
  if (stRxIrqEnable.rx0SfdDetDone == CB_TRUE) app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_SFD_DET_DONE_APP_IRQ , app_uwb_commtrx_rx0_sfd_det_done_irq_callback );

  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();

  //--------------------------------
  // TRX Quick Switch Mode - Start
  //--------------------------------
  cb_framework_uwb_qmode_trx_enable();    
  
  uint8_t appCommRxOnceDone = APP_FALSE;

  while(!appCommRxOnceDone)
  {
    switch(s_appCommRxState)
    {
      case EN_APP_STATE_RECEIVE:
          cb_framework_uwb_qmode_rx_start(EN_UWB_RX_0, &Rxpacketconfig, &stRxIrqEnable); // RX START
          s_appCommRxState = EN_APP_STATE_WAIT_RX_DONE;
        break;
      case EN_APP_STATE_WAIT_RX_DONE:
        if (s_simpleRxDoneFlag)
        {
          s_simpleRxDoneFlag = APP_FALSE;
          app_commtrx_rx_payload_and_timestamp_printout();
          cb_framework_uwb_qmode_rx_end(EN_UWB_RX_0);                  // RX END
          s_appCommRxState   = EN_APP_STATE_RECEIVE;
          appCommRxOnceDone = APP_TRUE;
        }
        break;
      default:
        break;
    }
  }
  //--------------------------------
  // TRX Quick Switch Mode - End
  //--------------------------------
  //cb_framework_uwb_qmode_trx_disable();  
}

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 * 
 * This function is called when the UWB RX0 Done IRQ is triggered. 
 */
void app_uwb_commtrx_rx0_done_irq_callback(void)
{
  s_simpleRxDoneFlag = APP_TRUE;
}

/**
 * @brief Callback function for the UWB RX0 PD Done IRQ.
 * 
 * This function is called when the UWB RX0 PD Done IRQ is triggered. 
 */
void app_uwb_commtrx_rx0_pd_done_irq_callback(void)
{
  app_uwb_commtrx_print("app_uwb_commtrx_rx0_pd_done_irq_callback()\n");
  app_irq_deregister_irqcallback (EN_IRQENTRY_UWB_RX0_PD_DONE_APP_IRQ, app_uwb_commtrx_rx0_pd_done_irq_callback);  
}

/**
 * @brief Callback function for the UWB RX0 SFD Detection Done IRQ.
 * 
 * This function is called when the UWB RX0 SFD Detection Done IRQ is triggered.
 */
void app_uwb_commtrx_rx0_sfd_det_done_irq_callback(void)
{
  app_uwb_commtrx_print("app_uwb_commtrx_rx0_sfd_det_done_irq_callback()\n");  
  app_irq_deregister_irqcallback (EN_IRQENTRY_UWB_RX0_SFD_DET_DONE_APP_IRQ, app_uwb_commtrx_rx0_sfd_det_done_irq_callback);  
}

/**
 * @brief Prints payload information for the UWB RX events.
 * 
 * This function fetches timestamps for various UWB RX events and prints them
 * via UART. It also prints the received payload size and the payload data.
 */
void app_commtrx_rx_payload_and_timestamp_printout(void)
{  
  app_uwb_commtrx_print("app_uwb_commtrx_rx0_done_irq_callback() ");
  
  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();
  
  if ((rxStatus.rx0_ok     == CB_TRUE) &&   \
      (rxStatus.sfd0_det   == CB_TRUE) &&  \
     ( rxStatus.pd0_det    == CB_TRUE))  {  app_uwb_commtrx_print("- status register: OK\n");     }
  else                                   { app_uwb_commtrx_print("- status register: NOT OK\n"); }  
  
  app_uwb_commtrx_print("--- Payload: ---\n");
  uint16_t rxPayloadSize = 0;
  cb_framework_uwb_get_rx_payload(&g_UWB_RXBANKMEMORY[0], &rxPayloadSize, &Rxpacketconfig);
  
  app_uwb_commtrx_print("payload size - %d\n", rxPayloadSize);
  app_uwb_commtrx_print("payload content - : ");
  
  for (uint16_t i=0; i < rxPayloadSize; i++)
  {
    app_uwb_commtrx_print("%x", g_UWB_RXBANKMEMORY[i]);
  }
  app_uwb_commtrx_print("\n");
  
  
  app_uwb_commtrx_print("--- RX Timestamp: ---\n");
  cb_uwbsystem_rx_tsutimestamp_st rxTsuTimestamp;
  cb_framework_uwb_get_rx_tsu_timestamp(&rxTsuTimestamp, EN_UWB_RX_0);
  app_uwb_commtrx_print("> rxTsu %fns\n", rxTsuTimestamp.rxTsu);
  
  app_irq_deregister_irqcallback (EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, app_uwb_commtrx_rx0_done_irq_callback); 
}
