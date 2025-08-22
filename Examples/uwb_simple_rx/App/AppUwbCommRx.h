/**
 * @file    AppUwbCommRx.h
 * @brief   [UWB] Header file for the Communication Transceiver (UWB) Module
 * @details This header file defines functions related to initializing, transmitting, and receiving data
 *          using the Ultra-Wideband (UWB) communication transceiver.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_COMM_RX_H
#define __APP_UWB_COMM_RX_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define APP_COMMTRX_Qmode             APP_TRUE

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
/**
 * @brief Receives a UWB packet using the appropriate mode.
 * 
 * This function initializes the UWB transceiver for reception and listens for an 
 * incoming UWB packet. It configures the packet settings, sets the receive port, 
 * and waits until the reception is complete.
 */
void app_commrx_qmode(void);

/**
 * @brief Receives a UWB packet in normal mode.
 * 
 * This function configures and initializes the UWB transceiver for packet reception
 * using normal mode settings. It sets up the appropriate packet configuration,
 * initializes the receiver, and waits for an incoming packet. The function handles
 * the reception process, including setting receive parameters, enabling the receiver,
 * and processing the received data.
 * 
 * @note This function differs from app_commrx_qmode() by using normal mode parameters
 *       instead of quick mode. Normal mode typically offers different trade-offs
 *       between power consumption, range, and data rate compared to quick mode.
 * 
 * @see app_commrx_qmode
 */
void app_commrx_nmode(void);

/**
 * @brief Prints payload information for the UWB RX events.
 * 
 * This function fetches timestamps for various UWB RX events and prints them
 * via UART. It also prints the received payload size and the payload data.
 */
void app_commrx_rx_payload_and_timestamp_printout(void);

#endif // __APP_UWB_COMM_RX_H
