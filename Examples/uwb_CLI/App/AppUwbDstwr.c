/**
 * @file    AppDSTWR.c
 * @brief   
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "AppUwbDstwr.h"
#include "AppSysIrqCallback.h"
#include "CB_timer.h"
#include "CB_scr.h"
#include "NonLIB_sharedUtils.h"
#include "CB_uwbframework.h"
#include <string.h>

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_DSTWR_USE_ABSOLUTE_TIMER   APP_TRUE
#define APP_UWB_DSTWR_UARTPRINT_ENABLE APP_TRUE

#if (APP_UWB_DSTWR_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_dstwr_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_dstwr_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_DSTWR_ENABLE_LOG           APP_TRUE
#define DEF_INITIATOR_RANGING_BIAS     0

#define DEF_SYNC_TX_PAYLOAD_SIZE       4
#define DEF_SYNC_ACK_RX_PAYLOAD_SIZE   3

#define DEF_RESPONDER_RANGING_BIAS     0

#define DEF_SYNC_RX_PAYLOAD_SIZE       4
#define DEF_SYNC_ACK_TX_PAYLOAD_SIZE   3


//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum{
  //IDLE STATE
  EN_APP_INI_STATE_IDLE = 0,
  //SYNC STATE
  EN_APP_INI_STATE_SYNC_TRANSMIT,
  EN_APP_INI_STATE_SYNC_WAIT_TX_DONE,
  EN_APP_INI_STATE_SYNC_RECEIVE,
  EN_APP_INI_STATE_SYNC_WAIT_RX_DONE,
  //DSTWR STATE
  EN_APP_INI_STATE_DSTWR_TRANSMIT_POLL,
  EN_APP_INI_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE,
  EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE,
  EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE,
  EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL,
  EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE,
  //RESULT SHARING STATE
  EN_APP_INI_STATE_RESULT_RECEIVE,
  EN_APP_INI_STATE_RESULT_WAIT_RX_DONE,
  //TERMINATE STATE
  EN_APP_INI_STATE_TERMINATE,
} app_uwbdstwr_initiatorstate_en;

typedef enum{
  //IDLE STATE
  EN_APP_RESP_STATE_IDLE = 0,
  //SYNC STATE  
  EN_APP_RESP_STATE_SYNC_RECEIVE,
  EN_APP_RESP_STATE_SYNC_WAIT_RX_DONE,
  EN_APP_RESP_STATE_SYNC_TRANSMIT,
  EN_APP_RESP_STATE_SYNC_WAIT_TX_DONE,
  //DSTWR STATE  
  EN_APP_RESP_STATE_DSTWR_RECEIVE_POLL,
  EN_APP_RESP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE,
  EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE,
  EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE,
  EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL,
  EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE,
  //RESULT SHARING STATE  
  EN_APP_RESP_STATE_RESULT_TRANSMIT,
  EN_APP_RESP_STATE_RESULT_WAIT_TX_DONE,
  //TERMINATE STATE  
  EN_APP_RESP_STATE_TERMINATE,
} app_uwbdstwr_responderstate_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
  volatile uint8_t TxDone;
  volatile uint8_t Rx0Done;
}app_uwbdstwr_irqstatus_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint8_t s_dstwrRunningFlag = APP_FALSE;

/* Default uwb packet configuration.*/
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

static app_uwbdstwr_irqstatus_st s_stIrqStatus = { APP_FALSE };
static app_uwbdstwr_initiatorstate_en s_enAppDstwrInitiatorState   = EN_APP_INI_STATE_IDLE;
static app_uwbdstwr_initiatorstate_en s_enAppFailureInitiatorState = EN_APP_INI_STATE_IDLE;

//  SYNC TX Payload                                       'S'  'Y'  'N'  'C'
static uint8_t s_syncTxPayload[DEF_SYNC_TX_PAYLOAD_SIZE] = {0x53,0x59,0x4E,0x43};
//  SYCN ACK payload                                           'A'  'C'  'K'
static uint8_t s_syncAckRxPayload[DEF_SYNC_ACK_RX_PAYLOAD_SIZE] = {0x41,0x43,0x4B};

static uint8_t s_applicationTimeout = APP_FALSE;

static cb_uwbsystem_rx_tsutimestamp_st      s_stIniRxTsuTimestamp0;
static cb_uwbsystem_tx_tsutimestamp_st      s_stIniTxTsuTimestamp0;
static cb_uwbsystem_tx_tsutimestamp_st      s_stIniTxTsuTimestamp1;
static cb_uwbsystem_rangingtroundtreply_st  s_stInitiatorTround1Treply2;
static cb_uwbframework_rangingdatacontainer_st  s_stInitiatorDataContainer = 
{
  .dstwrRangingBias = DEF_INITIATOR_RANGING_BIAS,
  .dstwrTroundTreply = {0}
};
static cb_uwbframework_rangingdatacontainer_st  s_stIniResponderDataContainer = {0};

static double s_measuredDistance = 0.0; // Measured Distance
static uint32_t s_appCycleCount  = 0;   // Logging Purpose: cycle count

