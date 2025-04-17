/**
 * @file    CB_uwbframework.c
 * @brief   UWB Framework interface
 * @details This file defines the interface for the UWB Framework, which provides
 *          a comprehensive set of functions for UWB communication, ranging, and
 *          positioning. It includes functions for initialization, transmission,
 *          reception, timing, ranging calculations, and Phase Difference of
 *          Arrival (PDoA) processing.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdlib.h>
#include <string.h>
#include "CB_CompileOption.h"
#include "CB_uwbframework.h"
#include "CB_system.h"
#include "CB_UwbDrivers.h"
#include "CB_aoa.h"

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
// GLOBAL VARIABLE SECTION
//-------------------------------
cb_uwbsystem_rx_cir_iqdata_st     g_stPdoaRxCirDataContainer[DEF_PDOA_NUMPKT_SUPERFRAME_MAX][DEF_PDOA_NUM_RX_USED][DEF_PDOA_NUM_CIR_DATASET];//CIR
cb_uwbalg_poa_outputperpacket_st  g_stPoaResult[DEF_PDOA_NUMPKT_SUPERFRAME_MAX];

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//------------------------------- 
//----------------------------------------------------------------//
//                      UWB INIT and OFF                          //
//----------------------------------------------------------------//
/**
 * @brief Initialize the UWB communication transmitter.
 *
 * This function initializes the UWB communication transmitter by performing various configuration steps.
 * It sets up the required parameters and resources for UWB communication.
 */
void cb_framework_uwb_init(void)
{
  cb_system_uwb_init();       // UWB Init
  cb_system_uwb_tx_memclr();  // TX memory clear
  cb_system_uwb_rx_memclr();  // RX memory clear
}

/**
 * @brief Deinitialize the UWB communication transmitter.
 *
 * This function deinitializes the UWB communication transmitter by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 */
void cb_framework_uwb_off(void)
{
  cb_system_uwb_off();        // UWB OFF
  cb_system_uwb_tx_memclr();  // TX memory clear
  cb_system_uwb_rx_memclr();  // RX memory clear
}

//----------------------------------------------------------------//
//                 COMM-TRX QUICK MODE API                        //
//----------------------------------------------------------------//
/**
 * @brief Enable the UWB communication transmitter in quick mode.
 *
 * This function enables the UWB communication transmitter in quick mode by performing various configuration steps.
 * It sets up the required parameters and resources for UWB communication.
 */
void cb_framework_uwb_qmode_trx_enable(void)
{
  cb_system_uwb_rx_top_init(); // RX TOP INIT
  cb_system_uwb_trx_init();    // TRX INIT
}

/**
 * @brief Disable the UWB communication transmitter in quick mode.
 *
 * This function disables the UWB communication transmitter in quick mode by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 */
void cb_framework_uwb_qmode_trx_disable(void)
{
  cb_system_uwb_trx_off();     // TRX OFF
  cb_system_uwb_rx_top_off();  // RX TOP OFF
}

/**
 * @brief Start UWB transmission in quick mode
 * 
 * @param txPacketConfig Configuration for the packet to be transmitted
 * @param txPayload Payload data to be transmitted
 * @param txIrqEnable Interrupt enable configuration for transmission
 */
void cb_framework_uwb_qmode_tx_start(cb_uwbsystem_packetconfig_st* txPacketConfig,cb_uwbsystem_txpayload_st* txPayload, cb_uwbsystem_tx_irqenable_st* stTxIrqEnable)
{
  cb_system_uwb_config_tx(txPacketConfig, txPayload, stTxIrqEnable);  // Config TX
  cb_system_uwb_tx_start();                                         // TX START
}

/**
 * @brief End the UWB communication transmitter in quick mode.
 *
 * This function ends the UWB communication transmitter in quick mode by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 */
void cb_framework_uwb_qmode_tx_end(void)
{
  cb_system_uwb_tx_stop();    // TX STOP
}

/**
 * @brief Start the UWB communication receiver in quick mode.
 *
 * This function starts the UWB communication receiver in quick mode by performing various configuration steps.
 * It sets up the required parameters and resources for UWB communication.
 * 
 * @param enRxPort The RX port to use for reception
 * @param rxPacketConfig Configuration for the packet to be received
 * @param stRxIrqEnable Interrupt enable configuration for reception
 */
