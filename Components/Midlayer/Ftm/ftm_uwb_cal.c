/**
 * @file    AppCYCal.c
 * @brief   Factory Calibration Module
 * @details This module provides functions used in Factory Calibration.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "CB_efuse.h"
#include "CB_system.h"
#include "CB_uwbframework.h"
#include "AppSysIrqCallback.h"
#include "CB_aoa.h"
#include "ftm_uwb_cal.h"
#include "ftm_cal_nvm.h"

#include "CB_system.h"
#include "CB_timer.h"
#include "CB_scr.h"

#include "NonLIB_sharedUtils.h"

#include "cmd_parser_uart.h"
#include "ftm_handler.h"
#include "dfu_uart.h"
#include "dfu_handler.h"
#define EN_DSTWR_INITIATOR 0
#define EN_DSTWR_RESPONDER 1
//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define FTM_UWB_CAL_UARTPRINT_ENABLE APP_TRUE
#if (FTM_UWB_CAL_UARTPRINT_ENABLE == APP_TRUE)
#include "cmd_parser_uart.h"
  #define LOG(...) app_uart_printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

#define RX0_PKT_OK_BIT 1<<0 //Postion [0]
void app_rngaoa_timer_init(uint16_t timeoutMs);
uint8_t app_rngaoa_validate_sync_ack_payload(void);
static void app_rngaoa_log(void);
void ftm_uwb_cal_rngaoa_tx_sequence(void);
void ftm_uwb_cal_rngaoa_rx_sequence(void);
void app_rngaoa_reset(void);
uint8_t app_rngaoa_validate_sync_payload(void);
void app_rngaoa_timer_off(void);
//-------------------------------
// DEFINE SECTION
//-------------------------------




//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static cb_uwbsystem_tx_irqenable_st txIrqEnable;
static cb_uwbsystem_tx_irqenable_st rxIrqEnable;
static uint8_t  s_simpleTxDoneFlag = APP_FALSE,s_rx_done = APP_FALSE,b_allow_receive = APP_FALSE;
static volatile uint32_t tx_num_of_packet = 0;
static volatile int16_t distance_bias = 0;
static volatile uint16_t tx_interval = 0x14;
static volatile cb_uwbsystem_rxport_en rx_channel = EN_UWB_RX_0;
static volatile uint32_t rx_num_of_packet = 0;
static cb_uwbsystem_txpayload_st s_stResultTxPayload;
static volatile enCALRxChannel rngaoa_rx_channel;
static uint8_t  s_countOfPdoaScheduledRx   = 0;
static uint8_t rangaoa_tx_freq = 0, rangaoa_rx_freq = 20,b_rangaoa_start = APP_FALSE;


#define DEF_RESPONDER_RANGING_BIAS     0
#define DEF_APP_RNGAOA_USE_ABSOLUTE_TIMER APP_TRUE



static cb_uwbsystem_rx_signalinfo_st    s_stRssiResults            = {0};
static cb_uwbsystem_rx_signalinfo_st    s_stRssiResults1           = {0};
static cb_uwbsystem_rx_signalinfo_st    s_stRssiResults2            = {0};
static double s_measuredDistance = 0.0; // Measured Distance
static uint32_t s_appCycleCount  = 0;   // Logging Purpose: cycle count
static uint8_t s_countOfPdoaScheduledTx = 0;


static float  s_aziResult = 0.0f;
static float  s_eleResult = 0.0f;
static float  last_aziResult = 0.0f;
static float  last_eleResult = 0.0f;
static uint32_t distance;
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/* Default tx packet configuration.*/
static cb_uwbsystem_packetconfig_st Txpacketconfig = {
  .prfMode            = EN_PRF_MODE_BPRF_62P4,                 // PRF mode selection
  .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
  .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
  .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_15,       // Preamble code index (9-32)
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
static cb_uwbsystem_packetconfig_st Rxpacketconfig = {
  .prfMode            = EN_PRF_MODE_BPRF_62P4,                 // PRF mode selection
  .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
  .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
  .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_15,       // Preamble code index (9-32)
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

/**
 * @brief Starts the periodic transmission of UWB signals for calibration purposes.
 * 
 * This function initializes the UWB system, sets up the transmission parameters, and begins the periodic
 * transmission of UWB signals. It configures the IRQ callbacks based on the number of packets to be transmitted,
 * and starts the transmission with the specified time interval between packets.
 */
 
static cb_uwbsystem_tx_irqenable_st stTxIrqEnable;

void ftm_uwb_cal_periodic_tx(void)
{
    
  uint8_t powercode = 28;
  ftm_cal_nvm_read_powercode(&powercode);

  uint8_t freqoffsetcalcode = 127;
  ftm_cal_nvm_read_freqoffset(&freqoffsetcalcode);
   
  int8_t statuscode  = 0;
  uint8_t preambleCodeIdx = 0 ;
  statuscode = ftm_cal_nvm_read_preamblecode(&preambleCodeIdx);
  if(statuscode == EN_CAL_OK)
  {
     Txpacketconfig.preambleCodeIndex = preambleCodeIdx;
  }

  cb_uwbsystem_systemconfig_st uwbSystemConfig =
  {   
    .channelNum           = EN_UWB_Channel_9,
    .bbpllFreqOffest_rf   = freqoffsetcalcode,
    .powerCode_tx         = powercode,
    .operationMode_rx     = EN_UWB_RX_OPERATION_MODE_COEXIST,
  };   
    
  cb_system_uwb_set_system_config(&uwbSystemConfig);

  cb_framework_uwb_init();
 
  cb_uwbsystem_txpayload_st txPayload;
  uint8_t payload[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
  txPayload.ptrAddress = &payload[0];
  txPayload.payloadSize = sizeof(payload);
  
  txIrqEnable.txDone  = CB_TRUE;
  txIrqEnable.sfdDone = CB_FALSE;
 
  cmd_parser_uart_rx_restart();
  
  cb_framework_uwb_tx_start(&Txpacketconfig, &txPayload, &txIrqEnable, EN_TRX_START_NON_DEFERRED);  // TX START	
  if(tx_num_of_packet)
  {
     tx_num_of_packet--;
  }   
  while(1)
  {
      
     uint8_t app_cmd_ready_flag = cmd_parser_uart_pooling_cmd();
     if(app_cmd_ready_flag)
     {                
        uint16_t received_length = cmd_parser_uart_received_length(); 
        uint8_t* received_buffer = cmd_parser_uart_received_buffer();
        cmd_parser_uart_process_buffer(received_buffer,received_length,ftm_halder_polling);
        memset(received_buffer, 0, received_length);
        cmd_parser_uart_rx_restart();
     }
     if(s_simpleTxDoneFlag)
     {
        s_simpleTxDoneFlag = APP_FALSE;
        cb_framework_uwb_tx_end(); 
        if(tx_num_of_packet) 
        {
           tx_num_of_packet--;
           cb_system_delay_in_us(tx_interval);
           cb_framework_uwb_tx_start(&Txpacketconfig, &txPayload, &txIrqEnable, EN_TRX_START_NON_DEFERRED);  
        
        } 
                
     }
     if(!tx_num_of_packet) 
     {
        break;
     }
  
  }


}


/**
 * @brief Stops the periodic UWB signal transmission.
 * 
 * This function stops the periodic UWB transmission by deregistering the IRQ callbacks associated
 * with the TX_DONE event and turning off the UWB transmitter.
 */
void ftm_uwb_cal_periodic_tx_Stop(void)
{
  LOG("ftm_uwb_cal_periodic_tx_Stop IRQ_Callback()");
  tx_num_of_packet = 0;
  cb_framework_uwb_tx_end();   
}



/**
 * @brief Starts the UWB reception for calibration purposes.
 * 
 * This function initializes the UWB system and sets up the reception parameters. It also registers the IRQ
 * callback for the RX0_DONE event, starts the UWB reception, and resets the packet count.
 */
void ftm_uwb_cal_comm_rx(void)
{
   
    uint8_t powercode = 28;
    ftm_cal_nvm_read_powercode(&powercode);
    
    uint8_t freqoffsetcalcode = 127;
    ftm_cal_nvm_read_freqoffset(&freqoffsetcalcode);
     
    int8_t statuscode  = 0;
    uint8_t preambleCodeIdx = 0 ;
    statuscode = ftm_cal_nvm_read_preamblecode(&preambleCodeIdx);
    if(statuscode == EN_CAL_OK)
    {
       Rxpacketconfig.preambleCodeIndex = preambleCodeIdx;
    }    
    cb_uwbsystem_systemconfig_st uwbSystemConfig =
    {   
    .channelNum           = EN_UWB_Channel_9,
    .bbpllFreqOffest_rf   = freqoffsetcalcode,
    .powerCode_tx         = powercode,
    .operationMode_rx     = EN_UWB_RX_OPERATION_MODE_COEXIST,
    };   
    
    cb_system_uwb_set_system_config(&uwbSystemConfig);
 
    cb_framework_uwb_init();
    cb_framework_uwb_qmode_trx_enable();  
    
    rx_num_of_packet = 0;
    s_rx_done = APP_FALSE;
    b_allow_receive = APP_TRUE;
    cb_uwbsystem_rx_irqenable_st stRxIrqEnable;
    stRxIrqEnable.rx0Done       = CB_TRUE;
    stRxIrqEnable.rx0PdDone     = CB_FALSE;
    stRxIrqEnable.rx0SfdDetDone = CB_FALSE; 
    
    cmd_parser_uart_rx_restart();
    
    cb_framework_uwb_rx_start(rx_channel, &Rxpacketconfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED); 
    while(1)
    {
       uint8_t app_cmd_ready_flag = cmd_parser_uart_pooling_cmd();
       if(app_cmd_ready_flag)
       {         
          uint16_t received_length = cmd_parser_uart_received_length(); 
          uint8_t* received_buffer = cmd_parser_uart_received_buffer();
          cmd_parser_uart_process_buffer(received_buffer,received_length,ftm_halder_polling);
        // Reset buffer 
          memset(received_buffer, 0, received_length);
          cmd_parser_uart_rx_restart();
       }
       if(s_rx_done == APP_TRUE)
       {
          s_rx_done = APP_FALSE;         
          rx_num_of_packet++;
          if(b_allow_receive == APP_FALSE)
          {
             break;
          }
          else
          {     
             cb_framework_ftm_uwb_rx_restart(rx_channel,&Rxpacketconfig,&stRxIrqEnable,EN_TRX_START_NON_DEFERRED);     
          }
       } 
       if(b_allow_receive == APP_FALSE)
       {
          break;
       }
       
    }

}


/**
 * @brief Stops the UWB reception for calibration purposes.
 * 
 * This function stops the UWB reception by deregistering the IRQ callback associated with the RX0_DONE event
 * and turning off the UWB receiver.
 */
void ftm_uwb_cal_comm_rx_Stop(void)
{
   b_allow_receive = APP_FALSE;
   cb_framework_uwb_rx_end(rx_channel);                 
}


/**
 * @brief Sets the number of packets for transmission during calibration.
 * 
 * This function sets the number of packets that will be transmitted during the calibration process.
 * 
 * @param TxNumOfPacket The number of packets to be transmitted.
 * 
 * @return Always returns EN_CAL_OK.
 */
enCALReturnCode ftm_uwb_cal_set_tx_packets(uint32_t TxNumOfPacket)
{
  tx_num_of_packet = TxNumOfPacket;
  return EN_CAL_OK;
}

/**
 * @brief Sets the transmission interval for periodic transmission.
 * 
 * @param TxInterval The desired transmission interval in milliseconds.
 *                   The function expects TxInterval to be greater than or equal to 0x14.
 *                   If the value is valid, the interval is converted to microseconds and set.
 * @return enCALReturnCode Returns EN_CAL_OK if the interval is successfully set.
 *                         Returns EN_CAL_FAILED if TxInterval is less than 0x14.
 */
enCALReturnCode ftm_uwb_cal_set_tx_interval(uint16_t TxInterval)
{
  if (TxInterval < 0x14) {
    return EN_CAL_FAILED;
  }
  tx_interval = TxInterval * 1000 / 8;
  return EN_CAL_OK;
}

/**
 * @brief Controls the start or stop of periodic UWB signal transmission.
 * 
 * This function enables or disables the periodic UWB transmission based on the input status.
 * If the status is `On`, it starts the transmission, and if the status is `Off`, it stops the transmission.
 * 
 * @param status The switch to turn the transmission `On` or `Off`.
 * @return Returns `EN_CAL_OK` if the operation is successful, otherwise returns `EN_CAL_FAILED`.
 */
enCALReturnCode ftm_uwb_cal_set_tx_onoff(enSwtich status)
{
  if (status == On) {
    ftm_uwb_cal_periodic_tx();
    return EN_CAL_OK;
  } else if (status == Off){
    ftm_uwb_cal_periodic_tx_Stop();
    return EN_CAL_OK;
  } else {
    return EN_CAL_FAILED;
  }
}

/**
 * @brief Sets the UWB RX channel for calibration.
 * 
 * This function sets the UWB RX channel for calibration by converting the provided calibration RX channel
 * to the corresponding UWB RX port and storing it in a global variable.
 * 
 * @param RxChannel The calibration RX channel to be set.
 * @return Returns `EN_CAL_OK` upon successful operation.
 */
enCALReturnCode ftm_uwb_cal_set_rx_channel(enCALRxChannel RxChannel)
{

  switch (RxChannel) {
    case enSingleAntenna1_RX:
      rx_channel = EN_UWB_RX_0;
      break;
    case enSingleAntenna2_RX:
      rx_channel = EN_UWB_RX_1;
      break;
    case enSingleAntenna3_RX:
      rx_channel = EN_UWB_RX_2;
      break;
    case enTripleAntenna1_2_3_RX:
      rx_channel = EN_UWB_RX_ALL;
      break;
    case enDualAntenna1_2_RX:
    case enDualAntenna2_3_RX:
    case enDualAntenna1_3_RX:
      break;
    default:
      rx_channel = EN_UWB_RX_ALL;
      break;
  }
  return EN_CAL_OK;
}

/**
 * @brief Controls the start or stop of UWB signal reception for calibration.
 * 
 * This function enables or disables the UWB reception based on the input status.
 * If the status is `On`, it starts the reception, and if the status is `Off`, it stops the reception.
 * 
 * @param status The switch to turn the reception `On` or `Off`.
 * @return Returns `EN_CAL_OK` if the operation is successful, otherwise returns `EN_CAL_FAILED`.
 */
enCALReturnCode ftm_uwb_cal_set_rx_onoff(enSwtich status)
{
  if (status == On) {
    ftm_uwb_cal_comm_rx();
    return EN_CAL_OK;
  } else if (status == Off) {
    ftm_uwb_cal_comm_rx_Stop();
    return EN_CAL_OK;
  } else {
    return EN_CAL_FAILED;
  }
}

/**
 * @brief Retrieves the number of received UWB packets during calibration.
 * 
 * This function returns the number of UWB packets received during the calibration process.
 * It also optionally prints the number of received packets if UART printing is enabled.
 * 
 * @param RxNumOfPacket Pointer to store the number of received packets.
 * @return Returns `EN_CAL_OK` upon successful operation.
 */
enCALReturnCode ftm_uwb_cal_get_rx_packets(uint32_t* RxNumOfPacket)
{
  *RxNumOfPacket = rx_num_of_packet;
  LOG("Number of Receive: %d", rx_num_of_packet);
  return EN_CAL_OK;
}


/**
 * @brief Suspends the RNGAOA process.
 *
 * This function stops the RNGAOA process by disabling the timer, deregistering IRQ callbacks,
 * and setting the loop flag to `APP_FALSE`.
 */
void ftm_uwb_cal_set_rngaoa_supend (void)
{   
  b_rangaoa_start = APP_FALSE;
  cb_framework_uwb_off();

}




//rangaoa define
#define DEF_SYNC_TX_PAYLOAD_SIZE       4
#define DEF_SYNC_ACK_RX_PAYLOAD_SIZE   3
#define DEF_SYNC_RX_PAYLOAD_SIZE       4

#define DEF_RNGAOA_ENABLE_LOG           APP_TRUE
#define DEF_INITIATOR_RANGING_BIAS     0

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
  EN_APP_STATE_DISTANCE_WAIT_TX_DONE,
    EN_APP_STATE_DISTANCE_WAIT_RX_DONE,
  //TERMINATE STATE
  EN_APP_STATE_TERMINATE,
  
  EN_APP_STATE_DSTWR_RECEIVE_POLL,
  EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE,
  EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE,
  EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE,
  EN_APP_STATE_DSTWR_RECEIVE_FINAL,
  EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE,

  EN_APP_STATE_PDOA_PREPARE,
  EN_APP_STATE_PDOA_RECEIVE,   
  EN_APP_STATE_PDOA_WAIT_RX_DONE,
  EN_APP_STATE_PDOA_POSTINGPROCESSING,  

  EN_APP_STATE_RESULT_TRANSMIT,
  EN_APP_STATE_RESULT_WAIT_TX_DONE,

} app_uwbrngaoa_state_en;



static cb_uwbsystem_pdoaresult_st         s_stPdoaOutputResult       = {0};

static uint8_t s_syncTxPayload  [DEF_SYNC_TX_PAYLOAD_SIZE]      = {0x53,0x59,0x4E,0x43};
                                       
static uint8_t s_syncAckPayload [DEF_SYNC_ACK_RX_PAYLOAD_SIZE]  = {0x41,0x43,0x4B};


//  SYNC RX Payload                                               'S'  'Y'  'N'  'C'
static uint8_t s_syncExpectedRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE] = {0x53,0x59,0x4E,0x43};

static cb_uwbsystem_packetconfig_st s_stUwbPacketConfig = 
{
  .prfMode            = EN_PRF_MODE_BPRF_62P4,                 // PRF mode selection
  .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
  .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
  .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_15,       // Preamble code index (9-32)
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

typedef struct
{
  volatile uint8_t TxDone;
  volatile uint8_t Rx0SfdDetected;
  volatile uint8_t Rx0Done;
  volatile uint8_t Rx1SfdDetected;
  volatile uint8_t Rx2SfdDetected;
    
}app_uwbrngaoa_irqstatus_st;



typedef struct
{
  cb_uwbframework_rangingdatacontainer_st   rangingDataContainer;
  cb_uwbframework_pdoadatacontainer_st      pdoaDataContainer;  
} app_rngaoa_responderdatacontainer_st;


static cb_uwbframework_rangingdatacontainer_st  s_stInitiatorDataContainer = 
{
  .dstwrRangingBias = DEF_INITIATOR_RANGING_BIAS,
  .dstwrTroundTreply = {0}
};
app_rngaoa_responderdatacontainer_st s_stResponderDataContainer = {
  .rangingDataContainer = { .dstwrRangingBias   = DEF_INITIATOR_RANGING_BIAS,
                            .dstwrTroundTreply  = {0}},
  .pdoaDataContainer    = { .rx0_rx1 = 0.0f,
                            .rx0_rx2 = 0.0f,
                            .rx1_rx2 = 0.0f,
                            .elevationEst = 0.0f,
                            .azimuthEst = 0.0f }
};


// PDOA Defines
#define DEF_PDOA_PD01_BIAS              (170.0f)// 3D
#define DEF_PDOA_PD02_BIAS              (40.0f) // 2D,3D
#define DEF_PDOA_PD12_BIAS              (10.0f) // 3D

static app_uwbrngaoa_irqstatus_st s_stIrqStatus             = { APP_FALSE };
static app_uwbrngaoa_state_en     s_enAppRngaoaState        = EN_APP_STATE_IDLE;
static app_uwbrngaoa_state_en     s_enAppRngAoaFailureState = EN_APP_STATE_IDLE;  
static uint8_t s_applicationTimeout = APP_FALSE;

static cb_uwbsystem_rx_tsutimestamp_st       s_stRxTsuTimestamp0;
static cb_uwbsystem_tx_tsutimestamp_st       s_stTxTsuTimestamp0;
static cb_uwbsystem_tx_tsutimestamp_st       s_stTxTsuTimestamp1;
static cb_uwbsystem_rangingtroundtreply_st  s_stInitiatorTround1Treply2;
static cb_uwbsystem_rx_tsutimestamp_st  s_stRxTsuTimestamp0;
static cb_uwbsystem_rx_tsutimestamp_st  rxTsuTimestamp1;
static cb_uwbsystem_rx_dbb_config_st s_stRxCfg_CfoGainBypass  = { 0 };

static float   s_pd01Bias = DEF_PDOA_PD01_BIAS;
static float   s_pd02Bias = DEF_PDOA_PD02_BIAS;
static float   s_pd12Bias = DEF_PDOA_PD12_BIAS;

/**
 * @brief Executes the RNGAOA sequence for UWB communication.
 *
 * This function performs the entire sequence for ranging, including the transmission 
 * and reception of synchronization (SYNC), distance measurement (DSTWR), 
 * and angle of arrival (PDOA) data. It supports the initiator (DSTWR_INITIATOR)
 * role and handles the corresponding operations
 * such as UWB initialization, ranging, synchronization, angle estimation, and result
 * logging.
 *
 * The process follows these steps:
 * 1. UWB initialization and synchronization (initiator transmits, responder listens).
 * 2. DSTWR ranging operation and handling failure or timeout scenarios.
 * 3. PDOA processing based on antenna configurations (2D/3D angle estimation).
 * 4. DSPDOA result transmission and reception for logging.
 * 5. Handles the termination and failure conditions throughout the sequence.
 *
 * @note The function handles both the initiator and responder roles, with different
 *       procedures for each. Timeouts and failures are checked at each stage to ensure
 *       robustness in the communication process.
 *
 */
void ftm_uwb_cal_rngaoa_tx_sequence(void)
{
#define DEF_RNGAOA_SYNC_ACK_TIMEOUT_MS           10
#define DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS    10 
#define DEF_RNGAOA_APP_CYCLE_TIME_MS             500
#define DEF_DSTWR_POLL_WAIT_TIME_MS              1
#define DEF_DSTWR_RESPONSE_WAIT_TIME_MS          0
#define DEF_DSTWR_FINAL_WAIT_TIME_MS             1  
#define DEF_NUMBER_OF_PDOA_REPEATED_TX           5
#define DEF_PDOA_TX_START_WAIT_TIME_MS           2
    
   
  uint32_t startTime      = 0;
  uint32_t iterationTime  = 0;  
  int16_t cal_tof = 0;
  uint8_t errorcode;
  errorcode = ftm_cal_nvm_read_tofcal(&cal_tof);
  if(errorcode == EN_CAL_OK)
  {
     distance_bias  = (cal_tof)/100;
     s_stInitiatorDataContainer.dstwrRangingBias = distance_bias;  
  }
  uint32_t rangAoaId ;
  uint8_t statuscode = ftm_cal_nvm_read_rngaoa_id(&rangAoaId);
  if(statuscode == EN_CAL_OK)
  {
     s_syncTxPayload[0] = (rangAoaId >> 24) & 0xff;
     s_syncTxPayload[1] = (rangAoaId >> 16) & 0xff;
     s_syncTxPayload[2] = (rangAoaId >> 8) & 0xff;
     s_syncTxPayload[3] =  rangAoaId & 0xff;
  }

  uint8_t preambleCodeIdx = 0 ;
  statuscode = ftm_cal_nvm_read_preamblecode(&preambleCodeIdx);
  if(statuscode == EN_CAL_OK)
  {
     s_stUwbPacketConfig.preambleCodeIndex = preambleCodeIdx;
  }  
  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();
  cmd_parser_uart_rx_restart();
  //--------------------------------
  // Configure Payload
  //--------------------------------
  cb_uwbsystem_txpayload_st stSyncTxPayloadPack  = {0};
  cb_uwbsystem_txpayload_st stDstwrTxPayloadPack = {0};
  cb_uwbsystem_txpayload_st pdoaTxPayloadPack   = {0};
  
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
  .timeoutValue         = 250,                          // 250us    :: (ABS timer) absolute timer timeout value, unit - us
  .eventCtrlMask        = EN_UWBCTRL_TX_START_MASK,     // tx start :: (action)    select action upon abs timeout 
  };  
  
  s_enAppRngaoaState = EN_APP_STATE_SYNC_TRANSMIT;
  
  while(b_rangaoa_start)
  {
    uint8_t app_cmd_ready_flag = cmd_parser_uart_pooling_cmd();
    if(app_cmd_ready_flag)
    {         
          uint16_t received_length = cmd_parser_uart_received_length(); 
          uint8_t* received_buffer = cmd_parser_uart_received_buffer();
          cmd_parser_uart_process_buffer(received_buffer,received_length,ftm_halder_polling);
        // Reset buffer 
          memset(received_buffer, 0, received_length);
          cmd_parser_uart_rx_restart();
    }
    switch (s_enAppRngaoaState)
    {
      //-------------------------------------
      // IDLE
      //-------------------------------------        
      case EN_APP_STATE_IDLE:
        // Wait for next cycle
        if (cb_hal_is_time_elapsed(iterationTime, rangaoa_tx_freq))
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
        startTime = cb_hal_get_tick();
        break;
      case EN_APP_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_RNGAOA_SYNC_ACK_TIMEOUT_MS))
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
            startTime = cb_hal_get_tick();
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
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_POLL_WAIT_TIME_MS))
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
          startTime = cb_hal_get_tick();  
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
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_RESPONSE_WAIT_TIME_MS))
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
          startTime = cb_hal_get_tick(); 
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
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_FINAL_WAIT_TIME_MS))
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
          startTime = cb_hal_get_tick();
        }
        break;
        
      //-------------------------------------
      // PDOA-TX
      //-------------------------------------
      case EN_APP_STATE_WAIT_RESPONDER_READY:
        if(cb_hal_is_time_elapsed(startTime, DEF_PDOA_TX_START_WAIT_TIME_MS) == CB_PASS)
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
          if (s_countOfPdoaScheduledTx <= DEF_NUMBER_OF_PDOA_REPEATED_TX)
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
        cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        s_enAppRngaoaState = EN_APP_STATE_RESULT_WAIT_RX_DONE;
        break;
      
      case EN_APP_STATE_RESULT_WAIT_RX_DONE:
        if (s_stIrqStatus.Rx0Done == APP_TRUE)
        {        
          s_stIrqStatus.Rx0Done = APP_FALSE;
          cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status(); 
          if (rxStatus.rx0_ok == CB_TRUE)
          {  
            uint16_t rxPayloadSize = cb_framework_uwb_get_rx_packet_size(&s_stUwbPacketConfig);
            cb_framework_uwb_get_rx_payload                           ((uint8_t*)(&s_stResponderDataContainer), rxPayloadSize);
            cb_framework_uwb_calculate_initiator_tround_treply        (&s_stInitiatorDataContainer, s_stTxTsuTimestamp0, s_stTxTsuTimestamp1, s_stRxTsuTimestamp0);
            s_measuredDistance = cb_framework_uwb_calculate_distance  (s_stInitiatorDataContainer, s_stResponderDataContainer.rangingDataContainer);
          }
          cb_framework_uwb_rx_end(EN_UWB_RX_0);
          s_enAppRngaoaState = EN_APP_STATE_DISTANCE_WAIT_TX_DONE;
        }
        break;
        
      case EN_APP_STATE_DISTANCE_WAIT_TX_DONE:
        {
           uint32_t dis = (uint32_t)s_measuredDistance;
           uint8_t dis_buff[4];
           dis_buff[0] = (dis >> 24) & 0xff;
           dis_buff[1] = (dis >> 16) & 0xff; 
           dis_buff[2] = (dis >> 8) & 0xff; 
           dis_buff[3] = dis & 0xff; 
           cb_uwbsystem_txpayload_st  dis_payload;
           dis_payload.ptrAddress = &dis_buff[0];
           dis_payload.payloadSize = sizeof(dis_buff);
           cb_framework_uwb_tx_start(&s_stUwbPacketConfig, &dis_payload, &stTxIrqEnable, EN_TRX_START_NON_DEFERRED);
           s_enAppRngaoaState = EN_APP_STATE_TERMINATE;
        }

      
        break;
      //-------------------------------------
      // Terminate
      //-------------------------------------       
      case EN_APP_STATE_TERMINATE:
        if (s_stIrqStatus.TxDone)
        {
           #if (DEF_RNGAOA_ENABLE_LOG == APP_TRUE)
              app_rngaoa_log();
            #endif        
            #if (APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
            cb_framework_uwb_disable_scheduled_trx(dstwrTreply2Config);
            #endif
            app_rngaoa_timer_off();
            app_rngaoa_reset();
            iterationTime = cb_hal_get_tick();
            s_enAppRngaoaState = EN_APP_STATE_IDLE;
        
        }
        break;
      
      default: break;
    }
  }

}



