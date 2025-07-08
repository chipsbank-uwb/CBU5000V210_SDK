/**
 * @file CB_UwbDrivers.h
 * @brief Header file for the CB UWB Drivers module.
 *
 * This file contains the declarations and definitions required for
 * the implementation of the Ultra-Wideband (UWB) driver functionalities.
 * It serves as an interface for interacting with UWB hardware components.
 *
 * @note Ensure proper inclusion of this file in relevant source files
 *       to access UWB driver functionalities.
 */
#ifndef __CB_UWB_DRIVER_H
#define __CB_UWB_DRIVER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_system_types.h"

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @enum enUwbCirRegisterData
 * @brief Enumeration for UWB CIR register data.
 */
typedef enum
{
  EN_UWB_CIR_REGISTER_DATA_0 = 0,
  EN_UWB_CIR_REGISTER_DATA_1
} enUwbCirRegisterData;

/**
 * @enum enUwbIrqEvent
 * @brief Enumeration for UWB IRQ events.
 * 
 * This enumeration defines various interrupt events that can occur in the UWB system.
 */
typedef enum 
{
  EN_UWB_IRQ_EVENT_RX0_DONE          = 1,
  EN_UWB_IRQ_EVENT_RX0_PD_DONE       = 2,
  EN_UWB_IRQ_EVENT_RX0_SFD_DET_DONE  = 3,
  EN_UWB_IRQ_EVENT_RX1_DONE          = 4,
  EN_UWB_IRQ_EVENT_RX1_PD_DONE       = 5,
  EN_UWB_IRQ_EVENT_RX1_SFD_DET_DONE  = 6,
  EN_UWB_IRQ_EVENT_RX2_DONE          = 7,
  EN_UWB_IRQ_EVENT_RX2_PD_DONE       = 8,
  EN_UWB_IRQ_EVENT_RX2_SFD_DET_DONE  = 9,
  EN_UWB_IRQ_EVENT_RX_STS_CIR_END    = 10,
  EN_UWB_IRQ_EVENT_RX_PHY_PHR        = 11,  
  EN_UWB_IRQ_EVENT_RX_DONE           = 12,
  EN_UWB_IRQ_EVENT_TX_DONE           = 13,
  EN_UWB_IRQ_EVENT_TX_SFD_MARK       = 14
} enUwbIrqEvent;

/**
 * @enum enUwbEnable
 * @brief Enumeration for enabling or disabling UWB functionality.
 * 
 * This enumeration is used to enable or disable specific UWB features.
 */
typedef enum
{
  EN_UWB_ENABLE = 0,
  EN_UWB_DISABLE
} enUwbEnable;

/**
 * @enum enUwbAbsoluteTimer
 * @brief Enumeration for UWB absolute timers.
 * 
 * This enumeration defines the available absolute timers in the UWB system.
 */
typedef enum
{
  EN_UWB_ABSOLUTE_TIMER_0 = 0,
  EN_UWB_ABSOLUTE_TIMER_1,
  EN_UWB_ABSOLUTE_TIMER_2,
  EN_UWB_ABSOLUTE_TIMER_3,
} enUwbAbsoluteTimer;

/**
 * @enum enUwbEventTimestampMask
 * @brief Enumeration for UWB event timestamp masks.
 * 
 * This enumeration defines the timestamp masks for various UWB events.
 */
typedef enum
{
  EN_UWBEVENT_TIMESTAMP_MASK_0 = 0,
  EN_UWBEVENT_TIMESTAMP_MASK_1,
  EN_UWBEVENT_TIMESTAMP_MASK_2,
  EN_UWBEVENT_TIMESTAMP_MASK_3,
  EN_UWBEVENT_TIMESTAMP_MASK_4,
  EN_UWBEVENT_TIMESTAMP_MASK_5,
  EN_UWBEVENT_TIMESTAMP_MASK_6,
  EN_UWBEVENT_TIMESTAMP_MASK_7,
  EN_UWBEVENT_TIMESTAMP_MASK_8,
  EN_UWBEVENT_TIMESTAMP_MASK_9,
  EN_UWBEVENT_TIMESTAMP_MASK_10,
  EN_UWBEVENT_TIMESTAMP_MASK_11,
  EN_UWBEVENT_TIMESTAMP_MASK_12,
  EN_UWBEVENT_TIMESTAMP_MASK_13,
  EN_UWBEVENT_TIMESTAMP_MASK_14,
  EN_UWBEVENT_TIMESTAMP_MASK_15  
} enUwbEventTimestampMask;

