/**
 * @file    AppUwbRngAoa.c
 * @brief   
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include <stdlib.h>
#include "AppUwbRngAoa.h"
#include "CB_Algorithm.h"
#include "CB_timer.h"
#include "CB_scr.h"
#include "NonLIB_sharedUtils.h"
#include "CB_uwbframework.h"
#include "AppSysIrqCallback.h"
#include "ftm_handler.h"
#include "cmd_parser_uart.h"
#include "dfu_handler.h"
#include "CB_Uart.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER   APP_TRUE
#define APP_UWB_RNGAOA_UARTPRINT_ENABLE APP_TRUE

#if (APP_UWB_RNGAOA_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_rngaoa_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_rngaoa_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_RNGAOA_ENABLE_LOG           APP_TRUE

#if APP_RNGAOA_ROLE == INITIATOR
  #define DEF_INITIATOR_RANGING_BIAS     0
  #define DEF_SYNC_TX_PAYLOAD_SIZE       4
  #define DEF_SYNC_ACK_RX_PAYLOAD_SIZE   3
#else
  #define DEF_RESPONDER_RANGING_BIAS     0
  #define DEF_SYNC_RX_PAYLOAD_SIZE       4
  #define DEF_SYNC_ACK_TX_PAYLOAD_SIZE   3
  // PDOA Defines
  #define DEF_PDOA_PD01_BIAS              (-86.0f)// 3D
  #define DEF_PDOA_PD02_BIAS              (-155.0f) // 2D,3D
  #define DEF_PDOA_PD12_BIAS              (-70.0f) // 3D
#endif

// PDOA Mode Configuration:
#define APP_PDOA_HIGH_ACCURACY_MODE   APP_FALSE // PDOA High Accuracy Mode: End then restart for better accuracy

//-------------------------------
// ENUM SECTION
//-------------------------------
#if APP_RNGAOA_ROLE == INITIATOR
typedef enum{
  //IDLE STATE
  EN_APP_STATE_IDLE = 0,
  //SYNC STATE
  EN_APP_STATE_SYNC_TRANSMIT,
  EN_APP_STATE_SYNC_WAIT_TX_DONE,
  EN_APP_STATE_SYNC_RECEIVE,
  EN_APP_STATE_SYNC_WAIT_RX_DONE,
  //DSTWR STATE
  EN_APP_STATE_DSTWR_TRANSMIT_POLL,
  EN_APP_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE,
  EN_APP_STATE_DSTWR_RECEIVE_RESPONSE,
  EN_APP_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE,
  EN_APP_STATE_DSTWR_TRANSMIT_FINAL,
  EN_APP_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE,
  // PDOA-TX STATE
  EN_APP_STATE_WAIT_RESPONDER_READY,             // Coordination with Responder
  EN_APP_STATE_PDOA_TRANSMIT,                    // PDOA-TX
  EN_APP_STATE_PDOA_WAIT_TX_DONE_N_REPEATED_TX,  // PDOA-TX
  //RESULT SHARING STATE
  EN_APP_STATE_RESULT_RECEIVE,
  EN_APP_STATE_RESULT_WAIT_RX_DONE,
  //DISTANCE FEEDBACK STATE
  EN_APP_STATE_DISTANCE_TRANSMIT,
  EN_APP_STATE_DISTANCE_WAIT_TX_DONE,
  //TERMINATE STATE
  EN_APP_STATE_TERMINATE,
} app_uwbrngaoa_state_en;
#else
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
  // PDOA-RX STATE
  EN_APP_STATE_PDOA_PREPARE,
  EN_APP_STATE_PDOA_RECEIVE,
  EN_APP_STATE_PDOA_WAIT_RX_DONE,
  EN_APP_STATE_PDOA_POSTINGPROCESSING,
  //RESULT SHARING STATE
  EN_APP_STATE_RESULT_TRANSMIT,
  EN_APP_STATE_RESULT_WAIT_TX_DONE,
  //DISTANCE FEEDBACK STATE
  EN_APP_STATE_DISTANCE_RECEIVE,
  EN_APP_STATE_DISTANCE_WAIT_RX_DONE,
  //TERMINATE STATE
  EN_APP_STATE_TERMINATE,
} app_uwbrngaoa_state_en;
#endif

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
#if APP_RNGAOA_ROLE == INITIATOR
typedef struct
{
  volatile uint8_t TxDone;
  volatile uint8_t Rx0Done;
}app_uwbrngaoa_irqstatus_st;
#else
typedef struct
{
  volatile uint8_t TxDone;
  volatile uint8_t Rx0SfdDetected;
  volatile uint8_t Rx0Done;
  volatile uint8_t Rx1SfdDetected;
  volatile uint8_t Rx2SfdDetected;
}app_uwbrngaoa_irqstatus_st;
#endif

typedef struct
{
  cb_uwbframework_rangingdatacontainer_st   rangingDataContainer;
  cb_uwbframework_pdoadatacontainer_st      pdoaDataContainer;  
} app_rngaoa_responderdatacontainer_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
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

static app_uwbrngaoa_irqstatus_st s_stIrqStatus             = { APP_FALSE };
static app_uwbrngaoa_state_en     s_enAppRngaoaState        = EN_APP_STATE_IDLE;
static app_uwbrngaoa_state_en     s_enAppRngAoaFailureState = EN_APP_STATE_IDLE;

#if APP_RNGAOA_ROLE == INITIATOR
//  SYNC TX Payload                                                 'S'  'Y'  'N'  'C'
static uint8_t s_syncTxPayload  [DEF_SYNC_TX_PAYLOAD_SIZE]      = {0x53,0x59,0x4E,0x43};
//  SYNC ACK payload                                                'A'  'C'  'K'
static uint8_t s_syncAckPayload [DEF_SYNC_ACK_RX_PAYLOAD_SIZE]  = {0x41,0x43,0x4B};
#else
//  SYNC Expected RX Payload                                        'S'  'Y'  'N'  'C'
static uint8_t s_syncExpectedRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE] = {0x53,0x59,0x4E,0x43};
//  SYNC ACK payload                                                'A'  'C'  'K'
static uint8_t s_syncAckPayload[DEF_SYNC_ACK_TX_PAYLOAD_SIZE]    = {0x41,0x43,0x4B};
static const uint8_t s_syncAckPayloadSize = sizeof(s_syncAckPayload);
#endif

static uint8_t s_applicationTimeout = APP_FALSE;

#if APP_RNGAOA_ROLE == INITIATOR
static cb_uwbsystem_rx_tsutimestamp_st       s_stRxTsuTimestamp0;
static cb_uwbsystem_tx_tsutimestamp_st       s_stTxTsuTimestamp0;
static cb_uwbsystem_tx_tsutimestamp_st       s_stTxTsuTimestamp1;
static cb_uwbsystem_rangingtroundtreply_st  s_stInitiatorTround1Treply2;

static cb_uwbframework_rangingdatacontainer_st  s_stInitiatorDataContainer =
{
  .dstwrRangingBias = DEF_INITIATOR_RANGING_BIAS,
  .dstwrTroundTreply = {0}
};

static app_rngaoa_responderdatacontainer_st s_stResponderDataContainer = {
  .rangingDataContainer = { .dstwrRangingBias   = 0,
                            .dstwrTroundTreply  = {0}},
  .pdoaDataContainer    = { .rx0_rx1 = 0.0f,
                            .rx0_rx2 = 0.0f,
                            .rx1_rx2 = 0.0f,
                            .elevationEst = 0.0f,
                            .azimuthEst = 0.0f }
};

static double s_measuredDistance = 0.0; // Measured Distance
static uint32_t s_appCycleCount  = 0;   // Logging Purpose: cycle count
static uint8_t s_countOfPdoaScheduledTx = 0;
#else
static cb_uwbsystem_rx_dbb_config_st s_stRxCfg_CfoGainBypass  = { 0 };

static cb_uwbsystem_rx_tsutimestamp_st       s_stRxTsuTimestamp0;
static cb_uwbsystem_tx_tsutimestamp_st       s_stTxTsuTimestamp0;
static cb_uwbsystem_rx_tsutimestamp_st       rxTsuTimestamp1;

static cb_uwbsystem_txpayload_st s_stResultTxPayload;

static uint8_t                            s_countOfPdoaScheduledRx   = 0;
static cb_uwbsystem_rx_signalinfo_st    s_stRssiResults            = {0};
static cb_uwbsystem_rx_signalinfo_st    s_stRssiResults_pdoaRx0    = {0};
static cb_uwbsystem_rx_signalinfo_st    s_stRssiResults_pdoaRx1    = {0};
static cb_uwbsystem_rx_signalinfo_st    s_stRssiResults_pdoaRx2    = {0};
static double                             s_measuredDistance         = 0.0;  // 由 initiator 回传的测距结果
static cb_uwbsystem_pdoaresult_st         s_stPdoaOutputResult       = {0};
static float                              s_pd01Bias                 = DEF_PDOA_PD01_BIAS;
static float                              s_pd02Bias                 = DEF_PDOA_PD02_BIAS;
static float                              s_pd12Bias                 = DEF_PDOA_PD12_BIAS;
static float                              s_aziResult                = 0.0f;
static float                              s_eleResult                = 0.0f;

static app_rngaoa_responderdatacontainer_st s_stResponderDataContainer =
{
  .rangingDataContainer = { .dstwrRangingBias   = DEF_RESPONDER_RANGING_BIAS,
                            .dstwrTroundTreply  = {0}},
  .pdoaDataContainer    = { .rx0_rx1 = 0.0f,
                            .rx0_rx2 = 0.0f,
                            .rx1_rx2 = 0.0f,
                            .elevationEst = 0.0f,
                            .azimuthEst = 0.0f }
};

static uint32_t s_appCycleCount  = 0;   // Logging Purpose: cycle count
#endif

#if APP_RNGAOA_ROLE == INITIATOR
//-------------------------------
// RNGAOA: INITIATOR SETUP
//-------------------------------
//-------------------------------------------------------
//    Initiator                         Responder
//     Idle                                Idle
//       |---------1. SYNC ----------------->|
//       |<------- 2. ACK  ------------------|
//     a |---------3. DSTWR (POLL) --------->| d
//     b |<--------4. DSTWR (RESPONSE) ------| e
//     c |---------5. DSTWR (FINAL) -------->| f
//       |---------6. PDOA (n cycles) ------>|
//       |<--------7. RESULT ----------------|
//       |---------8. DISTANCE ------------->|
//     Terminate                         Terminate
//
// DEF_RNGAOA_SYNC_ACK_TIMEOUT_US       : 1 + 2
// DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS: 3 + 4 + 5 + 6 + 7 + 8
// DEF_RNGAOA_APP_CYCLE_TIME_US         : Idle
// DEF_DSTWR_POLL_WAIT_TIME_US          : 3
// DEF_DSTWR_RESPONSE_WAIT_TIME_US      : 4
// DEF_DSTWR_FINAL_WAIT_TIME_US         : 5
// DEF_NUMBER_OF_PDOA_REPEATED_TX       : 6 (n cycles)
// DEF_PDOA_TX_START_WAIT_TIME_US       : 6 (wait responder enter rx)
//
// Initiator: Tround_1 = b - a
//            Treply_2 = c - d
// Responder: Treply_1 = e - d
//            Tround_2 = f - e
//
//  a: s_stTxTsuTimestamp0    d,e,f:   s_stInitiatorDataContainer.dstwrTroundTreply
//  b: s_stRxTsuTimestamp0
//  c: s_stTxTsuTimestamp1
//-------------------------------------------------------
#define DEF_RNGAOA_SYNC_ACK_TIMEOUT_US           MS_TO_US(10)
#define DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS    20
#define DEF_RNGAOA_APP_CYCLE_TIME_US             MS_TO_US(500)
#define DEF_DSTWR_POLL_WAIT_TIME_US              MS_TO_US(1)
#define DEF_DSTWR_RESPONSE_WAIT_TIME_US          MS_TO_US(0)
#define DEF_DSTWR_FINAL_WAIT_TIME_US             MS_TO_US(1)
#define DEF_NUMBER_OF_PDOA_REPEATED_TX           5
#define DEF_PDOA_TX_START_WAIT_TIME_US           MS_TO_US(2)
#else
//-------------------------------
// RNGAOA: RESPONDER SETUP
//-------------------------------
//-------------------------------------------------------
//    Initiator                         Responder
//     Idle                                Idle
//       |---------1. SYNC ----------------->|
//       |<------- 2. ACK  ------------------|
//     a |---------3. DSTWR (POLL) --------->| d
//     b |<--------4. DSTWR (RESPONSE) ------| e
//     c |---------5. DSTWR (FINAL) -------->| f
//       |---------6. PDOA (n cycles) ------>|
//       |<--------7. RESULT ----------------|
//       |---------8. DISTANCE ------------->|
//     Terminate                         Terminate
//
// DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS : 3 + 4 + 5 + 6 + 7 + 8
// DEF_RNGAOA_SYNC_RX_RESTART_TIMEOUT_US : 1
// DEF_RNGAOA_APP_CYCLE_TIME_US          : Idle
// DEF_DSTWR_RESPONSE_WAIT_TIME_US       : 4
// DEF_DSTWR_FINAL_WAIT_TIME_US          : 5
// DEF_NUMBER_OF_PDOA_REPEATED_RX        : 6 (n cycles)
// DEF_RNGAOA_RESULT_WAIT_TIME_US        : 7
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
#define DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS    20
#define DEF_RNGAOA_SYNC_RX_RESTART_TIMEOUT_US    MS_TO_US(4)
#define DEF_RNGAOA_APP_CYCLE_TIME_US             MS_TO_US(498)
#define DEF_DSTWR_RESPONSE_WAIT_TIME_US          MS_TO_US(1)
#define DEF_DSTWR_FINAL_WAIT_TIME_US             MS_TO_US(0)
#define DEF_NUMBER_OF_PDOA_REPEATED_RX           DEF_PDOA_NUMPKT_SUPERFRAME_MAX
#define DEF_RNGAOA_RESULT_WAIT_TIME_US           MS_TO_US(1)
#endif

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void    app_rngaoa_reset                        (void);
void    app_rngaoa_timeout_error_message_print  (void);
#if APP_RNGAOA_ROLE == INITIATOR
static uint8_t app_rngaoa_validate_sync_ack_payload    (void);
#else
static uint8_t app_rngaoa_validate_sync_payload        (void);
#endif
void    app_rngaoa_log                          (void);
static void    app_rngaoa_timer_init                   (uint16_t timeoutMs);
static void    app_rngaoa_timer_off                    (void);
 
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
#if APP_RNGAOA_ROLE == INITIATOR
static void initiator_cb_uwbapp_tx_done(void)
{
  s_stIrqStatus.TxDone = APP_TRUE;
}

static void initiator_cb_uwbapp_rx0_done(void)
{
  s_stIrqStatus.Rx0Done = APP_TRUE;
}

static void initiator_cb_timer_0(void)
{
  s_applicationTimeout = APP_TRUE;
  s_enAppRngAoaFailureState = s_enAppRngaoaState;
  s_enAppRngaoaState = EN_APP_STATE_TERMINATE;
}

static void initiator_irq_callback_init(void)
{
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ,  initiator_cb_uwbapp_tx_done);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, initiator_cb_uwbapp_rx0_done);
  app_irq_register_irqcallback(EN_IRQENTRY_TIMER_0_APP_IRQ,      initiator_cb_timer_0);
}

static void app_rngaoa_uart_command_init(void)
{
  dfu_cfg_t uart_cfg = {0};

  cmd_parser_uart_init();

  uart_cfg.responder = cmd_parser_uart_responder;
  uart_cfg.reinit = cmd_parser_uart_deinit;
  dfu_halder_init(&uart_cfg);

  ftm_handler_init();
}

static void app_rngaoa_uart_command_poll(void)
{
  uint8_t app_cmd_ready_flag = cmd_parser_uart_pooling_cmd();

  if (app_cmd_ready_flag)
  {
    uint16_t received_length = cmd_parser_uart_received_length();
    uint8_t* received_buffer = cmd_parser_uart_received_buffer();

    cmd_parser_uart_process_buffer(received_buffer, received_length, dfu_halder_polling);
    cmd_parser_uart_process_buffer(received_buffer, received_length, ftm_halder_polling);
    memset(received_buffer, 0, received_length);
    cmd_parser_uart_rx_restart();
  }
}

static void app_rngaoa_pause(void)
{
  app_rngaoa_reset();
  s_enAppRngaoaState = EN_APP_STATE_IDLE;
}

static void app_rngaoa_resume(uint32_t* iterationTime)
{
  app_rngaoa_reset();
  *iterationTime = cb_hal_get_time_us();
  s_enAppRngaoaState = EN_APP_STATE_SYNC_TRANSMIT;
}

void app_rngaoa_initiator(void)
{
  uint32_t startTime      = 0;
  uint32_t iterationTime  = 0;
  uint8_t ftm_active_last = ftm_halder_get_state();

  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();

  initiator_irq_callback_init();
  
  //--------------------------------
  // Configure Payload
  //--------------------------------
  cb_uwbsystem_txpayload_st stSyncTxPayloadPack  = {0};
  cb_uwbsystem_txpayload_st stDstwrTxPayloadPack = {0};
  cb_uwbsystem_txpayload_st pdoaTxPayloadPack   = {0};
  cb_uwbsystem_txpayload_st stDistanceTxPayloadPack = {0};

  stSyncTxPayloadPack.ptrAddress      = &s_syncTxPayload[0];
  stSyncTxPayloadPack.payloadSize     = sizeof(s_syncTxPayload);
  
  // RNGAOA Payload
  static uint8_t s_dstwrPayload[1]    = {0x1};
  stDstwrTxPayloadPack.ptrAddress     = &s_dstwrPayload[0];
  stDstwrTxPayloadPack.payloadSize    = sizeof(s_dstwrPayload);
  
  // PDOA Payload
  static uint8_t s_pdoaTxPayload[1]   = {0x2};
  pdoaTxPayloadPack.ptrAddress        = &s_pdoaTxPayload[0];
  pdoaTxPayloadPack.payloadSize       = sizeof(s_pdoaTxPayload);

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
  
  //--------------------------------
  // Configure absolute timer for scheduled PDOA-TX start
  //--------------------------------
  static cb_uwbframework_trx_scheduledconfig_st s_stPdoaRepeatedTxConfig = {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0   :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_28_TX_DONE,       // tx_done  :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0     :: (ABS timer) Select absolute timer
#if (APP_PDOA_HIGH_ACCURACY_MODE == APP_TRUE)
  .timeoutValue         = 800,                          // Minimum 800us for high accuracy mode
#else
  .timeoutValue         = 250,                          // 250us    :: (ABS timer) absolute timer timeout value, unit - us
#endif
  .eventCtrlMask        = EN_UWBCTRL_TX_START_MASK,     // tx start :: (action)    select action upon abs timeout
  };

  //--------------------------------
  // Configure absolute timer for scheduled DISTANCE-TX start
  //--------------------------------
  static cb_uwbframework_trx_scheduledconfig_st s_stDistanceTxConfig = {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0   :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_17_RX0_SFD_DET,   // rx0_sfd  :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0     :: (ABS timer) Select absolute timer
  .timeoutValue         = 700,                          // 700us    :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_TX_START_MASK,     // tx start :: (action)    select action upon abs timeout
  };

  s_enAppRngaoaState = EN_APP_STATE_SYNC_TRANSMIT;

  app_rngaoa_uart_command_init();

  while(1)
  {
    uint8_t ftm_active = ftm_halder_get_state();

    app_rngaoa_uart_command_poll();

    if (ftm_active == APP_TRUE)
    {
      if (ftm_active_last == APP_FALSE)
      {
        app_rngaoa_pause();
      }
      ftm_active_last = ftm_active;
      continue;
    }

    if (ftm_active_last == APP_TRUE)
    {
      app_rngaoa_resume(&iterationTime);
    }

    ftm_active_last = ftm_active;

    switch (s_enAppRngaoaState)
    {
      //-------------------------------------
      // IDLE
      //-------------------------------------        
      case EN_APP_STATE_IDLE:
        // Wait for next cycle
        if (cb_hal_is_time_elapsed_us(iterationTime, DEF_RNGAOA_APP_CYCLE_TIME_US) == CB_PASS)
        {
          s_enAppRngaoaState = EN_APP_STATE_SYNC_TRANSMIT;
        }
        break;

      //-------------------------------------
      // SYNC: TX
      //-------------------------------------        
      case EN_APP_STATE_SYNC_TRANSMIT:
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stSyncTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_SYNC_WAIT_TX_DONE;
        break;
      case EN_APP_STATE_SYNC_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          s_enAppRngaoaState = EN_APP_STATE_SYNC_RECEIVE;   
        }
        break;
        
      //-------------------------------------
      // SYNC: RX (ACK)
      //-------------------------------------        
      case EN_APP_STATE_SYNC_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_SYNC_WAIT_RX_DONE;
        startTime = cb_hal_get_time_us();
        break;
      case EN_APP_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed_us(startTime, DEF_RNGAOA_SYNC_ACK_TIMEOUT_US) == CB_PASS)
        {
          // if SYNC-ACK not received from Responder within 10ms, send SYNC again.
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppRngaoaState = EN_APP_STATE_SYNC_TRANSMIT;
        }
        else if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;
          uint32_t ackValidateResult = app_rngaoa_validate_sync_ack_payload();
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          if (ackValidateResult == APP_TRUE)
          {
            s_enAppRngaoaState = EN_APP_STATE_DSTWR_TRANSMIT_POLL;
            startTime = cb_hal_get_time_us();
          }
          else
          {
            // if SYNC-ACK payload validation failed, send SYNC again.
            s_enAppRngaoaState = EN_APP_STATE_SYNC_TRANSMIT;
          }
        }        
        break;
        
      //-------------------------------------
      // DS-TWR: POLL
      //-------------------------------------    
      case EN_APP_STATE_DSTWR_TRANSMIT_POLL:
        if (cb_hal_is_time_elapsed_us(startTime, DEF_DSTWR_POLL_WAIT_TIME_US) == CB_PASS)
        {
          app_rngaoa_timer_init(DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS);
          
          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_enable_scheduled_trx(s_stDstwrTround1Config);
          #endif
          
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE;
        }
        break;
      case EN_APP_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          
          cb_framework_uwb_get_tx_tsu_timestamp(&s_stTxTsuTimestamp0);
          cb_framework_uwb_tx_end();
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_RECEIVE_RESPONSE;
          startTime = cb_hal_get_time_us();  
        }
        break;
      //-------------------------------------
      // DS-TWR: RESPONSE
      //-------------------------------------       
      case EN_APP_STATE_DSTWR_RECEIVE_RESPONSE:
        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_enable_scheduled_trx(s_stDstwrTreply2Config);
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTround1Config);
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_DEFERRED);

          s_enAppRngaoaState = EN_APP_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE;
        #else
        if (cb_hal_is_time_elapsed_us(startTime, DEF_DSTWR_RESPONSE_WAIT_TIME_US) == CB_PASS)
        {
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE;
        }
        #endif
        break;
      case EN_APP_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE:      
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;
          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTreply2Config);
          #endif
          cb_framework_uwb_get_rx_tsu_timestamp(&s_stRxTsuTimestamp0, EN_UWB_RX_0);
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_TRANSMIT_FINAL;
          startTime = cb_hal_get_time_us(); 
        }
        break;
      //-------------------------------------
      // DS-TWR: FINAL
      //-------------------------------------         
      case EN_APP_STATE_DSTWR_TRANSMIT_FINAL:
        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE;
        #else
        if (cb_hal_is_time_elapsed_us(startTime, DEF_DSTWR_FINAL_WAIT_TIME_US) == CB_PASS)
        {
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE;
        }
        #endif
        break;
      case EN_APP_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_disable_scheduled_trx(s_stDstwrTreply2Config);
          #endif
          cb_framework_uwb_get_tx_tsu_timestamp(&s_stTxTsuTimestamp1);
          cb_framework_uwb_tx_end();
          s_enAppRngaoaState = EN_APP_STATE_WAIT_RESPONDER_READY;
          startTime = cb_hal_get_time_us();
        }
        break;
        
      //-------------------------------------
      // PDOA-TX
      //-------------------------------------
      case EN_APP_STATE_WAIT_RESPONDER_READY:
        if(cb_hal_is_time_elapsed_us(startTime, DEF_PDOA_TX_START_WAIT_TIME_US) == CB_PASS)
        {
          s_enAppRngaoaState = EN_APP_STATE_PDOA_TRANSMIT;
        }
        break;
        
      case EN_APP_STATE_PDOA_TRANSMIT:
        cb_framework_uwb_enable_scheduled_trx(s_stPdoaRepeatedTxConfig);
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &pdoaTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_PDOA_WAIT_TX_DONE_N_REPEATED_TX;
        break;
      
      case EN_APP_STATE_PDOA_WAIT_TX_DONE_N_REPEATED_TX:
        if (s_stIrqStatus.TxDone)
        {
          s_stIrqStatus.TxDone = APP_FALSE;  
          s_countOfPdoaScheduledTx++;          
          if (s_countOfPdoaScheduledTx < DEF_NUMBER_OF_PDOA_REPEATED_TX)
          {
            cb_framework_uwb_configure_scheduled_trx(s_stPdoaRepeatedTxConfig);
            cb_framework_uwb_tx_restart(&stTxIrqEnable, EN_TRX_START_DEFERRED);
          }
          else
          {
            cb_framework_uwb_disable_scheduled_trx(s_stPdoaRepeatedTxConfig);
            cb_framework_uwb_tx_end();
            s_countOfPdoaScheduledTx  = 0;  
            s_enAppRngaoaState   = EN_APP_STATE_RESULT_RECEIVE;  
          }
        }
        break;     
        
      //-------------------------------------
      // Ranging Result: RX
      //-------------------------------------         
      case EN_APP_STATE_RESULT_RECEIVE:
        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_enable_scheduled_trx(s_stDistanceTxConfig);
        #endif

        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_RESULT_WAIT_RX_DONE;
        break;
      case EN_APP_STATE_RESULT_WAIT_RX_DONE:
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {        
          s_stIrqStatus.Rx0Done = APP_FALSE;

          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_configure_scheduled_trx(s_stDistanceTxConfig);
          #endif

          cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();    
          if ((rxStatus.rx0_ok == CB_TRUE) && (rxStatus.crc_fail == CB_FALSE))  
          {  
            uint16_t rxPayloadSize = cb_framework_uwb_get_rx_packet_size(&s_stUwbPacketConfig);
            cb_framework_uwb_get_rx_payload                           ((uint8_t*)(&s_stResponderDataContainer), rxPayloadSize);
            cb_framework_uwb_calculate_initiator_tround_treply        (&s_stInitiatorDataContainer, s_stTxTsuTimestamp0, s_stTxTsuTimestamp1, s_stRxTsuTimestamp0);
            s_measuredDistance = cb_framework_uwb_calculate_distance  (s_stInitiatorDataContainer, s_stResponderDataContainer.rangingDataContainer);
          }
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppRngaoaState = EN_APP_STATE_DISTANCE_TRANSMIT;
        }
        break;
      //-------------------------------------
      // Measured Distance: TX
      //-------------------------------------
      case EN_APP_STATE_DISTANCE_TRANSMIT:
      {
        stDistanceTxPayloadPack.ptrAddress  = (uint8_t*)(&s_measuredDistance);
        stDistanceTxPayloadPack.payloadSize = sizeof(s_measuredDistance);

        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDistanceTxPayloadPack, &stTxIrqEnable, EN_TRX_START_DEFERRED);
        #else
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDistanceTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        #endif
        s_enAppRngaoaState = EN_APP_STATE_DISTANCE_WAIT_TX_DONE;
        break;
      }
      case EN_APP_STATE_DISTANCE_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;

          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_disable_scheduled_trx(s_stDistanceTxConfig);
          #endif

          cb_framework_uwb_tx_end();
          s_enAppRngaoaState = EN_APP_STATE_TERMINATE;
        }
        break;
      //-------------------------------------
      // Terminate
      //-------------------------------------       
      case EN_APP_STATE_TERMINATE:
        #if (DEF_RNGAOA_ENABLE_LOG == APP_TRUE)
          app_rngaoa_log();
        #endif        
        #if (APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_disable_scheduled_trx(dstwrTreply2Config);
        #endif
        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_disable_scheduled_trx(s_stDistanceTxConfig);
        #endif
        app_rngaoa_timer_off();
        app_rngaoa_reset();
        iterationTime = cb_hal_get_time_us();
        s_enAppRngaoaState = EN_APP_STATE_IDLE;
        break;
    }
  }
}
#else
static void app_rngaoa_uart_command_init(void)
{
  dfu_cfg_t uart_cfg = {0};

  cmd_parser_uart_init();

  uart_cfg.responder = cmd_parser_uart_responder;
  uart_cfg.reinit = cmd_parser_uart_deinit;
  dfu_halder_init(&uart_cfg);

  ftm_handler_init();
}

static void app_rngaoa_uart_command_poll(void)
{
  uint8_t app_cmd_ready_flag = cmd_parser_uart_pooling_cmd();

  if (app_cmd_ready_flag)
  {
    uint16_t received_length = cmd_parser_uart_received_length();
    uint8_t* received_buffer = cmd_parser_uart_received_buffer();

    cmd_parser_uart_process_buffer(received_buffer, received_length, dfu_halder_polling);
    cmd_parser_uart_process_buffer(received_buffer, received_length, ftm_halder_polling);
    memset(received_buffer, 0, received_length);
    cmd_parser_uart_rx_restart();
  }
}

static void app_rngaoa_pause(void)
{
  app_rngaoa_reset();
  s_enAppRngaoaState = EN_APP_STATE_IDLE;
}

static void app_rngaoa_resume(uint32_t* iterationTime)
{
  app_rngaoa_reset();
  *iterationTime = cb_hal_get_time_us();
  s_enAppRngaoaState = EN_APP_STATE_SYNC_RECEIVE;
}

static void responder_cb_uwbapp_tx_done(void)
{
  s_stIrqStatus.TxDone = APP_TRUE;
}

static void responder_cb_uwbapp_rx0_done(void)
{
  s_stIrqStatus.Rx0Done = APP_TRUE;
}

static void responder_cb_uwbapp_rx0_sfd_detected(void)
{
  s_stIrqStatus.Rx0SfdDetected = APP_TRUE;
}

static void responder_cb_uwbapp_rx1_sfd_detected(void)
{
  s_stIrqStatus.Rx1SfdDetected = APP_TRUE;
}

static void responder_cb_uwbapp_rx2_sfd_detected(void)
{
  s_stIrqStatus.Rx2SfdDetected = APP_TRUE;
}

static void responder_cb_timer_0(void)
{
  s_applicationTimeout = APP_TRUE;
  s_enAppRngAoaFailureState = s_enAppRngaoaState;
  s_enAppRngaoaState = EN_APP_STATE_TERMINATE;
}

static void responder_irq_callback_init(void)
{
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ,           responder_cb_uwbapp_tx_done);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ,          responder_cb_uwbapp_rx0_done);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_SFD_DET_DONE_APP_IRQ,  responder_cb_uwbapp_rx0_sfd_detected);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX1_SFD_DET_DONE_APP_IRQ,  responder_cb_uwbapp_rx1_sfd_detected);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX2_SFD_DET_DONE_APP_IRQ,  responder_cb_uwbapp_rx2_sfd_detected);
  app_irq_register_irqcallback(EN_IRQENTRY_TIMER_0_APP_IRQ,               responder_cb_timer_0);
}

void app_rngaoa_responder(void)
{
  uint32_t startTime      = 0;
  uint32_t iterationTime  = 0;
  uint8_t ftm_active_last = ftm_halder_get_state();

  cb_uwbalg_poa_outputperpacket_st g_stPoaResult[DEF_PDOA_NUMPKT_SUPERFRAME_MAX];

  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();
  responder_irq_callback_init();

  //--------------------------------
  // Configure Payload and IRQ
  //--------------------------------
  cb_uwbsystem_txpayload_st stSyncAckPayloadPack = {0};
  cb_uwbsystem_txpayload_st stDstwrTxPayloadPack = {0};

  stSyncAckPayloadPack.ptrAddress    = &s_syncAckPayload[0];
  stSyncAckPayloadPack.payloadSize   = sizeof(s_syncAckPayload);
  //  RNGAOA Payload
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

  // PDOA RX IRQ
  cb_uwbsystem_rx_irqenable_st stPdoaRxIrqEnable = { CB_FALSE };
  stPdoaRxIrqEnable.rx0SfdDetDone = CB_TRUE;
  stPdoaRxIrqEnable.rx1SfdDetDone = CB_TRUE;
  stPdoaRxIrqEnable.rx2SfdDetDone = CB_TRUE;

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

  static cb_uwbframework_trx_scheduledconfig_st s_stDistanceRxConfig = {
  .eventTimestampMask   = EN_UWBEVENT_TIMESTAMP_MASK_0, // mask 0    :: (Timestamp) Select timestamp mask to be used
  .eventIndex           = EN_UWBEVENT_28_TX_DONE,       // tx_done   :: (Timestamp) Select event to for timestamp capture
  .absTimer             = EN_UWB_ABSOLUTE_TIMER_0,      // abs0      :: (ABS timer) Select absolute timer
  .timeoutValue         = 500,                          // 500us     :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_RX0_START_MASK,    // rx0 start :: (action)    select action upon abs timeout
  };

  s_enAppRngaoaState = EN_APP_STATE_SYNC_RECEIVE;

  app_rngaoa_uart_command_init();

  while(1)
  {
    uint8_t ftm_active = ftm_halder_get_state();

    app_rngaoa_uart_command_poll();

    if (ftm_active == APP_TRUE)
    {
      if (ftm_active_last == APP_FALSE)
      {
        app_rngaoa_pause();
      }
      ftm_active_last = ftm_active;
      continue;
    }

    if (ftm_active_last == APP_TRUE)
    {
      app_rngaoa_resume(&iterationTime);
    }

    ftm_active_last = ftm_active;

    switch (s_enAppRngaoaState)
    {
      case EN_APP_STATE_IDLE:
      //-------------------------------------
      // IDLE
      //-------------------------------------
        // Wait for next cycle
        if (cb_hal_is_time_elapsed_us(iterationTime, DEF_RNGAOA_APP_CYCLE_TIME_US) == CB_PASS)
        {
          s_enAppRngaoaState = EN_APP_STATE_SYNC_RECEIVE;
        }
        break;
      //-------------------------------------
      // SYNC: RX
      //-------------------------------------
      case EN_APP_STATE_SYNC_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_SYNC_WAIT_RX_DONE;
        startTime = cb_hal_get_time_us();
        break;
      case EN_APP_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed_us(startTime, DEF_RNGAOA_SYNC_RX_RESTART_TIMEOUT_US) == CB_PASS)
        {
          s_enAppRngaoaState = EN_APP_STATE_SYNC_RECEIVE;
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
        }
        else if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;
          s_stRssiResults = cb_framework_uwb_get_rx_rssi(EN_UWB_RX_0);

          uint32_t syncValidateResult = app_rngaoa_validate_sync_payload();
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          if (syncValidateResult == APP_TRUE)
          {
            s_enAppRngaoaState = EN_APP_STATE_SYNC_TRANSMIT;
          }
          else
          {
            // if SYNC-ACK payload validation failed, wait for SYNC-RX again.
            s_enAppRngaoaState = EN_APP_STATE_SYNC_RECEIVE;
          }
        }
        break;
      //-------------------------------------
      // SYNC: TX (ACK)
      //-------------------------------------
      case EN_APP_STATE_SYNC_TRANSMIT:
        cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stSyncAckPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_SYNC_WAIT_TX_DONE;
        break;
      case EN_APP_STATE_SYNC_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;
          cb_framework_uwb_tx_end();
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_RECEIVE_POLL;
        }
        break;
      //-------------------------------------
      // DS-TWR: POLL
      //-------------------------------------
      case EN_APP_STATE_DSTWR_RECEIVE_POLL:
        app_rngaoa_timer_init(DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS);

        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_enable_scheduled_trx(s_stDstwrTreply1Config);
        #endif

        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE;
        break;
      case EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE:
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;

          cb_framework_uwb_get_rx_tsu_timestamp(&s_stRxTsuTimestamp0, EN_UWB_RX_0);
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE;
          startTime = cb_hal_get_time_us();
        }
        break;
      //-------------------------------------
      // DS-TWR: RESPONSE
      //-------------------------------------
      case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE:
        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_enable_scheduled_trx(s_stDstwrTround2Config);

          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTreply1Config);
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE;
        #else
        if (cb_hal_is_time_elapsed_us(startTime, DEF_DSTWR_RESPONSE_WAIT_TIME_US) == CB_PASS)
        {
          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &stDstwrTxPayloadPack, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE;
        }
        #endif
        break;
      case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE:
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;

          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_configure_scheduled_trx(s_stDstwrTround2Config);
          #endif

          cb_framework_uwb_get_tx_tsu_timestamp(&s_stTxTsuTimestamp0);
          cb_framework_uwb_tx_end();
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_RECEIVE_FINAL;
          startTime = cb_hal_get_time_us();
        }
        break;
      //-------------------------------------
      // DS-TWR: FINAL
      //-------------------------------------
      case EN_APP_STATE_DSTWR_RECEIVE_FINAL:
      {
        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE;
        #else
        if (cb_hal_is_time_elapsed_us(startTime, DEF_DSTWR_FINAL_WAIT_TIME_US) == CB_PASS)
        {
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE;
        }
        #endif
        break;
      }
      case EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE:
      {
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;

          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_disable_scheduled_trx(s_stDstwrTround2Config);
          #endif

          cb_framework_uwb_get_rx_tsu_timestamp(&rxTsuTimestamp1, EN_UWB_RX_0);
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppRngaoaState = EN_APP_STATE_PDOA_PREPARE;
        }
        break;
      }
      //-------------------------------------
      // PDOA-RX
      //-------------------------------------
      case EN_APP_STATE_PDOA_PREPARE:
        s_stRxCfg_CfoGainBypass.stRxGain = (cb_uwbsystem_rx_dbb_gain_st){
            .enableBypass = APP_TRUE,
            .gainValue    = s_stRssiResults.gainIdx
        };

        s_stRxCfg_CfoGainBypass.stRxCfo = (cb_uwbsystem_rx_dbb_cfo_st){
            .enableBypass = APP_TRUE,
            .cfoValue     = s_stRssiResults.cfoEst
        };
        cb_framework_uwb_rxconfig_cfo_gain(EN_UWB_CFO_GAIN_SET, &s_stRxCfg_CfoGainBypass);
        s_enAppRngaoaState = EN_APP_STATE_PDOA_RECEIVE;
        break;

      case EN_APP_STATE_PDOA_RECEIVE:
        cb_framework_uwb_rx_start(EN_UWB_RX_ALL, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_PDOA_WAIT_RX_DONE;
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
#if (APP_PDOA_HIGH_ACCURACY_MODE == APP_TRUE)
            cb_framework_uwb_rx_end(EN_UWB_RX_ALL);
            cb_framework_uwb_rx_start(EN_UWB_RX_ALL, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED);
#else
            cb_framework_uwb_rx_restart(EN_UWB_RX_ALL, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED);
#endif
          }
          else
          {
            s_stRssiResults_pdoaRx0 = cb_framework_uwb_get_rx_rssi(EN_UWB_RX_0);
            s_stRssiResults_pdoaRx1 = cb_framework_uwb_get_rx_rssi(EN_UWB_RX_1);
            s_stRssiResults_pdoaRx2 = cb_framework_uwb_get_rx_rssi(EN_UWB_RX_2);
            cb_framework_uwb_rx_end(EN_UWB_RX_ALL);
            s_countOfPdoaScheduledRx = 0;
            cb_framework_uwb_rxconfig_cfo_gain(EN_UWB_CFO_GAIN_RESET, NULL);
            s_enAppRngaoaState = EN_APP_STATE_PDOA_POSTINGPROCESSING;
          }
        }
        break;

      case EN_APP_STATE_PDOA_POSTINGPROCESSING:
      {
        // PDOA
        cb_framework_uwb_pdoa_calculate_result(&s_stPdoaOutputResult,EN_PDOA_3D_CALTYPE, DEF_NUMBER_OF_PDOA_REPEATED_RX);
        // AOA
        cb_framework_uwb_pdoa_calculate_aoa(s_stPdoaOutputResult.median, s_pd01Bias, s_pd02Bias, s_pd12Bias, &s_aziResult, &s_eleResult);

        s_enAppRngaoaState = EN_APP_STATE_RESULT_TRANSMIT;
        startTime = cb_hal_get_time_us();
        break;
      }
      //-------------------------------------
      // Ranging Result: TX
      //-------------------------------------
      case EN_APP_STATE_RESULT_TRANSMIT:
      {
        if (cb_hal_is_time_elapsed_us(startTime, DEF_RNGAOA_RESULT_WAIT_TIME_US) == CB_PASS)
        {
          cb_framework_uwb_calculate_responder_tround_treply(&s_stResponderDataContainer.rangingDataContainer, s_stTxTsuTimestamp0, s_stRxTsuTimestamp0, rxTsuTimestamp1);
          s_stResponderDataContainer.pdoaDataContainer.rx0_rx1      = s_stPdoaOutputResult.median.rx0_rx1;
          s_stResponderDataContainer.pdoaDataContainer.rx0_rx2      = s_stPdoaOutputResult.median.rx0_rx2;
          s_stResponderDataContainer.pdoaDataContainer.rx1_rx2      = s_stPdoaOutputResult.median.rx1_rx2;
          s_stResponderDataContainer.pdoaDataContainer.azimuthEst   = s_aziResult;
          s_stResponderDataContainer.pdoaDataContainer.elevationEst = s_eleResult;
          s_stResultTxPayload.ptrAddress = (uint8_t*)(&s_stResponderDataContainer);
          s_stResultTxPayload.payloadSize = sizeof(s_stResponderDataContainer);

          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_enable_scheduled_trx(s_stDistanceRxConfig);
          #endif

          cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &s_stResultTxPayload, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
          s_enAppRngaoaState = EN_APP_STATE_RESULT_WAIT_TX_DONE;
        }
        break;
      }
      case EN_APP_STATE_RESULT_WAIT_TX_DONE:
      {
        if (s_stIrqStatus.TxDone == APP_TRUE)
        {
          s_stIrqStatus.TxDone = APP_FALSE;

          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_configure_scheduled_trx(s_stDistanceRxConfig);
          #endif

          cb_framework_uwb_tx_end();
          s_enAppRngaoaState = EN_APP_STATE_DISTANCE_RECEIVE;
        }
        break;
      }
      //-------------------------------------
      // Measured Distance: RX
      //-------------------------------------
      case EN_APP_STATE_DISTANCE_RECEIVE:
      {
        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_DEFERRED);
        #else
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        #endif
        s_enAppRngaoaState = EN_APP_STATE_DISTANCE_WAIT_RX_DONE;
        break;
      }
      case EN_APP_STATE_DISTANCE_WAIT_RX_DONE:
      {
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {
          s_stIrqStatus.Rx0Done = APP_FALSE;

          #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_disable_scheduled_trx(s_stDistanceRxConfig);
          #endif

          cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();
          if ((rxStatus.rx0_ok == CB_TRUE) && (rxStatus.crc_fail == CB_FALSE))
          {
            cb_framework_uwb_get_rx_payload((uint8_t*)(&s_measuredDistance), sizeof(s_measuredDistance));
          }
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppRngaoaState = EN_APP_STATE_TERMINATE;
        }
        break;
      }
      //-------------------------------------
      // Terminate
      //-------------------------------------
      case EN_APP_STATE_TERMINATE:
      {
        #if (DEF_RNGAOA_ENABLE_LOG == APP_TRUE)
          app_rngaoa_log();
        #endif
        #if (DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
        cb_framework_uwb_disable_scheduled_trx(s_stDistanceRxConfig);
        #endif
        app_rngaoa_timer_off();
        app_rngaoa_reset();
        iterationTime = cb_hal_get_time_us();
        s_enAppRngaoaState = EN_APP_STATE_IDLE;
        break;
      }
    }
  }
}
#endif

/**
 * @brief   Resets all member variables
 * @details This function turns on resets all the member variables
*/
#if APP_RNGAOA_ROLE == INITIATOR
static void app_rngaoa_reset(void)
{
  s_measuredDistance = 0.0;

  s_stIrqStatus.TxDone         = APP_FALSE;
  s_stIrqStatus.Rx0Done        = APP_FALSE;
  s_applicationTimeout         = APP_FALSE;
  s_enAppRngAoaFailureState    = EN_APP_STATE_IDLE;
  memset(&s_stInitiatorDataContainer,   0, sizeof(s_stInitiatorDataContainer));
  memset(&s_stResponderDataContainer,   0, sizeof(s_stResponderDataContainer));
  s_stInitiatorDataContainer.dstwrRangingBias = DEF_INITIATOR_RANGING_BIAS;
  s_countOfPdoaScheduledTx  = 0;

  cb_framework_uwb_tsu_clear();
  cb_framework_uwb_tx_end();            // ensure propoer TX end upon abnormal condition
  cb_framework_uwb_rx_end(EN_UWB_RX_0); // ensure propoer RX end upon abnormal condition
}
#else
static void app_rngaoa_reset(void)
{
  s_stIrqStatus.TxDone         = APP_FALSE;
  s_stIrqStatus.Rx0SfdDetected = APP_FALSE;
  s_stIrqStatus.Rx0Done        = APP_FALSE;
  s_stIrqStatus.Rx1SfdDetected = APP_FALSE;
  s_stIrqStatus.Rx2SfdDetected = APP_FALSE;
  s_applicationTimeout         = APP_FALSE;
  s_enAppRngAoaFailureState          = EN_APP_STATE_IDLE;
  memset(&s_stResponderDataContainer, 0, sizeof(s_stResponderDataContainer));
  s_stResponderDataContainer.rangingDataContainer.dstwrRangingBias = DEF_RESPONDER_RANGING_BIAS;
  s_aziResult = 0.0f;
  s_eleResult = 0.0f;
  s_measuredDistance = 0.0;
  memset(&s_stRssiResults_pdoaRx0, 0, sizeof(s_stRssiResults_pdoaRx0));
  memset(&s_stRssiResults_pdoaRx1, 0, sizeof(s_stRssiResults_pdoaRx1));
  memset(&s_stRssiResults_pdoaRx2, 0, sizeof(s_stRssiResults_pdoaRx2));
  memset(&s_stPdoaOutputResult, 0, sizeof(s_stPdoaOutputResult));
  cb_framework_uwb_pdoa_reset_cir_data_container();
  cb_framework_uwb_tsu_clear();
  cb_framework_uwb_tx_end();            // ensure propoer TX end upon abnormal condition
  cb_framework_uwb_rx_end(EN_UWB_RX_0); // ensure propoer RX end upon abnormal condition
  cb_framework_uwb_rxconfig_cfo_gain(EN_UWB_CFO_GAIN_RESET, NULL); // ensure CFO and gain settings are reset upon abnormal condition
  s_countOfPdoaScheduledRx = 0;
}
#endif

