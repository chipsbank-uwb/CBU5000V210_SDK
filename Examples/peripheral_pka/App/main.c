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

/**
 *
 * Hardware Connections:  UART: GPIO00-RX, GPIO01-TX
 *
 * Program Flow:
 *    1. Initialize UART with the following parameters: BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE.
 *    2. Sign: Call app_pka_encrypt_rsa1024() to generate a signature.
 *    3. Verify: Call app_pka_verify_rsa1024() to verify the signature.
 *    4. Precompute Signature and Verification: Call app_pka_rsa_sign_and_verify_precompute() to demonstrate the use of precomputed values.
 *
 * Observations:
 *    1. Upon power-on, the UART prints a power-on reset flag.
 *    2. The UART prints "erifed success: 1", indicating that the signature verification passed. This confirms that the PKA module is functioning correctly.
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
    //Pka demo
    app_peripheral_pka_init();
    while(1);
}

