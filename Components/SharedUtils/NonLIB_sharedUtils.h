/**
 * @file    NonLIB_sharedUtils.h
 * @brief   Common utility functions for the Chipsbank UWB SDK
 * @details This file provides utility functions that are commonly used across
 *          the SDK, including timing operations (delays, ticks), and binary
 *          data manipulation functions like two's complement conversion.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __NONLIB_SHARED_UTILS_H
#define __NONLIB_SHARED_UTILS_H
#include <stdint.h>
#include "ARMCM33_DSP_FP.h"
#include "CB_Common.h"

/**
 * @brief Global system tick counter variable
 * 
 * This variable is incremented by the SysTick handler and used for timing operations.
 */
extern volatile uint32_t sysTickCounter;

/**
 * @brief Delays execution for a specified number of microseconds
 * 
 * This function implements a blocking delay for the specified duration using
 * CPU cycles for precise timing.
 * 
 * @param[in] microseconds The number of microseconds to delay
 */
void cb_hal_delay_in_us(uint32_t microseconds);

/**
 * @brief Delays execution for a specified number of milliseconds
 * 
 * This function implements a blocking delay for the specified duration using
 * the system tick timer.
 * 
 * @param[in] milliseconds The number of milliseconds to delay
 */
void cb_hal_delay_in_ms(uint32_t milliseconds);

/**
 * @brief Get the current system tick count
 * 
 * This function returns the current value of the system tick counter,
 * which can be used for timing operations.
 * 
 * @return The current tick count value
 */
uint32_t cb_hal_get_tick(void);

/**
 * @brief Check if a specified time period has elapsed
 * 
 * This function determines if the specified timeout period has elapsed since
 * the provided start tick value.
 * 
 * @param[in] start_tick The starting tick value to measure from
 * @param[in] timeout_ms The timeout period in milliseconds
 * 
 * @return CB_STATUS_OK if the time has elapsed, CB_STATUS_TIMEOUT otherwise
 */
CB_STATUS cb_hal_is_time_elapsed(uint32_t start_tick, uint32_t timeout_ms);

/**
 * @brief Convert a value to its two's complement representation.
 *
 * This function takes an unsigned integer value and the number of bits, and converts the 
 * value to its signed integer representation using two's complement arithmetic. If the 
 * value exceeds the positive range of the specified bit width, it will be adjusted to 
 * reflect its negative counterpart.
 *
 * @param[in] value The unsigned integer value to convert.
 * @param[in] bit The number of bits for the representation.
 *
 * @return The signed integer representation of the value.
 */
int32_t cb_utils_twos_complement(uint32_t value, int32_t bit);

#endif
