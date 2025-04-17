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
 *  UART module controlled via UART0: GPIO00 - RX, GPIO01 - TX
 *  IMU module controlled via I2C protocol: GPIO03 - SDA, GPIO06 - SCL
 *
 * Program Flow:
 *  1. Initialize UART: BPR = 115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
 *  2. Initialize the timer module with a 1-second interval
 *  3. Initialize the IMU module: multiplex pins, set as pull-up, configure I2C speed
 *  4. Print "Power on" via UART
 *  5. Read IMU data every 1 second and print via UART
 *
 * Expected Output:
 *  1. Upon powering on the chip, "Power on" is printed via UART
 *  2. Every 1 second, the IMU collects data and prints it via UART
 *     By changing the IMU's orientation, observe if the printed data changes
 *     to verify the correctness of I2C communication with the IMU module
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
    //I2C demo_init
    app_peripheral_i2c_init();
    while(1)
    {
      //I2C demo_loop
      app_peripheral_i2c_loop();
    }
}

