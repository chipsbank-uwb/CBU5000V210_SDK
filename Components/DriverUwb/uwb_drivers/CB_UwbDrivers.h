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
#include "CB_Common.h"
#include "CB_system_types.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_US_TO_NS        1000
#define DEF_ABS_TIMER_UNIT  8 // 1 count = 8ns

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------


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
  EN_UWBCTRL_TX_START_MASK  = 0b00000001,
  EN_UWBCTRL_TX_STOP_MASK   = 0b00000010,
  EN_UWBCTRL_RX0_START_MASK = 0b00000100,
  EN_UWBCTRL_RX0_STOP_MASK  = 0b00001000,
  EN_UWBCTRL_RX1_START_MASK = 0b00010000,
  EN_UWBCTRL_RX1_STOP_MASK  = 0b00100000,
  EN_UWBCTRL_RX2_START_MASK = 0b01000000,
  EN_UWBCTRL_RX2_STOP_MASK  = 0b10000000,
  EN_UWBCTRL_RX_ALL_START_MASK  = (EN_UWBCTRL_RX0_START_MASK | EN_UWBCTRL_RX1_START_MASK | EN_UWBCTRL_RX2_START_MASK),  
} enUwbEventControl;

/**
 * @enum enUwbRxCirType
 * @brief Enumeration for selecting the UWB CIR memory source.
 *
 * This enumeration specifies which Complex Impulse Response (CIR) memory
 * region to read from in the UWB hardware.
 */
typedef enum {
  EN_CIR_STS = 0, 
  EN_CIR_RX  = 1
} enUwbRxCirType;

//-----------------------------------------
// Typedefs for function pointer signatures
//-----------------------------------------

// ===== SYSTEM / POWER =====
/**
 * @brief Initializes the UWB chip.
 */
typedef void     (*uwbdriver_chip_init_fn)(void);

/**
 * @brief Initializes the UWB system with the provided configuration.
 * 
 * @param UwbSystemConfig Pointer to the UWB system configuration structure.
 */
typedef void     (*uwbdriver_uwb_init_fn)(cb_uwbsystem_systemconfig_st* UwbSystemConfig);

/**
 * @brief Initializes the UWB system RAM with the provided arguments.
 * 
 * @param args Array of arguments for RAM initialization.
 */
typedef void     (*uwbdriver_uwb_system_ram_init_fn)(uint32_t args[]);

/**
 * @brief Turns off the UWB system.
 */
typedef void     (*uwbdriver_uwb_off_fn)(void);

// ===== TRX / RX / TX CONTROL =====
/**
 * @brief Initializes the transceiver (TRX) module.
 */
typedef void     (*uwbdriver_trx_init_fn)(void);

/**
 * @brief Initializes the RX top module.
 */
typedef void     (*uwbdriver_rx_top_init_fn)(void);

/**
 * @brief Turns off the RX top module.
 */
typedef void     (*uwbdriver_rx_top_off_fn)(void);

/**
 * @brief Initializes the RX module.
 */