//-------------------------------
// DS-TWR: INITIATOR SETUP
//-------------------------------
//-------------------------------------------------------
//    Initiator                         Responder
//     Idle                                Idle
//       |---------1. SYNC ----------------->|
//       |<------- 2. ACK  ------------------|
//     a |---------3. DSTWR(POLL) ---------->| d
//     b |<--------4. DSTWR(RESPONSE) -------| e
//     c |---------5. DSTWR(FINAL) --------->| f
//       |<--------6. RESULT ----------------|
//     Terminate                         Terminate  
//
// DEF_DSTWR_INI_SYNC_ACK_TIMEOUT_MS       : 1 + 2
// DEF_DSTWR_INI_OVERALL_PROCESS_TIMEOUT_MS: 3 + 4 + 5 + 6
// DEF_DSTWR_INI_APP_CYCLE_TIME_MS         : Idle
// DEF_DSTWR_INI_POLL_WAIT_TIME_MS         : 3
// DEF_DSTWR_INI_RESPONSE_WAIT_TIME_MS     : 4
// DEF_DSTWR_INI_FINAL_WAIT_TIME_MS        : 5
//
// Initiator: Tround_1 = b - a
//            Treply_2 = c - d     
// Responder: Treply_1 = e - d
//            Tround_2 = f - e
//
//  a: s_stIniTxTsuTimestamp0    d,e,f:   s_stResponderDataContainer.dstwrTroundTreply
//  b: s_stIniRxTsuTimestamp0    
//  c: s_stIniTxTsuTimestamp1    
//-------------------------------------------------------
#define DEF_DSTWR_INI_SYNC_ACK_TIMEOUT_MS           10 
#define DEF_DSTWR_INI_OVERALL_PROCESS_TIMEOUT_MS    10 
#define DEF_DSTWR_INI_APP_CYCLE_TIME_MS             500
#define DEF_DSTWR_INI_POLL_WAIT_TIME_MS             1
#define DEF_DSTWR_INI_RESPONSE_WAIT_TIME_MS         0
#define DEF_DSTWR_INI_FINAL_WAIT_TIME_MS            1 


static app_uwbdstwr_responderstate_en appDstwrResponderState   = EN_APP_RESP_STATE_IDLE;
static app_uwbdstwr_responderstate_en appFailureResponderState = EN_APP_RESP_STATE_IDLE;

//  SYNC RX Payload                 'S'  'Y'  'N'  'C'
static uint8_t s_syncExpectedRxPayload[4] = {0x53,0x59,0x4E,0x43};
//  SYCN ACK payload                 'A'  'C'  'K'
static uint8_t s_syncAckPayload[DEF_SYNC_ACK_TX_PAYLOAD_SIZE] = {0x41,0x43,0x4B};
static const uint8_t s_syncAckPayloadSize = sizeof(s_syncAckPayload);

static cb_uwbsystem_txpayload_st s_stResultTxPayload;

static cb_uwbsystem_tx_tsutimestamp_st  s_stRespTxTsuTimestamp0;
static cb_uwbsystem_rx_tsutimestamp_st  s_stRespRxTsuTimestamp0;
static cb_uwbsystem_rx_tsutimestamp_st  s_stRespRxTsuTimestamp1;
static cb_uwbframework_rangingdatacontainer_st s_stRespResponderDataContainer = 
{
  .dstwrRangingBias = DEF_RESPONDER_RANGING_BIAS,
  .dstwrTroundTreply = {0}
};

//-------------------------------
// DS-TWR: RESPONDER SETUP
//-------------------------------
//-------------------------------------------------------
//    Initiator                         Responder
//     Idle                                Idle
//       |---------1. SYNC ----------------->|
//       |<------- 2. ACK  ------------------|
//     a |---------3. DSTWR(POLL) ---------->| d
//     b |<--------4. DSTWR(RESPONSE) -------| e
//     c |---------5. DSTWR(FINAL) --------->| f
//       |<--------6. RESULT ----------------|
//     Terminate                         Terminate  
//
// DEF_DSTWR_OVERALL_PROCESS_TIMEOUT_MS : 3 + 4 + 5 + 6
// DEF_DSTWR_SYNC_RX_RESTART_TIMEOUT_MS : 1
// DEF_DSTWR_APP_CYCLE_TIME_MS          : Idle 
// DEF_DSTWR_RESPONSE_WAIT_TIME_MS      : 4 
// DEF_DSTWR_FINAL_WAIT_TIME_MS         : 5 
// DEF_DSTWR_RESULT_WAIT_TIME_MS        : 6
//
// Initiator: Tround_1 = b - a
//            Treply_2 = c - d     
// Responder: Treply_1 = e - d
//            Tround_2 = f - e
//
//  a: -        d: s_stRespRxTsuTimestamp0
//  b: -        e: s_stRespTxTsuTimestamp0
//  c: -        f: s_stRespRxTsuTimestamp1
//-------------------------------------------------------
#define DEF_DSTWR_OVERALL_PROCESS_TIMEOUT_MS    10 
#define DEF_DSTWR_SYNC_RX_RESTART_TIMEOUT_MS    4
#define DEF_DSTWR_APP_CYCLE_TIME_MS             498
#define DEF_DSTWR_RESPONSE_WAIT_TIME_MS         1
#define DEF_DSTWR_FINAL_WAIT_TIME_MS            0
#define DEF_DSTWR_RESULT_WAIT_TIME_MS           1

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_dstwr_initiator_reset(void);
void app_dstwr_initiator_timeout_error_message_print(void);
uint8_t app_dstwr_initiator_validate_sync_ack_payload(void);
void app_dstwr_initiator_log(void);

