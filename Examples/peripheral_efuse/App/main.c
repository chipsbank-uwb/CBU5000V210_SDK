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
 * Hardware Connection: UART GPIO00-RX, GPIO01-TX	
 * 
 * Program Flow:   
 *   1. Initialize UART, configure UART parameters, BPR=115200, Stop bit = 1; BitOrder = LSB, Parity = NONE 
 *   2. Initialize timer, used for UART receive timeout control
 *   3. Initialize efuse module, call in while loop, execute corresponding operations when receiving corresponding commands
 *
 * Expected Output:						
 *   1. Chip power-on, print power-on flag
 *   2. According to UART prompt control instructions, send hexadecimal CCCC + command to control efuse module
 *   3. Observe whether UART output matches the executed command
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
    //Efuse demo_init 
    app_peripheral_efuse_demo_init();
    while(1)
    {
      //Efuse demo_loop
      app_peripheral_efuse_demo_loop();
    }
}

