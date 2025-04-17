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
 *   - UART: GPIO00-RX, GPIO01-TX
 *   - AIN Dedicated ADC Input Pin
 *
 * Program Flow:
 *   1. Initialize UART with settings: Baud Rate (BPR) = 115200, Stop Bit = 1, Bit Order = LSB, Parity = NONE
 *   2. In an infinite loop, call the ADC sampling function. The function samples the ADC multiple times every 0.5 seconds,
 *      averages the readings, and converts the result to a voltage value in the range of 0-3.3V.
 *
 * Expected Output:
 *   - After power-on, the UART tool prints the voltage value of the AIN pin every 0.5 seconds.
 *   - By changing the voltage connected to the AIN pin, you can observe the printed data to verify that the ADC is functioning correctly.
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
**/ 
int main(void)
{ 
    //ADC_demo_main
    DWT_Init();       //Initializes the DWT
    app_uart_init();  // Initialize UART for communication
    while(1)
    {
      //ADC_demo_loop
      app_adc_test_original_value(3);  //Measure the voltage from the AIN pin using gain setting 3
    }
}