void app_dstwr_responder_reset(void);
void app_dstwr_responder_timeout_error_message_print(void);
uint8_t app_dstwr_responder_validate_sync_payload(void);
void app_dstwr_responder_log(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void app_dstwr_initiator(void)
{  
  uint32_t startTime      = 0;
  uint32_t iterationTime  = 0;  
  
  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();
  
  //--------------------------------
  // Configure Payload
  //--------------------------------
  cb_uwbsystem_txpayload_st stSyncTxPayloadPack = {0};
  cb_uwbsystem_txpayload_st stDstwrTxPayloadPack = {0};
  
  stSyncTxPayloadPack.ptrAddress    = &s_syncTxPayload[0];
  stSyncTxPayloadPack.payloadSize   = sizeof(s_syncTxPayload);
  //  DSTWR Payload
  static uint8_t s_dstwrPayload[1]  = {0x1};
  stDstwrTxPayloadPack.ptrAddress   = &s_dstwrPayload[0];
  stDstwrTxPayloadPack.payloadSize  = sizeof(s_dstwrPayload);

  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // TX IRQ
  cb_uwbsystem_tx_irqenable_st stTxIrqEnable = { CB_FALSE} ;
  stTxIrqEnable.txDone  = APP_TRUE;
  // RX IRQ
  cb_uwbsystem_rx_irqenable_st stRxIrqEnable = { CB_FALSE } ;
  stRxIrqEnable.rx0Done = APP_TRUE;
  
  //--------------------------------
  // Configure absolute timer for scheduled TX start
  //--------------------------------
  static cb_uwbframework_trx_scheduledconfig_st s_stDstwrTround1Config = {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0    :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_28_TX_DONE,       // tx_done   :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0      :: (ABS timer) Select absolute timer
  .timeoutValue         = 500,                          // 500us     :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_RX0_START_MASK,    // rx0 start :: (action)    select action upon abs timeout 
  };
  
  static cb_uwbframework_trx_scheduledconfig_st s_stDstwrTreply2Config = {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0    :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_17_RX0_SFD_DET,   // rx_sfd    :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0      :: (ABS timer) Select absolute timer
  .timeoutValue         = 700,                          // 700us     :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_TX_START_MASK,     // tx start  :: (action)    select action upon abs timeout 
  };   
  
  s_enAppDstwrInitiatorState = EN_APP_INI_STATE_SYNC_TRANSMIT;
  
  app_uwb_dstwr_register_irqcallbacks();
  s_dstwrRunningFlag = APP_TRUE;
  
  while(s_dstwrRunningFlag == APP_TRUE)
  {
    switch (s_enAppDstwrInitiatorState)
    {
      //-------------------------------------
      // IDLE
      //-------------------------------------        
      case EN_APP_INI_STATE_IDLE:
        // Wait for next cycle
        if (cb_hal_is_time_elapsed(iterationTime, DEF_DSTWR_INI_APP_CYCLE_TIME_MS))
        {
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_SYNC_TRANSMIT;
        }
        break;

      //-------------------------------------
      // SYNC: TX
      //-------------------------------------        
      case EN_APP_INI_STATE_SYNC_TRANSMIT:
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stSyncTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppDstwrInitiatorState = EN_APP_INI_STATE_SYNC_WAIT_TX_DONE;
        break;
      case EN_APP_INI_STATE_SYNC_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_SYNC_RECEIVE;   
        }
        break;
        
      //-------------------------------------
      // SYNC: RX (ACK)
      //-------------------------------------        
      case EN_APP_INI_STATE_SYNC_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppDstwrInitiatorState = EN_APP_INI_STATE_SYNC_WAIT_RX_DONE;
        startTime = cb_hal_get_tick();
        break;
      case EN_APP_INI_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_INI_SYNC_ACK_TIMEOUT_MS))
        {
          // if SYNC-ACK not received from Responder within 10ms, send SYNC again.
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_SYNC_TRANSMIT;
        }
        else if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;
          uint32_t ackValidateResult = app_dstwr_initiator_validate_sync_ack_payload();
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          if (ackValidateResult == APP_TRUE)
          {
            s_enAppDstwrInitiatorState = EN_APP_INI_STATE_DSTWR_TRANSMIT_POLL;
            startTime = cb_hal_get_tick();
          }
          else
          {
            // if SYNC-ACK payload validation failed, send SYNC again.
            s_enAppDstwrInitiatorState = EN_APP_INI_STATE_SYNC_TRANSMIT;
          }
        }        
        break;
        
      //-------------------------------------
      // DS-TWR: POLL
      //-------------------------------------    
      case EN_APP_INI_STATE_DSTWR_TRANSMIT_POLL:
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_INI_POLL_WAIT_TIME_MS))
        {
          app_dstwr_timer_init(DEF_DSTWR_INI_OVERALL_PROCESS_TIMEOUT_MS);
          
          #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_enable_scheduled_trx(s_stDstwrTround1Config);
          #endif
          
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE;
        }
        break;
      case EN_APP_INI_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          
          cb_framework_uwb_get_tx_tsu_timestamp(&s_stIniTxTsuTimestamp0);
          cb_framework_uwb_tx_end();
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE;
          startTime = cb_hal_get_tick();  
        }
        break;
      //-------------------------------------
      // DS-TWR: RESPONSE
      //-------------------------------------       
      case EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE:
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_enable_scheduled_trx(s_stDstwrTreply2Config);
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTround1Config);
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_DEFERRED);

          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE;
        #else
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_INI_RESPONSE_WAIT_TIME_MS))
        {
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE;
        }
        #endif
        break;
      case EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE:      
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;
          #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTreply2Config);
          #endif
          cb_framework_uwb_get_rx_tsu_timestamp(&s_stIniRxTsuTimestamp0, EN_UWB_RX_0); // Store Rx CIR info
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL;
          startTime = cb_hal_get_tick(); 
        }
        break;
      //-------------------------------------
      // DS-TWR: FINAL
      //-------------------------------------         
      case EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL:
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_DEFERRED);
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE;
        #else
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_INI_FINAL_WAIT_TIME_MS))
        {
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE;
        }
        #endif
        break;
      case EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_disable_scheduled_trx(s_stDstwrTreply2Config);
          #endif
          cb_framework_uwb_get_tx_tsu_timestamp(&s_stIniTxTsuTimestamp1);
          cb_framework_uwb_tx_end();
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_RESULT_RECEIVE;
        }
        break;
      //-------------------------------------
      // Ranging Result: RX
      //-------------------------------------         
      case EN_APP_INI_STATE_RESULT_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppDstwrInitiatorState = EN_APP_INI_STATE_RESULT_WAIT_RX_DONE;
        break;
      case EN_APP_INI_STATE_RESULT_WAIT_RX_DONE:
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {        
          s_stIrqStatus.Rx0Done = APP_FALSE;
          
          cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();    
          if (rxStatus.rx0_ok == CB_TRUE)
          {  
            uint16_t rxPayloadSize = cb_framework_uwb_get_rx_packet_size(&s_stUwbPacketConfig);
            cb_framework_uwb_get_rx_payload                          ((uint8_t*)(&s_stIniResponderDataContainer), rxPayloadSize);
            cb_framework_uwb_calculate_initiator_tround_treply        (&s_stInitiatorDataContainer, s_stIniTxTsuTimestamp0, s_stIniTxTsuTimestamp1, s_stIniRxTsuTimestamp0);
            s_measuredDistance = cb_framework_uwb_calculate_distance  (s_stInitiatorDataContainer, s_stIniResponderDataContainer);
          }
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppDstwrInitiatorState = EN_APP_INI_STATE_TERMINATE;
        }
        break;
      //-------------------------------------
      // Terminate
      //-------------------------------------       
      case EN_APP_INI_STATE_TERMINATE:
        #if (DEF_DSTWR_ENABLE_LOG == APP_TRUE)
          app_dstwr_initiator_log();
        #endif        
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_disable_scheduled_trx(s_stDstwrTreply2Config);
        #endif
        app_dstwr_timer_off();
        app_dstwr_initiator_reset();
        iterationTime = cb_hal_get_tick();
        s_enAppDstwrInitiatorState = EN_APP_INI_STATE_IDLE;
        break;
    }
  }
  app_uwb_dstwr_deregister_irqcallbacks();
  s_appCycleCount = 0;
  #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
  cb_framework_uwb_disable_scheduled_trx(s_stDstwrTreply2Config);
  #endif
  app_dstwr_timer_off();
  app_dstwr_initiator_reset();
  cb_framework_uwb_off();
}

