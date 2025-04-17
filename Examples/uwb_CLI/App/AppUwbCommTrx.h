/**
 * @file    AppUwbCommTrx.h
 * @brief   [UWB] Header file for the Communication Transceiver (UWB) Module
 * @details This header file defines functions related to initializing, transmitting, and receiving data
 *          using the Ultra-Wideband (UWB) communication transceiver.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_COMM_TRX_H
#define __APP_UWB_COMM_TRX_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"
//#include "CB_commtrx.h"

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
void app_uwb_commtx_main(void);
void app_uwb_commrx_main(void);

/**
 * @brief Callback function for the UWB TX Done IRQ.
 * 
 * This function is called when the UWB TX Done IRQ is triggered. It fetches
 * the timestamp related to the TX Done event and prints it via UART.
 */
void app_uwb_commtrx_tx_done_irq_callback(void);

/**
 * @brief Callback function for the UWB TX SFD Mark IRQ.
 * 
 * This function is called when the UWB TX SFD Mark IRQ is triggered. It fetches
 * the timestamp related to the SFD Mark event and prints it via UART.
 */
void app_uwb_commtrx_tx_sfd_mark_irq_callback(void);

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 * 
 * This function is called when the UWB RX0 Done IRQ is triggered. 
 */
void app_uwb_commtrx_rx0_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX0 PD Done IRQ.
 * 
 * This function is called when the UWB RX0 PD Done IRQ is triggered. 
 */
void app_uwb_commtrx_rx0_pd_done_irq_callback(void);

/**
 * @brief Callback function for the UWB RX0 SFD Detection Done IRQ.
 * 
 * This function is called when the UWB RX0 SFD Detection Done IRQ is triggered.
 */
void app_uwb_commtrx_rx0_sfd_det_done_irq_callback(void);

/**
 * @brief Prints payload information for the UWB RX events.
 * 
 * This function fetches timestamps for various UWB RX events and prints them
 * via UART. It also prints the received payload size and the payload data.
 */
void app_commtrx_rx_payload_and_timestamp_printout(void);

#endif // __APP_UWB_COMM_TRX_H
