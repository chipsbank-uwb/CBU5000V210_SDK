/*
 * @file    main.c
 * @brief   Implementation file for the main module.
 * @details This file contains the implementation of the main module, including
 *          the main function, initialization functions, and other utility functions.
 *          It also includes necessary headers and sections for function prototypes,
 *          definitions, enums, structs/unions, and global variables.
 * @author  Chipsbank
 * @date    2024
 */
 
 /*
 * Hardware Connection: UART GPIO00-RX, GPIO01-TX, Baud Rate = 115200, Stop bit = 1, Bit Order = LSB, Parity = NONE
 *
 * Program Flow:
 *      1. Initialize UART, set WDT timeout time, enable WDT interrupt, and initialize WDT.
 *      2. Print startup flag via UART.
 *      3. After WDT times out (5 seconds), trigger an interrupt to print timeout information, and the chip resets.
 *
 * Phenomena:
 *      1. Upon power-on, the reset power-up flag and WDT start running flag can be seen printed on the UART.
 *      2. After exceeding the set timeout period, the WDT interrupt is triggered first, printing the timeout message in the interrupt handler.
 *      3. The reset power-up flag is printed again on the UART, indicating that the WDT timeout reset was successful.
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "main.h"

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief The main function of the program.
 * 
 * This function serves as the entry point of the program. It is called automatically
 * when the program starts executing. The function contains an infinite loop, which
 * ensures that the program continues running indefinitely.
 * 
 * @return An integer value representing the exit status of the program. 
 *         This function always returns 0, indicating successful execution.
 **/
int main(void)
{  
    //Wdt demo
    app_peripheral_wdt_init();
    while(1);
}