/**
 * @brief   Resets all member variables
 * @details This function turns on resets all the member variables
*/
void app_dstwr_initiator_reset(void)
{
  s_measuredDistance = 0.0;

  s_stIrqStatus.TxDone         = APP_FALSE;
  s_stIrqStatus.Rx0Done        = APP_FALSE;
  s_applicationTimeout       = APP_FALSE;
  s_enAppFailureInitiatorState          = EN_APP_INI_STATE_IDLE;
  memset(&s_stInitiatorDataContainer,   0, sizeof(s_stInitiatorDataContainer)); 
  memset(&s_stIniResponderDataContainer,   0, sizeof(cb_uwbframework_rangingdatacontainer_st));
  cb_framework_uwb_tsu_clear();
  cb_framework_uwb_tx_end();            // ensure propoer TX end upon abnormal condition
  cb_framework_uwb_rx_end(EN_UWB_RX_0); // ensure propoer RX end upon abnormal condition
}

void app_dstwr_responder(void)
{  
  uint32_t startTime      = 0;
  uint32_t iterationTime  = 0;  
  
  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();
  
  //--------------------------------
  // Configure Payload and IRQ
  //--------------------------------
  cb_uwbsystem_txpayload_st stSyncAckPayloadPack = {0};
  cb_uwbsystem_txpayload_st stDstwrTxPayloadPack = {0};  
  
  stSyncAckPayloadPack.ptrAddress    = &s_syncAckPayload[0];
  stSyncAckPayloadPack.payloadSize   = sizeof(s_syncAckPayload);
  //  DSTWR Payload
  static uint8_t s_dstwrPayload[1]  = {0x1};
  stDstwrTxPayloadPack.ptrAddress   = &s_dstwrPayload[0];
  stDstwrTxPayloadPack.payloadSize  = sizeof(s_dstwrPayload);  

  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // TX IRQ
  cb_uwbsystem_tx_irqenable_st stTxIrqEnable = { CB_FALSE} ;
  stTxIrqEnable.txDone  = APP_TRUE;
  // RX IRQ
  cb_uwbsystem_rx_irqenable_st stRxIrqEnable = { CB_FALSE } ;
  stRxIrqEnable.rx0Done = APP_TRUE;  
  
  //--------------------------------
  // Configure absolute timer for scheduled TX start
  //--------------------------------
  static cb_uwbframework_trx_scheduledconfig_st s_stDstwrTreply1Config = {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0    :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_17_RX0_SFD_DET,   // rx0_sfd   :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0      :: (ABS timer) Select absolute timer
  .timeoutValue         = 700,                          // 700us     :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_TX_START_MASK,     // tx start  :: (action)    select action upon abs timeout 
  };
  
  static cb_uwbframework_trx_scheduledconfig_st s_stDstwrTround2Config = {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0    :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_28_TX_DONE,       // tx_done   :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0      :: (ABS timer) Select absolute timer
  .timeoutValue         = 500,                          // 500us     :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_RX0_START_MASK,    // rx0 start :: (action)    select action upon abs timeout 
  };  
  
  appDstwrResponderState = EN_APP_RESP_STATE_SYNC_RECEIVE;
  
  app_uwb_dstwr_register_irqcallbacks();
  s_dstwrRunningFlag = APP_TRUE;
  
  while(s_dstwrRunningFlag == APP_TRUE)
  {
    switch (appDstwrResponderState)
    {
      case EN_APP_RESP_STATE_IDLE:
      //-------------------------------------
      // IDLE
      //-------------------------------------         
        // Wait for next cycle
        if (cb_hal_is_time_elapsed(iterationTime, DEF_DSTWR_APP_CYCLE_TIME_MS))
        {
          appDstwrResponderState = EN_APP_RESP_STATE_SYNC_RECEIVE;
        }
        break;
      //-------------------------------------
      // SYNC: RX
      //-------------------------------------       
      case EN_APP_RESP_STATE_SYNC_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        appDstwrResponderState = EN_APP_RESP_STATE_SYNC_WAIT_RX_DONE;
        startTime = cb_hal_get_tick();
        break;
      case EN_APP_RESP_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_SYNC_RX_RESTART_TIMEOUT_MS))
        {
          appDstwrResponderState = EN_APP_RESP_STATE_SYNC_RECEIVE;
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
        }
        else if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;
          uint32_t syncValidateResult = app_dstwr_responder_validate_sync_payload();
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          if (syncValidateResult == APP_TRUE)
          {
            appDstwrResponderState = EN_APP_RESP_STATE_SYNC_TRANSMIT;
          }
          else
          {
            // if SYNC-ACK payload validation failed, wait for SYNC-RX again.
            appDstwrResponderState = EN_APP_RESP_STATE_SYNC_RECEIVE;
          }
        }
        break;
      //-------------------------------------
      // SYNC: TX (ACK)
      //------------------------------------- 
      case EN_APP_RESP_STATE_SYNC_TRANSMIT:
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stSyncAckPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        appDstwrResponderState = EN_APP_RESP_STATE_SYNC_WAIT_TX_DONE;        
        break;
      case EN_APP_RESP_STATE_SYNC_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          appDstwrResponderState = EN_APP_RESP_STATE_DSTWR_RECEIVE_POLL; 
        }          
        break;
      //-------------------------------------
      // DS-TWR: POLL
      //-------------------------------------  
      case EN_APP_RESP_STATE_DSTWR_RECEIVE_POLL:
        app_dstwr_timer_init(DEF_DSTWR_OVERALL_PROCESS_TIMEOUT_MS);  

        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_enable_scheduled_trx(s_stDstwrTreply1Config);
        #endif
      
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        appDstwrResponderState = EN_APP_RESP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE;
        break;
      case EN_APP_RESP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE:
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {        
          s_stIrqStatus.Rx0Done = APP_FALSE;

          cb_framework_uwb_get_rx_tsu_timestamp(&s_stRespRxTsuTimestamp0, EN_UWB_RX_0); // Store Rx CIR info
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          appDstwrResponderState = EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE;
          startTime = cb_hal_get_tick();
        }
        break;
      //-------------------------------------
      // DS-TWR: RESPONSE
      //-------------------------------------            
      case EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE:
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_enable_scheduled_trx(s_stDstwrTround2Config);
      
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTreply1Config);
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_DEFERRED);
          appDstwrResponderState = EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE;
        #else
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_RESPONSE_WAIT_TIME_MS))
        {
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          appDstwrResponderState = EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE;
        }
        #endif
        break;
      case EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          
          #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTround2Config);
          #endif
          
          cb_framework_uwb_get_tx_tsu_timestamp(&s_stRespTxTsuTimestamp0);
          cb_framework_uwb_tx_end();
          appDstwrResponderState = EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL;
          startTime = cb_hal_get_tick();
        }
        break;
      //-------------------------------------
      // DS-TWR: FINAL
      //-------------------------------------         
      case EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL:
      {
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_DEFERRED);
          appDstwrResponderState = EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE;
        #else
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_FINAL_WAIT_TIME_MS))
        {
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
          appDstwrResponderState = EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE;
        }
        #endif
        break;
      }
      case EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE:
      {
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {          
          s_stIrqStatus.Rx0Done = APP_FALSE;
          
          #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_disable_scheduled_trx(s_stDstwrTround2Config);
          #endif
          
          cb_framework_uwb_get_rx_tsu_timestamp(&s_stRespRxTsuTimestamp1, EN_UWB_RX_0); // Store Rx CIR info
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          appDstwrResponderState = EN_APP_RESP_STATE_RESULT_TRANSMIT;
          startTime = cb_hal_get_tick();
        }
        break;
      }
      //-------------------------------------
      // Ranging Result: TX
      //-------------------------------------  
      case EN_APP_RESP_STATE_RESULT_TRANSMIT:
      {
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_RESULT_WAIT_TIME_MS))
        {
          cb_framework_uwb_calculate_responder_tround_treply(&s_stRespResponderDataContainer, s_stRespTxTsuTimestamp0, s_stRespRxTsuTimestamp0, s_stRespRxTsuTimestamp1);
          s_stResultTxPayload.ptrAddress = (uint8_t*)(&s_stRespResponderDataContainer.dstwrTroundTreply);
          s_stResultTxPayload.payloadSize = sizeof(cb_uwbframework_rangingdatacontainer_st);
          
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &s_stResultTxPayload, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          appDstwrResponderState = EN_APP_RESP_STATE_RESULT_WAIT_TX_DONE;
        }
        break;
      }
      case EN_APP_RESP_STATE_RESULT_WAIT_TX_DONE:
      {
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          appDstwrResponderState = EN_APP_RESP_STATE_TERMINATE;
        }
        break;
      }
      //-------------------------------------
      // Terminate
      //-------------------------------------  
      case EN_APP_RESP_STATE_TERMINATE:
      {
        #if (DEF_DSTWR_ENABLE_LOG == APP_TRUE)
          app_dstwr_responder_log();
        #endif   
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_disable_scheduled_trx(s_stDstwrTround2Config);
        #endif
        app_dstwr_timer_off();
        app_dstwr_responder_reset();
        iterationTime = cb_hal_get_tick();
        appDstwrResponderState = EN_APP_RESP_STATE_IDLE;
        break;
      }
    }
  }
  app_uwb_dstwr_deregister_irqcallbacks();
  s_appCycleCount = 0;
  #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
  cb_framework_uwb_disable_scheduled_trx(s_stDstwrTround2Config);
  #endif
  app_dstwr_timer_off();
  app_dstwr_responder_reset();
  cb_framework_uwb_off();
}

