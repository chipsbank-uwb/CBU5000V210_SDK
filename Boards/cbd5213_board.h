 /**
 * @file    log_uart.c
 * @brief   UART Commander Application Source File
 * @details This file contains functions for initializing, sending data, printing strings, 
 *          and deinitializing the UART module. It handles UART communication and ensures 
 *          thread safety when FreeRTOS is enabled. 
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __LOG_UART_H
#define __LOG_UART_H

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
void log_uart_init(void);
void log_uart_print(const char *data);
void board_key_init(void);
enGPIO_PinState board_key_input_status(void);
void board_led_init(void);
void board_led_ctrl(unsigned char is_on);
void board_gpio_interrupt_init(void);


#endif /*__LOG_UART_H*/
