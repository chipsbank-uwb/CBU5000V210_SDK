/**
 * @file    AppUwbDSTWR.h
 * @brief   
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_DSTWR_H
#define __APP_UWB_DSTWR_H

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
typedef struct
{
  uint8_t logOpt;
  uint32_t numberOfCycles;
  int32_t rangingBias;
} app_dstwrconfig_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_dstwr_initiator(void);
void app_dstwr_responder(void);
void app_dstwr_suspend(void);
/**
 * @brief   Initialize the application timer module.
 * @details This function turns on Timer 0 disables their interrupts, 
 *          and enables CPU Timer 0 interrupts.
 */
void app_dstwr_timer_init(uint16_t timeoutMs);

/**
 * @brief   Off the application timer module.
 * @details This function turns off Timer 0 disables their interrupts, 
 *          and disable CPU Timer 0 interrupts.
 */
void app_dstwr_timer_off(void);

/**
 * @brief Registers the IRQ callbacks for DSTWR-related interrupts.
 *
 * This function registers the necessary interrupt callbacks for UWB transmission, reception, 
 * and timer interrupts related to the DSTWR protocol, ensuring that the system responds 
 * appropriately to each event.
 */
void app_uwb_dstwr_register_irqcallbacks(void);

/**
 * @brief Deregisters the IRQ callbacks for DSTWR-related interrupts.
 *
 * This function removes the previously registered interrupt callbacks, stopping the 
 * system from responding to the UWB transmission, reception, and timer interrupts 
 * associated with the DSTWR protocol.
 */
void app_uwb_dstwr_deregister_irqcallbacks(void);

/**
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered. It fetches
 * the timestamp related to the TX Done event and prints it via UART.
 */
void app_uwb_dstwr_tx_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 * 
 * This function is called when the UWB RX0 Done IRQ is triggered. 
 */
void app_uwb_dstwr_rx0_done_irq_callback(void);

/**
 * @brief   Callback function for Timer 0 IRQ.
 *
 * This function is called when the Timer 0 IRQ is triggered.
 */
void app_uwb_dstwr_timer0_irq_callback(void);

#endif // __APP_UWB_DSTWR_H
