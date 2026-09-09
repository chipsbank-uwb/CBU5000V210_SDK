/**
 * @file    AppUwbRngAoa.h
 * @brief   
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_RNGAOA_H
#define __APP_UWB_RNGAOA_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define INITIATOR  1
#define RESPONDER  0
#define APP_RNGAOA_ROLE  INITIATOR  // Change to RESPONDER for responder mode

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
void app_rngaoa_initiator(void);
void app_rngaoa_responder(void);

#endif // __APP_UWB_RNGAOA_H
