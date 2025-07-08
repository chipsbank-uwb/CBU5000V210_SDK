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
#include <string.h>
#include "AppUwbPdoa.h"
#include "CB_system_types.h"
#include "CB_uwbframework.h"
#include "NonLIB_sharedUtils.h"

#include "CB_timer.h"
#include "CB_scr.h"

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
#define DEF_SYNC_RX_PAYLOAD_SIZE       4
#define DEF_SYNC_ACK_TX_PAYLOAD_SIZE   3
#define DEF_PDOA_PD01_BIAS              (170.0f)// 3D
#define DEF_PDOA_PD02_BIAS              (40.0f) // 2D,3D
#define DEF_PDOA_PD12_BIAS              (10.0f) // 3D

typedef enum {
  // IDLE STATE
  EN_APP_STATE_IDLE = 0,
  //SYNC STATE  
  EN_APP_STATE_SYNC_RECEIVE,
  EN_APP_STATE_SYNC_WAIT_RX_DONE,
  EN_APP_STATE_SYNC_TRANSMIT,
  EN_APP_STATE_SYNC_WAIT_TX_DONE,
  // PDOA-RX STATE
  EN_APP_STATE_PDOA_PREPARE,
  EN_APP_STATE_PDOA_RECEIVE,   
  EN_APP_STATE_PDOA_WAIT_RX_DONE,
  EN_APP_STATE_PDOA_POSTINGPROCESSING,
  // TERMINATE STATE  
  EN_APP_STATE_TERMINATE,
} app_uwbpdoa_responderstate_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
  volatile uint8_t TxDone;
  volatile uint8_t Rx0Done;
  volatile uint8_t Rx0SfdDetected;
  volatile uint8_t Rx1SfdDetected;
  volatile uint8_t Rx2SfdDetected;
}app_uwbpdoa_irqstatus_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint8_t s_applicationTimeout                 = APP_FALSE;
static cb_uwbsystem_rx_dbb_config_st s_stRxCfg_CfoGainBypass  = { 0 };

/* Default Rx packet configuration.*/
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
  .stsVCounter        = 0x1F9A3DE4UL,     
  .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
};

static app_uwbpdoa_irqstatus_st       s_stIrqStatus                = { APP_FALSE };
static app_uwbpdoa_responderstate_en  s_enAppPdoaResponderState    = EN_APP_STATE_IDLE;

//  SYNC RX Payload                                                 'S'  'Y'  'N'  'C'
static uint8_t s_syncExpectedRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE]   = {0x53,0x59,0x4E,0x43}; 
//  SYCN ACK payload                                                'A'  'C'  'K'
static uint8_t s_syncAckPayload[DEF_SYNC_ACK_TX_PAYLOAD_SIZE]      = {0x41,0x43,0x4B};

static uint8_t                          s_countOfPdoaScheduledRx   = 0;
static cb_uwbsystem_rx_signalinfo_st  s_stRssiResults            = {0};
static cb_uwbsystem_pdoaresult_st       s_stPdoaOutputResult       = {0};
static float                            s_pd01Bias                 = DEF_PDOA_PD01_BIAS;
static float                            s_pd02Bias                 = DEF_PDOA_PD02_BIAS;
static float                            s_pd12Bias                 = DEF_PDOA_PD12_BIAS;
static float                            s_aziResult                = 0.0f;
static float                            s_eleResult                = 0.0f;

//-------------------------------
// PDOA: RESPONDER SETUP
//-------------------------------
//-------------------------------------------------------
//    Initiator                         Responder
//     Idle                                Idle
//       |---------1. SYNC ----------------->|
//       |<------- 2. ACK  ------------------|
//       |---------3. PDOA (n cycles) ------>| 
//     Terminate                         Terminate  
//
// DEF_PDOA_OVERALL_PROCESS_TIMEOUT_MS : 3
// DEF_PDOA_SYNC_RX_RESTART_TIMEOUT_MS : 1
// DEF_PDOA_APP_CYCLE_TIME_MS          : Idle
// DEF_NUMBER_OF_PDOA_REPEATED_RX      : 3 (n cycles)
//
//-------------------------------------------------------
#define DEF_PDOA_OVERALL_PROCESS_TIMEOUT_MS   10 
#define DEF_NUMBER_OF_PDOA_REPEATED_RX        DEF_PDOA_NUMPKT_SUPERFRAME_MAX
#define DEF_PDOA_SYNC_RX_RESTART_TIMEOUT_MS   4 
#define DEF_PDOA_APP_CYCLE_TIME_MS            498

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
uint8_t app_pdoa_validate_sync_ack_payload(void);
void app_pdoa_reset(void);