void app_rngaoa_reset(void)
{

  s_stIrqStatus.Rx0SfdDetected = APP_FALSE;
  s_stIrqStatus.Rx1SfdDetected = APP_FALSE;
  s_stIrqStatus.Rx2SfdDetected = APP_FALSE;

  s_stResponderDataContainer.rangingDataContainer.dstwrRangingBias = DEF_RESPONDER_RANGING_BIAS;

  s_aziResult = 0.0f;
  s_eleResult = 0.0f;
  memset(&s_stPdoaOutputResult, 0, sizeof(s_stPdoaOutputResult));
  cb_framework_uwb_pdoa_reset_cir_data_container();
    
  s_measuredDistance = 0.0;

  s_stIrqStatus.TxDone         = APP_FALSE;
  s_stIrqStatus.Rx0Done        = APP_FALSE;
  s_applicationTimeout         = APP_FALSE;
  s_enAppRngAoaFailureState    = EN_APP_STATE_IDLE;
  memset(&s_stInitiatorDataContainer,   0, sizeof(s_stInitiatorDataContainer)); 
  memset(&s_stInitiatorDataContainer,   0, sizeof(cb_uwbframework_rangingdatacontainer_st));
  s_countOfPdoaScheduledTx  = 0;
  
  cb_framework_uwb_tsu_clear();
  cb_framework_uwb_tx_end();            // ensure propoer TX end upon abnormal condition
  cb_framework_uwb_rx_end(rx_channel); // ensure propoer RX end upon abnormal condition
  cb_framework_uwb_rxconfig_cfo_gain(EN_UWB_CFO_GAIN_RESET, NULL); // ensure CFO and gain settings are reset upon abnormal condition

}