typedef void     (*uwbdriver_rx_init_fn)(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Starts RX on specified port with gain configuration
 * 
 * @param enRxPort The RX port(s) to start (can be single port or combination)
 * @param s_sysBypassConfig Pointer to RX gain bypass configuration
 * @param trxStartMode The start mode of the TX process
 */

typedef void     (*uwbdriver_rx_start_fn)(cb_uwbsystem_rxport_en enRxPort,
                                          cb_uwbsystem_rx_dbb_gain_st* s_sysBypassConfig,
                                          cb_uwbsystem_trx_startmode_en trxStartMode);

/**
 * @brief Stops RX on specified port
 * 
 * @param enRxPort The RX port(s) to stop (can be single port or combination)
 */
typedef void     (*uwbdriver_rx_stop_fn)(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Turns off RX module on specified port
 * 
 * @param enRxPort The RX port(s) to turn off (can be single port or combination)
 */
typedef void     (*uwbdriver_rx_off_fn)(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Initializes the TX module.
 */
/**
 * @brief Initializes the TX module.
 */
typedef void     (*uwbdriver_tx_init_fn)(void);

/**
 * @brief Starts the TX process.
 */
typedef void     (*uwbdriver_tx_start_fn)(cb_uwbsystem_trx_startmode_en trxStartMode);

/**
 * @brief Starts the staged TX process.
 */
typedef void     (*uwbdriver_stage_tx_start_fn)(void);

/**
 * @brief Starts the staged RX0 process.
 */
typedef void     (*uwbdriver_stage_rx0_start_fn)(void);

/**
 * @brief Stops the TX process.
 */
typedef void     (*uwbdriver_tx_stop_fn)(void);

/**
 * @brief Turns off the TX module.
 */
typedef void     (*uwbdriver_tx_off_fn)(void);

/**
 * @brief Starts the TX CW
 */
typedef void     (*uwbdriver_tx_cw_start_fn)(void);

/**
 * @brief Freezes the TX PLL.
 */
typedef void     (*uwbdriver_tx_freezepll_fn)(void);

/**
 * @brief Unfreezes the TX PLL.
 */
typedef void     (*uwbdriver_tx_unfreezepll_fn)(void);

/**
 * @brief Config common tsu and time mask of TRX
 */
typedef void     (*uwbdriver_trx_config_cmn_tsu_timemask_fn)(void);


/**
 * @brief Configure the CIR correlation length.
 */
typedef void    (*uwbdriver_rx_configure_cir_correction_length_fn)(uint8_t correction_length);


// ===== IRQ CONTROL =====
/**
 * @brief Enables the specified UWB event IRQ.
 * 
 * @param event The UWB IRQ event to enable.
 */
typedef void     (*uwbdriver_enable_event_irq_fn)(enUwbIrqEvent event);

/**
 * @brief Disables the specified UWB event IRQ.
 * 
 * @param event The UWB IRQ event to disable.
 */
typedef void     (*uwbdriver_disable_event_irq_fn)(enUwbIrqEvent event);

/**
 * @brief Configures the IRQ mask for the specified UWB event.
 * 
 * @param event The UWB IRQ event to configure.
 */
typedef void     (*uwbdriver_irq_mask_configuration_fn)(enUwbIrqEvent event);

/**
 * @brief Resets the IRQ registers.
 */
typedef void     (*uwbdriver_irq_reset_registers_fn)(void);

// ===== MEMORY / BUFFERS =====
/**
 * @brief Retrieves the size of the UWB TX memory.
 * 
 * @return The size of the UWB TX memory.
 */
typedef uint32_t (*uwbdriver_get_uwb_tx_memory_size_fn)(void);

/**
 * @brief Retrieves the size of the UWB RX memory.
 * 
 * @return The size of the UWB RX memory.
 */
typedef uint32_t (*uwbdriver_get_uwb_rx_memory_size_fn)(void);

/**
 * @brief Retrieves the start address of the UWB TX memory.
 * 
 * @return Pointer to the start address of the UWB TX memory.
 */
typedef uint32_t*(*uwbdriver_get_uwb_tx_memory_start_addr_fn)(void);

/**
 * @brief Retrieves the start address of the UWB RX memory.
 * 
 * @return Pointer to the start address of the UWB RX memory.
 */
typedef uint32_t*(*uwbdriver_get_uwb_rx_memory_start_addr_fn)(void);

// ===== TIMING / TSU / INDICES =====
/**
 * @brief Retrieves the RX CIR control index.
 * 
 * @return The RX CIR control index.
 */
typedef uint16_t (*uwbdriver_get_rx_cir_ctl_idx_fn)(void);

/**
 * @brief Retrieves the TX TSU timestamp.
 * 
 * @param outTxTsu Pointer to the structure to store the TX TSU timestamp.
 */
typedef void     (*uwbdriver_get_tx_tsu_timestamp_fn)(cb_uwbsystem_tx_tsutimestamp_st* outTxTsu);

/**
 * @brief Retrieves the RX raw timestamp.
 * 
 * @param rxTsu Pointer to the structure to store the RX raw timestamp.
 */
typedef void     (*uwbdriver_get_rx_raw_timestamp_fn)(cb_uwbsystem_rx_tsu_st* rxTsu);

// ===== STATUS / SENSORS / KNOBS =====
/**
 * @brief Retrieves the TX RFPLL lock status.
 * 
 * @return The TX RFPLL lock status.
 */
typedef uint32_t (*uwbdriver_get_tx_rfpll_lock_fn)(void);

/**
 * @brief Retrieves the chip temperature.
 * 
 * @return The chip temperature as a float value.
 */
typedef float    (*uwbdriver_get_chip_temp_fn)(void);

/**
 * @brief Sets the RX threshold value.
 * 
 * @param threshold The threshold value to set.
 */
typedef void     (*uwbdriver_set_rx_threshold_fn)(uint32_t threshold);

/**
 * @brief Sets the initial RX gain value.
 * 
 * @param gainRxInit The initial RX gain value to set.
 */
typedef void     (*uwbdriver_set_gain_rx_init_fn)(uint32_t gainRxInit);

/**
 * @brief Configures the TX hardware timer interval.
 * 
 * @param timeInterval The time interval to configure.
 */
typedef void     (*uwbdriver_configure_tx_hw_timer_interval_fn)(uint32_t timeInterval);

/**
 * @brief Configures the AGC peak count value.
 * 
 * @param value The AGC peak count value to configure.
 */
typedef void     (*uwbdriver_configure_agc_peak_cnt_fn)(uint32_t value);

/**
 * @brief Retrieves the RX CIR quality flag.
 * 
 * @return The RX CIR quality flag as an 8-bit value.
 */
typedef uint8_t  (*uwbdriver_get_rx_cir_quality_flag_fn)(void);

// ===== RX/TX DATA CAPTURE & REGISTERS =====
/**
 * @brief Stores RX CIR register data into the destination array.
 * 
 * @param destArray Pointer to the destination array.
 * @param enRxPort The RX port to retrieve data from (EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2).
 * @param startingPosition The starting position in the CIR register.
 * @param numSamples The number of samples to store.
 */
typedef void     (*uwbdriver_get_rx_cir_register_fn)(cb_uwbsystem_rx_cir_iqdata_st* destArray,
                                                       cb_uwbsystem_rxport_en enRxPort,
                                                       uint32_t startingPosition,
                                                       uint32_t numSamples);

/**
 * @brief Stores RX TSU status and timestamp data.
 * 
 * @param p_rxTsuStatus Pointer to the RX TSU status structure.
 * @param p_rxTimeStampData Pointer to the RX timestamp data structure.
 * @param enRxPort The RX port to retrieve data from (EN_UWB_RX_0, EN_UWB_RX_1, or EN_UWB_RX_2).
 */
typedef void     (*uwbdriver_get_rx_tsu_status_fn)(cb_uwbsystem_rx_tsustatus_st* p_rxTsuStatus,
                                                     cb_uwbsystem_rx_tsu_st* p_rxTimeStampData,
                                                     cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Retrieves the RX TSU timestamp.
 * 
 * @param rxTsuTimestamp Pointer to the structure to store the RX TSU timestamp.
 * @param enRxPort The RX port to retrieve the timestamp from (EN_UWB_RX_0, EN_UWB_RX_1, or EN_UWB_RX_2).
 */
typedef void     (*uwbdriver_get_rx_tsu_timestamp_fn)(cb_uwbsystem_rx_tsutimestamp_st* rxTsuTimestamp,
                                                      cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Retrieves the RX TSU timestamp.
 * 
 * @param rxTsuTimestamp Pointer to the structure to store the RX TSU timestamp.
 * @param enRxPort The RX port to retrieve the timestamp from (EN_UWB_RX_0, EN_UWB_RX_1, or EN_UWB_RX_2).
 * @param scale_factor      Peak divisor for detection threshold; default 10. For dynamic
 *                          range, use 6, 10, or 18 for 15 dB, 20 dB, or 25 dB dynamic range.
 * @param le_noise_th_ip    Input noise floor; default 0. Effective threshold is
 *                          max(prefix estimate from taps 100..119, le_noise_th_ip).
 * @param mm_option         Multipath mitigation mode:
 *                          0 - conventional leading edge detection with precursor protection;
 *                          1 - near-range multipath: mitigation with interpolation compensation;
 *                          2 - near-range multipath: mitigation with near-energy compensation;
 *                          3 - near-range multipath: compensation of both 1 and 2.
 *
 */
typedef void     (*uwbdriver_get_rx_tsu_timestamp_lemm_fn)(cb_uwbsystem_rx_tsutimestamp_st* rxTsuTimestamp, cb_uwbsystem_rxport_en enRxPort, int scale_factor, int le_noise_th_ip, int mm_option);


/**
 * @brief Retrieves the RX DCOC values for the specified RX port.
 * 
 * @param enRxPort The RX port to retrieve the DCOC values from (EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2).
 * @return The RX DCOC values as a structure.
 */
typedef cb_uwbsystem_rx_dcoc_st
                 (*uwbdriver_get_rx_dcoc_fn)(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Retrieves the RX RSSI results for the specified RX ports.
 * 
 * @param rssiRxPorts The RX ports to retrieve the RSSI results from (can be single port or combination).
 * @return The RX RSSI results as a structure.
 */
typedef cb_uwbsystem_rx_signalinfo_st
                 (*uwbdriver_get_rx_rssi_fn)(cb_uwbsystem_rxport_en rssiRxPorts);

/**
 * @brief Retrieves the UWB RX ETC status register values.
 * 
 * @param etcStatus Pointer to the structure to store the ETC status register values.
 */
typedef void     (*uwbdriver_get_uwb_rx_etc_status_register_fn)(cb_uwbsystem_rx_etc_statusregister_st* const etcStatus);

/**
 * @brief Retrieves the UWB RX status register values.
 * 
 * @return The UWB RX status register values as a union.
 */
typedef cb_uwbsystem_rxstatus_un
                 (*uwbdriver_get_uwb_rx_status_register_fn)(void);

// ===== CFO / ABSOLUTE TIMERS / EVENT TIMESTAMPING =====
/**
 * @brief Configures the RX sync CFO estimation bypass CRS.
 * 
 * @param en Enable or disable the bypass CRS.
 * @param val The value to configure.
 */
typedef void     (*uwbdriver_configure_fixed_cfo_value_fn)(uint8_t en, int32_t val);

/**
 * @brief Enables the specified absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to enable.
 */
typedef void     (*uwbdriver_abs_timer_on_fn)(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Disables the specified absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to disable.
 */
typedef void     (*uwbdriver_abs_timer_off_fn)(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Clears the internal occurrence of the specified absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to clear.
 */
typedef void     (*uwbdriver_abs_timer_clear_internal_occurence_fn)(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Configures the timeout value for the specified absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to configure.
 * @param baseTime The base time for the timeout.
 * @param targetTimeoutTime The target timeout time.
 */
typedef void     (*uwbdriver_abs_timer_configure_timeout_value_fn)(enUwbAbsoluteTimer enAbsoluteTimer,
                                                                   uint32_t baseTime, uint32_t targetTimeoutTime);

/**
 * @brief Configures the event commander for the specified absolute timer.
 * 
 * @param control Enable or disable the event commander.
 * @param enAbsoluteTimer The absolute timer to configure.
 * @param uwbEventControl The event control configuration.
 */
typedef void     (*uwbdriver_abs_timer_configure_event_commander_fn)(enUwbEnable control,
                                                                     enUwbAbsoluteTimer enAbsoluteTimer,
                                                                     enUwbEventControl uwbEventControl);

/**
 * @brief Enables or disables the event timestamp functionality.
 * 
 * @param enable Enable or disable the event timestamp.
 */
typedef void     (*uwbdriver_enable_event_timestamp_fn)(enUwbEnable enable);

/**
 * @brief Configures the event timestamp mask for the specified event index.
 * 
 * @param eventTimestampMask The event timestamp mask to configure.
 * @param uwbEventIndex The event index to configure the mask for.
 */
typedef void     (*uwbdriver_configure_event_timestamp_mask_fn)(enUwbEventTimestampMask eventTimestampMask,
                                                                enUwbEventIndex uwbEventIndex);

/**
 * @brief Retrieves the event timestamp value for the specified mask.
 * 
 * @param eventTimestampMask The event timestamp mask to retrieve the value for.
 * @return The event timestamp value.
 */
typedef uint32_t (*uwbdriver_get_event_timestamp_in_ns_fn)(enUwbEventTimestampMask eventTimestampMask);

/**
 * @brief Inserts a UWB event into the APB event register.
 *
 * This function maps a given UWB event index to its corresponding
 * event mask definition and writes it to the UWB APB event register
 *
 * The function is typically used to signal hardware about a specific
 * UWB event (e.g., delta timer, absolute timer, RX/TX status).
 *
 * @param[in] uwbEventIndex
 *
 * @note If the event index does not match any case, no event mask
 *       is written (uwbEventMask remains 0).
 *
 * @return None.
 */
typedef void     (*uwbdriver_insert_apb_event_fn)(enUwbEventIndex uwbEventIndex);

/**
 * @brief Clears the TSU module.
 */
typedef void     (*uwbdriver_tsu_clear_fn)(void);

// ===== RX/TX START PREP =====
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
typedef void     (*uwbdriver_configure_prf_mode_psdu_data_rate_fn)(cb_uwbsystem_packetconfig_st* config,
                                                                   cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief  Configure UWB preamble code for transmission or reception
 * @param  config Pointer to UWB configuration parameter structure
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 * @note   - Valid preamble code index range: 9-32
 *         - Sets appropriate SHR code values
 *         - Adjusts sync configuration based on index range
 *         - Defaults to index 9 for BPRF or 25 for HPRF if invalid
 */
typedef void     (*uwbdriver_configure_preamble_code_index_fn)(cb_uwbsystem_packetconfig_st* config,
                                                               cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief  Configure preamble duration for UWB transmission or reception
 * @param  config Pointer to UWB configuration parameter structure
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 */
typedef void     (*uwbdriver_configure_preamble_duration_fn)(cb_uwbsystem_packetconfig_st* config,
                                                             cb_uwbsystem_configmodule_selection_en configTrxSelect);

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
typedef void     (*uwbdriver_configure_sfd_id_fn)(cb_uwbsystem_packetconfig_st* config,
                                                  cb_uwbsystem_configmodule_selection_en configTrxSelect);

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
typedef void     (*uwbdriver_configure_sts_fn)(cb_uwbsystem_packetconfig_st* config,
                                               cb_uwbsystem_configmodule_selection_en configTrxSelect);

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
typedef void     (*uwbdriver_configure_tx_phr_psdu_fn)(cb_uwbsystem_packetconfig_st* config,
                                                       cb_uwbsystem_txpayload_st* txPayload);

/**
 * @brief Get the received packet's PHR (Physical Layer Header).
 *
 * This function retrieves the Physical Layer Header (PHR) from the received packet, which
 * contains information about the packet's size and type.
 *
 * @return The received packet's PHR value.
 */
typedef uint32_t (*uwbdriver_get_rx_packet_phr_fn)(void);

/**
 * @brief Get the size of the received packet.
 *
 * This function calculates and returns the size of the received packet, excluding any 
 * necessary CRC bytes, based on the packet mode.
 *
 * @param  config Pointer to UWB configuration parameter structure 
 * @return The size of the received packet, excluding CRC.
 */
typedef uint16_t (*uwbdriver_get_rx_packet_size_fn)(cb_uwbsystem_packetconfig_st* config);

/**
 * @brief Get the Rx PHR Ranging bit infomation.
 *
 * This function retrive PHR Ranging bit infomation of the received packet
 *
 * @param  config Pointer to UWB configuration parameter structure 
 * @return Rx PHR Ranging bit.
 */
typedef uint8_t  (*uwbdriver_get_rx_phr_ranging_bit_fn)(cb_uwbsystem_packetconfig_st* config);

/**
 * @brief  Configure MAC FCS (Frame Check Sequence) type
 * @param  config Pointer to UWB configuration parameter structure
 * @param  configTrxSelect Select between TX/RX configuration (EN_UWB_CONFIG_RX/TX)
 * @note   FCS types:
 *         - CRC16 (default)
 *         - CRC32 (when macFcsType = EN_MAC_FCS_TYPE_CRC32)
 */
typedef void     (*uwbdriver_configure_mac_fcs_type_fn)(cb_uwbsystem_packetconfig_st* config,
                                                        cb_uwbsystem_configmodule_selection_en configTrxSelect);

/**
 * @brief Apply transmission power settings based on the provided power code.
 *
 * This function selects and applies the appropriate transmission power settings from the 
 * predefined power code table based on the provided power code. The settings are then 
 * written to the relevant registers to adjust the transmission power.
 *
 * @param powerCode The power code representing the desired power setting.
 */
typedef void     (*uwbdriver_configure_tx_power_fn)(uint8_t powerCode);

/**
 * @brief Configure the TX (transmit) timestamp capture.
 *
 * This function configures the UWB transmission timestamp capture for various events 
 * during transmission, including the TX start, SFD mark, STS mark, and TX done events.
 */
typedef void     (*uwbdriver_configure_tx_timestamp_capture_fn)(void);

/**
 * @brief Configure the RX (receive) timestamp capture.
 *
 * This function configures the UWB reception timestamp capture for various events 
 * during reception, including the RX start, SFD detection, status events, and RX done event.
 */
typedef void     (*uwbdriver_configure_rx_timestamp_capture_fn)(void);

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
typedef void     (*uwbdriver_get_tx_raw_timestamp_fn)(cb_uwbsystem_tx_timestamp_st* txTimestamp);

// ===== RADAR BLOCK =====
#define CB_DRIVER_RADAR_USE_1T1R 1U
#define CB_DRIVER_RADAR_USE_1T2R 2U
#define CB_DRIVER_RADAR_USE_1T3R 3U
/**
 * @brief Brings the radar TX/RX hardware online for sensing.
 *
 * @param powerCode   TX power code (1-60).
 * @param num_rx_mode Antenna topology: CB_DRIVER_RADAR_USE_1T1R or CB_DRIVER_RADAR_USE_1T2R
 */
typedef void     (*uwbdriver_radar_on_fn)(uint32_t powerCode, uint32_t num_rx_mode);

#define CB_UWB_DRIVER_RADAR_ON_API

/**
 * @brief Configures radar packet type, preamble, and RFRAME for a burst.
 *
 * Uses antenna topology stored by @ref cb_uwbdriver_radar_on (driver s_num_rx_mode).
 *
 * @param scaleBit     RX AGC scale bit (0-7).
 * @param modePrf      PRF packet mode: 0=M5P1, 1=M4P2, 2=M1P2
 * @param preambleCodeIndex Preamble code index (9-32); applied when modePrf is 0 or 1
 */
typedef void     (*uwbdriver_radar_config_fn)(uint32_t scaleBit, uint32_t modePrf,
                                              cb_uwbsystem_preamblecodeidx_en preambleCodeIndex);

#define CB_UWB_DRIVER_RADAR_CONFIG_API

/**
 * @brief Configure UWB preamble code index for radar (TX and RX together).
 *
 * @param preambleCodeIndex Preamble code index (9-32); applied as-is with no PRF or TRX checks.
 */
typedef void     (*uwbdriver_radar_configure_preamble_code_index_fn)(cb_uwbsystem_preamblecodeidx_en preambleCodeIndex);

/**
 * @brief Starts the radar system with specified gain settings.
 *
 * This function initiates the radar operation by starting TX and RX modules,
 * configuring timing registers, and setting the receive gain index based on
 * the current radar library configuration.
 *
 * @param gain_idx The gain index for the receiver (3-bit value, 0-7 range)
 */
typedef void     (*uwbdriver_radar_start_fn)(uint32_t gain_idx);

/**
 * @brief Retrieves the timestamp difference between TX and RX for radar operations
 *
 * This function reads the hardware timestamp registers and calculates the
 * difference between the TX and RX timestamps.
 *
 * @return Timestamp difference
 */
typedef uint32_t (*uwbdriver_radar_get_timestamp_diff_fn)(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Driver-level function to retrieve Channel Impulse Response (CIR) data for radar
 *
 * This function implements the low-level driver functionality to retrieve CIR data
 * from the specified UWB receiver port for radar applications. 
 * @param destArray Pointer to destination array of cb_uwbsystem_rx_cir_iqdata_st structures
 *                  to store the retrieved CIR I/Q data from hardware registers
 * @param enRxPort  The UWB receiver port to retrieve CIR data from (EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2)
 * @param NumCirSample Number of CIR samples to retrieve from the hardware registers
 * @param enableDetect Non-zero to run ratio-based first-CIR detect on the flagged RX port
 * @return CB_PASS if raw timestamp diff is in [8, 24]; otherwise CB_FAIL.
 *         Out-of-range diff is clamped to 8 for align math (py v5 L475-L476).
 */
typedef CB_STATUS (*uwbdriver_radar_get_cir_fn)(cb_uwbsystem_rx_cir_iqdata_st* destArray,
                                              cb_uwbsystem_rxport_en enRxPort,
                                              uint32_t NumCirSample,
                                              uint8_t enableDetect);

/**
 * @brief Stop radar TX and RX operations
 */
typedef void     (*uwbdriver_radar_stop_fn)(void);

/**
 * @brief Deinitializes and powers down the radar system.
 *
 * This function turns off all radar-related modules.
 */
typedef void     (*uwbdriver_radar_off_fn)(void);

/**
 * @brief Pulse RX domain reset (assert then release RX_RSTN).
 */
typedef void     (*uwbdriver_radar_reset_fn)(void);

/**
 * @brief Perform FFT processing on radar data.
 *
 * This function performs Fast Fourier Transform (FFT) processing on the provided data.
 * It supports different FFT lengths and can perform both forward and inverse FFT operations.
 *
 * @param fft_len The FFT length
 * @param pSrc Pointer to the source data array
 * @param ifftFlag Flag to indicate inverse FFT (1) or forward FFT (0)
 * @param doBitReverse Flag to indicate if bit reversal should be performed
 */
typedef void     (*uwbdriver_fft_fn)(cb_uwbradar_en fft_len, float* pSrc, uint8_t ifftFlag, uint8_t doBitReverse);

// ===== ADC / MISC =====

/**
 * @brief Performs ADC testing with specified gain stage.
 * 
 * This function reads the auxiliary ADC voltage value using the specified gain stage.
 * The function includes input validation to ensure the gain stage is within
 * the valid range.
 * 
 * Gain Stage Voltage Ranges:
 * | Gain Stage | Voltage Range (V) |
 * |------------|-------------------|
 * |     0      |   0.0 to 3.3      |
 * |     1      |   0.0 to 2.5      |
 * |     2      |   0.0 to 1.8      |
 * |     3      |   0.0 to 1.5      |
 * |     4      |   0.0 to 1.2      |
 * |     5      |   0.0 to 0.9      |
 * 
 * @param gain_stage ADC gain stage setting (valid range: 0-5).
 *                   Invalid values (>5) will cause the function to return 0.
 * 
 * @return float The ADC voltage reading value. Returns 0.0 for invalid gain_stage.
 * 
 * @note If an invalid gain_stage (>5) is provided, the function will return 0.0
 *       without performing any ADC operation.
 */
typedef float    (*uwbdriver_adc_read_AIN_voltage_fn)(uint8_t gain_stage);

/**
 * @brief Read ADC input and return a 10-bit scaled code value.
 *
 * This function reads the ADC voltage for the specified gain stage using
 * cb_adc_read_AIN_voltage(), then scales the result to a 10-bit range
 * (0 to 1024 inclusive) based on the full-scale voltage for that gain stage.
 *
 * Full-scale voltages per gain stage:
 * | Gain Stage | Full-Scale Voltage (V) |
 * |------------|------------------------|
 * |     0      | 3.3                    |
 * |     1      | 2.5                    |
 * |     2      | 1.8                    |
 * |     3      | 1.5                    |
 * |     4      | 1.2                    |
 * |     5      | 0.9                    |
 *
 * @param gain_stage ADC gain stage (0-5). Values >5 are invalid.
 * @return uint16_t Scaled ADC code in the range 0-1024.
 *         Returns 0 if gain_stage is invalid.
 */
typedef uint16_t (*uwbdriver_adc_read_AIN_10bit_code_fn)(uint8_t gain_stage);

/**
 * @brief  Retrieve Complex Impulse Response (CIR) I/Q samples from UWB hardware.
 *
 * This function reads the Complex Impulse Response (CIR) values (both In-phase (I) and 
 * Quadrature (Q) components) from the UWB device memory-mapped registers. The data is 
 * read from either the RX CIR or STS CIR memory region, depending on the selected type.
 * 
 * It clears the provided I and Q buffers before populating them with the retrieved samples.
 *
 * @param[out] p_cirIvalue Pointer to the buffer for storing In-phase (I) values.
 * @param[out] p_cirQvalue Pointer to the buffer for storing Quadrature (Q) values.
 * @param[in]  enUwbRxCir  Type of CIR data to read:
 *                         - EN_CIR_RX : Standard RX CIR
 *                         - EN_CIR_STS: STS CIR
 * @param[in]  size        Number of CIR samples to read.
 *
 * @note 
 * - The buffers `p_cirIvalue` and `p_cirQvalue` must have at least `size` elements allocated.
 * - This function directly accesses hardware registers via memory-mapped I/O.
 * - Each 32-bit register contains one I (upper 16 bits) and one Q (lower 16 bits) sample.
 *
 * @warning Ensure that `p_cirIvalue` and `p_cirQvalue` are not NULL.
 *
 * @retval None
 */
typedef void     (*uwbdriver_get_rx_cir_iq_fn)(int16_t* p_cirIvalue, int16_t* p_cirQvalue,
                                               enUwbRxCirType enUwbRxCir, uint32_t size);

/**
 * @brief Initializes the UWB chip with low power mode enabled.
 * 
 * @note This function is used to configure the UWB chip for power-efficient operation.
 *
 * @retval None
 */
typedef void     (*uwbdriver_chip_init_lowpower_enable_fn)(void);

//-----------------------------------------
// Accessor
//-----------------------------------------
// --- SYSTEM / POWER ---
uwbdriver_chip_init_fn            cb_getfn_uwbdriver_chip_init          (void);
uwbdriver_uwb_init_fn             cb_getfn_uwbdriver_uwb_init           (void);
uwbdriver_uwb_system_ram_init_fn  cb_getfn_uwbdriver_uwb_system_ram_init(void);
uwbdriver_uwb_off_fn              cb_getfn_uwbdriver_uwb_off            (void);
                                           
// --- TRX / RX / TX CONTROL ---           
uwbdriver_trx_init_fn       cb_getfn_uwbdriver_trx_init      (void);
uwbdriver_rx_top_init_fn    cb_getfn_uwbdriver_rx_top_init   (void); 
uwbdriver_rx_top_off_fn     cb_getfn_uwbdriver_rx_top_off    (void);
uwbdriver_rx_init_fn        cb_getfn_uwbdriver_rx_init       (void);
uwbdriver_rx_start_fn       cb_getfn_uwbdriver_rx_start      (void);
uwbdriver_rx_stop_fn        cb_getfn_uwbdriver_rx_stop       (void);
uwbdriver_rx_off_fn         cb_getfn_uwbdriver_rx_off        (void);

uwbdriver_tx_init_fn        cb_getfn_uwbdriver_tx_init       (void);    
uwbdriver_tx_start_fn       cb_getfn_uwbdriver_tx_start      (void);
uwbdriver_tx_stop_fn        cb_getfn_uwbdriver_tx_stop       (void);
uwbdriver_tx_off_fn         cb_getfn_uwbdriver_tx_off        (void);

uwbdriver_tx_cw_start_fn    cb_getfn_uwbdriver_tx_cw_start   (void);

uwbdriver_tx_freezepll_fn   cb_getfn_uwbdriver_tx_freezepll  (void);
uwbdriver_tx_unfreezepll_fn cb_getfn_uwbdriver_tx_unfreezepll(void);

uwbdriver_trx_config_cmn_tsu_timemask_fn        cb_getfn_uwbdriver_trx_config_cmn_tsu_timemask(void);
uwbdriver_rx_configure_cir_correction_length_fn cb_getfn_uwbdriver_rx_configure_cir_correction_length(void);

// --- IRQ CONTROL ---
uwbdriver_enable_event_irq_fn         cb_getfn_uwbdriver_enable_event_irq        (void);   
uwbdriver_disable_event_irq_fn        cb_getfn_uwbdriver_disable_event_irq       (void);
uwbdriver_irq_mask_configuration_fn   cb_getfn_uwbdriver_irq_mask_configuration  (void);
uwbdriver_irq_reset_registers_fn      cb_getfn_uwbdriver_irq_reset_registers     (void);

// --- MEMORY / BUFFERS ---
uwbdriver_get_uwb_tx_memory_size_fn        cb_getfn_uwbdriver_get_uwb_tx_memory_size       (void);
uwbdriver_get_uwb_rx_memory_size_fn        cb_getfn_uwbdriver_get_uwb_rx_memory_size       (void);
uwbdriver_get_uwb_tx_memory_start_addr_fn  cb_getfn_uwbdriver_get_uwb_tx_memory_start_addr (void);
uwbdriver_get_uwb_rx_memory_start_addr_fn  cb_getfn_uwbdriver_get_uwb_rx_memory_start_addr (void);

// --- TIMING / TSU / INDICES ---
uwbdriver_get_rx_cir_ctl_idx_fn    cb_getfn_uwbdriver_get_rx_cir_ctl_idx  (void);
uwbdriver_get_tx_tsu_timestamp_fn  cb_getfn_uwbdriver_get_tx_tsu_timestamp(void);
uwbdriver_get_rx_raw_timestamp_fn  cb_getfn_uwbdriver_get_rx_raw_timestamp(void);

// --- STATUS / SENSORS / KNOBS ---
uwbdriver_get_tx_rfpll_lock_fn               cb_getfn_uwbdriver_get_tx_rfpll_lock             (void);
uwbdriver_get_chip_temp_fn                   cb_getfn_uwbdriver_get_chip_temp                 (void);
uwbdriver_set_rx_threshold_fn                cb_getfn_uwbdriver_set_rx_threshold              (void);
uwbdriver_set_gain_rx_init_fn                cb_getfn_uwbdriver_set_gain_rx_init              (void);
uwbdriver_configure_tx_hw_timer_interval_fn  cb_getfn_uwbdriver_configure_tx_hw_timer_interval(void);
uwbdriver_configure_agc_peak_cnt_fn          cb_getfn_uwbdriver_configure_agc_peak_cnt        (void);
uwbdriver_get_rx_cir_quality_flag_fn         cb_getfn_uwbdriver_get_rx_cir_quality_flag       (void);

// --- RX/TX DATA CAPTURE & REGISTERS ---          
uwbdriver_get_rx_cir_register_fn             cb_getfn_uwbdriver_get_rx_cir_register            (void);
uwbdriver_get_rx_tsu_status_fn               cb_getfn_uwbdriver_get_rx_tsu_status              (void);
uwbdriver_get_rx_tsu_timestamp_fn            cb_getfn_uwbdriver_get_rx_tsu_timestamp           (void);
uwbdriver_get_rx_tsu_timestamp_lemm_fn       cb_getfn_uwbdriver_get_rx_tsu_timestamp_lemm      (void);
uwbdriver_get_rx_dcoc_fn                     cb_getfn_uwbdriver_get_rx_dcoc                    (void);
uwbdriver_get_rx_rssi_fn                     cb_getfn_uwbdriver_get_rx_rssi                    (void);
uwbdriver_get_uwb_rx_etc_status_register_fn  cb_getfn_uwbdriver_get_uwb_rx_etc_status_register (void);
uwbdriver_get_uwb_rx_status_register_fn      cb_getfn_uwbdriver_get_uwb_rx_status_register     (void);

// --- CFO / ABS TIMERS / EVENT TS ---
uwbdriver_configure_fixed_cfo_value_fn             cb_getfn_uwbdriver_configure_fixed_cfo_value           (void);
uwbdriver_abs_timer_on_fn                          cb_getfn_uwbdriver_abs_timer_on                        (void);
uwbdriver_abs_timer_off_fn                         cb_getfn_uwbdriver_abs_timer_off                       (void);
uwbdriver_abs_timer_clear_internal_occurence_fn    cb_getfn_uwbdriver_abs_timer_clear_internal_occurence  (void);
uwbdriver_abs_timer_configure_timeout_value_fn     cb_getfn_uwbdriver_abs_timer_configure_timeout_value   (void);
uwbdriver_abs_timer_configure_event_commander_fn   cb_getfn_uwbdriver_abs_timer_configure_event_commander (void);
uwbdriver_enable_event_timestamp_fn                cb_getfn_uwbdriver_enable_event_timestamp              (void);
uwbdriver_configure_event_timestamp_mask_fn        cb_getfn_uwbdriver_configure_event_timestamp_mask      (void);
uwbdriver_get_event_timestamp_in_ns_fn             cb_getfn_uwbdriver_get_event_timestamp_in_ns           (void);
uwbdriver_insert_apb_event_fn                      cb_getfn_uwbdriver_insert_apb_event                    (void);
uwbdriver_tsu_clear_fn                             cb_getfn_uwbdriver_tsu_clear                           (void);

// --- PACKET CONFIG ---
uwbdriver_configure_prf_mode_psdu_data_rate_fn      cb_getfn_uwbdriver_configure_prf_mode_psdu_data_rate (void);
uwbdriver_configure_preamble_code_index_fn          cb_getfn_uwbdriver_configure_preamble_code_index     (void);
uwbdriver_configure_preamble_duration_fn            cb_getfn_uwbdriver_configure_preamble_duration       (void);
uwbdriver_configure_sfd_id_fn                       cb_getfn_uwbdriver_configure_sfd_id                  (void);
uwbdriver_configure_sts_fn                          cb_getfn_uwbdriver_configure_sts                     (void);
uwbdriver_configure_tx_phr_psdu_fn                  cb_getfn_uwbdriver_configure_tx_phr_psdu             (void);
uwbdriver_get_rx_packet_phr_fn                      cb_getfn_uwbdriver_get_rx_packet_phr                 (void);
uwbdriver_get_rx_packet_size_fn                     cb_getfn_uwbdriver_get_rx_packet_size                (void);
uwbdriver_get_rx_phr_ranging_bit_fn                 cb_getfn_uwbdriver_get_rx_phr_ranging_bit            (void);
uwbdriver_configure_mac_fcs_type_fn                 cb_getfn_uwbdriver_configure_mac_fcs_type            (void);
uwbdriver_configure_tx_power_fn                     cb_getfn_uwbdriver_configure_tx_power                (void);
uwbdriver_configure_tx_timestamp_capture_fn         cb_getfn_uwbdriver_configure_tx_timestamp_capture    (void);
uwbdriver_configure_rx_timestamp_capture_fn         cb_getfn_uwbdriver_configure_rx_timestamp_capture    (void);
uwbdriver_get_tx_raw_timestamp_fn                   cb_getfn_uwbdriver_get_tx_raw_timestamp              (void);

// --- RADAR BLOCK ---
uwbdriver_radar_on_fn                   cb_getfn_uwbdriver_radar_on                 (void);
uwbdriver_radar_config_fn               cb_getfn_uwbdriver_radar_config             (void);
uwbdriver_radar_configure_preamble_code_index_fn cb_getfn_uwbdriver_radar_configure_preamble_code_index (void);
uwbdriver_radar_start_fn                cb_getfn_uwbdriver_radar_start              (void);
uwbdriver_radar_get_timestamp_diff_fn   cb_getfn_uwbdriver_radar_get_timestamp_diff (void);
uwbdriver_radar_get_cir_fn              cb_getfn_uwbdriver_radar_get_cir            (void);
uwbdriver_radar_stop_fn                 cb_getfn_uwbdriver_radar_stop               (void);
uwbdriver_radar_off_fn                  cb_getfn_uwbdriver_radar_off                (void);
uwbdriver_radar_reset_fn                cb_getfn_uwbdriver_radar_reset              (void);
uwbdriver_fft_fn                        cb_getfn_uwbdriver_fft                      (void);

// --- ADC / MISC ---   
uwbdriver_adc_read_AIN_voltage_fn         cb_getfn_uwbdriver_adc_read_AIN_voltage       (void);
uwbdriver_adc_read_AIN_10bit_code_fn      cb_getfn_uwbdriver_adc_read_AIN_10bit_code    (void);
uwbdriver_get_rx_cir_iq_fn                cb_getfn_uwbdriver_get_rx_cir_iq              (void);
uwbdriver_chip_init_lowpower_enable_fn    cb_getfn_uwbdriver_chip_init_lowpower_enable  (void);

#endif // __CB_UWB_DRIVER_H
