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
#include "CB_system_types.h"

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
void app_rx_per(void);
void app_uwbtrx_rx_per(cb_uwbsystem_rxport_en enRxPort, uint32_t rxTimeIntervalInMs);

#endif // __APP_UWB_COMM_PER_H
