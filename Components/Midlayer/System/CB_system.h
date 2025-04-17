/**
 * @file CB_system.h
 * @brief Declarations for functions related to the CB_system module.
 * @details This file contains the declarations of functions for initializing and controlling
 *          the Ultra-Wideband (UWB) communication system.
 * @author Chipsbank
 * @date 2024
 */

#ifndef __CB_SYSTEM_H
#define __CB_SYSTEM_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"
#include "CB_qspi.h"
#include "CB_system_types.h"
#include "CB_UwbDrivers.h"
#include "CB_Algorithm.h"
#include "CB_aoa.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Initializes the UWB RAM for transmission and reception.
 *
 * This function initializes the Ultra-Wideband (UWB) RAM by setting up the transmission (Tx)
 * and reception (Rx) RAM addresses and sizes. It prepares the memory by calling the
 * `CB_SYSTEM_UwbSysRamInit` function with the specified arguments. Once the addresses and sizes
 * are initialized, it clears the targeted memory regions for both transmission and reception
 * using `cb_system_uwb_tx_memclr` and `cb_system_uwb_rx_memclr` respectively.
 *
 * @param pTxRamAddr Pointer to the base address of the Tx RAM.
 * @param pRxRamAddr Pointer to the base address of the Rx RAM.
 * @param TxRamSize Size of the Tx RAM.
 * @param RxRamSize Size of the Rx RAM.
 */
void cb_system_uwb_ram_init(void* pTxRamAddr, void* pRxRamAddr,uint32_t TxRamSize,uint32_t RxRamSize);

/**
 * @brief Initialize the UWB communication transmitter.
 *
 * This function initializes the UWB communication transmitter by performing various configuration steps.
 * It sets up the required parameters and resources for UWB communication.
 */
void cb_system_uwb_init(void);

/**
 * @brief Turns off the UWB (Ultra-Wideband) system.
 */
void cb_system_uwb_off(void);

/**
 * @brief Initializes the UWB RX (Receiver) top module.
 */
void cb_system_uwb_rx_top_init(void);

/**
 * @brief Configures the UWB TX (Transmitter) settings.
 * 
 * @param config Pointer to the UWB packet configuration structure.
 * @param txPayload Pointer to the UWB TX payload structure.
 * @param stTxIrqEnable Pointer to the UWB TX interrupt enable structure.
 */
void cb_system_uwb_config_tx(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_txpayload_st* txPayload, cb_uwbsystem_tx_irqenable_st* stTxIrqEnable);

/**
 * @brief Configures the UWB RX (Receiver) settings.
 * 
 * @param config Pointer to the UWB packet configuration structure.
 * @param stRxIrqEnable Pointer to the UWB RX interrupt enable structure.
 */
void cb_system_uwb_config_rx(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable);
 
/**
 * @brief Configures the UWB RX operation mode.
 * 
 * @param mode The desired UWB RX operation mode.
 */
void cb_system_uwb_configure_rx_operation_mode(cb_uwbsystem_rxoperationmode_en mode);

/**
 * @brief Initialize the UWB communication for TX and RX
 *
 */
void cb_system_uwb_trx_init            (void);

/**
 * @brief Initialize the UWB communication for TX
 *
 */
void cb_system_uwb_tx_init             (void);

/**
 * @brief Initialize the UWB communication for RX
 *
 */
