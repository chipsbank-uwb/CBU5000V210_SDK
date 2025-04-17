/**
 * @file    AppUwbPeriodicRx.h
 * @brief   Implementation of UWB-COMMTRX: Periodic RX
 * @details This module provides functions to initialize, and receive data periodically using the UWB.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_PERIODIC_RX_H
#define __APP_UWB_PERIODIC_RX_H

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
  enUwbPacketMode packetMode;
  enUwbStsMode    stsMode;
  cb_uwbsystem_rxport_en     enRxPort;
  uint8_t         logOpt;
  uint32_t        packetsToRcv;
} stUwbPeriodicRxPacketConfig;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_uwb_periodicrx_receive_packet(const stUwbPeriodicRxPacketConfig* const packetConfig);
void APP_UWB_PeriodicRX_Stop(void);
void APP_UWB_PeriodicRx_RX0_DONE_IRQ_Callback(void);
void APP_UWB_PeriodicRx_RX1_DONE_IRQ_Callback(void);
void APP_UWB_PeriodicRx_RX2_DONE_IRQ_Callback(void);

#endif // __APP_UWB_PERIODIC_RX_H