/**
 * @enum enUwbEventIndex
 * @brief Enumeration for UWB event indices.
 * 
 * This enumeration defines the indices for various UWB events, including timers, RX, and TX events.
 */
typedef enum
{
  EN_UWBEVENT_0_DELTA_TIMER_REDUCED = 0,
  EN_UWBEVENT_1_DELTA_TIMER_REDUCED,
  EN_UWBEVENT_2_DELTA_TIMER_REDUCED,
  EN_UWBEVENT_3_DELTA_TIMER_REDUCED,
  EN_UWBEVENT_4_DELTA_TIMER_REDUCED,
  EN_UWBEVENT_5_DELTA_TIMER_REDUCED,
  EN_UWBEVENT_6_DELTA_TIMER_FULL,
  EN_UWBEVENT_7_DELTA_TIMER_FULL,
  EN_UWBEVENT_8_DELTA_TIMER_FULL,
  EN_UWBEVENT_9_DELTA_TIMER_FULL,
  EN_UWBEVENT_10_ABSOLUTE_TIMER,
  EN_UWBEVENT_11_ABSOLUTE_TIMER,
  EN_UWBEVENT_12_ABSOLUTE_TIMER,
  EN_UWBEVENT_13_ABSOLUTE_TIMER,
  EN_UWBEVENT_14_RX_DSR_OVERFLOW,
  EN_UWBEVENT_15_RX0_DONE,
  EN_UWBEVENT_16_RX0_PD,
  EN_UWBEVENT_17_RX0_SFD_DET,
  EN_UWBEVENT_18_RX1_DONE,
  EN_UWBEVENT_19_RX1_PD,
  EN_UWBEVENT_20_RX1_SFD_DET,
  EN_UWBEVENT_21_RX2_DONE,
  EN_UWBEVENT_22_RX2_PD,
  EN_UWBEVENT_23_RX2_SFD_DET,
  EN_UWBEVENT_24_RX_STS_CIR,
  EN_UWBEVENT_25_RX_PHR,
  EN_UWBEVENT_26_RX_DONE,
  EN_UWBEVENT_27_RX_ERROR,
  EN_UWBEVENT_28_TX_DONE,
  EN_UWBEVENT_29_TX_SFD,
  EN_UWBEVENT_30_TX_STS_1,
  EN_UWBEVENT_31_TX_STS_2
} enUwbEventIndex;

/**
 * @enum enUwbEventControl
 * @brief Enumeration for UWB event control masks.
 * 
 * This enumeration defines control masks for starting and stopping UWB TX and RX events.
 */
typedef enum
{
  EN_UWBCTRL_TX_START_MASK = 0,
  EN_UWBCTRL_TX_STOP_MASK,
  EN_UWBCTRL_RX0_START_MASK,
  EN_UWBCTRL_RX0_STOP_MASK,
  EN_UWBCTRL_RX1_START_MASK,
  EN_UWBCTRL_RX1_STOP_MASK,
  EN_UWBCTRL_RX2_START_MASK,
  EN_UWBCTRL_RX2_STOP_MASK  
} enUwbEventControl;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_US_TO_NS        1000
#define DEF_ABS_TIMER_UNIT  8 // 1 count = 8ns

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Initializes the UWB chip.
 */
void cb_uwbdriver_chip_init(void);

/**
 * @brief Initializes the UWB system with the provided configuration.
 * 
 * @param UwbSystemConfig Pointer to the UWB system configuration structure.
 */
void cb_uwbdriver_uwb_init(cb_uwbsystem_systemconfig_st* UwbSystemConfig);

/**
 * @brief Initializes the UWB system RAM with the provided arguments.
 * 
 * @param args Array of arguments for RAM initialization.
 */
void cb_uwbdriver_uwb_system_ram_init(uint32_t args[]);

/**
 * @brief Turns off the UWB system.
 */
void cb_uwbdriver_uwb_off(void);

/**
 * @brief Initializes the transceiver (TRX) module.
 */
void cb_uwbdriver_trx_init(void);

/**
 * @brief Initializes the RX top module.
 */
void cb_uwbdriver_rx_top_init(void);

/**
 * @brief Turns off the RX top module.
 */
void cb_uwbdriver_rx_top_off(void);

