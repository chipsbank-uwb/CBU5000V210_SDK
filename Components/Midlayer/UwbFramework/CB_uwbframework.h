/**
 * @file    CB_uwbframework.h
 * @brief   UWB Framework interface
 * @details This file defines the interface for the UWB Framework, which provides
 *          a comprehensive set of functions for UWB communication, ranging, and
 *          positioning. It includes functions for initialization, transmission,
 *          reception, timing, ranging calculations, and Phase Difference of
 *          Arrival (PDoA) processing.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __CB_UWB_FRAMEWORK_H
#define __CB_UWB_FRAMEWORK_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"
#include "CB_system_types.h"
#include "CB_UwbDrivers.h"
#include "CB_Algorithm.h"
#include "CB_aoa.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_ANTENNA_TYPE_TRIANGLE_UP     0
#define DEF_ANTENNA_TYPE_LSHAPE_UP       1
#define DEF_ANTENNA_TYPE_TRIANGLE_DOWN   2
#define DEF_ANTENNA_TYPE_LSHAPE_DOWN     3
#define DEF_ANTENNA_TYPE_TRIANGLE_RIGHT  4
#define DEF_ANTENNA_TYPE_TRIANGLE_LEFT   5

#define DEF_ANTENNA_POSITION_A  0
#define DEF_ANTENNA_POSITION_B  1
#define DEF_ANTENNA_POSITION_C  2

#define DEF_ANTENNA_PORT_RX0  0
#define DEF_ANTENNA_PORT_RX1  1
#define DEF_ANTENNA_PORT_RX2  2

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief Enumeration for transaction start modes
 */
typedef enum {
  EN_TRX_START_NON_DEFERRED, /**< Immediate start */
  EN_TRX_START_DEFERRED      /**< Deferred start */
} cb_uwbframework_trx_startmode_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief Configuration structure for scheduled UWB transactions
 */
typedef struct
{
  enUwbEventTimestampMask   eventTimestampMask; /**< (Timestamp) Select timestamp mask to be used */
  enUwbEventIndex           eventIndex;         /**< (Timestamp) Select event to for timestamp capture */
  enUwbAbsoluteTimer        absTimer;           /**< (ABS timer) Select absolute timer */
  uint32_t                  timeoutValue;       /**< (ABS timer) absolute timer timeout value, unit - us */
  enUwbEventControl         eventCtrlMask;      /**< (action)    select action upon abs timeout */
} cb_uwbframework_trx_scheduledconfig_st;

/**
 * @brief Structure containing CIR (Channel Impulse Response) information
 */
typedef struct
{
  cb_uwbsystem_rx_tsustatus_st      rxTsuStatus0;           /**< RX TSU status information */
  uint16_t              rxResponseMsgCirCtlIdx0; /**< RX response message CIR control index */
} cb_uwbframework_rx_cir_info_st;

/**
 * @brief Container for UWB ranging data
 */
typedef struct
{
  cb_uwbsystem_rangingtroundtreply_st dstwrTroundTreply;   /**< Double-sided two-way ranging round and reply times */
  int32_t dstwrRangingBias;                     /**< Ranging bias for calibration */
}cb_uwbframework_rangingdatacontainer_st;

/**
 * @brief Container for UWB Phase Difference of Arrival (PDoA) data
 */
typedef struct cb_uwbframework_pdoadatacontainer_st
{
  float rx0_rx1;                               /**< Phase difference between RX0 and RX1 */
  float rx0_rx2;                               /**< Phase difference between RX0 and RX2 */
  float rx1_rx2;                               /**< Phase difference between RX1 and RX2 */
  float elevationEst;                          /**< Estimated elevation angle */
  float azimuthEst;                            /**< Estimated azimuth angle */
}cb_uwbframework_pdoadatacontainer_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
/**
 * @brief Maximum number of packets per superframe for PDoA
 */
#define DEF_PDOA_NUMPKT_SUPERFRAME_MAX  5

/**
 * @brief Number of RX antennas used for PDoA
 */
#define DEF_PDOA_NUM_RX_USED            3

/**
 * @brief Number of CIR datasets for PDoA
 */
#define DEF_PDOA_NUM_CIR_DATASET        21

/**
 * @brief Offset for CIR dataset in PDoA
 */
#define DEF_PDOA_CIR_DATASET_OFFSET     10

//-------------------------------
// FUNCTION PROTOTYPE SECTION
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
void cb_framework_uwb_init(void);

