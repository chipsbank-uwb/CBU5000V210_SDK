/**
 * @file     app_sleep.h
 * @brief    Header file for managing CPU sleep and deep sleep modes.
 * @details  This header file provides declarations and prototypes for functions related to managing CPU sleep and deep sleep modes,
 * including functions for entering sleep and deep sleep modes for specified durations.
 * @author   Chipsbank
 * @date     2024
 */
 
#ifndef __APP_CPU_SLEEP_H
#define __APP_CPU_SLEEP_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
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

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Enters sleep mode for a specified duration.
 * @param slpduration_in_ms The duration to sleep in milliseconds.
 * @note This function delegates the sleep control to cb_sleep_control.
 */
void app_enter_sleep(uint32_t slpduration_in_ms);

/**
 * @brief Enters deep sleep mode for a specified duration.
 * @param slpduration_in_ms The duration to enter deep sleep mode in milliseconds.
 * @note This function delegates the deep sleep control to cb_deep_sleep_control.
 */
void app_enter_deep_sleep(uint32_t slpduration_in_ms);

/**
 * @brief Main function for the peripheral sleep demo.
 *
 * This function initializes the UART, prints relevant messages, 
 * enters sleep mode for a specified duration, and then exits sleep mode.
 */
void app_peripheral_sleep_demo_init(void);

/**
 * @brief This function demonstrates the peripheral sleep functionality by running a loop 
 *        and putting the system into sleep mode after a specified number of iterations.
 * 
 * @param sleep_time The duration in milliseconds for which the system will sleep.
 * 
 * The function increments a static counter `cnt` on each call. After the counter reaches 30, 
 * it enters sleep mode for the specified duration (`sleep_time`). Before entering sleep mode, 
 * it checks the UART interrupt flags to ensure no active interrupts are pending.
 * 
 * @note The function uses `cb_system_delay_in_ms` for delay and `app_enter_sleep` to enter sleep mode.
 */
void app_peripheral_sleep_demo_loop(uint16_t sleep_time);

#endif //__APP_CPU_SLEEP_H

