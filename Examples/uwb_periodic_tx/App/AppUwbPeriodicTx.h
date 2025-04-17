/**
 * @file    AppUwbPeriodicTx.h
 * @brief   Implementation of UWB-COMMTRX: Periodic TX
 * @details This module provides functions to initialize, transmit, and receive data using the UWB communication transceiver.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_PERIODIC_TX_H
#define __APP_UWB_PERIODIC_TX_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"


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
  uint32_t        timeInterval;
} stUwbPeriodicTxPacketConfig;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
//stUwbPeriodicTxPacketConfig uwbPeriodicPacketConfig;
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_periodic_tx(void);

void app_uwb_periodic_tx_transmitpacket(void);

#endif // __APP_UWB_PERIODIC_TX_H
