/**
 * @file    AppUwbDstwr.c
 * @brief   
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbDstwr.h"
#include "CB_Algorithm.h"
#include "CB_timer.h"
#include "CB_scr.h"
#include "NonLIB_sharedUtils.h"
#include "CB_uwbframework.h"

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
#define DEF_RESPONDER_RANGING_BIAS     0

#define DEF_SYNC_RX_PAYLOAD_SIZE       4
#define DEF_SYNC_ACK_TX_PAYLOAD_SIZE   3

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum{
  //IDLE STATE
  EN_APP_STATE_IDLE = 0,
  //SYNC STATE  
  EN_APP_STATE_SYNC_RECEIVE,
  EN_APP_STATE_SYNC_WAIT_RX_DONE,
  EN_APP_STATE_SYNC_TRANSMIT,
  EN_APP_STATE_SYNC_WAIT_TX_DONE,
  //DSTWR STATE  
  EN_APP_STATE_DSTWR_RECEIVE_POLL,
  EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE,
  EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE,
  EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE,
  EN_APP_STATE_DSTWR_RECEIVE_FINAL,
  EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE,
  //RESULT SHARING STATE  
  EN_APP_STATE_RESULT_TRANSMIT,
  EN_APP_STATE_RESULT_WAIT_TX_DONE,
  //TERMINATE STATE  
  EN_APP_STATE_TERMINATE,
} app_uwbdstwr_state_en;

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
static uint8_t s_applicationTimeout         = APP_FALSE;

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

static app_uwbdstwr_irqstatus_st  s_stIrqStatus               = {APP_FALSE};
static app_uwbdstwr_state_en      s_enAppDstwrState           = EN_APP_STATE_IDLE;
static app_uwbdstwr_state_en      s_enAppDstwrFailureState    = EN_APP_STATE_IDLE;

//  SYNC RX Payload                                               'S'  'Y'  'N'  'C'
static uint8_t s_syncExpectedRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE]  = {0x53,0x59,0x4E,0x43};
//  SYCN ACK payload                                              'A'  'C'  'K'
static uint8_t s_syncAckPayload[DEF_SYNC_ACK_TX_PAYLOAD_SIZE]     = {0x41,0x43,0x4B};

static cb_uwbsystem_txpayload_st uwbResultTxPayload;

static cb_uwbsystem_tx_tsutimestamp_st  s_stTxTsuTimestamp0;
static cb_uwbsystem_rx_tsutimestamp_st  s_stRxTsuTimestamp0;
static cb_uwbsystem_rx_tsutimestamp_st  rxTsuTimestamp1;
static cb_uwbframework_rangingdatacontainer_st s_stResponderDataContainer = 
{
  .dstwrRangingBias = DEF_RESPONDER_RANGING_BIAS,
  .dstwrTroundTreply = {0}
};

static uint32_t s_appCycleCount = 0;   // Logging Purpose: cycle count

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
//  a: -        d: s_stRxTsuTimestamp0
//  b: -        e: s_stTxTsuTimestamp0
//  c: -        f: rxTsuTimestamp1
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
void    app_dstwr_reset                         (void);
void    app_dstwr_timeout_error_message_print   (void);
uint8_t app_dstwr_validate_sync_payload         (void);
void    app_dstwr_log                           (void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
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
  cb_uwbsystem_txpayload_st stSyncAckPayloadPack    = {0};
  cb_uwbsystem_txpayload_st stDstwrTxPayloadPack    = {0};  
  
  stSyncAckPayloadPack.ptrAddress     = &s_syncAckPayload[0];
  stSyncAckPayloadPack.payloadSize    = sizeof(s_syncAckPayload);
  //  DSTWR Payload
  static uint8_t s_dstwrPayload[1]    = {0x1};
  stDstwrTxPayloadPack.ptrAddress     = &s_dstwrPayload[0];
  stDstwrTxPayloadPack.payloadSize    = sizeof(s_dstwrPayload);  

  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // TX IRQ
  cb_uwbsystem_tx_irqenable_st stTxIrqEnable = {CB_FALSE};
  stTxIrqEnable.txDone                       = APP_TRUE;
  // RX IRQ
  cb_uwbsystem_rx_irqenable_st stRxIrqEnable = {CB_FALSE};
  stRxIrqEnable.rx0Done                      = APP_TRUE;  
  
  //--------------------------------
  // Configure absolute timer for scheduled TX start
  //--------------------------------
  static cb_uwbframework_trx_scheduledconfig_st s_stDstwrTreply1Config = 
  {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0    :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_17_RX0_SFD_DET,   // rx0_sfd   :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0      :: (ABS timer) Select absolute timer
  .timeoutValue         = 700,                          // 700us     :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_TX_START_MASK,     // tx start  :: (action)    select action upon abs timeout 
  };
  
  static cb_uwbframework_trx_scheduledconfig_st s_stDstwrTround2Config = 
  {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0    :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_28_TX_DONE,       // tx_done   :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0      :: (ABS timer) Select absolute timer
  .timeoutValue         = 500,                          // 500us     :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_RX0_START_MASK,    // rx0 start :: (action)    select action upon abs timeout 
  };  
  
  s_enAppDstwrState = EN_APP_STATE_SYNC_RECEIVE;
 
  while(1)
  {
    switch (s_enAppDstwrState)
    {
      case EN_APP_STATE_IDLE:
      //-------------------------------------
      // IDLE
      //-------------------------------------         
        // Wait for next cycle
        if (cb_hal_is_time_elapsed(iterationTime, DEF_DSTWR_APP_CYCLE_TIME_MS))
        {
          s_enAppDstwrState = EN_APP_STATE_SYNC_RECEIVE;
        }
        break;
        
      //-------------------------------------
      // SYNC: RX
      //-------------------------------------       
      case EN_APP_STATE_SYNC_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppDstwrState = EN_APP_STATE_SYNC_WAIT_RX_DONE;
        startTime = cb_hal_get_tick();
        break;
      case EN_APP_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_SYNC_RX_RESTART_TIMEOUT_MS))
        {
          s_enAppDstwrState = EN_APP_STATE_SYNC_RECEIVE;
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
        }
        else if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;
          uint32_t syncValidateResult = app_dstwr_validate_sync_payload();
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          if (syncValidateResult == APP_TRUE)
          {
            s_enAppDstwrState = EN_APP_STATE_SYNC_TRANSMIT;
          }
          else
          {
            // if SYNC-ACK payload validation failed, wait for SYNC-RX again.
            s_enAppDstwrState = EN_APP_STATE_SYNC_RECEIVE;
          }
        }
        break;
      //-------------------------------------
      // SYNC: TX (ACK)
      //------------------------------------- 
      case EN_APP_STATE_SYNC_TRANSMIT:
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stSyncAckPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppDstwrState = EN_APP_STATE_SYNC_WAIT_TX_DONE;        
        break;
      case EN_APP_STATE_SYNC_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          s_enAppDstwrState = EN_APP_STATE_DSTWR_RECEIVE_POLL; 
        }          
        break;
      //-------------------------------------
      // DS-TWR: POLL
      //-------------------------------------  
      case EN_APP_STATE_DSTWR_RECEIVE_POLL:
        app_dstwr_timer_init(DEF_DSTWR_OVERALL_PROCESS_TIMEOUT_MS);  

        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_enable_scheduled_trx(s_stDstwrTreply1Config);
        #endif
      
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppDstwrState = EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE;
        break;
      case EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE:
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {        
          s_stIrqStatus.Rx0Done = APP_FALSE;

          cb_framework_uwb_get_rx_tsu_timestamp(&s_stRxTsuTimestamp0, EN_UWB_RX_0);
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppDstwrState = EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE;
          startTime = cb_hal_get_tick();
        }
        break;
      //-------------------------------------
      // DS-TWR: RESPONSE
      //-------------------------------------            
      case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE:
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_enable_scheduled_trx(s_stDstwrTround2Config);
      
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTreply1Config);
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_DEFERRED);
          s_enAppDstwrState = EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE;
        #else
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_RESPONSE_WAIT_TIME_MS))
        {
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppDstwrState = EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE;
        }
        #endif
        break;
      case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          
          #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTround2Config);
          #endif
          
          cb_framework_uwb_get_tx_tsu_timestamp(&s_stTxTsuTimestamp0);
          cb_framework_uwb_tx_end();
          s_enAppDstwrState = EN_APP_STATE_DSTWR_RECEIVE_FINAL;
          startTime = cb_hal_get_tick();
        }
        break;
      //-------------------------------------
      // DS-TWR: FINAL
      //-------------------------------------         
      case EN_APP_STATE_DSTWR_RECEIVE_FINAL:
      {
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_DEFERRED);
          s_enAppDstwrState = EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE;
        #else
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_FINAL_WAIT_TIME_MS))
        {
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppDstwrState = EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE;
        }
        #endif
        break;
      }
      case EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE:
      {
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {          
          s_stIrqStatus.Rx0Done = APP_FALSE;
          
          #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_disable_scheduled_trx(s_stDstwrTround2Config);
          #endif
          
          cb_framework_uwb_get_rx_tsu_timestamp(&rxTsuTimestamp1, EN_UWB_RX_0);
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppDstwrState = EN_APP_STATE_RESULT_TRANSMIT;
          startTime = cb_hal_get_tick();
        }
        break;
      }
      //-------------------------------------
      // Ranging Result: TX
      //-------------------------------------  
      case EN_APP_STATE_RESULT_TRANSMIT:
      {
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_RESULT_WAIT_TIME_MS))
        {
          cb_framework_uwb_calculate_responder_tround_treply(&s_stResponderDataContainer, s_stTxTsuTimestamp0, s_stRxTsuTimestamp0, rxTsuTimestamp1);
          uwbResultTxPayload.ptrAddress = (uint8_t*)(&s_stResponderDataContainer.dstwrTroundTreply);
          uwbResultTxPayload.payloadSize = sizeof(cb_uwbframework_rangingdatacontainer_st);
          
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &uwbResultTxPayload, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppDstwrState = EN_APP_STATE_RESULT_WAIT_TX_DONE;
        }
        break;
      }
      case EN_APP_STATE_RESULT_WAIT_TX_DONE:
      {
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          s_enAppDstwrState = EN_APP_STATE_TERMINATE;
        }
        break;
      }
      //-------------------------------------
      // Terminate
      //-------------------------------------  
      case EN_APP_STATE_TERMINATE:
      {
        #if (DEF_DSTWR_ENABLE_LOG == APP_TRUE)
          app_dstwr_log();
        #endif   
        #if (APP_DSTWR_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_disable_scheduled_trx(s_stDstwrTround2Config);
        #endif
        app_dstwr_timer_off();
        app_dstwr_reset();
        iterationTime = cb_hal_get_tick();
        s_enAppDstwrState = EN_APP_STATE_IDLE;
        break;
      }
    }
  }  
}

/**
 * @brief   Resets all member variables
 * @details This function turns on resets all the member variables
*/
void app_dstwr_reset(void)
{
  s_stIrqStatus.TxDone         = APP_FALSE;
  s_stIrqStatus.Rx0Done        = APP_FALSE;
  s_applicationTimeout       = APP_FALSE;
  s_enAppDstwrFailureState          = EN_APP_STATE_IDLE;
  memset(&s_stResponderDataContainer,   0, sizeof(cb_uwbframework_rangingdatacontainer_st));
  cb_framework_uwb_tsu_clear();
  cb_framework_uwb_tx_end();            // ensure propoer TX end upon abnormal condition
  cb_framework_uwb_rx_end(EN_UWB_RX_0); // ensure propoer RX end upon abnormal condition
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
  
  // Configure TIMER0-EVENT0 in One_shot mode that timeout every DEF_DSTWR_OVERALL_PROCESS_TIMEOUT_MS.
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

uint8_t app_dstwr_validate_sync_payload(void)
{
  uint8_t  result                                     = APP_TRUE;
  uint16_t rxPayloadSize                              = 0;
  uint8_t  syncRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE]    = {0};
  
  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();
          
  if (rxStatus.rx0_ok == CB_TRUE)
  {
    cb_framework_uwb_get_rx_payload(&syncRxPayload[0], &rxPayloadSize, &s_stUwbPacketConfig);
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

/**
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered.
 */
void cb_uwbapp_tx_done_irqhandler(void)
{
  s_stIrqStatus.TxDone = APP_TRUE;
}

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 * 
 * This function is called when the UWB RX0 Done IRQ is triggered. 
 */
void cb_uwbapp_rx0_done_irqcb(void)
{
  s_stIrqStatus.Rx0Done = APP_TRUE;
}

/**
 * @brief   Callback function for Timer 0 IRQ.
 *
 * This function is called when the Timer 0 IRQ is triggered.
 */
void cb_timer_0_app_irq_callback (void)
{
  s_applicationTimeout = APP_TRUE;
  s_enAppDstwrState = EN_APP_STATE_TERMINATE;
}

/**
 * @brief Dstwr Logging function, at different verbosity 
 * @param DeviceRole The role of the current device.
 * @details Logging verbosity level is set globally, at file-scope. @see s_logOpt
 * 0: Simple Log (default)
 * 1: Simple Log + CIR log
 * 2: Full log (Simple log + CIR log + Status log)
 */
void app_dstwr_log(void) 
{
  if (!s_applicationTimeout)
  {
    app_uwb_dstwr_print("Cycle:%u - Ranging Successful\n", s_appCycleCount++); 
  }
  else
  {
    app_dstwr_timeout_error_message_print();
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
void app_dstwr_timeout_error_message_print()
{
  switch (s_enAppDstwrFailureState)
  {
    case EN_APP_STATE_IDLE:
      break;
    case EN_APP_STATE_SYNC_RECEIVE:
      break;
    case EN_APP_STATE_SYNC_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:SYNC RX\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_SYNC_TRANSMIT:
      break;
    case EN_APP_STATE_SYNC_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:SYNC ACK\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_POLL:
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR RX POLL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE:
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR TX RESPONSE\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_FINAL:
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:DSTWR RX FINAL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_RESULT_TRANSMIT:
      break;
    case EN_APP_STATE_RESULT_WAIT_TX_DONE:
      app_uwb_dstwr_print("Cycle:%u, Timeout:FINAL RESULT\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_TERMINATE:
      break;    
  }
}
