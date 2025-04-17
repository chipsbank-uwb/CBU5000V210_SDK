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

/**
 * Hardware Connection: GPIO00-RX and GPIO01-TX, BPR=115200, Stop bit = 1; BitOrder = LSB, Parity = NONE
 *
 * Program Flow:
 *    1. Input the sleep duration.
 *    2. The UART prints a power-on message and enters sleep mode.
 *    3. After sleep mode ends, the UART prints a wake-up message.
 *
 * Expected Output:
 *    1. Upon power-on, the UART prints a power-on indicator.
 *    2. A message indicating entry into sleep mode for 3 seconds is printed.
 *    3. After 3 seconds, the system exits sleep mode, and the UART prints an exit indicator.
 *
 * Note: In this routine, Uart needs to use FIFO mode, using SDMA mode will first go to sleep, exit sleep before printing
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "main.h"

/**
 * @brief Initializes the Data Watchpoint and Trace (DWT) unit for performance measurement.
 * 
 * This function checks if the DWT unit is enabled. If not, it enables the DWT unit
 * and configures it to count processor cycles for performance measurement.
 * 
 */
void DWT_Init(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
 * @brief The main function of the program.
 * 
 * This function serves as the entry point of the program. It is called automatically
 * when the program starts executing. The function contains an infinite loop, which
 * ensures that the program continues running indefinitely.
 * 
 * @return An integer value representing the exit status of the program. 
 *         This function always returns 0, indicating successful execution.
 */
int main(void)
{  
    // Initializes the Data Watchpoint and Trace (DWT) 
    DWT_Init();
    
    //sleep demo
    app_peripheral_sleep_demo_init();	                    
    while(1)
    {
        app_peripheral_sleep_demo_loop(5000);
    };
}

