/**
 * @file    AppUwbTRXMemoryPool.h
 * @brief   [UWB] Header file for UWB TRX memory pool.
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_TRX_MEMPOOL_H
#define __APP_UWB_TRX_MEMPOOL_H

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

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
extern uint8_t g_UWB_TXBANKMEMORY[4096] __attribute__((section("SPECIFIC_UWB_TXBANK_RAMMEMORY")));
extern uint8_t g_UWB_RXBANKMEMORY[4096] __attribute__((section("SPECIFIC_UWB_RXBANK_RAMMEMORY")));

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

#endif // __APP_UWB_TRX_MEMPOOL_H
