/**
 * @file    AppUwbRngAoa.h
 * @brief   
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_RNGAOA_H
#define __APP_UWB_RNGAOA_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
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
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

void app_rngaoa_initiator(void);
void app_rngaoa_responder(void);
void app_rngaoa_suspend(void);

/**
 * @brief   Initialize the application timer module.
 * @details This function turns on Timer 0 disables their interrupts,
 *          and enables CPU Timer 0 interrupts.
 */
void app_rngaoa_timer_init(uint16_t timeoutMs);

/**
 * @brief   Off the application timer module.
 * @details This function turns off Timer 0 disables their interrupts,
 *          and disable CPU Timer 0 interrupts.
 */
void app_rngaoa_timer_off(void);

/**
 * @brief Suspends the UWB RNGAOA operation.
 * 
 * This function performs the following actions to suspend the UWB RNGAOA operation:
 * - Sets the `s_applicationTimeout` flag to true, indicating a timeout state.
 * - Calls `CB_DSTWR_Suspend()` to suspend the DSTWR functionality.
 * - Calls `CB_PDOA_SuspendOperation()` to suspend the AOA operation.
 * - Sets the `stRNGAOA_LoopFlag` to false, halting the RNGAOA loop.
 * - Sets the `stSuspendFlag` to true to indicate that the operation is suspended.
 * - Prints a message indicating that the RNGAOA operation has been stopped.
 * 
 */
void APP_UWB_RNGAOA_Suspend(void);

/**
 * @brief Registers the interrupt callbacks for the UWB RNGAOA application.
 * 
 * This function registers the following interrupt callbacks:
 * - UWB TX Done interrupt callback
 * - UWB RX0 Power Done interrupt callback
 * - UWB RX0 Start Frame Detection Done interrupt callback
 * - UWB RX0 Done interrupt callback
 * - Timer 0 interrupt callback
 * 
 * The callbacks are registered with the appropriate IRQ entries to handle interrupts 
 * in the UWB RNGAOA application.
 */
void app_uwb_rngaoa_register_irqcallbacks(void);

/**
 * @brief Deregisters the interrupt callbacks for the UWB RNGAOA application.
 * 
 * This function deregisters the following interrupt callbacks:
 * - UWB TX Done interrupt callback
 * - UWB RX0 Power Done interrupt callback
 * - UWB RX0 Start Frame Detection Done interrupt callback
 * - UWB RX0 Done interrupt callback
 * - Timer 0 interrupt callback
 * 
 * The callbacks are removed from the respective IRQ entries, ensuring that the 
 * interrupts no longer trigger the registered callback functions.
 */
void app_uwb_rngaoa_deregister_irqcallbacks(void);

/**
 * @brief Callback function for the UWB TX Done IRQ.
 *
 * This function is called when the UWB TX Done IRQ is triggered. It fetches
 * the timestamp related to the TX Done event and prints it via UART.
 */
void app_uwb_rngaoa_tx_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 *
 * This function is called when the UWB RX0 Done IRQ is triggered.
 */
void app_uwb_rngaoa_rx0_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX0 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX0 SFD Detection Done IRQ is triggered.
 */
void app_uwb_rngaoa_rx0_sfd_det_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX1 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX1 SFD Detection Done IRQ is triggered.
 */
void app_uwb_rngaoa_rx1_sfd_det_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX2 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX2 SFD Detection Done IRQ is triggered.
 */
void app_uwb_rngaoa_rx2_sfd_det_done_irq_callback(void);

/**
 * @brief   Callback function for Timer 0 IRQ.
 *
 * This function is called when the Timer 0 IRQ is triggered.
 */
void app_uwb_rngaoa_timer0_irq_callback(void);



#endif // __APP_UWB_RNGAOA_H
