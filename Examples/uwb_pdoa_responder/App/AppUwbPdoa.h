/**
 * @file    AppUwbPdoa.h
 * @brief   [UWB] Phase Difference of Arrival(UWB) Feature Module
 * @details This module provides functions used for determining the position of an object or a tag.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_PDOA_H
#define __APP_UWB_PDOA_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>

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
/**
 * @brief Initializes and starts the PDoA responder for UWB communication.
 * 
 * This function configures and initializes the Phase Difference of Arrival (PDoA) 
 * responder role for UWB communication. It sets up the UWB packet configuration, 
 * initializes the PDoA configuration with a specified integration cycle, and 
 * assigns the device role as a responder.
 * 
 * The function then starts the PDoA measurement process by calling `APP_PDOA_GetAll()`, 
 * which retrieves PDoA data based on the configured parameters.
 */
void app_pdoa_responder(void);

void app_pdoa_timer_init(uint16_t timeoutMs);
void app_pdoa_timer_off(void);

#endif // __APP_UWB_PDOA_H