void cb_framework_uwb_qmode_rx_start(cb_uwbsystem_rxport_en enRxPort,cb_uwbsystem_packetconfig_st* rxPacketConfig, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable)
{
  cb_system_uwb_config_rx(rxPacketConfig, stRxIrqEnable);   // RX Config
  cb_system_uwb_rx_start(enRxPort);                         // RX START
}

/**
 * @brief End the UWB communication receiver in quick mode.
 *
 * This function ends the UWB communication receiver in quick mode by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 * 
 * @param enRxPort The RX port to stop
 */
void cb_framework_uwb_qmode_rx_end (cb_uwbsystem_rxport_en enRxPort)
{
  cb_system_uwb_rx_stop(enRxPort);          // RX STOP
}

//----------------------------------------------------------------//
//                 COMM-TRX NORMAL MODE API                       //
//----------------------------------------------------------------//
/**
 * @brief Start the UWB communication transmitter in normal mode.
 *
 * This function starts the UWB communication transmitter in normal mode by performing various configuration steps.
 * It sets up the required parameters and resources for UWB communication.
 * 
 * @param txPacketConfig Configuration for the packet to be transmitted
 * @param txPayload Payload data to be transmitted
 * @param stTxIrqEnable Interrupt enable configuration for transmission
 * @param trxStartMode Start mode (immediate or deferred)
 */
void cb_framework_uwb_tx_start(cb_uwbsystem_packetconfig_st* txPacketConfig, cb_uwbsystem_txpayload_st* txPayload, cb_uwbsystem_tx_irqenable_st* stTxIrqEnable, cb_uwbframework_trx_startmode_en trxStartMode)
{
  cb_system_uwb_tx_init     ();                                      // TX Init
  cb_system_uwb_config_tx   (txPacketConfig, txPayload, stTxIrqEnable);// TX Config
  
  switch (trxStartMode)
  {
    case EN_TRX_START_NON_DEFERRED:
      cb_system_uwb_tx_start ();        // TX Start     
      break;
    case EN_TRX_START_DEFERRED:
      cb_system_uwb_tx_start_prepare(); // TX Start (deferred)
      break;
  }  
}

/**
 * @brief End the UWB communication transmitter in normal mode.
 *
 * This function ends the UWB communication transmitter in normal mode by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 */
void cb_framework_uwb_tx_end(void)
{
  cb_system_uwb_tx_stop();  // TX Stop
  cb_system_uwb_tx_off();   // TX Off
}

/**
 * @brief Start UWB reception in normal mode
 * 
 * @param enRxPort The RX port to use for reception
 * @param rxPacketConfig Configuration for the packet to be received
 * @param stRxIrqEnable Interrupt enable configuration for reception
 * @param trxStartMode Start mode (immediate or deferred)
 */
void cb_framework_uwb_rx_start(cb_uwbsystem_rxport_en enRxPort, cb_uwbsystem_packetconfig_st* rxPacketConfig, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable, cb_uwbframework_trx_startmode_en trxStartMode)
{
  cb_system_uwb_rx_top_init    ();
  cb_system_uwb_rx_init       (enRxPort);
  cb_system_uwb_config_rx     (rxPacketConfig, stRxIrqEnable);
  
  switch (trxStartMode)
  {
    case EN_TRX_START_NON_DEFERRED:
      cb_system_uwb_rx_start  (enRxPort);      
      break;
    case EN_TRX_START_DEFERRED:
      cb_system_uwb_rx_start_prepare();
      break;
  }
}

/**
 * @brief End UWB reception in normal mode
 * 
 * @param enRxPort The RX port to stop
 */
void cb_framework_uwb_rx_end(cb_uwbsystem_rxport_en enRxPort)
{
  cb_system_uwb_rx_stop    (enRxPort);
  cb_system_uwb_rx_off     (enRxPort);
  cb_system_uwb_rx_top_off ();
}

//----------------------------------------------------------------//
//                 TX & RX payload API                            //
//----------------------------------------------------------------//
/**
 * @brief Get the size of a received UWB packet
 * 
 * @param config Packet configuration
 * @return uint16_t Size of the received packet in bytes
 */
