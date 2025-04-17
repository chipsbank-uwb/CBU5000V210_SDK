/**
 * @file    CB_Sleep_DeepSleep.h
 * @brief   Header file for sleep and deep sleep functions
 * @details This file contains the declarations of functions related to sleep and deep sleep modes.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __CB_SLEEP_DEEPSLEEP_H
#define __CB_SLEEP_DEEPSLEEP_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_scr.h"

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
 * @brief Enters sleep mode for a specified duration.
 * @details       This function enters sleep mode for the specified duration in milliseconds.
 * @param[in] slpduration_in_ms The duration of sleep mode in milliseconds.
 * @return CB_STATUS The status of the operation (CB_PASS or CB_FAIL).
 */
CB_STATUS cb_sleep_control(uint32_t slpduration_in_ms);

/**
 * @brief Enters deep sleep mode for a specified duration.
 * @details This function enters deep sleep mode for the specified duration in milliseconds.
 * @param[in] slpduration_in_ms The duration of deep sleep mode in milliseconds.
 * @return CB_STATUS The status of the operation (CB_PASS or CB_FAIL).
 */
CB_STATUS cb_deep_sleep_control(uint32_t slpduration_in_ms);

#endif //__CB_SLEEP_DEEPSLEEP_H