/**
 * @brief   Resets all member variables
 * @details This function turns on resets all the member variables
*/
void app_dstwr_responder_reset(void)
{
  s_stIrqStatus.TxDone         = APP_FALSE;
  s_stIrqStatus.Rx0Done        = APP_FALSE;
  s_applicationTimeout       = APP_FALSE;
  appFailureResponderState          = EN_APP_RESP_STATE_IDLE;
  memset(&s_stRespResponderDataContainer,   0, sizeof(cb_uwbframework_rangingdatacontainer_st));
  cb_framework_uwb_tsu_clear();
  cb_framework_uwb_tx_end();            // ensure propoer TX end upon abnormal condition
  cb_framework_uwb_rx_end(EN_UWB_RX_0); // ensure propoer RX end upon abnormal condition
}


void app_dstwr_suspend(void)
{
  s_dstwrRunningFlag = APP_FALSE;
}
/**
 * @brief   Initialize the application timer module.
 * @details This function turns on Timer 0 disables their interrupts, 
 *          and enables CPU Timer 0 interrupts.
 */
void app_dstwr_timer_init(uint16_t timeoutMs)
{
  // Turn on Timer 0
  cb_scr_timer0_module_on();

  // Disable Timer's module interrupt
  cb_timer_disable_interrupt();
  
  // Enable CPU Timer 0,1,2,3 Interrupt
  NVIC_EnableIRQ(TIMER_0_IRQn);
  
  // Configure TIMER0-EVENT0 in One_shot mode that timeout every DEF_DSTWR_INI_OVERALL_PROCESS_TIMEOUT_MS.
  stTimerSetUp stTimerSetup;
  stTimerSetup.Timer                       = EN_TIMER_0;
  stTimerSetup.TimerMode                   = EN_TIMER_MODE_ONE_SHOT;
  stTimerSetup.TimeUnit                    = EN_TIMER_MS;

  stTimerSetup.stTimeOut.timeoutVal[0]     = timeoutMs;
  stTimerSetup.stTimeOut.timeoutVal[1]     = 0;
  stTimerSetup.stTimeOut.timeoutVal[2]     = 0;
  stTimerSetup.stTimeOut.timeoutVal[3]     = 0;
  stTimerSetup.stTimeOut.TimerTimeoutEvent  = DEF_TIMER_TIMEOUT_EVENT_0;
  stTimerSetup.TimerEvtComMode              = EN_TIMER_EVTCOM_MODE_00;
  stTimerSetup.AutoStartTimer               = EN_START_TIMER_ENABLE;
  stTimerSetup.TimerEvtComEnable            = EN_TIMER_EVTCOM_DISABLE;
  stTimerSetup.TimerInterrupt               = EN_TIMER_INTERUPT_ENABLE;
  stTimerSetup.stPulseWidth.NumberOfCycles  = 0;
  stTimerSetup.stPulseWidth.TimerPulseWidth = EN_TIMER_PULSEWIDTH_DISABLE;
  
  cb_timer_configure_timer(&stTimerSetup);
}