uint16_t cb_framework_uwb_get_rx_packet_size(cb_uwbsystem_packetconfig_st* config)
{
  return cb_system_uwb_get_rx_packet_size(config);
}

/**
 * @brief Get the payload of a received UWB packet
 * 
 * @param pRxpayloadAddress Pointer to store the received payload
 * @param SizeInByte Pointer to store the size of the payload in bytes
 * @param config Packet configuration
 */
void cb_framework_uwb_get_rx_payload(uint8_t* pRxpayloadAddress, uint16_t* SizeInByte, cb_uwbsystem_packetconfig_st* config)
{
  *SizeInByte = cb_system_uwb_get_rx_packet_size(config);
  cb_system_uwb_rx_get_payload(pRxpayloadAddress, SizeInByte);
}

/**
 * @brief Get the ranging bit from the PHR of a received packet
 * 
 * @param config Packet configuration
 * @return uint8_t The ranging bit value
 */
uint8_t cb_framework_uwb_get_rx_phr_ranging_bit(cb_uwbsystem_packetconfig_st* config)
{
  return cb_system_uwb_get_rx_phr_ranging_bit(config);
}

//----------------------------------------------------------------//
//                 TX & RX status API                             //
//----------------------------------------------------------------//
/**
 * @brief Get the status of UWB reception
 * 
 * @return cb_uwbsystem_rxstatus_un The current RX status
 */
cb_uwbsystem_rxstatus_un cb_framework_uwb_get_rx_status(void)
{
  return cb_system_uwb_get_rx_status();
}

/**
 * @brief Get th UWB rx PHR status
 * 
 * @return stUwbRxPhrStatus The current RX phr status
 */
cb_uwbsystem_rx_phrstatus_st cb_framework_uwb_get_rx_phr_status(void)
{
  return cb_system_uwb_get_rx_phr_status();
}

/**
 * @brief check if RX PHR empty
 * 
 * @return uint8_t status of PHR content
 */
uint8_t cb_framework_uwb_is_rx_phr_empty(void)
{
  return (cb_system_uwb_get_rx_packet_phr() == 0);
}
      
//----------------------------------------------------------------//
//                 TX & RX Timestamp API                          //
//----------------------------------------------------------------//
/**
 * @brief Get raw timestamp for UWB transmission
 * 
 * @param txTimestamp Pointer to store the TX timestamp
 */
void cb_framework_uwb_get_tx_raw_timestamp(cb_uwbsystem_tx_timestamp_st* txTimestamp)
{
  cb_system_uwb_get_tx_raw_timestamp(txTimestamp);
}

/**
 * @brief Get TSU timestamp for UWB reception
 * 
 * @param rxTsuTimestamp Pointer to store the RX TSU timestamp
 * @param enRxPort The RX port from which to get the timestamp
 */
void cb_framework_uwb_get_rx_tsu_timestamp(cb_uwbsystem_rx_tsutimestamp_st* rxTsuTimestamp, cb_uwbsystem_rxport_en enRxPort)
{
  cb_system_uwb_get_rx_tsu_timestamp(rxTsuTimestamp, enRxPort);
}

/**
 * @brief Get TSU timestamp for UWB transmission
 * 
 * @param txTsuTimestamp Pointer to store the TX TSU timestamp
 */
void cb_framework_uwb_get_tx_tsu_timestamp(cb_uwbsystem_tx_tsutimestamp_st *txTsuTimestamp)
{
  cb_system_uwb_get_tx_tsu_timestamp(txTsuTimestamp);
}

/**
 * @brief Clear the TSU (Time Stamp Unit)
 */
void cb_framework_uwb_tsu_clear(void)
{
  cb_system_uwb_tsu_clear();
}

//----------------------------------------------------------------//
//                 RX Data Extraction API                         //
//----------------------------------------------------------------//
/**
 * @brief Get RSSI (Received Signal Strength Indicator) for UWB reception
 * 
 * @param rssiRxPorts Bitmask of RX ports to get RSSI from
 * @return cb_uwbsystem_rxall_signalinfo_st RSSI results for the specified ports
 */
cb_uwbsystem_rxall_signalinfo_st cb_framework_uwb_get_rx_rssi(uint8_t rssiRxPorts)
{
  return cb_system_uwb_get_rx_rssi(rssiRxPorts);
}