void ftm_uwb_cal_rngaoa_rx_sequence(void)
{
    
#define DEF_RNGAOA_OVERALL_PROCESS_TIMEOUT_MS    10
#define DEF_RNGAOA_SYNC_RX_RESTART_TIMEOUT_MS    4
#define DEF_RNGAOA_APP_CYCLE_TIME_MS             498
#define DEF_DSTWR_RESPONSE_WAIT_TIME_MS          1
#define DEF_DSTWR_FINAL_WAIT_TIME_MS             0
#define DEF_NUMBER_OF_PDOA_REPEATED_RX           DEF_PDOA_NUMPKT_SUPERFRAME_MAX
#define DEF_RNGAOA_RESULT_WAIT_TIME_MS           1
  
  uint32_t startTime      = 0;
  uint32_t iterationTime  = 0;  
  
  cb_uwbalg_poa_outputperpacket_st g_stPoaResult[DEF_PDOA_NUMPKT_SUPERFRAME_MAX];
  
  uint8_t errorcode;
  int16_t cal_tof = 0;
  errorcode = ftm_cal_nvm_read_tofcal(&cal_tof);
  if(errorcode == EN_CAL_OK)
  {
     distance_bias  = (cal_tof)/100;
     s_stResponderDataContainer.rangingDataContainer.dstwrRangingBias = distance_bias;  
  }
  uint32_t rangAoaId ;
  uint8_t statuscode = ftm_cal_nvm_read_rngaoa_id(&rangAoaId);
  if(EN_CAL_OK == statuscode)
  {
     s_syncExpectedRxPayload[0] = (rangAoaId >> 24) & 0xff;
     s_syncExpectedRxPayload[1] = (rangAoaId >> 16) & 0xff;
     s_syncExpectedRxPayload[2] = (rangAoaId >> 8) & 0xff;
     s_syncExpectedRxPayload[3] =  rangAoaId & 0xff;
  }
  
  uint8_t preambleCodeIdx = 0 ;
  statuscode = ftm_cal_nvm_read_preamblecode(&preambleCodeIdx);
  if(statuscode == EN_CAL_OK)
  {
     s_stUwbPacketConfig.preambleCodeIndex = preambleCodeIdx;
  }  
  
  stCaliAoa aoacal1;
  uint8_t index = 0;
  uint8_t aoa_status = ftm_cal_nvm_read_aoacal(index, &aoacal1);
  if(aoa_status != EN_CAL_OK)
  {
     aoacal1.Calaoah = 0xffff;
     aoacal1.Calaoav = 0xffff;
     aoacal1.Calpdoa1 = 0xffff;
     aoacal1.Calpdoa2 = 0xffff;
  } 
  else
  {
     s_pd01Bias = aoacal1.Calaoah ;
     s_pd02Bias = aoacal1.Calaoav;
     s_pd12Bias = aoacal1.Calpdoa1;
  }
  //--------------------------------
  // Init
  //--------------------------------
  cb_framework_uwb_init();
  cmd_parser_uart_rx_restart();
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
  
  s_enAppRngaoaState = EN_APP_STATE_SYNC_RECEIVE;
  
  while(b_rangaoa_start)
  {
    uint8_t app_cmd_ready_flag = cmd_parser_uart_pooling_cmd();
    if(app_cmd_ready_flag)
    {         
        uint16_t received_length = cmd_parser_uart_received_length(); 
        uint8_t* received_buffer = cmd_parser_uart_received_buffer();
        cmd_parser_uart_process_buffer(received_buffer,received_length,ftm_halder_polling);
        memset(received_buffer, 0, received_length);
        cmd_parser_uart_rx_restart();
    }
    switch (s_enAppRngaoaState)
    {
      case EN_APP_STATE_IDLE:
      //-------------------------------------
      // IDLE
      //-------------------------------------         
        // Wait for next cycle
        if (cb_hal_is_time_elapsed(iterationTime, rangaoa_rx_freq))
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
        startTime = cb_hal_get_tick();
        break;
      case EN_APP_STATE_SYNC_WAIT_RX_DONE:
        if (cb_hal_is_time_elapsed(startTime, DEF_RNGAOA_SYNC_RX_RESTART_TIMEOUT_MS))
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
          startTime = cb_hal_get_tick();
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
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_RESPONSE_WAIT_TIME_MS))
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
          startTime = cb_hal_get_tick();
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
        if (cb_hal_is_time_elapsed(startTime, DEF_DSTWR_FINAL_WAIT_TIME_MS))
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
        if (rngaoa_rx_channel == enDualAntenna1_3_RX) 
           cb_framework_uwb_rx_start(EN_UWB_RX_02, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        else
           cb_framework_uwb_rx_start(EN_UWB_RX_ALL, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED); 
        s_enAppRngaoaState = EN_APP_STATE_PDOA_WAIT_RX_DONE;
        break;
      
      case EN_APP_STATE_PDOA_WAIT_RX_DONE:
        if (s_stIrqStatus.Rx0SfdDetected && s_stIrqStatus.Rx2SfdDetected)
        {
          if((rngaoa_rx_channel != enDualAntenna1_3_RX))
          {
             if(s_stIrqStatus.Rx1SfdDetected)
                s_stIrqStatus.Rx1SfdDetected = APP_FALSE;
             else
             {
                break;
             }                 
          }
          s_stIrqStatus.Rx0SfdDetected = APP_FALSE; 
         
          s_stIrqStatus.Rx2SfdDetected = APP_FALSE;
          
          cb_framework_uwb_pdoa_store_cir_data(s_countOfPdoaScheduledRx);    
          
          if (rngaoa_rx_channel != enDualAntenna1_3_RX) 
          {
              s_stRssiResults1 = cb_framework_uwb_get_rx_rssi(EN_UWB_RX_1);
          }
          s_stRssiResults2 = cb_framework_uwb_get_rx_rssi(EN_UWB_RX_2);
            
          s_countOfPdoaScheduledRx++;
          if (s_countOfPdoaScheduledRx < DEF_NUMBER_OF_PDOA_REPEATED_RX)
          {
            if (rngaoa_rx_channel == enDualAntenna1_3_RX) 
               cb_framework_uwb_rx_restart(EN_UWB_RX_02, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED);
            else
               cb_framework_uwb_rx_restart(EN_UWB_RX_ALL, &s_stUwbPacketConfig, &stPdoaRxIrqEnable, EN_TRX_START_NON_DEFERRED);
          }
          else 
          {
            if (rngaoa_rx_channel == enDualAntenna1_3_RX) 
               cb_framework_uwb_rx_end(EN_UWB_RX_02); 
            else                
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
        if(rngaoa_rx_channel == enDualAntenna1_3_RX) 
           cb_framework_uwb_pdoa_calculate_result(&s_stPdoaOutputResult,EN_PDOA_2D_CALTYPE, DEF_NUMBER_OF_PDOA_REPEATED_RX);  
        else
           cb_framework_uwb_pdoa_calculate_result(&s_stPdoaOutputResult,EN_PDOA_3D_CALTYPE, DEF_NUMBER_OF_PDOA_REPEATED_RX);        
        // AOA
        cb_framework_uwb_pdoa_calculate_aoa(s_stPdoaOutputResult.median, s_pd01Bias, s_pd02Bias, s_pd12Bias, &s_aziResult, &s_eleResult);
          
        last_aziResult = s_aziResult;
        last_eleResult = s_eleResult;
        s_enAppRngaoaState = EN_APP_STATE_RESULT_TRANSMIT;
        startTime = cb_hal_get_tick();
        break;
      }      
      //-------------------------------------
      // Ranging Result: TX
      //-------------------------------------  
      case EN_APP_STATE_RESULT_TRANSMIT:
      {
        if (cb_hal_is_time_elapsed(startTime, DEF_RNGAOA_RESULT_WAIT_TIME_MS))
        {
          cb_framework_uwb_calculate_responder_tround_treply(&s_stResponderDataContainer.rangingDataContainer, s_stTxTsuTimestamp0, s_stRxTsuTimestamp0, rxTsuTimestamp1);
          s_stResponderDataContainer.pdoaDataContainer.rx0_rx1      = s_stPdoaOutputResult.median.rx0_rx1;
          s_stResponderDataContainer.pdoaDataContainer.rx0_rx2      = s_stPdoaOutputResult.median.rx0_rx2;
          s_stResponderDataContainer.pdoaDataContainer.rx1_rx2      = s_stPdoaOutputResult.median.rx1_rx2;
          s_stResponderDataContainer.pdoaDataContainer.azimuthEst   = s_aziResult;
          s_stResponderDataContainer.pdoaDataContainer.elevationEst = s_eleResult;
          s_stResultTxPayload.ptrAddress = (uint8_t*)(&s_stResponderDataContainer);
          s_stResultTxPayload.payloadSize = sizeof(s_stResponderDataContainer);
          
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
          cb_framework_uwb_tx_end();
          s_enAppRngaoaState = EN_APP_STATE_DISTANCE_WAIT_RX_DONE;
          cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_stUwbPacketConfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED);
        }
        break;
      }
      
      case EN_APP_STATE_DISTANCE_WAIT_RX_DONE:
        if(s_stIrqStatus.Rx0Done == APP_TRUE)  
        {  
           cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status(); 
           if (rxStatus.rx0_ok == CB_TRUE)
           {
               uint8_t payload[10];
               s_stIrqStatus.Rx0Done = APP_FALSE;
               uint16_t rxPayloadSize = cb_framework_uwb_get_rx_packet_size(&s_stUwbPacketConfig);
               cb_framework_uwb_get_rx_payload(payload, rxPayloadSize);
               distance = payload[0] << 24 | payload[1] << 16 | payload[2]<< 8| payload[3];
               
           }
           cb_framework_uwb_rx_end(EN_UWB_RX_0);
           s_enAppRngaoaState = EN_APP_STATE_TERMINATE;
        
        }
       break;
      
      
      //-------------------------------------
      // Terminate
      //-------------------------------------  
      case EN_APP_STATE_TERMINATE:
      {
        #if (DEF_RNGAOA_ENABLE_LOG == APP_TRUE)
          app_rngaoa_log();
        #endif   
        #if (APP_RNGAOA_USE_ABSOLUTE_TIMER == APP_TRUE)
          cb_framework_uwb_disable_scheduled_trx(dstwrTround2Config);
        #endif
        app_rngaoa_timer_off();
        app_rngaoa_reset();
        iterationTime = cb_hal_get_tick();
        s_enAppRngaoaState = EN_APP_STATE_IDLE;
        break;
      }
      default: break;
    }
  }  
}