/**
 * @brief   Off the application timer module.
 * @details This function turns off Timer 0 disables their interrupts, 
 *          and disable CPU Timer 0 interrupts.
 */
void app_dstwr_timer_off(void)
{
  // Disable Timer 0
  cb_timer_disable_timer(EN_TIMER_0);
  
  // Turn off Timer 0
  cb_scr_timer0_module_off();

  // Enable CPU Timer 0,1,2,3 Interrupt
  NVIC_DisableIRQ(TIMER_0_IRQn);
}

uint8_t app_dstwr_initiator_validate_sync_ack_payload(void)
{
  uint8_t  result = APP_TRUE;
  uint8_t  syncAckPayloadReceived[DEF_SYNC_ACK_RX_PAYLOAD_SIZE] = {0};
  
  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();
          
  if (rxStatus.rx0_ok   == CB_TRUE)
  {  
    cb_framework_uwb_get_rx_payload(&syncAckPayloadReceived[0], DEF_SYNC_ACK_RX_PAYLOAD_SIZE);
    for (uint16_t i = 0; i < DEF_SYNC_ACK_RX_PAYLOAD_SIZE; i++)
    {
      if (syncAckPayloadReceived[i] != s_syncAckRxPayload[i])
        result = APP_FALSE;
    }
  }
  else
  {
    result = APP_FALSE;
  }  
  return result;
}