/**
 * @brief Get ETC status register for UWB reception
 * 
 * @param etcStatus Pointer to store the ETC status register
 */
void cb_framework_uwb_get_rx_etc_status_register(cb_uwbsystem_rx_etc_statusregister_st* const etcStatus)
{
  cb_system_uwb_get_rx_etc_status_register(etcStatus);
}

/**
 * @brief Store CIR (Channel Impulse Response) register data
 * 
 * @param destArray Destination array to store CIR data
 * @param enRxPort The RX port from which to get CIR data
 * @param startingPosition Starting position in the CIR register
 * @param numSamples Number of samples to store
 */
void cb_framework_uwb_store_rx_cir_register(cb_uwbsystem_rx_cir_iqdata_st* destArray, cb_uwbsystem_rxport_en enRxPort, uint32_t startingPosition, uint32_t numSamples)
{
  cb_system_uwb_store_rx_cir_register(destArray, enRxPort, startingPosition, numSamples);
}

//----------------------------------------------------------------//
//                 Ranging API                                    //
//----------------------------------------------------------------//
/**
 * @brief Calculate distance based on ranging data
 * 
 * @param s_stInitiatorDataContainer Ranging data from the initiator
 * @param s_stResponderDataContainer Ranging data from the responder
 * @return double Calculated distance in meters
 */
double cb_framework_uwb_calculate_distance(cb_uwbframework_rangingdatacontainer_st s_stInitiatorDataContainer, cb_uwbframework_rangingdatacontainer_st s_stResponderDataContainer)
{
  double DS_TWR_T_Prop = cb_system_uwb_alg_prop_calculation(&s_stInitiatorDataContainer.dstwrTroundTreply, 
                                                         &s_stResponderDataContainer.dstwrTroundTreply);
  int32_t initiatorRangingBias = s_stInitiatorDataContainer.dstwrRangingBias;
  int32_t responderRangingBias = s_stResponderDataContainer.dstwrRangingBias;
  
  return ((DS_TWR_T_Prop * 30) - 18617 + (double)s_stInitiatorDataContainer.dstwrRangingBias + (double)s_stResponderDataContainer.dstwrRangingBias);
}

/**
 * @brief Calculate round and reply times for the initiator
 * 
 * @param s_stInitiatorDataContainer Pointer to store the calculated data
 * @param s_stTxTsuTimestamp0 First TX TSU timestamp
 * @param s_stTxTsuTimestamp1 Second TX TSU timestamp
 * @param s_stRxTsuTimestamp0 RX TSU timestamp
 */
void cb_framework_uwb_calculate_initiator_tround_treply(cb_uwbframework_rangingdatacontainer_st* s_stInitiatorDataContainer, cb_uwbsystem_tx_tsutimestamp_st s_stTxTsuTimestamp0, cb_uwbsystem_tx_tsutimestamp_st s_stTxTsuTimestamp1, cb_uwbsystem_rx_tsutimestamp_st s_stRxTsuTimestamp0)
{    
  // Calculate Initiator's Tround and Treply
  s_stInitiatorDataContainer->dstwrTroundTreply.T_round_int  = s_stRxTsuTimestamp0.rxTsuInt - s_stTxTsuTimestamp0.txTsuInt;
  s_stInitiatorDataContainer->dstwrTroundTreply.T_round_frac = (int16_t)s_stRxTsuTimestamp0.rxTsuFrac - (int16_t)s_stTxTsuTimestamp0.txTsuFrac;

  s_stInitiatorDataContainer->dstwrTroundTreply.T_reply_int   = s_stTxTsuTimestamp1.txTsuInt - s_stRxTsuTimestamp0.rxTsuInt;
  s_stInitiatorDataContainer->dstwrTroundTreply.T_reply_frac  = (int16_t)s_stTxTsuTimestamp1.txTsuFrac - (int16_t)s_stRxTsuTimestamp0.rxTsuFrac;
}

/**
 * @brief Calculate round and reply times for the responder
 * 
 * @param s_stResponderDataContainer Pointer to store the calculated data
 * @param s_stTxTsuTimestamp0 TX TSU timestamp
 * @param s_stRxTsuTimestamp0 First RX TSU timestamp
 * @param s_stRxTsuTimestamp1 Second RX TSU timestamp
 */
