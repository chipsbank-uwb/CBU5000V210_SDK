/**
 * @file    AppUwbCommTx.h
 * @brief   [UWB] Header file for the Communication Transceiver (UWB) Module
 * @details This header file defines functions related to initializing, transmitting, and receiving data
 *          using the Ultra-Wideband (UWB) communication transceiver.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_COMM_TX_H
#define __APP_UWB_COMM_TX_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"
#include "CB_uwbframework.h"

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
 * @brief Transmits a UWB packet using the appropriate mode.
 * 
 * This function configures and transmits a UWB packet based on the current packet mode 
 * (either BPRF or HPRF). It initializes the packet configuration structure, sets the 
 * payload data accordingly, and transmits the packet via the UWB transceiver. 
 */
void app_commtx_qmode(void);
void app_commtx_nmode(void);

#endif // __APP_UWB_COMM_TX_H