uint8_t app_dstwr_responder_validate_sync_payload(void)
{
  uint8_t  result = APP_TRUE;
  uint8_t  syncRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE] = {0};
  
  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();
          
  if (rxStatus.rx0_ok   == CB_TRUE)
  {
    cb_framework_uwb_get_rx_payload(&syncRxPayload[0], DEF_SYNC_RX_PAYLOAD_SIZE);
    for (uint16_t i = 0; i < DEF_SYNC_RX_PAYLOAD_SIZE; i++)
    {
      if (syncRxPayload[i] != s_syncExpectedRxPayload[i])
      {
        result = APP_FALSE;
      }
    }
  }
  else
  {
    result = APP_FALSE;
  }
  
  return result;
}

/**
 * @brief Registers the IRQ callbacks for DSTWR-related interrupts.
 *
 * This function registers the necessary interrupt callbacks for UWB transmission, reception, 
 * and timer interrupts related to the DSTWR protocol, ensuring that the system responds 
 * appropriately to each event.
 */
void app_uwb_dstwr_register_irqcallbacks(void)
{  
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ, app_uwb_dstwr_tx_done_irq_callback);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, app_uwb_dstwr_rx0_done_irq_callback);
  app_irq_register_irqcallback(EN_IRQENTRY_TIMER_0_APP_IRQ, app_uwb_dstwr_timer0_irq_callback);
}

/**
 * @brief Deregisters the IRQ callbacks for DSTWR-related interrupts.
 *
 * This function removes the previously registered interrupt callbacks, stopping the 
 * system from responding to the UWB transmission, reception, and timer interrupts 
 * associated with the DSTWR protocol.
 */
void app_uwb_dstwr_deregister_irqcallbacks(void)
{
  app_irq_deregister_irqcallback (EN_IRQENTRY_UWB_TX_DONE_APP_IRQ, app_uwb_dstwr_tx_done_irq_callback);
  app_irq_deregister_irqcallback (EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, app_uwb_dstwr_rx0_done_irq_callback);
  app_irq_deregister_irqcallback (EN_IRQENTRY_TIMER_0_APP_IRQ, app_uwb_dstwr_timer0_irq_callback);
}