/**
 * @brief Controls the RNGAOA transmission state (On/Off).
 * 
 * This function enables or disables the RNGAOA transmission depending on the provided status.
 * If the status is set to `On`, the function initializes the RNGAOA in initiator mode with the current frequency and relevant parameters.
 * It also sets a task flag (`task_e_execute`) to start the transmission.
 * If the status is set to `Off`, it suspends the RNGAOA transmission.
 * 
 * @param s tatus The transmission state: `On` to start RNGAOA transmission, `Off` to stop it.
 * @return Returns `EN_CAL_OK` upon successful operation, and `EN_CAL_FAILED` if the status is invalid.
 */
enCALReturnCode ftm_uwb_cal_set_rngaoa_tx_onoff(enSwtich status) 
{
  if (status == On) {
    uint8_t powercode = 0;
    ftm_cal_nvm_read_powercode(&powercode);
    uint8_t freqoffsetcalcode = 127;
    ftm_cal_nvm_read_freqoffset(&freqoffsetcalcode);      
//    uint8_t preamble_code=9;
//    ftm_cal_nvm_read_preamblecode(&preamble_code);    

//    int16_t tof = 0;
//    ftm_cal_nvm_read_tofcal(&tof);
//    stCaliAoa aoacal1;
//    uint8_t index = 0;
//    ftm_cal_nvm_read_aoacal(index,&aoacal1);
    uint8_t rngaoa_freq = 20;
    ftm_cal_nvm_read_rngaoa_freq(&rngaoa_freq);
    
    if(rngaoa_freq == 10 || rngaoa_freq == 0x10)
    {
       rangaoa_tx_freq = 100; 
    }
    else if(rngaoa_freq == 20 || rngaoa_freq == 0x20)
    {
       rangaoa_tx_freq = 50; 
    }
    else if(rngaoa_freq == 50 || rngaoa_freq == 0x50)
    {
       rangaoa_tx_freq = 20;
    }
    else
    { 
       rangaoa_tx_freq = 50; 
    }
    
    
    cb_uwbsystem_systemconfig_st uwbSystemConfig =
    {   
      .channelNum           = EN_UWB_Channel_9,
      .bbpllFreqOffest_rf   = freqoffsetcalcode,
      .powerCode_tx         = powercode,
      .operationMode_rx     = EN_UWB_RX_OPERATION_MODE_COEXIST,
    };   
    
    cb_system_uwb_set_system_config(&uwbSystemConfig);
    
    b_rangaoa_start = APP_TRUE;
    ftm_uwb_cal_rngaoa_tx_sequence();

    return EN_CAL_OK;
  } else if (status == Off) {
    ftm_uwb_cal_set_rngaoa_supend();
    return EN_CAL_OK;
  } else {
    return EN_CAL_FAILED;
  }
}

