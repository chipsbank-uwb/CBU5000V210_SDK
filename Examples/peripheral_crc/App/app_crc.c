/**
 * @file    app_crc.c
 * @brief   CRC Application Module
 * @details This module provides functions for CRC calculation functions.
 * @author  Chipsbank
 * @date    2024
 */
 
 //-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "app_crc.h"
#include "CB_crc.h"
#include "AppSysIrqCallback.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_CRC_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_CRC_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_crc_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_crc_print(...)
#endif

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

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Initializes the CRC module.
 */
void app_crc_init(void)
{
    cb_crc_init();
}

/**
 * @brief Handles the CRC interrupt.
 */
void app_crc_irq_callback(void)
{
    app_crc_print("\nCRC Result (with IRQ): 0x%X\n", cb_crc_get_crc_result());
    app_irq_deregister_irqcallback(EN_IRQENTRY_CRC_APP_IRQ, app_crc_irq_callback);
}

/**
 * @brief Demonstrates CRC-8 calculation.
 * @param test_string Data string to calculate CRC-8 on.
 * 
 * Verify the correctness of the CRC calculation with the website below:
 * https://crccalc.com/?crc=APP%20SDK%20CRC-8%20Demo&method=crc8&datatype=0&outtype=0
 */ 
void app_crc8_test(uint8_t *test_string) 
{
    CB_STATUS status;

    // Print the data to be used for CRC-8 calculations
    app_crc_print("\n\nData to be performed for CRC8 calculations:  %s\n", test_string);

    // Configure the CRC algorithm for CRC-8/SMBUS
    cb_crc_algo_config(EN_CRC8, EN_InitValZero, EN_CRCRefOut_Disable, EN_CRCRefIn_Disable, 0x07, 0x00);  /*CRC-8/SMBUS*/

    // Perform CRC-8 calculation using polling method
    status = cb_crc_process_from_input_data(test_string, (uint16_t) strlen((char *) test_string), EN_CRC_ReInit_Enable);

    // Check if the CRC-8 calculation was successful
    if (status == CB_PASS)
    {
        app_crc_print("\nCRC-8 Result (with polling): 0x%2X\n", cb_crc_get_crc_result());
    }
    else 
    {
        app_crc_print("\nCRC-8 operation failed\n");
    }

    // Reconfigure the CRC algorithm for CRC-8/SMBUS (same configuration as before)
    cb_crc_algo_config(EN_CRC8, EN_InitValZero, EN_CRCRefOut_Disable, EN_CRCRefIn_Disable, 0x07, 0x00);  /*CRC-8/SMBUS*/

    // Register the IRQ callback function for CRC-8 calculation
    app_irq_deregister_irqcallback(EN_IRQENTRY_CRC_APP_IRQ, app_crc_irq_callback);

    // Perform CRC-8 calculation using interrupt method
    cb_crc_process_from_memory((uint32_t) test_string, (uint16_t) strlen((char *) test_string), EN_CRC_ReInit_Enable, EN_CRC_IRQ_Enable);

    // Wait until the CRC-8 calculation is complete (using polling to check idle status)
    while (cb_crc_check_idle() != CB_PASS);

    // Print completion message for CRC-8 calculation with IRQ
    app_crc_print("\nCRC-8 with IRQ finished!\n");
}

/**
 * @brief Demonstrates CRC-32 calculation.
 * @param test_string Data string to calculate CRC-16 on.
 *
 * Verify the correctness of the CRC calculation with the website below:
 * https://crccalc.com/?crc=APP%20SDK%20CRC-16%20Demo&method=crc16&datatype=0&outtype=0
 */ 