/**
 * @brief Initializes RX0 module.
 */
void cb_uwbdriver_rx0_init(void);

/**
 * @brief Initializes RX1 module.
 */
void cb_uwbdriver_rx1_init(void);

/**
 * @brief Initializes RX2 module.
 */
void cb_uwbdriver_rx2_init(void);

/**
 * @brief Initializes RX0 and RX2 module.
 */
void cb_uwbdriver_rx02_init(void);

/**
 * @brief Initializes all RX modules.
 */
void cb_uwbdriver_rx_all_init(void);

/**
 * @brief Starts the TX process.
 */
void cb_uwbdriver_tx_start(void);

/**
 * @brief Starts the staged TX process.
 */
void cb_uwbdriver_stage_tx_start(void);

/**
 * @brief Starts the staged RX0 process.
 */
void cb_uwbdriver_stage_rx0_start(void);

/**
 * @brief Stops the TX process.
 */
void cb_uwbdriver_tx_stop(void);

/**
 * @brief Freezes the TX PLL.
 */
void cb_uwbdriver_tx_freezepll(void);

/**
 * @brief Unfreezes the TX PLL.
 */
void cb_uwbdriver_tx_unfreezepll(void);

/**
 * @brief Starts RX on specified port with gain configuration
 * 
 * @param enRxPort The RX port(s) to start (can be single port or combination)
 * @param s_sysBypassConfig Pointer to RX gain bypass configuration
 */
void cb_uwbdriver_rx_start(cb_uwbsystem_rxport_en enRxPort, cb_uwbsystem_rx_dbb_gain_st* s_sysBypassConfig);

/**
 * @brief Stops RX on specified port
 * 
 * @param enRxPort The RX port(s) to stop (can be single port or combination)
 */