/**
 * @brief   Initialize the application timer module.
 * @details This function turns on Timer 0 disables their interrupts, 
 *          and enables CPU Timer 0 interrupts.
 */
static void app_rngaoa_timer_init(uint16_t timeoutMs)
{
  // Turn on Timer 0
  cb_scr_timer0_module_on();

  // Disable Timer's module interrupt
  cb_timer_disable_interrupt();
  
  // Enable CPU Timer 0,1,2,3 Interrupt
  NVIC_EnableIRQ(TIMER_0_IRQn);

  // Configure TIMER0-EVENT0 in One_shot mode that timeout every DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS ms.
  stTimerSetUp stTimerSetup;
  stTimerSetup.Timer                        = EN_TIMER_0;
  stTimerSetup.TimerMode                    = EN_TIMER_MODE_ONE_SHOT;
  stTimerSetup.TimeUnit                     = EN_TIMER_MS;

  stTimerSetup.stTimeOut.timeoutVal[0]      = timeoutMs;
  stTimerSetup.stTimeOut.timeoutVal[1]      = 0;
  stTimerSetup.stTimeOut.timeoutVal[2]      = 0;
  stTimerSetup.stTimeOut.timeoutVal[3]      = 0;
  stTimerSetup.stTimeOut.TimerTimeoutEvent  = DEF_TIMER_TIMEOUT_EVENT_0;
  stTimerSetup.TimerEvtComMode              = EN_TIMER_EVTCOM_MODE_00;
  stTimerSetup.AutoStartTimer               = EN_START_TIMER_ENABLE;
  stTimerSetup.TimerEvtComEnable            = EN_TIMER_EVTCOM_DISABLE;
  stTimerSetup.TimerInterrupt               = EN_TIMER_INTERUPT_ENABLE;
  stTimerSetup.stPulseWidth.NumberOfCycles  = 0;
  stTimerSetup.stPulseWidth.TimerPulseWidth = EN_TIMER_PULSEWIDTH_DISABLE;
  
  cb_timer_configure_timer(&stTimerSetup);
}

