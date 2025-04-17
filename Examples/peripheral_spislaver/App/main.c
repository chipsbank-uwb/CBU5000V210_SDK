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
*    UART: GPIO00-RX, GPIO01-TX
*    SPI: GPIO03-MISO, GPIO05-CS, GPIO06-CLK, GPIO07-MOSI
*
* Program Flow:
*    1. Initialize UART, configure UART parameters: Baud Rate = 115200, Stop Bits = 1, Bit Order = LSB, Parity = NONE
*    2. Initialize SPI, set parameters: SDMA mode, MBS, 2M, Mode 0
*    3. In the while loop, receive UART command  prepare 51-byte data for SPI Slave. Wait for the host to send data and respond, then print on UART
*
* Expected Output:
*    1. Upon power-on, the chip prints a power-on flag
*    2. After the host sends data, the slave's UART will display the received host data, and the host will also receive the slave's data
*    3. Compare the transmitted and received data between the master and slave to observe if the transmission and reception are consistent
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
    //SPI Slaver demo_main	
    app_peripheral_spi_slaver_init();
    while(1)
    {
    //SPI Slaver demo_loop
    app_peripheral_spi_slave_loop();
    }
}
