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
void APP_UWB_PeriodicTX_Init(void);
void app_uwb_periodic_tx_transmitpacket();
void APP_UWB_PeriodicTX_Stop(void);

void APP_UWB_PeriodicTX_TX_DONE_IRQ_Callback(void);
void APP_UWB_PeriodicTX_SFD_MARK_IRQ_Callback(void);

#endif // __APP_UWB_PERIODIC_TX_H