#if APP_RNGAOA_ROLE == INITIATOR
static uint8_t app_rngaoa_validate_sync_ack_payload(void)
{
  uint8_t  result = APP_TRUE;
  uint8_t  syncAckPayloadReceived[DEF_SYNC_ACK_RX_PAYLOAD_SIZE] = {0};

  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();

  if ((rxStatus.rx0_ok == CB_TRUE) && (rxStatus.crc_fail == CB_FALSE))
  {
    cb_framework_uwb_get_rx_payload(&syncAckPayloadReceived[0], DEF_SYNC_ACK_RX_PAYLOAD_SIZE);
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
#else
static uint8_t app_rngaoa_validate_sync_payload(void)
{
  uint8_t  result = APP_TRUE;
  uint8_t  syncRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE] = {0};

  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();

  if ((rxStatus.rx0_ok == CB_TRUE) && (rxStatus.crc_fail == CB_FALSE))
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
#endif

/**
 * @brief   Off the application timer module.
 * @details This function turns off Timer 0 disables their interrupts, 
 *          and disable CPU Timer 0 interrupts.
 */
static void app_rngaoa_timer_off(void)
{
  // Disable Timer 0
  cb_timer_disable_timer(EN_TIMER_0);
  
  // Turn off Timer 0
  cb_scr_timer0_module_off();
  
  // Enable CPU Timer 0,1,2,3 Interrupt
  NVIC_DisableIRQ(TIMER_0_IRQn);
}

/**
 * @brief Dstwr Logging function, at different verbosity
 * @param DeviceRole The role of the current device.
 * @details Logging verbosity level is set globally, at file-scope. @see s_logOpt
 * 0: Simple Log (default)
 * 1: Simple Log + CIR log
 * 2: Full log (Simple log + CIR log + Status log)
 */
#if APP_RNGAOA_ROLE == INITIATOR
void app_rngaoa_log(void)
{
  if (!s_applicationTimeout)
  {
    app_uwb_rngaoa_print("Cycle:%u, D:%fcm,", s_appCycleCount++, s_measuredDistance);

    /*Printout*/
    app_uwb_rngaoa_print("PD01:%f, PD02:%f, PD12:%f (in degrees),",(double)s_stResponderDataContainer.pdoaDataContainer.rx0_rx1,(double)s_stResponderDataContainer.pdoaDataContainer.rx0_rx2,(double)s_stResponderDataContainer.pdoaDataContainer.rx1_rx2);
    app_uwb_rngaoa_print("azimuth: %f degrees,elevation: %f degrees\n", (double)s_stResponderDataContainer.pdoaDataContainer.azimuthEst,(double)s_stResponderDataContainer.pdoaDataContainer.elevationEst);
  }
  else
  {
    app_rngaoa_timeout_error_message_print();
    return;
  }
}
#else
void app_rngaoa_log(void)
{
  if (!s_applicationTimeout)
  {
    app_uwb_rngaoa_print("Cycle:%u, D:%fcm,", s_appCycleCount++, s_measuredDistance);
    app_uwb_rngaoa_print("RSSI:(%d,%d,%d)dBm,", s_stRssiResults_pdoaRx0.rssiRx, s_stRssiResults_pdoaRx1.rssiRx, s_stRssiResults_pdoaRx2.rssiRx);

    /*Printout*/
    app_uwb_rngaoa_print("PD01:%f, PD02:%f, PD12:%f (in degrees),",s_stPdoaOutputResult.median.rx0_rx1,s_stPdoaOutputResult.median.rx0_rx2,s_stPdoaOutputResult.median.rx1_rx2);
    app_uwb_rngaoa_print("azimuth: %f degrees,elevation: %f degrees\n", (double)s_aziResult,(double)s_eleResult);
  }
  else
  {
    app_rngaoa_timeout_error_message_print();
    return;
  }
}
#endif

/**
 * @brief Prints a timeout error message based on the current process state.
 *
 * This function logs an error message with the current cycle count and the 
 * specific process state where a timeout occurred. It provides descriptive 
 * feedback for debugging and monitoring the RNGAOA protocol.
 *
 * @param[in] state  The current state of the RNGAOA process when the timeout occurred.
 *                   Possible values:
 *                   - ``EN_IDLE``: Idle state.
 *                   - ``EN_STATE_SYNCING``: Syncing state.
 *                   - ``EN_STATE_RNGAOA``: RNGAOA state.
 *                   - ``EN_STATE_RESULT``: Final result state.
 *                   - ``EN_STATE_DIST_OOB``: Distance out-of-bounds state.
 */
#if APP_RNGAOA_ROLE == INITIATOR
void app_rngaoa_timeout_error_message_print(void)
{
  switch (s_enAppRngAoaFailureState)
  {
    case EN_APP_STATE_IDLE:
      break;
    case EN_APP_STATE_SYNC_TRANSMIT:
      break;
    case EN_APP_STATE_SYNC_WAIT_TX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:SYNC TX\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_SYNC_RECEIVE:
      break;
    case EN_APP_STATE_SYNC_WAIT_RX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:SYNC ACK\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_POLL:
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:RNGAOA TX POLL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_RESPONSE:
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:RNGAOA RX RESPONSE\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_FINAL:
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:RNGAOA TX FINAL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_WAIT_RESPONDER_READY:             // Coordination with Responder
      break;
    case EN_APP_STATE_PDOA_TRANSMIT:                    // PDOA-TX
      break;
    case EN_APP_STATE_PDOA_WAIT_TX_DONE_N_REPEATED_TX:  // PDOA-TX
      break;
    case EN_APP_STATE_RESULT_RECEIVE:
      break;
    case EN_APP_STATE_RESULT_WAIT_RX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:FINAL RESULT\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DISTANCE_TRANSMIT:
      break;
    case EN_APP_STATE_DISTANCE_WAIT_TX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:DISTANCE TX\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_TERMINATE:
      break;
  }
}
#else
void app_rngaoa_timeout_error_message_print(void)
{
  switch (s_enAppRngAoaFailureState)
  {
    case EN_APP_STATE_IDLE:
      break;
    case EN_APP_STATE_SYNC_RECEIVE:
      break;
    case EN_APP_STATE_SYNC_WAIT_RX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:SYNC RX\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_SYNC_TRANSMIT:
      break;
    case EN_APP_STATE_SYNC_WAIT_TX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:SYNC ACK\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_POLL:
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:DSTWR RX POLL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE:
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:DSTWR TX RESPONSE\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_FINAL:
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:DSTWR RX FINAL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_PDOA_PREPARE:
      break;
    case EN_APP_STATE_PDOA_RECEIVE:
      break;
    case EN_APP_STATE_PDOA_WAIT_RX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:PDOA RX\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_PDOA_POSTINGPROCESSING:
      break;
    case EN_APP_STATE_RESULT_TRANSMIT:
      break;
    case EN_APP_STATE_RESULT_WAIT_TX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:FINAL RESULT\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DISTANCE_RECEIVE:
      break;
    case EN_APP_STATE_DISTANCE_WAIT_RX_DONE:
      app_uwb_rngaoa_print("Cycle:%u, Timeout:DISTANCE RX\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_TERMINATE:
      break;
  }
}
#endif

