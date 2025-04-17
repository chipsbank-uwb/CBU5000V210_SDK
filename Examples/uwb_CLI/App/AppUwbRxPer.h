/**
 * @file    AppUwbRxPer.h
 * @brief   [UWB] Header file for UWB TRX RX PER measurement.
 * @details This file contains function prototypes and necessary includes for implementing
 *          UWB TRX RX PER (Packet Error Rate) measurement. It defines functions for initializing
 *          the measurement, handling IRQ callbacks, and configuring timer events.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_COMM_PER_H
#define __APP_UWB_COMM_PER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"
#include "CB_commtrx.h"

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
void app_uwbtrx_rx_per(cb_uwbsystem_rxport_en enRxPort, uint32_t rxTimeIntervalInMs);

void APP_UWB_RXPER_TIMER_0_IRQ_Callback(void);
void APP_UWB_RXPER_TX_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_TX_SFD_MARK_IRQ_Callback(void);
void APP_UWB_RXPER_RX0_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_RX0_PD_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_RX0_SFD_DET_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_RX1_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_RX1_PD_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_RX1_SFD_DET_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_RX2_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_RX2_PD_DONE_IRQ_Callback(void);
void APP_UWB_RXPER_RX2_SFD_DET_DONE_IRQ_Callback(void);

#endif // __APP_UWB_COMM_PER_H