/**
 * @brief Deinitialize the UWB communication transmitter.
 *
 * This function deinitializes the UWB communication transmitter by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 */ 
void cb_framework_uwb_off(void);

//----------------------------------------------------------------//
//                 COMM-TRX QUICK MODE API                        //
//----------------------------------------------------------------//

/**
 * @brief Enable the UWB communication transmitter in quick mode.
 *
 * This function enables the UWB communication transmitter in quick mode by performing various configuration steps.
 * It sets up the required parameters and resources for UWB communication.
 */
void cb_framework_uwb_qmode_trx_enable     (void);

/**
 * @brief Disable the UWB communication transmitter in quick mode.
 *
 * This function disables the UWB communication transmitter in quick mode by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 */
void cb_framework_uwb_qmode_trx_disable    (void);

/**
 * @brief Start UWB transmission in quick mode
 * 
 * @param txPacketConfig Configuration for the packet to be transmitted
 * @param txPayload Payload data to be transmitted
 * @param stTxIrqEnable Interrupt enable configuration for transmission
 */
void cb_framework_uwb_qmode_tx_start       (cb_uwbsystem_packetconfig_st* txPacketConfig,cb_uwbsystem_txpayload_st* txPayload, cb_uwbsystem_tx_irqenable_st* stTxIrqEnable);

/**
 * @brief End the UWB communication transmitter in quick mode.
 *
 * This function ends the UWB communication transmitter in quick mode by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 */
void cb_framework_uwb_qmode_tx_end         (void);

/**
 * @brief Start the UWB communication receiver in quick mode.
 *
 * This function starts the UWB communication receiver in quick mode by performing various configuration steps.
 * It sets up the required parameters and resources for UWB communication.
 * 
 * @param rxPacketConfig Configuration for the packet to be received
 * @param stRxIrqEnable Interrupt enable configuration for reception
 */
void cb_framework_uwb_qmode_rx_start(cb_uwbsystem_packetconfig_st* rxPacketConfig, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable);
/**
 * @brief End the UWB communication receiver in quick mode.
 *
 * This function ends the UWB communication receiver in quick mode by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 * 
 */
void cb_framework_uwb_qmode_rx_end         (void);

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
void cb_framework_uwb_tx_start  (cb_uwbsystem_packetconfig_st* txPacketConfig, cb_uwbsystem_txpayload_st* txPayload, cb_uwbsystem_tx_irqenable_st* stTxIrqEnable, cb_uwbframework_trx_startmode_en trxStartMode);

/**
 * @brief End the UWB communication transmitter in normal mode.
 *
 * This function ends the UWB communication transmitter in normal mode by performing various cleanup steps.
 * It releases the allocated resources and resets the UWB communication system.
 */
void cb_framework_uwb_tx_end    (void);

/**
 * @brief Restart UWB transmission in normal mode
 * 
 * This function restarts the UWB transmission by stopping the current transmission,
 * reconfiguring the TX interrupts, and starting a new transmission based on the specified mode.
 * 
 * @param stTxIrqEnable Interrupt enable configuration for transmission
 * @param trxStartMode Start mode (immediate or deferred)
 */
void cb_framework_uwb_tx_restart(cb_uwbsystem_tx_irqenable_st* stTxIrqEnable, cb_uwbframework_trx_startmode_en trxStartMode);

/**
 * @brief Start UWB reception in normal mode
 * 
 * @param enRxPort The RX port to use for reception
 * @param rxPacketConfig Configuration for the packet to be received
 * @param stRxIrqEnable Interrupt enable configuration for reception
 * @param trxStartMode Start mode (immediate or deferred)
 */
void cb_framework_uwb_rx_start(cb_uwbsystem_rxport_en enRxPort, cb_uwbsystem_packetconfig_st* rxPacketConfig, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable, cb_uwbframework_trx_startmode_en trxStartMode);

/**
 * @brief End UWB reception in normal mode
 * 
 * @param enRxPort The RX port to stop
 */
