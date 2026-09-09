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
 *   2. Execute Flash basic operation demo (Init -> Erase -> Write -> Read -> Verify)
 *
 * Expected Output:						
 *   1. Chip power-on, print Flash test log via UART
 *   2. UART output shows: Flash init, erase, write, read, verify results
 *   3. All operations passed indicates Flash module works normally
 *
 * AES Encryption Notice:
 *   To enable Flash AES encryption feature, please enable the macro ENABLE_FLASH_AES in CB_flash.c
*/
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "main.h"
#include "app_flash.h"

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
    //Execute Flash read/write/erase/verify demo test
		app_flash_test_process();
    while(1)
    {
    }
}