/**
 * @brief Sets the RNGAOA receiving channel.
 * 
 * This function configures the RNGAOA receiving channel to the specified channel.
 * 
 * @param RxChannel The receiving channel to be set.
 * @return Returns `EN_CAL_OK` upon successful operation.
 */
enCALReturnCode ftm_uwb_cal_set_rngaoa_rx_channel(enCALRxChannel RxChannel)
{
  rngaoa_rx_channel = RxChannel;
  return EN_CAL_OK;
}

/**
 * @brief Turns RNGAOA reception on or off.
 * 
 * This function enables or disables RNGAOA reception based on the provided status and channel configuration.
 * 
 * @param status The desired status: `On` to start reception, `Off` to stop reception.
 * @return Returns `EN_CAL_OK` upon successful operation. Returns `EN_CAL_FAILED` if an invalid status is provided.
 */
enCALReturnCode ftm_uwb_cal_set_rngaoa_rx_onoff(enSwtich status)
{
  if (status == On) {
    uint8_t powercode = 28;
    ftm_cal_nvm_read_powercode(&powercode); 

    uint8_t freqoffsetcalcode = 127;
    ftm_cal_nvm_read_freqoffset(&freqoffsetcalcode);      
//    uint8_t preamble_code=9;
//    ftm_cal_nvm_read_preamblecode(&preamble_code);    

//    int16_t tof = 0;
//    ftm_cal_nvm_read_tofcal(&tof);
//    stCaliAoa aoacal1;
//    uint8_t index = 0;
//    ftm_cal_nvm_read_aoacal(index,&aoacal1);
    uint8_t rngaoa_freq = 20;
    ftm_cal_nvm_read_rngaoa_freq(&rngaoa_freq);
    
    if(rngaoa_freq == 10 || rngaoa_freq == 0x10)
    {
       rangaoa_rx_freq = 100 - 4; 
    }
    else if(rngaoa_freq == 20 || rngaoa_freq == 0x20)
    {
       rangaoa_rx_freq = 50 - 4; 
    }
    else if(rngaoa_freq == 50 || rngaoa_freq == 0x50)
    {
       rangaoa_rx_freq = 20 - 4;
    }
    else
    { 
       rangaoa_rx_freq = 50 - 4; 
    }
 
    cb_uwbsystem_systemconfig_st uwbSystemConfig =
    {   
      .channelNum           = EN_UWB_Channel_9,
      .bbpllFreqOffest_rf   = freqoffsetcalcode,
      .powerCode_tx         = powercode,
      .operationMode_rx     = EN_UWB_RX_OPERATION_MODE_COEXIST,
    };   
    
    cb_system_uwb_set_system_config(&uwbSystemConfig);
    b_rangaoa_start = APP_TRUE;
    ftm_uwb_cal_rngaoa_rx_sequence();
    return EN_CAL_OK;
  } else if (status == Off) {
    ftm_uwb_cal_set_rngaoa_supend();
    return EN_CAL_OK;
  } else {
    return EN_CAL_FAILED;
	}

}