void cb_framework_uwb_rx_end    (cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Restarts the UWB receiver with a new configuration.
 *
 * This function stops the current UWB receiver, applies the specified interrupt
 * configuration, and restarts the receiver based on the selected start mode.
 * 
 * @param[in] enRxPort      RX port to be restarted (e.g., RX0, RX1, RX2, or ALL).
 * @param[in] stRxIrqEnable Pointer to the RX interrupt configuration structure.
 * @param[in] trxStartMode  Start mode selection:
 *                           - EN_TRX_START_NON_DEFERRED: start immediately.
 *                           - EN_TRX_START_DEFERRED: prepare for later start.
 */
void cb_framework_uwb_rx_restart(cb_uwbsystem_rxport_en enRxPort, cb_uwbsystem_packetconfig_st* rxPacketConfig, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable, cb_uwbframework_trx_startmode_en trxStartMode);

//----------------------------------------------------------------//
//                 TX & RX payload API                            //
//----------------------------------------------------------------//
/**
 * @brief Get the size of a received UWB packet
 * 
 * @param config Packet configuration
 * @return uint16_t Size of the received packet in bytes
 */
uint16_t cb_framework_uwb_get_rx_packet_size  (cb_uwbsystem_packetconfig_st* config);

/**
 * @brief Get the payload of a received UWB packet
 * 
 * @param pRxpayloadAddress Pointer to store the received payload
 * @param SizeInByte Pointer to store the size of the payload in bytes
 * @param config Packet configuration
 */
void cb_framework_uwb_get_rx_payload(uint8_t* pRxpayloadAddress, uint16_t* SizeInByte, cb_uwbsystem_packetconfig_st* config);

/**
 * @brief Get the ranging bit from the PHR of a received packet
 * 
 * @param config Packet configuration
 * @return uint8_t The ranging bit value
 */
uint8_t cb_framework_uwb_get_rx_phr_ranging_bit(cb_uwbsystem_packetconfig_st* config);

//----------------------------------------------------------------//
//                 TX & RX status API                            //
//----------------------------------------------------------------//
/**
 * @brief Get the status of UWB reception
 * 
 * @return cb_uwbsystem_rxstatus_un The current RX status
 */
cb_uwbsystem_rxstatus_un cb_framework_uwb_get_rx_status(void);

/**
 * @brief Get th UWB rx PHR status
 * 
 * @return stUwbRxPhrStatus The current RX phr status
 */
cb_uwbsystem_rx_phrstatus_st cb_framework_uwb_get_rx_phr_status(void);

/**
 * @brief check if RX PHR empty
 * 
 * @return uint8_t status of PHR content
 */
uint8_t cb_framework_uwb_is_rx_phr_empty(void);

//----------------------------------------------------------------//
//                 TX & RX Timestamp API                          //
//----------------------------------------------------------------//
/**
 * @brief Get raw timestamp for UWB transmission
 * 
 * @param txTimestamp Pointer to store the TX timestamp
 */
void cb_framework_uwb_get_tx_raw_timestamp     (cb_uwbsystem_tx_timestamp_st* txTimestamp);

/**
 * @brief Get TSU timestamp for UWB reception
 * 
 * @param rxTsuTimestamp Pointer to store the RX TSU timestamp
 * @param enRxPort The RX port from which to get the timestamp
 */
void cb_framework_uwb_get_rx_tsu_timestamp     (cb_uwbsystem_rx_tsutimestamp_st* rxTsuTimestamp, cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Get TSU timestamp for UWB transmission
 * 
 * @param txTsuTimestamp Pointer to store the TX TSU timestamp
 */
void cb_framework_uwb_get_tx_tsu_timestamp     (cb_uwbsystem_tx_tsutimestamp_st *txTsuTimestamp);

/**
 * @brief Clear the TSU (Time Stamp Unit)
 */
void cb_framework_uwb_tsu_clear(void);

//----------------------------------------------------------------//
//                 RX Data Extraction API                         //
//----------------------------------------------------------------//
/**
 * @brief Get RSSI (Received Signal Strength Indicator) for UWB reception
 * 
 * @param rssiRxPorts Bitmask of RX ports to get RSSI from
 * @return cb_uwbsystem_rx_signalinfo_st RSSI results for the specified ports
 */
cb_uwbsystem_rx_signalinfo_st cb_framework_uwb_get_rx_rssi(uint8_t rssiRxPorts);

/**
 * @brief Get ETC status register for UWB reception
 * 
 * @param etcStatus Pointer to store the ETC status register
 */
void cb_framework_uwb_get_rx_etc_status_register(cb_uwbsystem_rx_etc_statusregister_st* const etcStatus);

/**
 * @brief Store CIR (Channel Impulse Response) register data
 * 
 * @param destArray Destination array to store CIR data
 * @param enRxPort The RX port from which to get CIR data
 * @param startingPosition Starting position in the CIR register
 * @param numSamples Number of samples to store
 */
void cb_framework_uwb_store_rx_cir_register(cb_uwbsystem_rx_cir_iqdata_st* destArray, cb_uwbsystem_rxport_en enRxPort, uint32_t startingPosition, uint32_t numSamples);

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
double cb_framework_uwb_calculate_distance  (cb_uwbframework_rangingdatacontainer_st s_stInitiatorDataContainer, cb_uwbframework_rangingdatacontainer_st s_stResponderDataContainer);

/**
 * @brief Calculate round and reply times for the initiator
 * 
 * @param s_stInitiatorDataContainer Pointer to store the calculated data
 * @param s_stTxTsuTimestamp0 First TX TSU timestamp
 * @param s_stTxTsuTimestamp1 Second TX TSU timestamp
 * @param s_stRxTsuTimestamp0 RX TSU timestamp
 */
void cb_framework_uwb_calculate_initiator_tround_treply(cb_uwbframework_rangingdatacontainer_st* s_stInitiatorDataContainer, cb_uwbsystem_tx_tsutimestamp_st s_stTxTsuTimestamp0, cb_uwbsystem_tx_tsutimestamp_st s_stTxTsuTimestamp1, cb_uwbsystem_rx_tsutimestamp_st s_stRxTsuTimestamp0);

/**
 * @brief Calculate round and reply times for the responder
 * 
 * @param s_stResponderDataContainer Pointer to store the calculated data
 * @param s_stTxTsuTimestamp0 TX TSU timestamp
 * @param s_stRxTsuTimestamp0 First RX TSU timestamp
 * @param rxTsuTimestamp1 Second RX TSU timestamp
 */
void cb_framework_uwb_calculate_responder_tround_treply(cb_uwbframework_rangingdatacontainer_st* s_stResponderDataContainer, cb_uwbsystem_tx_tsutimestamp_st s_stTxTsuTimestamp0, cb_uwbsystem_rx_tsutimestamp_st s_stRxTsuTimestamp0, cb_uwbsystem_rx_tsutimestamp_st rxTsuTimestamp1);

//----------------------------------------------------------------//
//                 Scheduled TRX with abs timer API               //
//----------------------------------------------------------------//
/**
 * @brief Configure event timestamp mask for UWB events
 * 
 * This function configures which UWB events will generate timestamps. The timestamp mask 
 * determines which events are monitored and recorded for timing purposes.
 * 
 * @param eventTimestampMask Bitmask specifying which events should generate timestamps
 * @param eventIndex Index of the event to configure (0-31)
 */
void cb_framework_uwb_configure_event_time_stamp_mask(enUwbEventTimestampMask eventTimestampMask, enUwbEventIndex eventIndex);

/**
 * @brief Enable scheduled UWB transmit/receive operations
 * 
 * Activates scheduled UWB transactions based on the provided configuration. This allows
 * for timed/periodic UWB operations like using absolute timer.
 * absolute timers.
 * 
 * @param repeatedTrxConfig Configuration structure containing timing parameters and 
 *                         operation modes for the scheduled transactions
 */
void cb_framework_uwb_enable_scheduled_trx     (cb_uwbframework_trx_scheduledconfig_st repeatedTrxConfig);

/**
 * @brief Disable scheduled UWB transmit/receive operations
 * 
 * Stops any active scheduled UWB transactions and disables the absolute timers
 * associated with the provided configuration.
 * 
 * @param repeatedTrxConfig Configuration structure of the scheduled transactions
 *                         to disable
 */
void cb_framework_uwb_disable_scheduled_trx    (cb_uwbframework_trx_scheduledconfig_st repeatedTrxConfig);

/**
 * @brief Configure parameters for scheduled UWB transactions
 * 
 * Sets up the configuration for scheduled UWB transactions including timing parameters,
 * operation modes (TX/RX), and event triggers. This function is called
 * after a transaction completes to reconfigure the system for the next scheduled
 * transaction in periodic operations.
 * 
 * @param repeatedTrxConfig Configuration structure containing all parameters for
 *                         the scheduled UWB transactions
 */
void cb_framework_uwb_configure_scheduled_trx  (cb_uwbframework_trx_scheduledconfig_st repeatedTrxConfig);

//----------------------------------------------------------------//
//                             PDOA API                           //
//----------------------------------------------------------------//


/**
 * @brief Process CIR data for PDoA
 * 
 * @param calType Type of PDoA calculation
 * @param packageNum Package number
 * @param numRxUsed Number of RX antennas used
 * @param cirRegisterData CIR register data
 * @param cirDataSize Size of CIR data
 * @return cb_uwbalg_poa_outputperpacket_st PDoA output for the packet
 */
cb_uwbalg_poa_outputperpacket_st cb_framework_uwb_pdoa_cir_processing(enUwbPdoaCalType calType, uint8_t packageNum, const uint8_t numRxUsed, const cb_uwbsystem_rx_cir_iqdata_st *cirRegisterData, uint16_t cirDataSize);

/**
 * @brief Calculate PDoA result
 * 
 * @param s_stPdoaOutputResult Pointer to store the PDoA result
 * @param CIR_CalculationType Type of CIR calculation
 * @param NumOfPackage Number of packages
 */
void cb_framework_uwb_pdoa_calculate_result(cb_uwbsystem_pdoaresult_st *s_stPdoaOutputResult,enUwbPdoaCalType CIR_CalculationType, uint8_t NumOfPackage);

/**
 * @brief Calculate Angle of Arrival (AoA) from PDoA data
 * 
 * @param pdoa_result PDoA 3D data containing phase differences between antenna pairs
 * @param pd01_bias Phase difference bias between antenna 0 and 1
 * @param pd02_bias Phase difference bias between antenna 0 and 2  
 * @param pd12_bias Phase difference bias between antenna 1 and 2
 * @param azi_result Pointer to store the calculated azimuth angle in degrees
 * @param ele_result Pointer to store the calculated elevation angle in degrees
 */
void cb_framework_uwb_pdoa_calculate_aoa(cb_uwbsystem_pdoa_3ddata_st pdoa_result, float pd01_bias, float pd02_bias, float pd12_bias, float* azi_result, float* ele_result);
/**
 * @brief Calculate mean of an array of doubles
 * 
 * @param array Array of doubles
 * @param size Size of the array
 * @return double Mean value
 */
double cb_framework_uwb_pdoa_calculate_mean(double* array, uint32_t size);

/**
 * @brief Comparison function for qsort
 * 
 * @param a First element to compare
 * @param b Second element to compare
 * @return int Result of comparison
 */
int cb_framework_uwb_pdoa_qsort_compare (const void * a, const void * b);

/**
 * @brief Calculate mean and median of PDoA estimates
 * 
 * @param s_pdoaEstimated Array of PDoA estimates
 * @param NumOfPackage Number of packages
 * @param mean Pointer to store the mean value
 * @param median Pointer to store the median value
 */
void cb_framework_uwb_pdoa_calculate_mean_and_median(double *s_pdoaEstimated, uint8_t NumOfPackage, double *mean, double *median);

/**
 * @brief Configure the Antenna attribute structure
 * 
 * @param p_ant_attr Pointer to the Antenna attribute structure
 */
void cb_framework_uwb_pdoa_configure_ant(st_antenna_attribute_3d* p_ant_attr);

/**
 * @brief Configure the LUT (Look-Up Table) attribute structure
 * 
 * @param p_lut_attr Pointer to the LUT attribute structure
 */
void cb_framework_uwb_pdoa_configure_lut(cb_uwbaoa_lut_attribute_st* p_lut_attr);

/**
 * @brief Reset CIR data container for PDoA
 */
void cb_framework_uwb_pdoa_reset_cir_data_container(void);

/**
 * @brief Store CIR data for PDoA
 * 
 * @param countOfPdoaScheduledRx Count of scheduled PDoA receptions
 */
void cb_framework_uwb_pdoa_store_cir_data(uint8_t countOfPdoaScheduledRx);

void cb_framework_ftm_uwb_rx_restart(cb_uwbsystem_rxport_en enRxPort, cb_uwbsystem_packetconfig_st* rxPacketConfig, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable, cb_uwbframework_trx_startmode_en trxStartMode);

//----------------------------------------------------------------//
//                    RX DBB CONFIG BYPASS                        //
//----------------------------------------------------------------//

/**
 * @brief Configure CFO and gain settings for UWB receiver
 *
 * This function configures the Carrier Frequency Offset (CFO) and gain settings
 * for the UWB receiver. It either resets the settings to default values or
 * applies custom configuration.
 *
 * @param enReset Reset flag - if EN_UWB_CFO_GAIN_RESET, resets to defaults,
 *                otherwise applies custom config
 * @param stBypass_cfg Pointer to custom CFO and gain configuration structure.
 *                     Only used if enReset is not EN_UWB_CFO_GAIN_RESET.
 */
void cb_framework_uwb_rxconfig_cfo_gain(cb_uwbsystem_rxconfig_cfo_gain_en enReset, cb_uwbsystem_rx_dbb_config_st* stRxCfg_CfoGainBypass);

#endif /*__CB_UWB_FRAMEWORK_H*/
