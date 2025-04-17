/**
 * @file    AppUwbCommTx.c
 * @brief   [UWB] Communication Transceiver (UWB) Module
 * @details This module provides functions to initialize, transmit, and receive data using the UWB communication transceiver.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbCommTx.h"
#include "CB_uwbframework.h"
#include "AppUwbTRXMemoryPool.h"
#include "NonLIB_sharedUtils.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_COMMTX_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_COMMTX_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_commtx_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_commtx_print(...)
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

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_commtx_print_tx_timestamp(void);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint8_t s_simpleTxDoneFlag             = APP_FALSE;
static app_uwbcomtx_state_en s_appCommTxState = EN_APP_STATE_IDLE;

/* Default Rx packet configuration.*/
static cb_uwbsystem_packetconfig_st Txpacketconfig = {
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
void app_commtx_qmode(void)
{  
  app_uwb_commtx_print("APP_COMM_TX Quick Mode\n");  
  
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

  //--------------------------------
  // TRX Quick Switch Mode - Start
  //--------------------------------
  cb_framework_uwb_qmode_trx_enable();  
  
  s_appCommTxState = EN_APP_STATE_TRANSMIT;
  uint32_t startTime = 0;
  while(1)
  {
    switch (s_appCommTxState)
    {
      case EN_APP_STATE_IDLE:
        if(cb_hal_is_time_elapsed(startTime, 500) == CB_PASS)   // Delay 500ms
        {
          s_appCommTxState = EN_APP_STATE_TRANSMIT;
        }
        break;
      case EN_APP_STATE_TRANSMIT:
        cb_framework_uwb_qmode_tx_start(&Txpacketconfig, &txPayload, &stTxIrqEnable);  // TX START
        s_appCommTxState = EN_APP_STATE_WAIT_TX_DONE;
        break;
      case EN_APP_STATE_WAIT_TX_DONE:
        if (s_simpleTxDoneFlag)
        {
          s_simpleTxDoneFlag = APP_FALSE;
          app_commtx_print_tx_timestamp();
          cb_framework_uwb_qmode_tx_end();                        // TX END
          s_appCommTxState = EN_APP_STATE_IDLE;
          startTime = cb_hal_get_tick();
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
 * @brief Transmits a UWB packet using the appropriate mode.
 * 
 * This function configures and transmits a UWB packet based on the current packet mode 
 * (either BPRF or HPRF). It initializes the packet configuration structure, sets the 
 * payload data accordingly, and transmits the packet via the UWB transceiver. 
 */
void app_commtx_nmode(void)
{  
  app_uwb_commtx_print("APP_COMM_TX Normal Mode\n");  
  
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

  //--------------------------------
  // TRX Normal Mode - Start
  //--------------------------------
  s_appCommTxState = EN_APP_STATE_TRANSMIT;
  uint32_t startTime = 0;
  while(1)
  {
    switch (s_appCommTxState)
    {
      case EN_APP_STATE_IDLE:
        if(cb_hal_is_time_elapsed(startTime, 500) == CB_PASS)   // Delay 500ms
        {
          s_appCommTxState = EN_APP_STATE_TRANSMIT;
        }
        break;
      case EN_APP_STATE_TRANSMIT:
        cb_framework_uwb_tx_start(&Txpacketconfig, &txPayload, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);  // TX START
        s_appCommTxState = EN_APP_STATE_WAIT_TX_DONE;
        break;
      case EN_APP_STATE_WAIT_TX_DONE:
        if (s_simpleTxDoneFlag)
        {
          s_simpleTxDoneFlag = APP_FALSE;
          app_commtx_print_tx_timestamp();
          cb_framework_uwb_tx_end();                        // TX END
          s_appCommTxState = EN_APP_STATE_IDLE;
          startTime = cb_hal_get_tick();
        }
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Prints the TX timestamp information.
 * 
 * This function retrieves the TX timestamp information and prints it via UART.
 */
void app_commtx_print_tx_timestamp(void)
{
  cb_uwbsystem_tx_timestamp_st txTimestamp;  
  cb_framework_uwb_get_tx_raw_timestamp(&txTimestamp);
  
  app_uwb_commtx_print("APP_UWB_TSTP_TX_DONE_IRQ_Callback(): TX timestamp\n");
  app_uwb_commtx_print(" > txStart  - %uns\n", txTimestamp.txStart);
  app_uwb_commtx_print(" > sfdMark  - %uns\n", txTimestamp.sfdMark);
  app_uwb_commtx_print(" > sts1Mark - %uns\n", txTimestamp.sts1Mark);
  app_uwb_commtx_print(" > sts2Mark - %uns\n", txTimestamp.sts2Mark);
  app_uwb_commtx_print(" > txDone   - %uns\n", txTimestamp.txDone);
}

/**
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered.
 */
void cb_uwbapp_tx_done_irqhandler(void)
{ 
  s_simpleTxDoneFlag = APP_TRUE;
}

/**
 * @brief Callback function for the UWB TX SFD Mark IRQ.
 * 
 * This function is called when the UWB TX SFD Mark IRQ is triggered. It fetches
 * the timestamp related to the SFD Mark event and prints it via UART.
 */
void cb_uwbapp_tx_sfd_mark_irqhandler(void)
{ 
  cb_uwbsystem_tx_timestamp_st txTimestamp;
  cb_framework_uwb_get_tx_raw_timestamp(&txTimestamp);
  
  app_uwb_commtx_print("APP_UWB_COMMTRX_TX_SFD_MARK_IRQ_Callback(timestamp - %u)\n", txTimestamp.sfdMark);
}