//-------------------------------
// FUNCTION BODY SECTION
//------------------------------
void app_pdoa_responder(void)
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
  static cb_uwbsystem_txpayload_st s_stSyncAckPayloadPack = 
  {
    .ptrAddress  = &s_syncAckPayload[0],
    .payloadSize = sizeof(s_syncAckPayload),
  };  
  
  //--------------------------------
  // Configure IRQ
  //--------------------------------
  // SYNC RX IRQ
  cb_uwbsystem_rx_irqenable_st stSyncRxIrqEnable    = { CB_FALSE };   
  stSyncRxIrqEnable.rx0Done                         = CB_TRUE; 
  // SYNC TX IRQ
  cb_uwbsystem_tx_irqenable_st s_stSyncTxIrqEnable  = { CB_FALSE };
  s_stSyncTxIrqEnable.txDone                        = CB_TRUE;
  // PDOA RX IRQ
  cb_uwbsystem_rx_irqenable_st stPdoaRxIrqEnable    = { CB_FALSE };   
  stPdoaRxIrqEnable.rx0SfdDetDone                   = CB_TRUE;  
  stPdoaRxIrqEnable.rx1SfdDetDone                   = CB_TRUE;  
  stPdoaRxIrqEnable.rx2SfdDetDone                   = CB_TRUE;  
  
  s_enAppPdoaResponderState = EN_APP_STATE_SYNC_RECEIVE;
  
  while(1)
  {
    switch (s_enAppPdoaResponderState)
    {  
      case EN_APP_STATE_IDLE:
      //-------------------------------------
      // IDLE
      //-------------------------------------         
        // Wait for next cycle
        if (cb_hal_is_time_elapsed(iterationTime, DEF_PDOA_APP_CYCLE_TIME_MS))
        {
          s_enAppPdoaResponderState = EN_APP_STATE_SYNC_RECEIVE;
        }
      break;
      
      //-------------------------------------
      // SYNC: RX 
      //-------------------------------------      
      case EN_APP_STATE_SYNC_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stSyncRxIrqEnable, EN_TRX_START_NON_DEFERRED); // RX START
        s_enAppPdoaResponderState = EN_APP_STATE_SYNC_WAIT_RX_DONE;
        startTime = cb_hal_get_tick();
        break;
      case EN_APP_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_PDOA_SYNC_RX_RESTART_TIMEOUT_MS) == APP_TRUE)
        {
          s_enAppPdoaResponderState = EN_APP_STATE_SYNC_RECEIVE;
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
        }
        else if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;    
          uint8_t syncValidationResult = app_pdoa_validate_sync_ack_payload();
          if (syncValidationResult == APP_TRUE)
          {
            s_stRssiResults = cb_framework_uwb_get_rx_rssi(EN_UWB_RX_0);
            s_enAppPdoaResponderState = EN_APP_STATE_SYNC_TRANSMIT;
          }
          else
          {
            // if SYNC-ACK payload validation failed, wait for SYNC-RX again.
            s_enAppPdoaResponderState = EN_APP_STATE_SYNC_RECEIVE;
          }
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
        }
        break;
      //-------------------------------------
      // SYNC: TX (ACK)
      //-------------------------------------  
      case EN_APP_STATE_SYNC_TRANSMIT:
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &s_stSyncAckPayloadPack, &s_stSyncTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppPdoaResponderState = EN_APP_STATE_SYNC_WAIT_TX_DONE;
        break;
      case EN_APP_STATE_SYNC_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          s_enAppPdoaResponderState = EN_APP_STATE_PDOA_PREPARE;
        }
        break;
        
      //-------------------------------------
      // PDOA-RX
      //-------------------------------------  
      case EN_APP_STATE_PDOA_PREPARE:
        app_pdoa_timer_init(DEF_PDOA_OVERALL_PROCESS_TIMEOUT_MS);  
        s_stRxCfg_CfoGainBypass.stRxGain = (cb_uwbsystem_rx_dbb_gain_st){
            .enableBypass = APP_TRUE,
            .gainValue    = s_stRssiResults.gainIdx
        };

        s_stRxCfg_CfoGainBypass.stRxCfo = (cb_uwbsystem_rx_dbb_cfo_st){
            .enableBypass = APP_TRUE,
            .cfoValue     = s_stRssiResults.cfoEst
        };
        cb_framework_uwb_rxconfig_cfo_gain(EN_UWB_CFO_GAIN_SET, &s_stRxCfg_CfoGainBypass);
        s_enAppPdoaResponderState = EN_APP_STATE_PDOA_RECEIVE;
        break;
        
      case EN_APP_STATE_PDOA_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_ALL, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppPdoaResponderState = EN_APP_STATE_PDOA_WAIT_RX_DONE;
        break;
      
      case EN_APP_STATE_PDOA_WAIT_RX_DONE:
        if (s_stIrqStatus.Rx0SfdDetected && s_stIrqStatus.Rx1SfdDetected && s_stIrqStatus.Rx2SfdDetected)
        {
          s_stIrqStatus.Rx0SfdDetected = APP_FALSE; 
          s_stIrqStatus.Rx1SfdDetected = APP_FALSE; 
          s_stIrqStatus.Rx2SfdDetected = APP_FALSE;
          
          cb_framework_uwb_pdoa_store_cir_data(s_countOfPdoaScheduledRx);     
            
          s_countOfPdoaScheduledRx++;
          if (s_countOfPdoaScheduledRx < DEF_NUMBER_OF_PDOA_REPEATED_RX)
          {
            cb_framework_uwb_rx_restart(EN_UWB_RX_ALL, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED);
          }
          else 
          {
            cb_framework_uwb_rx_end(EN_UWB_RX_ALL);   
            s_countOfPdoaScheduledRx = 0;
            cb_framework_uwb_rxconfig_cfo_gain(EN_UWB_CFO_GAIN_RESET, NULL);
            s_enAppPdoaResponderState = EN_APP_STATE_PDOA_POSTINGPROCESSING;
          }
        }
        break;
        
      case EN_APP_STATE_PDOA_POSTINGPROCESSING:
        // PDOA
        cb_framework_uwb_pdoa_calculate_result(&s_stPdoaOutputResult,EN_PDOA_3D_CALTYPE, DEF_NUMBER_OF_PDOA_REPEATED_RX);
        app_uwb_pdoa_print("PD01:%f, PD02:%f, PD12:%f (in degrees)\n",s_stPdoaOutputResult.median.rx0_rx1,s_stPdoaOutputResult.median.rx0_rx2,s_stPdoaOutputResult.median.rx1_rx2);          
        
        // AOA
        cb_framework_uwb_pdoa_calculate_aoa(s_stPdoaOutputResult.median, s_pd01Bias, s_pd02Bias, s_pd12Bias, &s_aziResult, &s_eleResult);
        app_uwb_pdoa_print("azimuth: %f degrees\nelevation: %f degrees\n", (double)s_aziResult,(double)s_eleResult);    
        
        s_enAppPdoaResponderState = EN_APP_STATE_TERMINATE;
        break;
      case EN_APP_STATE_TERMINATE:
        app_pdoa_timer_off();
        iterationTime = cb_hal_get_tick();
        s_enAppPdoaResponderState = EN_APP_STATE_IDLE;
        app_pdoa_reset();
        break;
      default:
        break;      
    }
  }
}