/**
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered. It fetches
 * the timestamp related to the TX Done event and prints it via UART.
 */
void app_uwb_dstwr_tx_done_irq_callback(void)
{
  s_stIrqStatus.TxDone = APP_TRUE;
}


/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 * 
 * This function is called when the UWB RX0 Done IRQ is triggered. 
 */
void app_uwb_dstwr_rx0_done_irq_callback(void)
{
  s_stIrqStatus.Rx0Done = APP_TRUE;
}

/**
 * @brief   Callback function for Timer 0 IRQ.
 *
 * This function is called when the Timer 0 IRQ is triggered.
 */
void app_uwb_dstwr_timer0_irq_callback (void)
{
  s_applicationTimeout = APP_TRUE;
  s_enAppFailureInitiatorState = s_enAppDstwrInitiatorState;
  s_enAppDstwrInitiatorState = EN_APP_INI_STATE_TERMINATE;
  
  appFailureResponderState = appDstwrResponderState;
  appDstwrResponderState = EN_APP_RESP_STATE_TERMINATE;
}

void app_dstwr_initiator_log(void) 
{
  if (!s_applicationTimeout)
  {
    app_uwb_dstwr_print("Cycle:%u, D:%fcm\n", s_appCycleCount++, s_measuredDistance);      
  }
  else
  {
    app_dstwr_initiator_timeout_error_message_print();
    return;
  } 
}

void app_dstwr_responder_log(void) 
{
  if (!s_applicationTimeout)
  {
    app_uwb_dstwr_print("Cycle:%u - Ranging Successful\n", s_appCycleCount++); 
  }
  else
  {
    app_dstwr_responder_timeout_error_message_print();
    return;
  }
}

/**
 * @brief Prints a timeout error message based on the current process state.
 *
 * This function logs an error message with the current cycle count and the 
 * specific process state where a timeout occurred. It provides descriptive 
 * feedback for debugging and monitoring the DSTWR protocol.
 *
 * @param[in] state  The current state of the DSTWR process when the timeout occurred.
 *                   Possible values:
 *                   - ``EN_IDLE``: Idle state.
 *                   - ``EN_STATE_SYNCING``: Syncing state.
 *                   - ``EN_STATE_DSTWR``: DSTWR state.
 *                   - ``EN_STATE_RESULT``: Final result state.
 *                   - ``EN_STATE_DIST_OOB``: Distance out-of-bounds state.
 */
void app_dstwr_initiator_timeout_error_message_print(void)
{
  switch (s_enAppFailureInitiatorState)
  {
    case EN_APP_INI_STATE_IDLE:
      break;
    case EN_APP_INI_STATE_SYNC_TRANSMIT:
      break;
    case EN_APP_INI_STATE_SYNC_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:SYNC TX\n", s_appCycleCount++);
      break;
    case EN_APP_INI_STATE_SYNC_RECEIVE:
      break;
    case EN_APP_INI_STATE_SYNC_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:SYNC ACK\n", s_appCycleCount++);
      break;
    case EN_APP_INI_STATE_DSTWR_TRANSMIT_POLL:
      break;
    case EN_APP_INI_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR TX POLL\n", s_appCycleCount++);
      break;
    case EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE:
      break;
    case EN_APP_INI_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR RX RESPONSE\n", s_appCycleCount++);
      break;
    case EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL:
      break;
    case EN_APP_INI_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR TX FINAL\n", s_appCycleCount++);
      break;
    case EN_APP_INI_STATE_RESULT_RECEIVE:
      break;
    case EN_APP_INI_STATE_RESULT_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:FINAL RESULT\n", s_appCycleCount++);
      break;
    case EN_APP_INI_STATE_TERMINATE:
      break;    
  }
}

void app_dstwr_responder_timeout_error_message_print()
{
  switch (appFailureResponderState)
  {
    case EN_APP_RESP_STATE_IDLE:
      break;
    case EN_APP_RESP_STATE_SYNC_RECEIVE:
      break;
    case EN_APP_RESP_STATE_SYNC_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:SYNC RX\n", s_appCycleCount++);
      break;
    case EN_APP_RESP_STATE_SYNC_TRANSMIT:
      break;
    case EN_APP_RESP_STATE_SYNC_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:SYNC ACK\n", s_appCycleCount++);
      break;
    case EN_APP_RESP_STATE_DSTWR_RECEIVE_POLL:
      break;
    case EN_APP_RESP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR RX POLL\n", s_appCycleCount++);
      break;
    case EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE:
      break;
    case EN_APP_RESP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR TX RESPONSE\n", s_appCycleCount++);
      break;
    case EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL:
      break;
    case EN_APP_RESP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR RX FINAL\n", s_appCycleCount++);
      break;
    case EN_APP_RESP_STATE_RESULT_TRANSMIT:
      break;
    case EN_APP_RESP_STATE_RESULT_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:FINAL RESULT\n", s_appCycleCount++);
      break;
    case EN_APP_RESP_STATE_TERMINATE:
      break;    
  }
}

