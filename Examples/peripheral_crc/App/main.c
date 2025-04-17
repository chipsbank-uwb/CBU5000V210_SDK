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
 * Hardware connection: UART GPIO00-RX, GPIO01-TX
 * 
 * Program flow:
 *    1. Set data for different CRC calculations
 *    2. Initialize UART, configure UART parameters, BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
 *    3. Initialize CRC, input data for calculation
 *    4. Call CRC8, CRC16, CRC32 test routines, print results via UART
 *
 * Expected Output:
 *    1. After power-on, UART first prints the power-on flag, then prints the results of CRC8, CRC16, and CRC32 calculations
 *    2. User logs in to the website: https://crccalc.com/?crc=APP%20SDK%20CRC-16%20Demo&method=crc16&datatype=0&outtype=0
 *    3. In the website input box, enter the same data as in the program(Select ASCLL input, hexadecimal output), 
 *				 select the corresponding CRC calculation method, and generate the corresponding result
 *    4. User compares the website calculation result with the UART output data to determine if the CRC calculation is correct
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
*/
int main(void)
{  
    //CRC demo
    app_peripheral_crc_demo_init();
    while(1);
}
