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
 * Hardware connection: UART GPIO00-RX, GPIO01-TX  BPR=115200, Stop bit = 1; BitOrder = LSB, Parity = NONE
 * 											LED-GPIO02
 * Program flow:
 * 			1. Initialize UART, configure UART parameters, BPR=115200, Stop bit = 1; BitOrder = LSB, Parity = NONE
 * 			2. Set timer timing to 1s, enable interrupt, and count and print in the interrupt, initialize Timer
 * 			3. Initialize GPIO02, output mode
 * 			4. Start the timer, trigger interrupt every 1s, print incrementing count
 *
 * Expected Output:
 * 			1. Chip powers on, print power-on flag
 * 			2. UART sends print data every 1s
 * 			3. LED blinks every 1s
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
    //Timer demo
    app_peripheral_timer_init();
    while(1);
}