void app_pdoa_reset(void)
{
  cb_framework_uwb_pdoa_reset_cir_data_container();
  memset(&s_stIrqStatus, APP_FALSE, sizeof(s_stIrqStatus));
  s_applicationTimeout       = APP_FALSE;
  s_countOfPdoaScheduledRx   = 0;
  cb_framework_uwb_tx_end();            // ensure propoer TX end upon abnormal condition
  cb_framework_uwb_rx_end(EN_UWB_RX_0); // ensure propoer RX end upon abnormal condition
  cb_framework_uwb_rxconfig_cfo_gain(EN_UWB_CFO_GAIN_RESET, NULL); // ensure CFO and gain settings are reset upon abnormal condition

}



void app_pdoa_timer_init(uint16_t timeoutMs)
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

void app_pdoa_timer_off(void)
{
  // Disable Timer 0
  cb_timer_disable_timer(EN_TIMER_0);
  
  // Turn off Timer 0
  cb_scr_timer0_module_off();
  
  // Enable CPU Timer 0,1,2,3 Interrupt
  NVIC_DisableIRQ(TIMER_0_IRQn);
}

uint8_t app_pdoa_validate_sync_ack_payload(void)
{
  uint8_t  result = APP_TRUE;
  uint16_t rxPayloadSize = 0;
  uint8_t  syncRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE] = {0};
  
  cb_uwbsystem_rxstatus_un unRxStatus = cb_framework_uwb_get_rx_status();
  
  if (unRxStatus.rx0_ok == CB_TRUE)
  {
    cb_framework_uwb_get_rx_payload(&syncRxPayload[0], &rxPayloadSize, &s_stUwbPacketConfig);
    for (uint16_t i = 0; i < DEF_SYNC_RX_PAYLOAD_SIZE; i++)
    {
      if (syncRxPayload[i] != s_syncExpectedRxPayload[i])
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

void cb_uwbapp_rx0_sfd_detected_irqcb(void)
{
  s_stIrqStatus.Rx0SfdDetected = APP_TRUE; 
}

void cb_uwbapp_rx1_sfd_detected_irqcb(void)
{
  s_stIrqStatus.Rx1SfdDetected = APP_TRUE; 
}

void cb_uwbapp_rx2_sfd_detected_irqcb(void)
{
  s_stIrqStatus.Rx2SfdDetected = APP_TRUE; 
}

void cb_timer_0_app_irq_callback (void)
{
  s_applicationTimeout = APP_TRUE;
  s_enAppPdoaResponderState = EN_APP_STATE_TERMINATE;
}
