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
 * @brief Initializes and starts the PDoA initiator for UWB communication.
 * 
 * This function configures and initializes the Phase Difference of Arrival (PDoA) 
 * initiator role for UWB communication. It sets up the UWB packet configuration, 
 * initializes the PDoA configuration with a specified integration cycle, and 
 * assigns the device role as an initiator.
 * 
 * The function then starts the PDoA transmission using the configured parameters 
 * and introduces a delay of 500 ms after transmission.
 */
void app_pdoa_initiator(void);

#endif // __APP_UWB_PDOA_H