void app_crc16_test(uint8_t *test_string) 
{
    CB_STATUS status;

    app_crc_print("\n\nData to be performed for CRC16 calculations:  %s\n", test_string);

    cb_crc_algo_config(EN_CRC16, EN_InitValZero, EN_CRCRefOut_Enable, EN_CRCRefIn_Enable, 0x8005, 0x0000);  /*CRC-16/ARC*/
    status = cb_crc_process_from_input_data(test_string, (uint16_t) strlen((char *) test_string),EN_CRC_ReInit_Enable);

    if (status == CB_PASS)
    {
      app_crc_print("\nCRC-16 Result (with polling): 0x%4X\n", cb_crc_get_crc_result());
    }
    else 
    {
      app_crc_print("\nCRC-16 operation failed\n");
    }

    cb_crc_algo_config(EN_CRC16, EN_InitValZero, EN_CRCRefOut_Enable,EN_CRCRefIn_Enable, 0x8005, 0x0000);  /*CRC-16/ARC*/
    app_irq_deregister_irqcallback(EN_IRQENTRY_CRC_APP_IRQ, app_crc_irq_callback);
    cb_crc_process_from_memory((uint32_t) test_string, (uint16_t) strlen((char *) test_string), EN_CRC_ReInit_Enable, EN_CRC_IRQ_Enable);

    while (cb_crc_check_idle() != CB_PASS);
    app_crc_print("\nCRC-16 with IRQ finished!\n");
}

/**
  * @brief Demonstrates CRC-32 calculation.
 * @param test_string Data string to calculate CRC-32 on.
 *
 * Verify the correctness of the CRC calculation with the website below:
 * https://crccalc.com/?crc=APP%20SDK%20CRC-32%20Demo&method=crc32&datatype=0&outtype=0
 */ 
void app_crc32_test(uint8_t *test_string) 
{
    CB_STATUS status;

    app_crc_print("\n\nData to be performed for CRC32 calculations:  %s\n", test_string);

    cb_crc_algo_config(EN_CRC32, EN_InitValOne, EN_CRCRefOut_Enable, EN_CRCRefIn_Enable, 0x04C11DB7, 0xFFFFFFFF);  /*CRC-32*/
    status = cb_crc_process_from_input_data(test_string, (uint16_t) strlen((char *) test_string),EN_CRC_ReInit_Enable);

    if (status == CB_PASS)
    {
      app_crc_print("\nCRC-32 Result (with polling): 0x%4X\n", cb_crc_get_crc_result());
    }
    else 
    {
      app_crc_print("\nCRC-32 operation failed\n");
    }

    cb_crc_algo_config(EN_CRC32, EN_InitValOne, EN_CRCRefOut_Enable, EN_CRCRefIn_Enable, 0x04C11DB7, 0xFFFFFFFF);  /*CRC-32/MPEG-2*/
    app_irq_deregister_irqcallback(EN_IRQENTRY_CRC_APP_IRQ, app_crc_irq_callback);
    cb_crc_process_from_memory((uint32_t) test_string, (uint16_t) strlen((char *) test_string), EN_CRC_ReInit_Enable, EN_CRC_IRQ_Enable);

    while (cb_crc_check_idle()!= CB_PASS);
    app_crc_print("\nCRC-32 with IRQ finished!\n");
}

/**
 * @brief Demonstrates the usage of CRC (Cyclic Redundancy Check) functionality in the application peripheral.
 * 
 * This function initializes the UART and CRC modules, then performs CRC8, CRC16, and CRC32 calculations on predefined data strings.
 * It demonstrates how to use the CRC module in the APP SDK to ensure data integrity.
 */
void app_peripheral_crc_demo_init(void)
{
    // Data for CRC calculation: CRC8 uses "Hello World !" as sample data
    uint8_t crc8_data[] =  "Hello World !";         
    // Data for CRC calculation: CRC16 uses "APP SDK CRC-16 Demo" as sample data
    uint8_t crc16_data[] = "APP SDK CRC-16 Demo";
    // Data for CRC calculation: CRC32 uses "APP SDK CRC-32 Demo" as sample data
    uint8_t crc32_data[] = "APP SDK CRC-32 Demo";

    // UART initialization: GPIO00-RX, GPIO01-TX, BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
    app_uart_init();
	
    // Print power-on reset flag
    app_crc_print("\n\nPower-on reset\n");     

    // CRC module initialization
    app_crc_init();                           

    // Perform CRC8 calculation on the data
    app_crc8_test(crc8_data);                   

    // Perform CRC16 calculation on the data
    app_crc16_test(crc16_data);                  
        
    // Perform CRC32 calculation on the data
    app_crc32_test(crc32_data);                      
}
