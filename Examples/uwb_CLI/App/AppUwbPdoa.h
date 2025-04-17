/**
 * @file    AppUwbPdoa.h
 * @brief   [UWB] Phase Difference of Arrival(UWB) Feature Module
 * @details This module provides functions used for determining the position of an object or a tag.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_PDOA_H
#define __APP_UWB_PDOA_H

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
void app_pdoa_initiator(void);
void app_pdoa_responder(void);
void app_pdoa_suspend(void);

void app_pdoa_timer_init(uint16_t timeoutMs);
void app_pdoa_timer_off(void);

/**
 * @brief   Initialize the application timer module.
 * @details This function turns on Timer 0 disables their interrupts,
 *          and enables CPU Timer 0 interrupts.
 */
void app_uwb_pdoa_timer_init(void);

/**
 * @brief   Off the application timer module.
 * @details This function turns off Timer 0 disables their interrupts,
 *          and disable CPU Timer 0 interrupts.
 */
void app_uwb_pdoa_timer_off(void);

void app_uwb_pdoa_register_irqcallbacks(void);

void app_uwb_pdoa_deregister_irqcallbacks(void);

/**
 * @brief Callback function for the UWB TX Done IRQ.
 *
 * This function is called when the UWB TX Done IRQ is triggered.
 */
void app_uwb_pdoa_tx_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 *
 * This function is called when the UWB RX0 Done IRQ is triggered.
 */
void app_uwb_pdoa_rx0_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX0 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX0 SFD Detection Done IRQ is triggered.
 */
void app_uwb_pdoa_rx0_sfd_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX1 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX1 SFD Detection Done IRQ is triggered.
 */
void app_uwb_pdoa_rx1_sfd_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX2 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX2 SFD Detection Done IRQ is triggered.
 */
void app_uwb_pdoa_rx2_sfd_done_irq_callback(void);

/**
 * @brief   Callback function for Timer 0 IRQ.
 *
 * This function is called when the Timer 0 IRQ is triggered.
 */
void app_uwb_pdoa_timer0_irq_callback (void);

#endif // __APP_UWB_PDOA_H
