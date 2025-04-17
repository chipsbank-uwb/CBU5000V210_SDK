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
 * Hardware Connections: UART GPIO00-RX, GPIO01-TX
 *
 * Program Flow:
 *    1. Initialize UART and configure UART parameters: BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
 *    2. Initialize DMA module and register interrupt callback function, and clear four DMA target memory regions
 *    3. Configure DMA channel 0, set source address, destination address, data length, etc.
 *    4. Enable DMA channel and print source array content and destination array content before transmission
 *    5. Set DMA request, start transmission, and wait for transmission to complete
 *    6. Print destination array content after transmission
 *    7. Disable DMA channel
 *
 * Expected Output:
 *    1. Upon power-on, the power-on flag is printed first
 *    2. Print source address data and destination address data before enabling DMA transmission
 *    3. After transmission is complete, print source address data and destination address data after transmission, observe if the destination address data matches the source address data
 *    4. If the destination address data matches the source address data after DMA transmission, it indicates that the DMA transmission is correct and the DMA module can be used normally
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
    //DMA demo
    app_peripheral_dma_demo_init();
    while(1);
}

