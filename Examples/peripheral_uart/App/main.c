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
 *   UART: GPIO00-RX, GPIO01-TX
 *   SPI:  GPIO03-MISO, GPIO05-CS, GPIO06-CLK, GPIO07-MOSI (short MOSI and MISO for testing)
 *
 * Program Flow:
 *   1. Initialize UART, configure UART parameters: BPR=115200, Stop bit = 1, Bit Order = LSB, Parity = NONE
 *   2. Initialize SPI, set parameters: SDMA mode, MBS, 512K, Mode 0
 *   3. In the while loop, receive UART command to control SPI to send 50 bytes of data
 *
 * Expected Output:
 *   1. Upon power-on, print power-on flag
 *   2. If a slave is connected, The serial port tool sends arbitrary data and the host starts to send and receive data (slave must be ready to receive data). Observe if the master and slave transmission and reception are consistent.
 *   3. If only testing the master, short the master's MOSI and MISO, send 50 bytes of data.
 *   4. UART displays the sent and received data. Observe if the sent and received data are consistent. Consistency indicates that SPI transmission and reception are normal.
 */
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "main.h"

//-------------------------------
// GLOBAL VARIABLE SECTION
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
 */
int main(void)
{  
    //Uart demo_main
    app_uart_init();
    app_uart_echo_demo();
    while(1);
}

