/**
 * @file    app_uart.h
 * @brief   [CPU Subsystem] UART Application Module Header
 * @details This header file contains function prototypes for UART initialization and printing formatted output to UART.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_CPU_UART_H
#define __APP_CPU_UART_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"
#include "CB_Uart.h"

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
 * @brief   Initializes the UART module for communication.
 * @details This function turns on the UART0 module, configures the I/O multiplexer for UART0 RX and TX pins,
 *          enables the UART0 interrupt, and initializes UART0 with SDMA mode.
 */
void app_uart_init(void);

/**
 * @brief Changes the baud rate of the UART module.
 * @param baudrate The new baud rate to be set for the UART module.
 */
void app_uart_change_baudrate(enUartBaudrate baudrate);

/**
 * @brief Callback function for UART0 RXD ready interrupt.
 */
void app_uart_0_rxd_ready_callback(void);

/**
 * @brief Callback function for UART0 RX buffer full interrupt.
 */
void app_uart_0_rxb_full_callback(void);

/**
 * @brief Callback function for UART0 break error interrupt.
 */
void APP_UART_0_BREAK_ERR_Callback(void);

/**
 * @brief   Prints formatted output to UART.
 * @details This function formats the output string based on the provided format and arguments using vsnprintf,
 *          and transmits each character of the formatted string through UART.
 * @param   format The format string specifying how subsequent arguments are converted for output.
 * @param   ... Additional arguments to substitute into the format string.
 */
void app_uart_printf(const char *format, ...);

#endif  // __APP_CPU_UART_H
