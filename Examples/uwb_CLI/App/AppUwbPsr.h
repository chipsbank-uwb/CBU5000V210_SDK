/**
 * @file    AppUwbPsr.h
 * @brief   
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_PSR_H
#define __APP_UWB_PSR_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_system.h"
#include "CB_commtrx.h"
#include <stdint.h>

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum
{
  EN_PSR_RX = 1,
  EN_PSR_TX = 2
}enUwbPsrTRX;

typedef enum
{
	EN_PSR_SINGLE_MODE = 1,
	EN_PSR_FULL_MODE = 2
}enUwbPsrMode;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct 
{
	enUwbPsrTRX trxMode;
	enUwbPsrMode psrMode;
	uint32_t scanDuration;
} stPreambleScanningParameters;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Initializes the UWB PSR (Preamble Scanning Receiver) with the provided scanning parameters.
 * 
 * This function initializes the global UWB preamble scanning configuration, including setting up the scanning parameters, packet configuration, and the preamble scanning state. It also prepares the receiver for operation by clearing any previous settings and ensuring that the system is ready for the scanning process. 
 * The function must be called before the main scanning sequence starts, ensuring that the necessary configuration is in place before entering the main program flow.
 * 
 * @param[in] params The preamble scanning parameters to initialize the system with. 
 * These parameters define the scanning mode, TRX mode, and other key configurations for the UWB PSR. The members of the `stPreambleScanningParameters` struct are:
 * 
 * - `trxMode`: The TRX mode used for the scanning. Can be either `EN_PSR_RX` (Receiver mode) or `EN_PSR_TX` (Transmitter mode).
 * - `psrMode`: The scanning mode, either `EN_PSR_SINGLE_MODE` for a single scan or `EN_PSR_CONTINUOUS_MODE` for continuous scanning.
 * - `scanDuration`: The duration (in ms) for which the scanning should be performed in a single cycle.
 * - `uwbPacketConfig`: A structure containing UWB packet configuration details, including extended arguments for the receiver configuration.
 * 
 * @note This function should be called before the main application logic to ensure that the scanning configuration is properly set up before any scanning operations are performed.
 * 
 * @see app_uwb_psr_sequence
 */
void app_uwb_psr_init(stPreambleScanningParameters params);

/**
  * @brief Main function that runs PSR functionalities based on initialized variables
  */
void app_uwb_psr_deal(void);

/**
 * @brief Executes the preamble scanning sequence for the PSR (Preamble Scanning Receiver).
 * 
 * This function performs the scanning sequence by configuring the UWB receiver, processing the received signals, and evaluating the results based on preamble code indices. It handles the scanning of multiple preamble codes, waits for SFD (Start of Frame Delimiters), and checks for valid PHR (Preamble Header) conditions. Depending on the scanning mode (single or continuous), it either returns a result for the detected preamble code index or continues scanning.
 * 
 * The function works in the following sequence:
 * - Initializes various system configurations and IRQ settings.
 * - Starts scanning by cycling through preamble code indices.
 * - Receives data and waits for frame detection.
 * - Verifies the validity of the frame header and retries if necessary.
 * - Stops scanning if the scan duration is reached or if the system is in suspend mode.
 * 
 * @return The detected preamble code index based on the scanning results.
 * 
 * @note The function handles two scanning modes:
 *       - **Single Mode**: Stops after scanning once and returns the detected preamble code index.
 *       - **Continuous Mode**: Continues scanning without returning until a valid result is obtained or the scan duration ends.
 * 
 */
cb_uwbsystem_preamblecodeidx_en app_uwb_psr_sequence(void);

/**
 * @brief Suspends the UWB Preamble Scanning Receiver (PSR) operation.
 * 
 * This function sets the suspend flag to true, effectively suspending the current UWB PSR operation. It is typically used to halt the scanning process. 
 * The function logs a message indicating that the PSR operation has been stopped.
 */
void APP_UWB_PSR_Suspend(void);

/**
 * @brief Registers interrupt callbacks for UWB PSR IRQ events.
 * 
 * This function registers interrupt service routine (ISR) callbacks for various UWB PSR (Preamble Scanning Receiver) IRQ events. The registered callbacks handle different stages of the UWB reception and transmission process, ensuring that appropriate actions are taken when these events occur.
 * 
 * @note This function should be called to enable handling of UWB PSR interrupts. Failure to register these callbacks may result in missing or incorrect processing of UWB events.
 */
void APP_UWB_PSR_RegisterIrqCallbacks(void);

/**
 * @brief Deregisters interrupt callbacks for UWB PSR IRQ events.
 * 
 * This function deregisters interrupt service routine (ISR) callbacks for UWB PSR IRQ events. It is used to remove the previously registered callbacks, effectively disabling further handling of these interrupts.
 * 
 * @note This function should be called to disable UWB PSR interrupt handling. After calling this function, the system will no longer respond to the UWB PSR IRQ events.
 */
void APP_UWB_PSR_DeregisterIrqCallbacks(void);

/**
 * @brief Displays the status table of the UWB preamble code index.
 * 
 * This function displays a table showing the status of each UWB preamble code index. It prints whether a particular preamble code index is "Used" or "Not Used", based on the scanning mode and status of each preamble code. The display is customized depending on the scanning mode: either single mode or a mode that handles multiple preamble codes.
 * 
 * @param[in] preambleCodeIdx The current preamble code index to be displayed.
 * 
 * @note This function is typically used for debugging and monitoring the status of the UWB scanning process. The table can help identify which preamble code indices have been used during the scanning.
 */
void app_uwb_psr_display_table(cb_uwbsystem_preamblecodeidx_en preambleCodeIdx);

/**
 * @brief Calculates the time passed in microseconds based on the CPU cycle count.
 * 
 * This function computes the elapsed time in microseconds by comparing the current CPU cycle count to a given start count. The calculation takes into account the possibility of the cycle counter overflow and uses the system clock frequency (SystemCoreClock) to convert the cycle difference into microseconds.
 * 
 * @param[in] cpuCycleStartCount The starting CPU cycle count from which to calculate the elapsed time.
 * 
 * @return The elapsed time in microseconds (us)
 * 
 * @note This function assumes that the CPU cycle counter (`DWT->CYCCNT`) is running and provides accurate cycle counts.
 */
uint32_t app_uwb_psr_cpucycle_get_time_us(uint32_t cpuCycleStartCount);

/**
 * @brief Callback function for the UWB TX Done IRQ.
 *
 * This function is called when the UWB TX Done IRQ is triggered. It fetches
 * the timestamp related to the TX Done event and prints it via UART.
 */
void APP_UWB_PSR_TX_DONE_IRQ_Callback(void);

/**
 * @brief Callback function for the UWB RX0 PD Done IRQ.
 *
 * This function is called when the UWB RX0 PD Done IRQ is triggered.
 */
void APP_UWB_PSR_RX0_PD_DONE_IRQ_CallBack(void);

/**
 * @brief Callback function for the UWB RX0 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX0 SFD Detection Done IRQ is triggered.
 */
void APP_UWB_PSR_RX0_SFD_DET_DONE_IRQ_Callback(void);

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 *
 * This function is called when the UWB RX0 Done IRQ is triggered.
 */
void APP_UWB_PSR_RX0_DONE_IRQ_Callback(void);


#endif // __APP_UWB_RC_H