void cb_uwbdriver_rx_stop(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Turns off RX module on specified port
 * 
 * @param enRxPort The RX port(s) to turn off (can be single port or combination)
 */
void cb_uwbdriver_rx_off(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Initializes the TX module.
 */
void cb_uwbdriver_tx_init(void);

/**
 * @brief Turns off the TX module.
 */
void cb_uwbdriver_tx_off(void);

/**
 * @brief Sets the RX threshold value.
 * 
 * @param threshold The threshold value to set.
 */
void cb_uwbdriver_set_rx_threshold(uint32_t threshold);

/**
 * @brief Sets the initial RX gain value.
 * 
 * @param gainRxInit The initial RX gain value to set.
 */
void cb_uwbdriver_set_gain_rx_init(uint32_t gainRxInit);

/**
 * @brief Retrieves the TX RFPLL lock status.
 * 
 * @return The TX RFPLL lock status.
 */
uint32_t cb_uwbdriver_get_tx_rfpll_lock(void);

/**
 * @brief Enables the specified UWB event IRQ.
 * 
 * @param event The UWB IRQ event to enable.
 */
void cb_uwbdriver_enable_event_irq(enUwbIrqEvent event);

/**
 * @brief Disables the specified UWB event IRQ.
 * 
 * @param event The UWB IRQ event to disable.
 */
void cb_uwbdriver_disable_event_irq(enUwbIrqEvent event);

/**
 * @brief Configures the IRQ mask for the specified UWB event.
 * 
 * @param event The UWB IRQ event to configure.
 */
void cb_uwbdriver_irq_mask_configuration(enUwbIrqEvent event);

/**
 * @brief Resets the IRQ registers.
 */
void cb_uwbdriver_irq_reset_registers(void);

/**
 * @brief Retrieves the size of the UWB TX memory.
 * 
 * @return The size of the UWB TX memory.
 */
uint32_t cb_uwbdriver_get_uwb_tx_memory_size(void);

/**
 * @brief Retrieves the size of the UWB RX memory.
 * 
 * @return The size of the UWB RX memory.
 */
uint32_t cb_uwbdriver_get_uwb_rx_memory_size(void);

/**
 * @brief Retrieves the start address of the UWB TX memory.
 * 
 * @return Pointer to the start address of the UWB TX memory.
 */
uint32_t* cb_uwbdriver_get_uwb_tx_memory_start_addr(void);

/**
 * @brief Retrieves the start address of the UWB RX memory.
 * 
 * @return Pointer to the start address of the UWB RX memory.
 */
uint32_t* cb_uwbdriver_get_uwb_rx_memory_start_addr(void);

/**
 * @brief Retrieves the RX CIR control index.
 * 
 * @return The RX CIR control index.
 */
uint16_t cb_uwbdriver_get_rx_cir_ctl_idx(void);

/**
 * @brief Retrieves the chip temperature.
 * 
 * @return The chip temperature as a float value.
 */
float cb_uwbdriver_get_chip_temp(void);

/**
 * @brief Retrieves the TX TSU timestamp.
 * 
 * @param outTxTsu Pointer to the structure to store the TX TSU timestamp.
 */
void cb_uwbdriver_get_tx_tsu_timestamp(cb_uwbsystem_tx_tsutimestamp_st* outTxTsu);

/**
 * @brief Retrieves the RX raw timestamp.
 * 
 * @param rxTsu Pointer to the structure to store the RX raw timestamp.
 */
void cb_uwbdriver_get_rx_raw_timestamp(cb_uwbsystem_rx_tsu_st* rxTsu);

/**
 * @brief Configures the TX hardware timer interval.
 * 
 * @param timeInterval The time interval to configure.
 */
void cb_uwbdriver_configure_tx_hw_timer_interval(uint32_t timeInterval);

/**
 * @brief Configures the AGC peak count value.
 * 
 * @param value The AGC peak count value to configure.
 */
void cb_uwbdriver_configure_agc_peak_cnt(uint32_t value);

/**
 * @brief Retrieves the RX CIR quality flag.
 * 
 * @return The RX CIR quality flag as an 8-bit value.
 */
uint8_t cb_uwbdriver_get_rx_cir_quality_flag(void);

/**
 * @brief Stores RX CIR register data into the destination array.
 * 
 * @param destArray Pointer to the destination array.
 * @param enRxPort The RX port to retrieve data from (EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2).
 * @param startingPosition The starting position in the CIR register.
 * @param numSamples The number of samples to store.
 */
void cb_uwbdriver_store_rx_cir_register(cb_uwbsystem_rx_cir_iqdata_st* destArray, cb_uwbsystem_rxport_en enRxPort, uint32_t startingPosition, uint32_t numSamples);

/**
 * @brief Stores RX TSU status and timestamp data.
 * 
 * @param p_rxTsuStatus Pointer to the RX TSU status structure.
 * @param p_rxTimeStampData Pointer to the RX timestamp data structure.
 * @param enRxPort The RX port to retrieve data from (EN_UWB_RX_0, EN_UWB_RX_1, or EN_UWB_RX_2).
 */
void cb_uwbdriver_store_rx_tsu_status(cb_uwbsystem_rx_tsustatus_st* p_rxTsuStatus, cb_uwbsystem_rx_tsu_st* p_rxTimeStampData, cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Retrieves the RX TSU timestamp.
 * 
 * @param rxTsuTimestamp Pointer to the structure to store the RX TSU timestamp.
 * @param enRxPort The RX port to retrieve the timestamp from (EN_UWB_RX_0, EN_UWB_RX_1, or EN_UWB_RX_2).
 */
void cb_uwbdriver_get_rx_tsu_timestamp(cb_uwbsystem_rx_tsutimestamp_st* rxTsuTimestamp, cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Retrieves the RX DCOC values for the specified RX port.
 * 
 * @param enRxPort The RX port to retrieve the DCOC values from (EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2).
 * @return The RX DCOC values as a structure.
 */
cb_uwbsystem_rx_dcoc_st cb_uwbdriver_get_rx_dcoc(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Retrieves the RX RSSI results for the specified RX ports.
 * 
 * @param rssiRxPorts The RX ports to retrieve the RSSI results from (can be single port or combination).
 * @return The RX RSSI results as a structure.
 */
cb_uwbsystem_rx_signalinfo_st cb_uwbdriver_get_rx_rssi(cb_uwbsystem_rxport_en rssiRxPorts);

/**
 * @brief Retrieves the UWB RX ETC status register values.
 * 
 * @param etcStatus Pointer to the structure to store the ETC status register values.
 */
void cb_uwbdriver_get_uwb_rx_etc_status_register(cb_uwbsystem_rx_etc_statusregister_st* const etcStatus);

/**
 * @brief Retrieves the UWB RX status register values.
 * 
 * @return The UWB RX status register values as a union.
 */
cb_uwbsystem_rxstatus_un cb_uwbdriver_get_uwb_rx_status_register(void);

/**
 * @brief Configures the RX sync CFO estimation bypass CRS.
 * 
 * @param en Enable or disable the bypass CRS.
 * @param val The value to configure.
 */
void cb_uwbdriver_configure_fixed_cfo_value(uint8_t en, uint32_t val);

/**
 * @brief Enables the specified absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to enable.
 */
void cb_uwbdriver_abs_timer_on(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Disables the specified absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to disable.
 */
void cb_uwbdriver_abs_timer_off(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Clears the internal occurrence of the specified absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to clear.
 */
void cb_uwbdriver_abs_timer_clear_internal_occurence(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Configures the timeout value for the specified absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to configure.
 * @param baseTime The base time for the timeout.
 * @param targetTimeoutTime The target timeout time.
 */
void cb_uwbdriver_abs_timer_configure_timeout_value(enUwbAbsoluteTimer enAbsoluteTimer, uint32_t baseTime, uint32_t targetTimeoutTime);

/**
 * @brief Configures the event commander for the specified absolute timer.
 * 
 * @param control Enable or disable the event commander.
 * @param enAbsoluteTimer The absolute timer to configure.
 * @param uwbEventControl The event control configuration.
 */
void cb_uwbdriver_abs_timer_configure_event_commander(enUwbEnable control, enUwbAbsoluteTimer enAbsoluteTimer, enUwbEventControl uwbEventControl);

/**
 * @brief Enables or disables the event timestamp functionality.
 * 
 * @param enable Enable or disable the event timestamp.
 */
void cb_uwbdriver_enable_event_timestamp(enUwbEnable enable);

/**
 * @brief Configures the event timestamp mask for the specified event index.
 * 
 * @param eventTimestampMask The event timestamp mask to configure.
 * @param uwbEventIndex The event index to configure the mask for.
 */
void cb_uwbdriver_configure_event_timestamp_mask(enUwbEventTimestampMask eventTimestampMask, enUwbEventIndex uwbEventIndex);

/**
 * @brief Retrieves the event timestamp value for the specified mask.
 * 
 * @param eventTimestampMask The event timestamp mask to retrieve the value for.
 * @return The event timestamp value.
 */
uint32_t cb_uwbdriver_get_event_timestamp_in_ns(enUwbEventTimestampMask eventTimestampMask);

/**
 * @brief Clears the TSU module.
 */
void cb_uwbdriver_tsu_clear(void);

/**
 * @brief Prepares the RX module for starting only RX0.
 */
void cb_uwbdriver_rx_start_prepare(void);

/**
 * @brief Prepares the TX module for starting.
 */
void cb_uwbdriver_tx_start_prepare(void);

/**
 * @brief  Configure PRF mode and PSDU data rate settings
 * @param  config Pointer to UWB configuration parameter structure 
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 * @note   Handles different PRF modes:
 *         - BPRF: Base Pulse Repetition Frequency
 *         - HPRF_124P8: High PRF 124.8MHz
 *         - HPRF_249P6: High PRF 249.6MHz
 *         - LG4A: Low Rate Long Range Mode
 */
void cb_uwbdriver_configure_prf_mode_psdu_data_rate(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief  Configure UWB preamble code for transmission or reception
 * @param  config Pointer to UWB configuration parameter structure
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 * @note   - Valid preamble code index range: 9-32
 *         - Sets appropriate SHR code values
 *         - Adjusts sync configuration based on index range
 *         - Defaults to index 9 for BPRF or 25 for HPRF if invalid
 */
void cb_uwbdriver_configure_preamble_code_index(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief  Configure preamble duration for UWB transmission or reception
 * @param  config Pointer to UWB configuration parameter structure
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 */
void cb_uwbdriver_configure_preamble_duration(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief  Configure SFD (Start Frame Delimiter) parameters
 * @param  config Pointer to UWB configuration parameter structure
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 * @note   SFD ID options and lengths:
 *         - ID 0: 8 symbols
 *         - ID 1: 4 symbols
 *         - ID 2: 8 symbols
 *         - ID 3: 16 symbols
 *         - ID 4: 32 symbols
 */
void cb_uwbdriver_configure_sfd_id(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief  Configure STS (Scrambled Timestamp Sequence) parameters
 * @param  config Pointer to UWB configuration parameter structure
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 * @note   Configures:
 *         - RFRAME configuration
 *         - STS length and gap
 *         - AES keys and initialization vectors
 *         - Number of STS segments (0-4)
 */
void cb_uwbdriver_configure_sts(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief Configure PHR (PHY Header) and PSDU (PHY Service Data Unit) parameters for transmission
 *
 * This function configures the PHY Header (PHR) and PSDU fields for UWB packet transmission.
 * @param config Pointer to UWB configuration parameter structure
 * @param txPayload Pointer to TxPayload structure containing:
 *                  - payloadSize: Size of payload data in bytes
 *                  - phrRangingBit: Ranging enable bit for PHR (0/1)
 *                  - payload: Pointer to payload data buffer
 * @note   Different configurations for:
 *         - BPRF mode: 7-bit payload length
 *         - HPRF modes: 12-bit payload length
 */
void cb_uwbdriver_configure_tx_phr_psdu(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_txpayload_st* txPayload);

/**
 * @brief Get the received packet's PHR (Physical Layer Header).
 *
 * This function retrieves the Physical Layer Header (PHR) from the received packet, which
 * contains information about the packet's size and type.
 *
 * @return The received packet's PHR value.
 */
uint32_t cb_uwbdriver_get_rx_packet_phr(void);

/**
 * @brief Get the size of the received packet.
 *
 * This function calculates and returns the size of the received packet, excluding any 
 * necessary CRC bytes, based on the packet mode.
 *
 * @param  config Pointer to UWB configuration parameter structure 
 * @return The size of the received packet, excluding CRC.
 */
uint16_t cb_uwbdriver_get_rx_packet_size(cb_uwbsystem_packetconfig_st* config);

/**
 * @brief Get the Rx PHR Ranging bit infomation.
 *
 * This function retrive PHR Ranging bit infomation of the received packet
 *
 * @param  config Pointer to UWB configuration parameter structure 
 * @return Rx PHR Ranging bit.
 */
uint8_t cb_uwbdriver_get_rx_phr_ranging_bit(cb_uwbsystem_packetconfig_st* config);

/**
 * @brief  Configure MAC FCS (Frame Check Sequence) type
 * @param  config Pointer to UWB configuration parameter structure
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 * @note   FCS types:
 *         - CRC16 (default)
 *         - CRC32 (when macFcsType = EN_MAC_FCS_TYPE_CRC32)
 */
void cb_uwbdriver_configure_mac_fcs_type(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief Apply transmission power settings based on the provided power code.
 *
 * This function selects and applies the appropriate transmission power settings from the 
 * predefined power code table based on the provided power code. The settings are then 
 * written to the relevant registers to adjust the transmission power.
 *
 * @param powerCode The power code representing the desired power setting.
 */
void cb_uwbdriver_configure_tx_power(uint8_t powerCode);

/**
 * @brief Configure the TX (transmit) timestamp capture.
 *
 * This function configures the UWB transmission timestamp capture for various events 
 * during transmission, including the TX start, SFD mark, STS mark, and TX done events.
 */
void cb_uwbdriver_configure_tx_timestamp_capture(void);

/**
 * @brief Configure the RX (receive) timestamp capture.
 *
 * This function configures the UWB reception timestamp capture for various events 
 * during reception, including the RX start, SFD detection, status events, and RX done event.
 */
void cb_uwbdriver_configure_rx_timestamp_capture(void);

/**
 * @brief Retrieve the TX timestamps.
 *
 * This function retrieves the timestamps related to the transmission start, 
 * SFD mark, STS1 mark, STS2 mark, and transmission completion. These timestamps 
 * are read from specific memory-mapped registers and stored in the provided 
 * `cb_uwbsystem_tx_timestamp_st` structure.
 *
 * The `cb_uwbsystem_tx_timestamp_st` structure contains the following members:
 * - `txStart`: TX start timestamp.
 * - `sfdMark`: SFD mark timestamp.
 * - `sts1Mark`: STS 1 mark timestamp.
 * - `sts2Mark`: STS 2 mark timestamp.
 * - `txDone`: TX done timestamp.
 *
 * @param txTimestamp Pointer to a `cb_uwbsystem_tx_timestamp_st` structure where the 
 *                    retrieved timestamps will be stored.
 */
void cb_uwbdriver_get_tx_raw_timestamp(cb_uwbsystem_tx_timestamp_st* txTimestamp);

/**
 * @brief Clear hardware tsu
 *
 */
void cb_uwbdriver_tsu_clear(void);

#endif // __CB_UWB_DRIVER_H
