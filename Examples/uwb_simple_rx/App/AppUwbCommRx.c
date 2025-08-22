/**
 * @file    AppUwbCommRx.c
 * @brief   [UWB] Communication Transceiver (UWB) Module
 * @details This module provides functions to initialize, transmit, and receive data using the UWB communication transceiver.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbCommRx.h"
#include "CB_uwbframework.h"
#include "AppUwbTRXMemoryPool.h"
#include "NonLIB_sharedUtils.h"
//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_COMMRX_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_COMMRX_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_commrx_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_commrx_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_SIMPLE_RX_TIMEOUT_MS   600 

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum{
  EN_APP_STATE_RECEIVE,
  EN_APP_STATE_WAIT_RX_DONE,
} app_uwbcomrx_state_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint32_t startTime                     = 0; 
static uint8_t s_simpleRxDoneFlag             = APP_FALSE;
static app_uwbcomrx_state_en s_appCommRxState = EN_APP_STATE_RECEIVE;

/* Default Rx packet configuration.*/
static cb_uwbsystem_packetconfig_st Rxpacketconfig = {
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
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Receives a UWB packet using the appropriate mode.
 * 
 * This function initializes the UWB transceiver for reception and listens for an 
 * incoming UWB packet. It configures the packet settings, sets the receive port, 
 * and waits until the reception is complete.
 */
void app_commrx_qmode(void)
{
  app_uwb_commrx_print("APP_COMM_RX(enRxPort: 0) Quick Mode\n");
  
  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // RX IRQ
  cb_uwbsystem_rx_irqenable_st stRxIrqEnable = { CB_FALSE };   
  stRxIrqEnable.rx0Done                      = CB_TRUE;
  stRxIrqEnable.rx0PdDone                    = CB_FALSE;
  stRxIrqEnable.rx0SfdDetDone                = CB_FALSE; 

  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();

  //--------------------------------
  // TRX Quick Switch Mode - Start
  //--------------------------------
  cb_framework_uwb_qmode_trx_enable();    
  
  while(1)
  {
    switch(s_appCommRxState)
    {
      case EN_APP_STATE_RECEIVE:
          cb_framework_uwb_qmode_rx_start(&Rxpacketconfig, &stRxIrqEnable); // RX START
          startTime = cb_hal_get_tick();
          s_appCommRxState = EN_APP_STATE_WAIT_RX_DONE;
        break;
      case EN_APP_STATE_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_SIMPLE_RX_TIMEOUT_MS) == APP_TRUE)  
        {
          cb_framework_uwb_qmode_rx_end();
          s_appCommRxState = EN_APP_STATE_RECEIVE;
        }
        else if (s_simpleRxDoneFlag)
        {
          s_simpleRxDoneFlag = APP_FALSE;
          app_commrx_rx_payload_and_timestamp_printout();
          cb_framework_uwb_qmode_rx_end();                  // RX END
          s_appCommRxState   = EN_APP_STATE_RECEIVE;
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
 * @brief Receives a UWB packet using the appropriate mode.
 * 
 * This function initializes the UWB transceiver for reception and listens for an 
 * incoming UWB packet. It configures the packet settings, sets the receive port, 
 * and waits until the reception is complete.
 */
void app_commrx_nmode(void)
{
  app_uwb_commrx_print("APP_COMM_RX(enRxPort: 0) Normal Mode\n");
  
  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // RX IRQ
  cb_uwbsystem_rx_irqenable_st stRxIrqEnable = { CB_FALSE };  
  stRxIrqEnable.rx0Done       = CB_TRUE;
  stRxIrqEnable.rx0PdDone     = CB_FALSE;
  stRxIrqEnable.rx0SfdDetDone = CB_FALSE; 

  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();

  //--------------------------------
  // TRX Normal Mode - Start
  //--------------------------------  
  while(1)
  {
    switch(s_appCommRxState)
    {
      case EN_APP_STATE_RECEIVE:
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &Rxpacketconfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED); // RX START
          startTime = cb_hal_get_tick();
          s_appCommRxState = EN_APP_STATE_WAIT_RX_DONE;
        break;
      case EN_APP_STATE_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_SIMPLE_RX_TIMEOUT_MS) == APP_TRUE)  
        {
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_appCommRxState = EN_APP_STATE_RECEIVE;
        }
        else if (s_simpleRxDoneFlag)
        {
          s_simpleRxDoneFlag = APP_FALSE;
          app_commrx_rx_payload_and_timestamp_printout();
          cb_framework_uwb_rx_end(EN_UWB_RX_0);                  // RX END
          s_appCommRxState   = EN_APP_STATE_RECEIVE;
        }
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 * 
 * This function is called when the UWB RX0 Done IRQ is triggered. 
 */
void cb_uwbapp_rx0_done_irqcb(void)
{
  s_simpleRxDoneFlag = APP_TRUE;
}

/**
 * @brief Callback function for the UWB RX0 PD Done IRQ.
 * 
 * This function is called when the UWB RX0 PD Done IRQ is triggered. 
 */
void cb_uwbapp_rx0_preamble_detected_irqcb(void)
{
  app_uwb_commrx_print("APP_UWB_COMMTRX_RX0_PD_DONE_IRQ_Callback()\n");
}

/**
 * @brief Callback function for the UWB RX0 SFD Detection Done IRQ.
 * 
 * This function is called when the UWB RX0 SFD Detection Done IRQ is triggered.
 */
void cb_uwbapp_rx0_sfd_detected_irqcb(void)
{
  app_uwb_commrx_print("APP_UWB_COMMTRX_RX0_SFD_DET_DONE_IRQ_Callback()\n");  
}

/**
 * @brief Prints payload information for the UWB RX events.
 * 
 * This function fetches timestamps for various UWB RX events and prints them
 * via UART. It also prints the received payload size and the payload data.
 */
void app_commrx_rx_payload_and_timestamp_printout(void)
{  
  // max payload size to be received (based on the ex_simple_tx size)
  #define DEF_HPRF_RX_SIZE 2000 
  
  app_uwb_commrx_print("APP_UWB_COMMTRX_RX0_DONE_IRQ_Callback() ");
  
  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();
  
  if ((rxStatus.rx0_ok == CB_TRUE) && (rxStatus.sfd0_det == CB_TRUE) && (rxStatus.pd0_det == CB_TRUE)) 
  {  
    app_uwb_commrx_print("- status register: OK\n");
      
    app_uwb_commrx_print("--- Payload: ---\n");
    uint16_t rxPayloadSize = cb_framework_uwb_get_rx_packet_size(&Rxpacketconfig);
    
    static uint8_t s_payload[DEF_HPRF_RX_SIZE] = {0};
    cb_framework_uwb_get_rx_payload(&s_payload[0], rxPayloadSize);
    
    app_uwb_commrx_print("payload size - %d\n", rxPayloadSize);
    app_uwb_commrx_print("payload content - : ");
    
    for (uint16_t i=0; i < rxPayloadSize; i++)
    {
      app_uwb_commrx_print("%x", s_payload[i]);
    }
    app_uwb_commrx_print("\n");

    app_uwb_commrx_print("--- RX Timestamp: ---\n");
    cb_uwbsystem_rx_tsutimestamp_st rxTsuTimestamp;
    cb_framework_uwb_get_rx_tsu_timestamp(&rxTsuTimestamp, EN_UWB_RX_0);
    app_uwb_commrx_print("> rxTsu %fns\n", rxTsuTimestamp.rxTsu);
  }
  else                
  { 
    app_uwb_commrx_print("- status register: NOT OK\n"); 
  }  
}

