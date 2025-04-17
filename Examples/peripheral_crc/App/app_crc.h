/**
 * @file    app_crc.h
 * @brief   CRC Application Module Header
 * @details This module provides functions for CRC functions.
 * @author  Chipsbank
 * @date    2024
 */

 #ifndef __APP_CPU_CRC_H
 #define __APP_CPU_CRC_H

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

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Initializes the CRC module.
 */
void app_crc_init(void);

/**
 * @brief Handles the CRC interrupt.
 */
void app_crc_irq_callback(void);

/**
 * @brief Demonstrates CRC-8 calculation.
 * @param test_string Data string to calculate CRC-8 on.
 * 
 * Verify the correctness of the CRC calculation with the website below:
 * https://crccalc.com/?crc=APP%20SDK%20CRC-8%20Demo&method=crc8&datatype=0&outtype=0
 */ 
void app_crc8_test(uint8_t *test_string);

/**
 * @brief Demonstrates CRC-32 calculation.
 * @param test_string Data string to calculate CRC-16 on.
 *
 * Verify the correctness of the CRC calculation with the website below:
 * https://crccalc.com/?crc=APP%20SDK%20CRC-16%20Demo&method=crc16&datatype=0&outtype=0
 */ 
void app_crc16_test(uint8_t *test_string);

/**
  * @brief Demonstrates CRC-32 calculation.
 * @param test_string Data string to calculate CRC-32 on.
 *
 * Verify the correctness of the CRC calculation with the website below:
 * https://crccalc.com/?crc=APP%20SDK%20CRC-32%20Demo&method=crc32&datatype=0&outtype=0
 */ 
void app_crc32_test(uint8_t *test_string);

/**
 * @brief Demonstrates the usage of CRC (Cyclic Redundancy Check) functionality in the application peripheral.
 * 
 * This function initializes the UART and CRC modules, then performs CRC8, CRC16, and CRC32 calculations on predefined data strings.
 * It demonstrates how to use the CRC module in the APP SDK to ensure data integrity.
 */
void app_peripheral_crc_demo_init(void);

#endif