void cb_framework_uwb_calculate_responder_tround_treply(cb_uwbframework_rangingdatacontainer_st* s_stResponderDataContainer, cb_uwbsystem_tx_tsutimestamp_st s_stTxTsuTimestamp0, cb_uwbsystem_rx_tsutimestamp_st s_stRxTsuTimestamp0, cb_uwbsystem_rx_tsutimestamp_st s_stRxTsuTimestamp1)
{
  // Calculate Responder's Tround and Treply
  s_stResponderDataContainer->dstwrTroundTreply.T_reply_int  = s_stTxTsuTimestamp0.txTsuInt - s_stRxTsuTimestamp0.rxTsuInt;
  s_stResponderDataContainer->dstwrTroundTreply.T_reply_frac = (int16_t)s_stTxTsuTimestamp0.txTsuFrac - (int16_t)s_stRxTsuTimestamp0.rxTsuFrac;
  
  s_stResponderDataContainer->dstwrTroundTreply.T_round_int  = s_stRxTsuTimestamp1.rxTsuInt - s_stTxTsuTimestamp0.txTsuInt;
  s_stResponderDataContainer->dstwrTroundTreply.T_round_frac = (int16_t)s_stRxTsuTimestamp1.rxTsuFrac - (int16_t)s_stTxTsuTimestamp0.txTsuFrac;  
}

//----------------------------------------------------------------//
//                  Scheduled TRX with abs timer API              //
//----------------------------------------------------------------//
/**
 * @brief Configure event timestamp mask
 * 
 * @param eventTimestampMask Event timestamp mask to configure
 * @param eventIndex Event index to configure
 */
void cb_framework_uwb_configure_event_time_stamp_mask(enUwbEventTimestampMask eventTimestampMask, enUwbEventIndex eventIndex)
{
  cb_system_uwb_configure_event_timestamp_mask (eventTimestampMask, eventIndex);
}  

/**
 * @brief Enable scheduled UWB transactions
 * 
 * @param repeatedTrxConfig Configuration for scheduled transactions
 */
void cb_framework_uwb_enable_scheduled_trx(cb_uwbframework_trx_scheduledconfig_st repeatedTrxConfig)
{
  cb_system_uwb_configure_event_timestamp_mask (repeatedTrxConfig.eventTimestampMask, repeatedTrxConfig.eventIndex);  
  cb_system_uwb_enable_event_timestamp         (EN_UWB_ENABLE);
  cb_system_uwb_abs_timer_on                   (repeatedTrxConfig.absTimer);
}

/**
 * @brief Disable scheduled UWB transactions
 * 
 * @param repeatedTrxConfig Configuration for scheduled transactions
 */
void cb_framework_uwb_disable_scheduled_trx(cb_uwbframework_trx_scheduledconfig_st repeatedTrxConfig)
{
  cb_system_uwb_abs_timer_configure_event_commander(EN_UWB_DISABLE,
                                                    repeatedTrxConfig.absTimer,
                                                    repeatedTrxConfig.eventCtrlMask);
  cb_system_uwb_abs_timer_off          (repeatedTrxConfig.absTimer);
  cb_system_uwb_enable_event_timestamp (EN_UWB_DISABLE);
}

/**
 * @brief Configure scheduled UWB transactions
 * 
 * @param repeatedTrxConfig Configuration for scheduled transactions
 */
void cb_framework_uwb_configure_scheduled_trx(cb_uwbframework_trx_scheduledconfig_st repeatedTrxConfig)
{  
  cb_system_uwb_abs_timer_configure_timeout_value  (repeatedTrxConfig.absTimer,
                                                    cb_system_uwb_get_event_timestamp_value(repeatedTrxConfig.eventTimestampMask),
                                                    repeatedTrxConfig.timeoutValue);
  cb_system_uwb_abs_timer_configure_event_commander(EN_UWB_ENABLE,
                                                    repeatedTrxConfig.absTimer,
                                                    repeatedTrxConfig.eventCtrlMask);
  cb_system_uwb_abs_timer_clear_internal_occurence (repeatedTrxConfig.absTimer);
}

