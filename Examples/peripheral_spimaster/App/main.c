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
 *   2. Initialize SPI, set parameters: SDMA mode, MBS, 2M, Mode 0
 *   3. In the while loop, receive UART command to control SPI to send 11 bytes of data
 *
 * Expected Output:
 *   1. Upon power-on, print power-on flag
 *   2. The master sends and receives data every 2s
 *   3. UART displays the sent and received data. Observe if the sent and received data are consistent. Consistency indicates that SPI transmission and reception are normal.
 */
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "main.h"

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
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
    
    //SPI Master demo_main
    app_peripheral_spi_master_init();
    while(1)
    {
      //SPI Master demo_loop
      app_peripheral_spi_master_loop();
    }
}