/**
 * @brief Retrieves the RNGAOA reception results.
 * 
 * This function retrieves the latest RNGAOA reception results, including distance, angle, phase differences, and signal strength.
 * Results are stored in the provided data container for further processing.
 * 
 * @param stRNGAOARxDataContainer Pointer to a data container to store the retrieved results.
 * @return Returns `EN_CAL_OK` upon successful operation.
 */
enCALReturnCode ftm_uwb_cal_get_rngaoa_rx_resulf(stRNGAOARx* stRNGAOARxDataContainer)
{
    
  stRNGAOARxDataContainer->dis = (distance*100);
  stRNGAOARxDataContainer->aoah = (int16_t)last_aziResult;
  stRNGAOARxDataContainer->aoav = (int16_t)last_eleResult;
  stRNGAOARxDataContainer->pdoah = 0;
  stRNGAOARxDataContainer->pdoav = 0;
  stRNGAOARxDataContainer->avg_rsl1 = (int16_t)s_stRssiResults.rssiRx;
  stRNGAOARxDataContainer->avg_rsl2 = (int16_t)s_stRssiResults1.rssiRx;
  stRNGAOARxDataContainer->avg_rsl3 = (int16_t)s_stRssiResults2.rssiRx;
  
  return EN_CAL_OK;
}