//----------------------------------------------------------------//
//                             PDOA API                           //
//----------------------------------------------------------------//
/**
 * @brief Initialize UWB PDoA (Phase Difference of Arrival) reception
 * 
 * This function initializes the UWB system for PDoA reception by setting up
 * the RX top, initializing all RX ports, configuring the packet parameters,
 * and setting up the carrier frequency offset estimation.
 * 
 * @param rxPacketConfig Packet configuration for reception
 * @param stRxIrqEnable Interrupt enable configuration for reception
 * @param cfoEst Carrier frequency offset estimation
 */
void cb_framework_uwb_pdoa_rx_init(cb_uwbsystem_packetconfig_st* rxPacketConfig, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable , uint32_t cfoEst)
{
  cb_system_uwb_rx_top_init   ();
  cb_system_uwb_rx_init       (EN_UWB_RX_ALL);
  cb_system_uwb_config_rx     (rxPacketConfig, stRxIrqEnable);
  cb_system_uwb_configure_rx_sync_cfo_est_bypass_crs(cfoEst,1);
}

/**
 * @brief Start UWB PDoA reception
 * 
 * This function starts the PDoA reception process by configuring the RX
 * interrupts and starting reception on all RX ports with the specified gain.
 * 
 * @param stRxIrqEnable Interrupt enable configuration for reception
 * @param gain Gain setting for reception
 */
void cb_framework_uwb_pdoa_rx_start(cb_uwbsystem_rx_irqenable_st* stRxIrqEnable, uint32_t gain)
{ 
  uint32_t args_gainidx[1];
  args_gainidx[0] = gain;
  
  cb_system_uwb_configure_rx_irq  (stRxIrqEnable);
  cb_system_uwb_pdoa_rx_all_start (args_gainidx);
}

/**
 * @brief Stop UWB PDoA reception
 * 
 * This function stops the PDoA reception process on all RX ports.
 */
void cb_framework_uwb_pdoa_rx_stop(void)
{
  cb_system_uwb_rx_stop(EN_UWB_RX_ALL);
}

/**
 * @brief End UWB PDoA reception
 * 
 * This function terminates the PDoA reception by turning off all RX ports
 * and the RX top module, releasing system resources.
 */
void cb_framework_uwb_pdoa_rx_end(void)
{
  cb_system_uwb_rx_off     (EN_UWB_RX_ALL);
  cb_system_uwb_rx_top_off ();
}

/**
 * @brief Stage scheduled transmission for PDoA
 * 
 * This function prepares a scheduled transmission for PDoA by configuring
 * the TX interrupts and preparing the TX start operation.
 * 
 * @param pdoaTxIrqEnable Interrupt enable configuration for transmission
 */
void cb_framework_uwb_pdoa_stage_scheduled_tx(cb_uwbsystem_tx_irqenable_st *pdoaTxIrqEnable)
{
  cb_system_uwb_configure_tx_irq(pdoaTxIrqEnable);
  cb_system_uwb_tx_start_prepare();
}

/**
 * @brief Process CIR data for PDoA
 * 
 * This function processes the Channel Impulse Response (CIR) data for PDoA
 * calculations, delegating to the system-level implementation.
 * 
 * @param calType Type of PDoA calculation (2D or 3D)
 * @param packageNum Package number in the superframe
 * @param numRxUsed Number of RX antennas used
 * @param cirRegisterData CIR register data
 * @param cirDataSize Size of CIR data
 * @return cb_uwbalg_poa_outputperpacket_st PDoA output for the packet
 */
cb_uwbalg_poa_outputperpacket_st cb_framework_uwb_pdoa_cir_processing(enUwbPdoaCalType calType, uint8_t packageNum, const uint8_t numRxUsed, const cb_uwbsystem_rx_cir_iqdata_st *cirRegisterData, uint16_t cirDataSize)
{
  return cb_system_uwb_pdoa_cir_processing(calType, packageNum, numRxUsed, cirRegisterData, cirDataSize);
}

/**
 * @brief Reset CIR data container for PDoA
 * 
 * This function clears the CIR data container used for PDoA processing,
 * preparing it for new data collection.
 */
void cb_framework_uwb_pdoa_reset_cir_data_container(void)
{
  memset(&g_stPdoaRxCirDataContainer[0], 0x00, sizeof(g_stPdoaRxCirDataContainer));
}

