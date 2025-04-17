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
 *  Hardware Connections: UART GPIO00-RX, GPIO01-TX
 * 
 *  Program Flow:
 *  		1. Set the data and key to be encrypted
 *  		2. Initialize UART with the following parameters: Baud Rate = 115200, Stop Bits = 1, Bit Order = LSB, Parity = None
 *  		3. Call ECB, CMAC, and CCM encryption/decryption functions, and output the results to the UART
 * 
 * Expected Output:
 *  		1. Upon power-on, the UART first prints a power-on reset message, followed by the results of ECB, CMAC, and CCM encryption/decryption
 *  		2. ECB mode,User logs in to the website: https://www.toolhelper.cn/SymmetricEncryption/AES��Or any AES encryption and decryption website��
 *  		3. CMAC mode,User logs in to the website:https://artjomb.github.io/cryptojs-extension/
 * 			4. CCM mode ,User logs in to the website:https://const.net.cn/tool/aes/aes-ccm/
 *      3. In the website input box, enter the same data as in the program and select the corresponding encryption/decryption method to generate the corresponding results
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
    //Crypto Demo
    app_peripheral_crypto_init(); 
    while(1);
}