uint8_t app_rngaoa_validate_sync_ack_payload(void)
{
  uint8_t  result = APP_TRUE;
  uint16_t rxPayloadSize = 0;
  uint8_t  syncAckPayloadReceived[DEF_SYNC_ACK_RX_PAYLOAD_SIZE] = {0};
  
  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();
          
  if (rxStatus.rx0_ok   == CB_TRUE)
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



static void app_rngaoa_timeout_error_message_print(void)
{
  switch (s_enAppRngAoaFailureState)
  {
    case EN_APP_STATE_IDLE:
      break;
    case EN_APP_STATE_SYNC_TRANSMIT:
      break;
    case EN_APP_STATE_SYNC_WAIT_TX_DONE:
      LOG("Cycle:%u, Timeout:SYNC TX\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_SYNC_RECEIVE:
      break;
    case EN_APP_STATE_SYNC_WAIT_RX_DONE:
      LOG("Cycle:%u, Timeout:SYNC ACK\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_POLL:
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_POLL_WAIT_TX_DONE:
      LOG("Cycle:%u, Timeout:RNGAOA TX POLL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_RESPONSE:
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_RESPONSE_WAIT_RX_DONE:
      LOG("Cycle:%u, Timeout:RNGAOA RX RESPONSE\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_FINAL:
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_FINAL_WAIT_TX_DONE:
      LOG("Cycle:%u, Timeout:RNGAOA TX FINAL\n", s_appCycleCount++);
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
      LOG("Cycle:%u, Timeout:FINAL RESULT\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_TERMINATE:
      break;    
      
    case EN_APP_STATE_DSTWR_RECEIVE_POLL:
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_POLL_WAIT_RX_DONE:
      LOG("Cycle:%u, Timeout:DSTWR RX POLL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE:
      break;
    case EN_APP_STATE_DSTWR_TRANSMIT_RESPONSE_WAIT_TX_DONE:
      LOG("Cycle:%u, Timeout:DSTWR TX RESPONSE\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_FINAL:
      break;
    case EN_APP_STATE_DSTWR_RECEIVE_FINAL_WAIT_RX_DONE:
      LOG("Cycle:%u, Timeout:DSTWR RX FINAL\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_PDOA_PREPARE:
      break;
    case EN_APP_STATE_PDOA_RECEIVE:
      break;
    case EN_APP_STATE_PDOA_WAIT_RX_DONE:
      LOG("Cycle:%u, Timeout:PDOA RX\n", s_appCycleCount++);
      break;
    case EN_APP_STATE_PDOA_POSTINGPROCESSING:
      break;
    case EN_APP_STATE_RESULT_TRANSMIT:
      break;
    case EN_APP_STATE_RESULT_WAIT_TX_DONE:
      LOG("Cycle:%u, Timeout:FINAL RESULT\n", s_appCycleCount++);
      break;
    
    default: break;
 
  }
}

static void app_rngaoa_log(void) 
{
  if (!s_applicationTimeout)
  {
    LOG("Cycle:%u, D:%fcm\n", s_appCycleCount++, s_measuredDistance);

    /*Printout*/
    LOG("PD01:%f, PD02:%f, PD12:%f (in degrees)\n",(double)s_stResponderDataContainer.pdoaDataContainer.rx0_rx1,(double)s_stResponderDataContainer.pdoaDataContainer.rx0_rx2,(double)s_stResponderDataContainer.pdoaDataContainer.rx1_rx2);          
    LOG("azimuth: %f degrees\nelevation: %f degrees\n", (double)s_stResponderDataContainer.pdoaDataContainer.azimuthEst,(double)s_stResponderDataContainer.pdoaDataContainer.elevationEst);         
  }
  else
  {
    app_rngaoa_timeout_error_message_print();
    return;
  } 
}


uint8_t app_rngaoa_validate_sync_payload(void)
{
  uint8_t  result = APP_TRUE;
  uint16_t rxPayloadSize = 0;
  uint8_t  syncRxPayload[DEF_SYNC_RX_PAYLOAD_SIZE] = {0};
  
  cb_uwbsystem_rxstatus_un rxStatus = cb_framework_uwb_get_rx_status();
          
  if (rxStatus.rx0_ok   == CB_TRUE)
  {
    cb_framework_uwb_get_rx_payload(&syncRxPayload[0],DEF_SYNC_RX_PAYLOAD_SIZE);
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
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered.
 */
void cb_uwbapp_tx_done_irqhandler(void)
{
  s_stIrqStatus.TxDone = APP_TRUE;
  s_simpleTxDoneFlag = APP_TRUE;

}


void cb_uwbapp_rx0_done_irqcb(void)
{
  s_stIrqStatus.Rx0Done = APP_TRUE;
  s_rx_done = APP_TRUE;

}


void cb_uwbapp_rx1_done_irqcb(void)
{
  s_stIrqStatus.Rx0Done = APP_TRUE;
  s_rx_done = APP_TRUE;

}

void cb_uwbapp_rx2_done_irqcb(void)
{
  s_stIrqStatus.Rx0Done = APP_TRUE;
  s_rx_done = APP_TRUE;
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


void app_rngaoa_timer_init(uint16_t timeoutMs)
{
 
  cb_scr_timer0_module_on();
  cb_timer_disable_interrupt();  
  NVIC_EnableIRQ(TIMER_0_IRQn);
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


void app_rngaoa_timer_off(void)
{
  // Disable Timer 0
  cb_timer_disable_timer(EN_TIMER_0);
  
  // Turn off Timer 0
  cb_scr_timer0_module_off();

  // Enable CPU Timer 0,1,2,3 Interrupt
  NVIC_DisableIRQ(TIMER_0_IRQn);
}

/**
 * @brief   Callback function for Timer 0 IRQ.
 *
 * This function is called when the Timer 0 IRQ is triggered.
 */
void cb_timer_0_app_irq_callback (void)
{
  s_applicationTimeout = APP_TRUE;
  s_enAppRngAoaFailureState = s_enAppRngaoaState;
  s_enAppRngaoaState = EN_APP_STATE_TERMINATE;
}
