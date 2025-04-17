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
 * Hardware Connections:
 *   GPIO2, GPIO3: Set as output mode (pull-up), connected to LED1 and LED2 respectively
 *   GPIO5: Set as edge-triggered mode, measures external interrupt (low-level trigger, toggles GPIO2 output in interrupt)
 *   GPIO4: Set as input mode, detects high/low level changes
 *
 * Program Flow:
 *   1. Initialize GPIO2, GPIO3, GPIO4, and GPIO5 with their corresponding modes
 *   2. Use GPIO4 as an external input to detect high/low levels: LED2 turns on for high level, turns off for low level
 *   3. Use GPIO5 as a button: when pressed, toggle the state of GPIO2 and trigger an interrupt
 *
 * Expected Output:
 *   1. When GPIO4 input is high, LED2 turns on; when low, LED2 turns off
 *   2. When GPIO5 input has a falling edge, the state of LED1 toggles. If connected to a UART, it prints an interrupt entry flag
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
    //GPIO_demo_main
    app_peripheral_gpio_demo_init();
    while(1)
    {
      //GPIO_demo_loop
      app_peripheral_gpio_demo_loop();
    }
}
