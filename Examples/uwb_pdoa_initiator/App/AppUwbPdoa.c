/**
 * @file    AppUwbPdoa.c
 * @brief   [UWB] Phase Difference of Arrival(UWB) Feature Module
 * @details This module provides functions used for determining the position of an object or a tag.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "AppUwbPdoa.h"
#include "CB_system_types.h"
#include "CB_uwbframework.h"
#include "NonLIB_sharedUtils.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_PDOA_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_PDOA_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_pdoa_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_pdoa_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_SYNC_TX_PAYLOAD_SIZE        4
#define DEF_SYNC_ACK_RX_PAYLOAD_SIZE    3

typedef enum{
  // IDLE STATE  
  EN_APP_STATE_IDLE = 0,                        // IDLE
  // SYNC STATE
  EN_APP_STATE_SYNC_TRANSMIT,
  EN_APP_STATE_SYNC_WAIT_TX_DONE,
  EN_APP_STATE_SYNC_RECEIVE,
  EN_APP_STATE_SYNC_WAIT_RX_DONE,
  // PDOA-TX STATE
  EN_APP_STATE_WAIT_RESPONDER_READY,            // Coordination with Responder
  EN_APP_STATE_PDOA_TRANSMIT,                   // PDOA-TX
  EN_APP_STATE_PDOA_WAIT_TX_DONE_N_REPEATED_TX, // PDOA-TX
  // TERMINATE STATE    
  EN_APP_STATE_TERMINATE
} app_uwbpdoa_initiatorstate_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
  volatile uint8_t TxDone;
  volatile uint8_t Rx0Done;
}app_uwbpdoa_irqstatus_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static app_uwbpdoa_initiatorstate_en  s_enAppPdoaInitiatorState   = EN_APP_STATE_IDLE;
static uint8_t                        s_countOfPdoaScheduledTx    = 0;
static app_uwbpdoa_irqstatus_st       s_stIrqStatus               = {APP_FALSE};

/* Default Tx packet configuration.*/
static cb_uwbsystem_packetconfig_st s_stUwbPacketConfig = 
{
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

//  SYNC TX Payload                                                         'S'  'Y'  'N'  'C'
static uint8_t        s_syncTxPayload [DEF_SYNC_TX_PAYLOAD_SIZE]        = {0x53,0x59,0x4E,0x43}; 
//  SYNC ACK payload                                                        'A'  'C'  'K'
static uint8_t        s_syncAckPayload[DEF_SYNC_ACK_RX_PAYLOAD_SIZE]    = {0x41,0x43,0x4B};  
static const uint8_t  s_syncAckPayloadSize                              = sizeof(s_syncAckPayload);
// PDOA TX payload                                                         '1'
static uint8_t        s_pdoaTxPayload                                   = 0x01;
  
//-------------------------------
// PDOA: INITIATOR SETUP
//-------------------------------
//-------------------------------------------------------
//    Initiator                         Responder
//     Idle                                Idle
//       |---------1. SYNC ----------------->|
//       |<------- 2. ACK  ------------------|
//       |---------3. PDOA (n cycles) ------>| 
//     Terminate                         Terminate  
//
// DEF_DSTWR_SYNC_ACK_TIMEOUT_MS       : 1 + 2
// DEF_DSTWR_APP_CYCLE_TIME_MS         : Idle
// DEF_NUMBER_OF_PDOA_REPEATED_TX      : 3 (n cycles)
// DEF_PDOA_TX_START_WAIT_TIME_MS      : 3 (wait responder enter rx)
//
//-------------------------------------------------------
#define DEF_PDOA_SYNC_ACK_TIMEOUT_MS        2 
#define DEF_PDOA_APP_CYCLE_TIME_MS          500
#define DEF_NUMBER_OF_PDOA_REPEATED_TX      5
#define DEF_PDOA_TX_START_WAIT_TIME_MS      4

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
uint8_t app_pdoa_validate_sync_ack_payload(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void app_pdoa_initiator(void)
{
  uint32_t iterationTime = 0;
  uint32_t startTime = 0;
  
  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();

  //--------------------------------
  // Configure Payload 
  //--------------------------------
  static cb_uwbsystem_txpayload_st s_stSyncPayloadPack = 
  {
    .ptrAddress  = &s_syncTxPayload[0],
    .payloadSize = sizeof(s_syncTxPayload),
  };
  static cb_uwbsystem_txpayload_st s_stPdoaTxPayload = 
  {
    .ptrAddress = &s_pdoaTxPayload,
    .payloadSize = sizeof(s_pdoaTxPayload),
  };
  
  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // SYNC TX IRQ
  static cb_uwbsystem_tx_irqenable_st s_stSyncTxIrqEnable = { CB_FALSE };  
  s_stSyncTxIrqEnable.txDone                              = CB_TRUE;
  // SYNC RX IRQ
  static cb_uwbsystem_rx_irqenable_st s_stSyncRxIrqEnable = { CB_FALSE };  
  s_stSyncRxIrqEnable.rx0Done                             = CB_TRUE;
  // PDOA TX IRQ
  static cb_uwbsystem_tx_irqenable_st s_stPdoaTxIrqEnable = { CB_FALSE };  
  s_stPdoaTxIrqEnable.txDone                              = CB_TRUE;
  
  //--------------------------------
  // Configure absolute timer for scheduled PDOA-TX start
  //--------------------------------
  static cb_uwbframework_trx_scheduledconfig_st s_stPdoaRepeatedTxConfig =
  {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0   :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_28_TX_DONE,       // tx_done  :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0     :: (ABS timer) Select absolute timer
  .timeoutValue         = 250,                          // 250us    :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_TX_START_MASK,     // tx start :: (action)    select action upon abs timeout 
  };
    
  s_enAppPdoaInitiatorState = EN_APP_STATE_SYNC_TRANSMIT;

  while(1)
  {
    switch (s_enAppPdoaInitiatorState)
    {
      //-------------------------------------
      // IDLE
      //-------------------------------------      
      case EN_APP_STATE_IDLE:
        // Wait for next cycle
        if(cb_hal_is_time_elapsed(iterationTime, DEF_PDOA_APP_CYCLE_TIME_MS) == CB_PASS)
        {
          s_enAppPdoaInitiatorState = EN_APP_STATE_SYNC_TRANSMIT;
        }
        break;
      
      //-------------------------------------
      // SYNC: TX
      //-------------------------------------
      case EN_APP_STATE_SYNC_TRANSMIT:
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &s_stSyncPayloadPack, &s_stSyncTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppPdoaInitiatorState = EN_APP_STATE_SYNC_WAIT_TX_DONE;
        break;
      case EN_APP_STATE_SYNC_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          s_enAppPdoaInitiatorState = EN_APP_STATE_SYNC_RECEIVE;
        }
        break;
        
      //-------------------------------------
      // SYNC: RX (ACK)
      //-------------------------------------
      case EN_APP_STATE_SYNC_RECEIVE:  
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &s_stSyncRxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppPdoaInitiatorState = EN_APP_STATE_SYNC_WAIT_RX_DONE;
          startTime = cb_hal_get_tick();
        break;
      case EN_APP_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_PDOA_SYNC_ACK_TIMEOUT_MS) == CB_PASS)
        {
          // if SYNC-ACK not received from Responder within 10ms, send SYNC again.
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppPdoaInitiatorState = EN_APP_STATE_SYNC_TRANSMIT;
        }
        else if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;
          uint8_t ackValidateResult = app_pdoa_validate_sync_ack_payload();
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          if (ackValidateResult == APP_TRUE)
          {
            s_enAppPdoaInitiatorState = EN_APP_STATE_WAIT_RESPONDER_READY;
            startTime = cb_hal_get_tick();
          }
          else
          {
            // if SYNC-ACK payload validation failed, send SYNC again.
            s_enAppPdoaInitiatorState = EN_APP_STATE_SYNC_TRANSMIT;
          }
        }
        break;
      
      //-------------------------------------
      // PDOA-TX
      //-------------------------------------
      case EN_APP_STATE_WAIT_RESPONDER_READY:
        if(cb_hal_is_time_elapsed(startTime, DEF_PDOA_TX_START_WAIT_TIME_MS) == CB_PASS)
        {
          s_enAppPdoaInitiatorState = EN_APP_STATE_PDOA_TRANSMIT;
        }
        break;
      case EN_APP_STATE_PDOA_TRANSMIT:
        cb_framework_uwb_enable_scheduled_trx(s_stPdoaRepeatedTxConfig);
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &s_stPdoaTxPayload, &s_stPdoaTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppPdoaInitiatorState = EN_APP_STATE_PDOA_WAIT_TX_DONE_N_REPEATED_TX;
        break;
      case EN_APP_STATE_PDOA_WAIT_TX_DONE_N_REPEATED_TX:
        if (s_stIrqStatus.TxDone)
        {
          s_stIrqStatus.TxDone = APP_FALSE;  
          s_countOfPdoaScheduledTx++;          
          if (s_countOfPdoaScheduledTx <= DEF_NUMBER_OF_PDOA_REPEATED_TX)
          {
            cb_framework_uwb_configure_scheduled_trx(s_stPdoaRepeatedTxConfig);
            cb_framework_uwb_tx_restart(&s_stPdoaTxIrqEnable, EN_TRX_START_DEFERRED);
          }
          else
          {
            cb_framework_uwb_disable_scheduled_trx(s_stPdoaRepeatedTxConfig);
            cb_framework_uwb_tx_end();
            s_countOfPdoaScheduledTx  = 0;  
            startTime               = cb_hal_get_tick();
            s_enAppPdoaInitiatorState   = EN_APP_STATE_TERMINATE;  
          }
        }
        break;
      //-------------------------------------
      // Terminate
      //-------------------------------------       
      case EN_APP_STATE_TERMINATE:     
        app_uwb_pdoa_print("[PDOA TX Done]\n");
        iterationTime = cb_hal_get_tick();
        s_enAppPdoaInitiatorState = EN_APP_STATE_IDLE;
        break;
      default:
        break;
    }
  }
}

uint8_t app_pdoa_validate_sync_ack_payload(void)
{
  uint8_t  result = APP_TRUE;
  uint16_t rxPayloadSize = 0;
  uint8_t  syncAckPayloadReceived[DEF_SYNC_ACK_RX_PAYLOAD_SIZE];
  
  cb_uwbsystem_rxstatus_un unRxStatus = cb_framework_uwb_get_rx_status();
  
  if (unRxStatus.rx0_ok == CB_TRUE)
  { 
    cb_framework_uwb_get_rx_payload(&syncAckPayloadReceived[0], &rxPayloadSize, &s_stUwbPacketConfig);
    for (uint16_t i = 0; i < DEF_SYNC_ACK_RX_PAYLOAD_SIZE; i++)
    {
      if (syncAckPayloadReceived[i] != s_syncAckPayload[i])
        result = APP_FALSE;
    }
  }
  else
  {
    result = APP_FALSE;
  } 
  return result;
}

void cb_uwbapp_tx_done_irqhandler(void)
{ 
  s_stIrqStatus.TxDone = APP_TRUE;
}

void cb_uwbapp_rx0_done_irqcb(void)
{
  s_stIrqStatus.Rx0Done = APP_TRUE;
}