/**
 * @brief Store CIR data for PDoA
 * 
 * This function stores CIR data from all three RX antennas for PDoA processing,
 * capturing a specified number of samples at the calculated position.
 * 
 * @param countOfPdoaScheduledRx Count of scheduled PDoA receptions
 */
void cb_framework_uwb_pdoa_store_cir_data(uint8_t countOfPdoaScheduledRx)
{
  cb_system_uwb_store_rx_cir_register(g_stPdoaRxCirDataContainer[countOfPdoaScheduledRx][EN_UWB_RX_0], EN_UWB_RX_0, (cb_system_uwb_get_rx_cir_ctl_idx() - DEF_PDOA_CIR_DATASET_OFFSET), DEF_PDOA_NUM_CIR_DATASET);
  cb_system_uwb_store_rx_cir_register(g_stPdoaRxCirDataContainer[countOfPdoaScheduledRx][EN_UWB_RX_1], EN_UWB_RX_1, (cb_system_uwb_get_rx_cir_ctl_idx() - DEF_PDOA_CIR_DATASET_OFFSET), DEF_PDOA_NUM_CIR_DATASET);
  cb_system_uwb_store_rx_cir_register(g_stPdoaRxCirDataContainer[countOfPdoaScheduledRx][EN_UWB_RX_2], EN_UWB_RX_2, (cb_system_uwb_get_rx_cir_ctl_idx() - DEF_PDOA_CIR_DATASET_OFFSET), DEF_PDOA_NUM_CIR_DATASET);
}

/**
 * @brief Calculate PDoA result
 * 
 * This function calculates the Phase Difference of Arrival (PDoA) result
 * from the stored CIR data. It processes each phase difference between
 * antenna pairs and computes statistical measures (mean and median).
 * 
 * @param s_stPdoaOutputResult Pointer to store the PDoA result
 * @param CIR_CalculationType Type of CIR calculation (2D or 3D)
 * @param NumOfPackage Number of packages in the superframe
 */
void cb_framework_uwb_pdoa_calculate_result(cb_uwbsystem_pdoaresult_st *s_stPdoaOutputResult,enUwbPdoaCalType CIR_CalculationType, uint8_t NumOfPackage) 
{
  static double s_pdoaEstimated[DEF_PDOA_NUMPKT_SUPERFRAME_MAX];
  uint8_t phaseIdx_startoffset = 0;
    
  if (NumOfPackage > DEF_PDOA_NUMPKT_SUPERFRAME_MAX)
  {
    s_stPdoaOutputResult->stRxstatus = CB_FALSE; //Error
    return;
  }
  if (CIR_CalculationType == EN_PDOA_2D_CALTYPE)
  {
    phaseIdx_startoffset = 2;//Single phase Rx0-Rx2 only
  }
  else
  {
    phaseIdx_startoffset = 0;//Three phase
  }
  
  for( uint8_t i = 0; i < DEF_PDOA_NUMPKT_SUPERFRAME_MAX; i++)
  {
    g_stPoaResult[i] = cb_framework_uwb_pdoa_cir_processing(EN_PDOA_3D_CALTYPE, i, DEF_PDOA_NUM_RX_USED, &g_stPdoaRxCirDataContainer[0][0][0], DEF_PDOA_NUM_CIR_DATASET);
  }  
  
  // Process each phase difference (0:Rx0-Rx1, 1: Rx1-Rx2, 2: Rx0-Rx2)
  for (uint8_t phaseIdx = phaseIdx_startoffset; phaseIdx < 3; phaseIdx++) 
  {
    for (uint8_t pktcnt = 0; pktcnt < NumOfPackage; pktcnt++) 
    {
      switch (phaseIdx) 
      {
        case 0: s_pdoaEstimated[pktcnt] = cb_system_uwb_alg_pdoa_estimation(g_stPoaResult[pktcnt].rx0, g_stPoaResult[pktcnt].rx1); break;
        case 1: s_pdoaEstimated[pktcnt] = cb_system_uwb_alg_pdoa_estimation(g_stPoaResult[pktcnt].rx1, g_stPoaResult[pktcnt].rx2); break;
        case 2: s_pdoaEstimated[pktcnt] = cb_system_uwb_alg_pdoa_estimation(g_stPoaResult[pktcnt].rx0, g_stPoaResult[pktcnt].rx2); break;
      }
    }
    // Compute mean and median
    switch (phaseIdx) 
    {
      case 0: cb_framework_uwb_pdoa_calculate_mean_and_median(s_pdoaEstimated, NumOfPackage, &s_stPdoaOutputResult->mean.rx0_rx1, &s_stPdoaOutputResult->median.rx0_rx1); break;
      case 1: cb_framework_uwb_pdoa_calculate_mean_and_median(s_pdoaEstimated, NumOfPackage, &s_stPdoaOutputResult->mean.rx1_rx2, &s_stPdoaOutputResult->median.rx1_rx2); break;
      case 2: cb_framework_uwb_pdoa_calculate_mean_and_median(s_pdoaEstimated, NumOfPackage, &s_stPdoaOutputResult->mean.rx0_rx2, &s_stPdoaOutputResult->median.rx0_rx2); break;
    }
  }
  s_stPdoaOutputResult->stRxstatus = CB_TRUE; //success
}