void cb_system_uwb_rx_init             (cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Start the UWB communication for TX
 *
 */
void cb_system_uwb_tx_start             (void);

/**
 * @brief Start the UWB communication for TX (For deferred TX)
 *
 */
void cb_system_uwb_tx_start_prepare(void);

/**
 * @brief Start the UWB communication for RX (For deferred RX)
 *
 */
void cb_system_uwb_rx_start_prepare(void);

/**
 * @brief Restart the UWB communication for TX
 *
 */
void cb_system_uwb_stage_tx_start           (void);

/**
 * @brief Start the UWB communication for RX (For deferred RX)
 *
 */
void cb_system_uwb_stage_rx_start (void);

/**
 * @brief Stop the UWB communication for TX
 *
 */
void cb_system_uwb_tx_stop             (void);

/**
 * @brief Starts the UWB (Ultra-Wideband) receiver on the specified port.
 * 
 * @param enRxPort The UWB receiver port to start.
 */
void cb_system_uwb_rx_start(cb_uwbsystem_rxport_en enRxPort);
 
/**
 * @brief Stops the UWB (Ultra-Wideband) receiver on the specified port.
 * 
 * @param enRxPort The UWB receiver port to stop.
 */
void cb_system_uwb_rx_stop (cb_uwbsystem_rxport_en enRxPort);
 
/**
 * @brief Turns off the UWB (Ultra-Wideband) receiver on the specified port.
 * 
 * @param enRxPort The UWB receiver port to turn off.
 */
void cb_system_uwb_rx_off (cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Starts the UWB PDOA (Phase Difference of Arrival) RX process for all channels.
 *
 * This function initializes and starts the reception process for UWB PDOA
 * on all channels using the specified gain index. It is typically used in
 * scenarios where precise location or angle-of-arrival measurements are required.
 *
 * @param[in] pdoaGainIdx Pointer to a variable containing the gain index to be used
 *                        for the PDOA reception process. The gain index determines
 *                        the amplification level for the received signal.
 *
 * @note Ensure that the provided pointer is valid and points to a properly
 *       initialized gain index value before calling this function.
 */
void cb_system_uwb_pdoa_rx_all_start(uint32_t* pdoaGainIdx);

/**
 * @brief Configures the UWB transmission interrupts.
 *
 * This function resets the necessary registers before configuring UWB transmission interrupts.
 * It enables or disables interrupts based on the provided parameters.
 *
 * @param irqEnable Pointer to structure containing the UWB interrupt enable flags.
 *                  - txDone: If true, enables the transmission done interrupt.
 *                  - sfdDone: If true, enables the SFD mark interrupt.
 */
void cb_system_uwb_configure_tx_irq(cb_uwbsystem_tx_irqenable_st* irqEnable);

/**
 * @brief Configures the UWB reception interrupts.
 *
 * This function configures UWB reception interrupts based on the provided parameters.
 * It sets up the necessary masks and enables the corresponding interrupts.
 *
 * @param stRxIrqEnable Pointer to structure containing the UWB reception interrupt enable flags.
 *                    - rx0Done: If true, enables the RX0 done interrupt.
 *                    - rx0PdDone: If true, enables the RX0 preamble detected interrupt.
 *                    - rx0SfdDetDone: If true, enables the RX0 SFD detected interrupt.
 *                    - rx1Done: If true, enables the RX1 done interrupt.
 *                    - rx1PdDone: If true, enables the RX1 preamble detected interrupt.
 *                    - rx1SfdDetDone: If true, enables the RX1 SFD detected interrupt.
 *                    - rx2Done: If true, enables the RX2 done interrupt.
 *                    - rx2PdDone: If true, enables the RX2 preamble detected interrupt.
 *                    - rx2SfdDetDone: If true, enables the RX2 SFD detected interrupt.
 *                    - rxStsCirEnd: If true, enables the RX status CIR end interrupt.
 *                    - rxPhrDetected: If true, enables the RX PHR detected interrupt.
 *                    - rxDone: If true, enables the RX done interrupt.
 */
void cb_system_uwb_configure_rx_irq(cb_uwbsystem_rx_irqenable_st* stRxIrqEnable);

/**
* @brief Get the Rx Done timstamp info from the cb_uwbsystem_rx_tsu_st struct
* @return rx_done timestamp
*/
uint32_t cb_system_uwb_get_rx_done_timestamp(void);

/**
 * @brief Reinitializes the UWB periodic transmission system.
 *
 * This function reconfigure the digital side for UWB TX.
 */
void cb_system_uwb_tx_restart(void);

/**
 * @brief Delay in milliseconds(ms)
 */
void cb_system_delay_in_ms(uint32_t milliseconds);

/**
 * @brief Delay in microseconds(us)
 */
void cb_system_delay_in_us(uint32_t microseconds);

/**
 * @brief Prepare Payload/PSDU for UWB Tx Phy
 */
void cb_system_uwb_tx_prepare_payload(uint8_t* pTxpayloadAddress,uint16_t SizeInByte);

/**
 * @brief Read UWB Rx Payload/PSDU Content
 */
void cb_system_uwb_rx_get_payload(uint8_t* pRxpayloadAddress,uint16_t* SizeInByte);

/**
 * @brief Clear UWB Rx PSDU Memory
 */
void cb_system_uwb_rx_memclr(void);

/**
 * @brief Clear UWB Tx PSDU Memory
 */
void cb_system_uwb_tx_memclr(void);

/**
 * @brief Sets the frequency offset calibration code.
 *
 * This function updates the global variable `g_freqOffsetCalCode`
 * with the provided calibration code. This code is used to adjust
 * the frequency offset for accurate signal processing.
 *
 * @param enCalcode The frequency offset calibration code to be set.
 *                  This value should be within the valid range for
 *                  frequency calibration.
 */
void cb_system_uwb_set_freq_offset_calcode(uint8_t enCalcode);

/**
 * @brief Stops the transmitter and receiver operation.
 *
 * This function halts the operation of the transmitter and receiver by
 * calling the external system function to stop the TRX.
 */
void cb_system_uwb_trx_stop(void);

/**
 * @brief Retrieves the Packet Header (PHR) status.
 *
 * This function reads the packet status from the RX0 registers and extracts
 * the PHR status information including the PHR security and dedicated
 * indicators as well as the RX0 packet status.
 *
 * @return cb_uwbsystem_rx_phrstatus_st A structure containing the PHR status flags:
 *         - phrSec: Indicates if the PHR is secure (1) or not (0).
 *         - phrDed: Indicates if the PHR is dedicated (1) or not (0).
 *         - rx0Ok:  Indicates if the RX0 packet is okay (1) or not (0).
 */
cb_uwbsystem_rx_phrstatus_st cb_system_uwb_get_rx_phr_status(void);

/**
 * @brief Gets the received packet's Packet Header (PHR).
 *
 * This function retrieves the PHR from the received packet by reading
 * the relevant bits from the RX packet status register.
 *
 * @return uint32_t The value of the received packet's PHR.
 */
uint32_t cb_system_uwb_get_rx_packet_phr(void);

/**
 * @brief Sets the UWB threshold value.
 *
 * This function sets the global UWB threshold value to the specified
 * parameter. This value can be used for various threshold checks
 * within the UWB system.
 *
 * @param threshold The threshold value to be set for the UWB system.
 */
void cb_system_uwb_set_rx_threshold(uint32_t threshold);

/**
 * @brief Initializes the receiver gain setting.
 *
 * This function sets the initial gain value for the receiver. The gain
 * value is limited to a maximum of 7 to ensure it stays within the
 * allowable range. If the provided gain exceeds this maximum, it is
 * capped to 7.
 *
 * @param gainRxInit The initial gain value for the receiver (0 to 7).
 */
void cb_system_uwb_set_gain_rx_init(uint32_t gainRxInit);

/**
 * @brief Retrieves the current temperature of the chip.
 *
 * This function reads the chip's temperature
 *
 * @return The current temperature of the chip in degrees Celsius.
 */
float cb_system_get_chip_temperature(void);

/**
 * @brief Perform a one-time calibration of the RC clock.
 *
 * This function calibrates the drift time of the RC clock
 * to improve timing accuracy. The calibration process adjusts
 * the RC clock to compensate for environmental or operational
 * factors that might affect its frequency stability.
 * @return CB_PASS or CB_FAIL. Fail if calibration is in-progress.
 */
CB_STATUS cb_system_rc_calibration(void);

/**
 * @brief Start periodic RC clock calibration with a specified interval.
 *
 * This function enables periodic calibration of the RC clock
 * The periodic calibration helps to continuously compensate for
 * RC clock drift, ensuring accurate timing over extended periods.
 * This is especially useful in systems where timing precision is
 * critical, and the RC clock drift may vary due to environmental
 * or operational changes.
 *
 * @param PeriodicIntervalInMs The interval time, in milliseconds
 * @return CB_PASS or CB_FAIL. Fail if calibration is in-progress.
 */
CB_STATUS cb_system_start_periodic_rc_calibration(uint32_t PeriodicIntervalInMs);

/**
 * @brief Stop RC clock calibration.
 *
 * This function disables the calibration process of the RC
 * clock. It can be used when periodic calibration is no longer
 * required if the timing precision is not critical.
 */
void cb_system_stop_rc_calibration(void);


/**
 * @brief Get the transmission power code from the uwb system configuration.
 * 
 * This function returns the current transmission power code as stored in
 * the system configuration.
 * 
 * @return uint8_t The uwb transmission power code.
 */
uint8_t cb_system_uwb_get_tx_power_code(void);

/**
 * @brief Sets the UWB transmission power code.
 * 
 * @param TxPowerCode The transmission power code to be set.
 */
void cb_system_uwb_set_tx_power_code(uint8_t TxPowerCode);

/**
 * @brief Retrieves the current UWB receiver operation mode.
 * 
 * @return The current UWB receiver operation mode as an cb_uwbsystem_rxoperationmode_en enum.
 */
cb_uwbsystem_rxoperationmode_en cb_system_uwb_get_rx_opmode(void);

/**
 * @brief Sets the UWB receiver operation mode.
 * 
 * @param RxOperationMode The receiver operation mode to be set, as an cb_uwbsystem_rxoperationmode_en enum.
 */
void cb_system_uwb_set_rx_opmode(cb_uwbsystem_rxoperationmode_en RxOperationMode);

/**
 * @brief Retrieves the current UWB antenna ID.
 * 
 * @return The current UWB antenna ID as an cb_uwbsystem_rxantenna_en enum.
 */
cb_uwbsystem_rxantenna_en cb_system_uwb_get_antenna_id(void);

/**
 * @brief Sets the UWB antenna ID.
 * 
 * @param antennaIdx The antenna ID to be set, as an cb_uwbsystem_rxantenna_en enum.
 */
void cb_system_uwb_set_antenna_id(cb_uwbsystem_rxantenna_en antennaIdx);

/**
 * @brief Retrieves the current UWB system configuration.
 * 
 * @return The current UWB system configuration as an cb_uwbsystem_systemconfig_st structure.
 */
cb_uwbsystem_systemconfig_st cb_system_uwb_get_system_config(void);

/**
 * @brief Sets the UWB system configuration.
 * 
 * @param newConfig Pointer to the new UWB system configuration to be set.
 */
void cb_system_uwb_set_system_config(cb_uwbsystem_systemconfig_st* newConfig);

// ------------------------------------------
// SYSTEM_CORE Related functions
// ------------------------------------------

/**
 * @brief Initializes the chip system, including reading the chip ID, 
 *        initializing PMU, setting RF parameters, and configuring voltage 
 *        and temperature sensing.
 * 
 * This function performs several key chip initialization tasks:
 *  - Reads the chip ID and checks the TO (Test Operations) version.
 *  - Initializes the PMU (Power Management Unit) based on the TO version.
 *  - Sets the RF (Radio Frequency) parameters.
 *  - Configures voltage and temperature sensing, including temperature compensation.
 */
void cb_system_chip_init(void);

/**
 * @brief Get the received packet's PHR (Physical Layer Header).
 *
 * This function retrieves the Physical Layer Header (PHR) from the received packet, which
 * contains information about the packet's size and type.
 *
 * @return The received packet's PHR value.
 */
uint16_t cb_system_uwb_get_rx_packet_size(cb_uwbsystem_packetconfig_st* config);

/**
 * @brief Get the Rx PHR Ranging bit infomation.
 *
 * This function retrive PHR Ranging bit infomation of the received packet
 *
 * @param  config Pointer to UWB configuration parameter structure 
 * @return Rx PHR Ranging bit.
 */
uint8_t cb_system_uwb_get_rx_phr_ranging_bit(cb_uwbsystem_packetconfig_st* config);

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
void cb_system_uwb_get_tx_raw_timestamp(cb_uwbsystem_tx_timestamp_st* txTimestamp);

/**
 * @brief Retrieves the transmitter TSU (Timestamp Unit) timestamp.
 *
 * This function reads the transmitter timestamp from the hardware register, processes it to remove the lower
 * 2 bits, and then converts it into integer and fractional parts. The calculated timestamp is stored in the 
 * provided `cb_uwbsystem_tx_tsutimestamp_st` structure.
 *
 * @param[out] p_txTsuTimestamp  Pointer to a `cb_uwbsystem_tx_tsutimestamp_st` structure where the retrieved timestamp will 
 *                                be stored. The structure will be updated with the integer and fractional 
 *                                components of the timestamp.
 */
void cb_system_uwb_get_tx_tsu_timestamp(cb_uwbsystem_tx_tsutimestamp_st * outTxTsu);

/**
 * @brief Fetches RxTSU data from the hardware registers.
 *
 * This function reads data from specific memory addresses corresponding to various
 * RxTSU events and stores them into the provided data structure.
 *
 * @param rxTsu Pointer to the structure where RxTSU data will be stored.
 */
void cb_system_uwb_get_rx_raw_timestamp(cb_uwbsystem_rx_tsu_st* rxTsu);

/**
 * @brief Retrieves the RX TSU timestamp.
 *
 * This function retrieves the timestamp of the RX SFD Mark, which consists of an integer part 
 * and a fractional part. The integer part represents the time in counts of 1/124.8MHz (~8ns), 
 * while the fractional part represents the time in counts of 1/124.8MHz/512 (~15.6ps). 
 * The complete RX TSU timestamp is stored in the provided `cb_uwbsystem_rx_tsutimestamp_st` structure, 
 * which includes:
 * - `rxTsuInt`: The integer part of the RX TSU timestamp, indicating the time in 
 *   counts of 1/124.8MHz.
 * - `rxTsuFrac`: The fractional part of the RX TSU timestamp, indicating the time 
 *   in counts of 1/124.8MHz/512.
 * - `rxTsu`: The complete RX TSU timestamp as a double, combining both the integer 
 *   and fractional parts for higher precision.
 *
 * @param rxTsuTimestamp Pointer to the `cb_uwbsystem_rx_tsutimestamp_st` structure where the retrieved 
 *                      RX TSU timestamp will be stored.
 * @param enRxPort        The RX port identifier used for fetching the RX TSU data.
 */
void cb_system_uwb_get_rx_tsu_timestamp(cb_uwbsystem_rx_tsutimestamp_st* rxTsuTimestamp, cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Stores the RX TSU status and timestamp data.
 *
 * This function retrieves the RX TSU status and timestamp data from the hardware registers
 * and stores them in the provided structures. It also handles the RX port selection for
 * the data retrieval.
 *
 * @param p_rxTsuStatus Pointer to the structure where the RX TSU status will be stored.
 * @param p_rxTimeStampData Pointer to the structure where the RX timestamp data will be stored.
 * @param enRxPort The RX port from which to retrieve the data (e.g., EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2).
 */
void cb_system_uwb_store_rx_tsu_status(cb_uwbsystem_rx_tsustatus_st* p_rxTsuStatus, cb_uwbsystem_rx_tsu_st* p_rxTimeStampData, cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Copies CIR register data into the provided array of `cb_uwbsystem_rx_cir_iqdata_st`.
 *
 * @param destArray Pointer to the destination array of `cb_uwbsystem_rx_cir_iqdata_st`.
 * @param enRxPort RX port number (used for address calculation).
 * @param startingPosition Offset within the CIR register memory.
 * @param numSamples Number of `cb_uwbsystem_rx_cir_iqdata_st` samples to copy.
 */
void cb_system_uwb_store_rx_cir_register(cb_uwbsystem_rx_cir_iqdata_st* destArray, cb_uwbsystem_rxport_en enRxPort, uint32_t startingPosition, uint32_t numSamples);

/**
 * @brief Processes the CIR (Channel Impulse Response) data for UWB PDOA (Phase Difference of Arrival).
 *
 * This function processes the CIR data for UWB PDOA calibration and returns the results.
 * It takes the calibration type, package number, number of RX channels used, and CIR data
 * as input parameters. The function performs the necessary calculations and returns the
 * processed results in a `cb_uwbalg_poa_outputperpacket_st` structure.
 *
 * @param calType The calibration type to be used for processing.
 * @param packageNum The package number for which the processing is done.
 * @param numRxUsed The number of RX channels used in the processing.
 * @param cirRegisterData Pointer to the CIR register data to be processed.
 * @param cirDataSize The size of the CIR data in bytes.
 *
 * @return A structure containing the processed results of type `cb_uwbalg_poa_outputperpacket_st`.
 */
cb_uwbalg_poa_outputperpacket_st cb_system_uwb_pdoa_cir_processing(enUwbPdoaCalType calType, uint8_t packageNum, const uint8_t numRxUsed, const cb_uwbsystem_rx_cir_iqdata_st *cirRegisterData, uint16_t cirDataSize);

/**
 * @brief Retrieve the CIR quality flag.
 *
 * This function retrieves the quality flag for the Channel Impulse Response (CIR) by
 * first storing the CIR register data and then performing a quality check on the stored
 * data. It calls the `cb_system_uwb_store_rx_cir_register` function to obtain the necessary 
 * register values and subsequently calls `CB_SYSTEM_cir_quality_check` to evaluate 
 * the quality of the CIR.
 *
 * @return The CIR quality flag as a uint8_t, indicating the quality status of the CIR.
 */
uint8_t cb_system_uwb_get_rx_cir_quality_flag(void);

/**
 * @brief Get the Digital Compensation Offset Correction (DCOC) for a specified receiver port.
 *
 * This function retrieves the Digital Compensation Offset Correction (DCOC) values for 
 * a specified UWB receiver port. It reads the corresponding hardware registers to obtain 
 * the I and Q components of the DCOC, which are then converted from unsigned to signed 
 * integers using two's complement representation.
 *
 * @param enRxPort The UWB receiver port from which to get the DCOC (e.g., EN_UWB_RX_0, 
 *               EN_UWB_RX_1, EN_UWB_RX_2, or EN_UWB_RX_ALL).
 *
 * @return A struct of type `cb_uwbsystem_rx_dcoc_st` containing the DCOC Q and I values.
 */
cb_uwbsystem_rx_dcoc_st cb_system_uwb_get_rx_dcoc(cb_uwbsystem_rxport_en enRxPort);

/**
 * @brief Retrieve the RSSI (Received Signal Strength Indicator) values for specified RX ports.
 *
 * This function reads the RSSI values from the specified UWB RX ports
 * and compensates for gain based on predefined compensation values. 
 * The function returns a structure containing the RSSI results for 
 * each RX port specified in the `rssiRxPorts` parameter.
 *
 * @param rssiRxPorts A bitmask indicating which RX ports to retrieve 
 *                    RSSI values from. Valid values are 
 *                    EN_UWB_RX_0, EN_UWB_RX_1, and EN_UWB_RX_2.
 *
 * @return A structure of type `cb_uwbsystem_rxall_signalinfo_st` containing the RSSI 
 *         values, gain indices, CFO estimates, and DCOC results for 
 *         the specified RX ports.
 *
 */
cb_uwbsystem_rxall_signalinfo_st cb_system_uwb_get_rx_rssi(uint8_t rssiRxPorts);

/**
 * @brief Get the etc status logging
 *
 * This function retrieves the status register value of the etc register status.\
 *
 * @param etcStatus Pointer to output cb_uwbsystem_rx_etc_statusregister_st.
 */
void cb_system_uwb_get_rx_etc_status_register(cb_uwbsystem_rx_etc_statusregister_st* const etcStatus);

/**
 * @brief Gets the TX RF PLL lock status
 *
 * @return The lock status of the TX RF PLL
 */
uint32_t cb_system_uwb_get_tx_rf_pll_lock(void);

/**
 * @brief Get the status register value of the UWB communication module.
 *
 * This function retrieves the status register value of the UWB communication module.
 * It reads the status register and returns the value.
 *
 * @return Status register value.
 */
cb_uwbsystem_rxstatus_un cb_system_uwb_get_rx_status(void);

/**
 * @brief Configures sync cfo est bypass crs value
 *
 * @param[in] val cfoEst value
 * @param[in] en enable or disable (1/0)
*/
void cb_system_uwb_configure_rx_sync_cfo_est_bypass_crs(uint32_t val, uint8_t en);

/**
 * @brief Reads the Cir Ctl Idx value
 *
 * @return CirCtlidx value
*/
uint16_t cb_system_uwb_get_rx_cir_ctl_idx(void);

/**
 * @brief Turns off the UWB transceiver.
 */
void cb_system_uwb_trx_off(void);

/**
 * @brief Turns off the UWB receiver top.
 */
void cb_system_uwb_rx_top_off(void);

/**
 * @brief Turns off the UWB transmitter.
 */
void cb_system_uwb_tx_off(void);

/**
 * @brief Enables the UWB absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to enable.
 */
void cb_system_uwb_abs_timer_on(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Disables the UWB absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to disable.
 */
void cb_system_uwb_abs_timer_off(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Clears the internal occurrence of the UWB absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to clear.
 */
void cb_system_uwb_abs_timer_clear_internal_occurence(enUwbAbsoluteTimer enAbsoluteTimer);

/**
 * @brief Configures the timeout value for the UWB absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to configure.
 * @param baseTime The base time for the timer.
 * @param targetTimeoutTime The target timeout time.
 */
void cb_system_uwb_abs_timer_configure_timeout_value(enUwbAbsoluteTimer enAbsoluteTimer, uint32_t baseTime, uint32_t targetTimeoutTime);

/**
 * @brief Configures the event commander for the UWB absolute timer.
 * 
 * @param control The enable/disable control for the event.
 * @param enAbsoluteTimer The absolute timer to configure.
 * @param uwbEventControl The event control settings.
 */
void cb_system_uwb_abs_timer_configure_event_commander(enUwbEnable control, enUwbAbsoluteTimer enAbsoluteTimer, enUwbEventControl uwbEventControl);

/**
 * @brief Enables or disables the event timestamp functionality.
 * 
 * @param enable Enable or disable control.
 */
void cb_system_uwb_enable_event_timestamp(enUwbEnable enable);

/**
 * @brief Configures the event timestamp mask.
 * 
 * @param eventTimestampMask The mask for the event timestamp.
 * @param uwbEventIndex The index of the UWB event.
 */
void cb_system_uwb_configure_event_timestamp_mask(enUwbEventTimestampMask eventTimestampMask, enUwbEventIndex uwbEventIndex);

/**
 * @brief Retrieves the value of the event timestamp.
 * 
 * @param eventTimestampMask The mask for the event timestamp.
 * @return The value of the event timestamp.
 */
uint32_t cb_system_uwb_get_event_timestamp_value(enUwbEventTimestampMask eventTimestampMask);

/**
 * @brief Clears the TSU (Timestamp Unit).
 */
void cb_system_uwb_tsu_clear(void);

/**
 * @brief Estimates the PDOA (Phase Difference of Arrival) using the given angles.
 * 
 * @param poa_deg1 The first angle in degrees.
 * @param poa_deg2 The second angle in degrees.
 * @return The estimated PDOA value.
 */
double cb_system_uwb_alg_pdoa_estimation(double poa_deg1, double poa_deg2);

/**
 * @brief Calculates the propagation time using ranging results.
 * 
 * @param result1 The first ranging result.
 * @param result2 The second ranging result.
 * @return The calculated propagation time.
 */
double cb_system_uwb_alg_prop_calculation(cb_uwbsystem_rangingtroundtreply_st* result1, cb_uwbsystem_rangingtroundtreply_st* result2);

/**
 * @brief Compensates for the 3D antenna bias in AOA (Angle of Arrival) calculations.
 * 
 * @param pdoa_raw The raw PDOA 3D data.
 * @param azi_pd_bias The azimuth phase difference bias.
 * @param ele_pd_bias The elevation phase difference bias.
 * @return The compensated AOA data.
 */
stAOA_CompensatedData cb_system_uwb_aoa_antenna_3d_bias_comp(cb_uwbsystem_pdoa_3ddata_st pdoa_raw, float azi_pd_bias, float ele_pd_bias);

/**
 * @brief Calculates the 3D AOA (Angle of Arrival) using compensated data.
 * 
 * @param AOA_PD The compensated AOA data.
 * @param azi_result Pointer to store the azimuth result.
 * @param ele_result Pointer to store the elevation result.
 */
void cb_system_uwb_aoa_antenna_3d(stAOA_CompensatedData* AOA_PD, float* azi_result, float* ele_result);

#endif /*__CB_SYSTEM_H*/