/**
 * @brief Calculate Angle of Arrival (AoA) from PDoA data
 * 
 * @param pdoa_result PDoA 3D data
 * @param azi_pd_bias Azimuth phase difference bias
 * @param ele_pd_bias Elevation phase difference bias
 * @param azi_result Pointer to store the azimuth result
 * @param ele_result Pointer to store the elevation result
 */
void cb_framework_uwb_pdoa_calculate_aoa(cb_uwbsystem_pdoa_3ddata_st pdoa_result, float azi_pd_bias, float ele_pd_bias, float* azi_result, float* ele_result)
{
  stAOA_CompensatedData stAoaPd = {0};
  stAoaPd = cb_system_uwb_aoa_antenna_3d_bias_comp(pdoa_result, azi_pd_bias, ele_pd_bias);
  cb_system_uwb_aoa_antenna_3d(&stAoaPd, azi_result, ele_result);
}

/**
 * @brief Calculate mean of an array of doubles
 * 
 * @param array Array of doubles
 * @param size Size of the array
 * @return double Mean value
 */
double cb_framework_uwb_pdoa_calculate_mean(double* array, uint32_t size) 
{
  double sum = 0.0;
  for (uint32_t i = 0; i < size; i++)
  {
    sum += array[i];
  }
  return sum / size;
}

/**
 * @brief Comparison function for qsort
 * 
 * @param a First element to compare
 * @param b Second element to compare
 * @return int Result of comparison
 */
int cb_framework_uwb_pdoa_qsort_compare (const void * a, const void * b)
{
  int ret;
  if (*(const double*)a - *(const double*)b >= 0)
  {
    ret = 1;
  }
  else
  {
    ret = -1;
  }
  return (ret);
}

/**
 * @brief Calculate mean and median of PDoA estimates
 * 
 * @param s_pdoaEstimated Array of PDoA estimates
 * @param NumOfPackage Number of packages
 * @param mean Pointer to store the mean value
 * @param median Pointer to store the median value
 */
void cb_framework_uwb_pdoa_calculate_mean_and_median(double *s_pdoaEstimated, uint8_t NumOfPackage, double *mean, double *median)
{
  if (NumOfPackage > DEF_PDOA_NUMPKT_SUPERFRAME_MAX)
  {
    return;
  }
  double tempbuf[DEF_PDOA_NUMPKT_SUPERFRAME_MAX] = {0};

  // Calculate mean
  *mean = cb_framework_uwb_pdoa_calculate_mean(s_pdoaEstimated, NumOfPackage);

  // Sort the array to calculate median
  memcpy(tempbuf, s_pdoaEstimated, NumOfPackage * sizeof(double));
  qsort(tempbuf, NumOfPackage, sizeof(double), cb_framework_uwb_pdoa_qsort_compare);

  // Calculate median
  if (NumOfPackage % 2 == 0) 
  {
    *median = (tempbuf[NumOfPackage / 2 - 1] + tempbuf[NumOfPackage / 2]) / 2.0;
  } 
  else 
  {
    *median = tempbuf[(NumOfPackage - 1) / 2];
  }
}
