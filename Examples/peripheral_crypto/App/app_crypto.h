/**
 * @file    app_crypto.h
 * @brief   CRYPTO Application Module Header
 * @details This module provides functions for AES encryption and decryption functions.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_CPU_CRYPTO_H
#define __APP_CPU_CRYPTO_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Initializes the CRYPTO module.
 */
void app_crypto_init(void);

/**
 * @brief Interrupt handler for CRYPTO module.
 */
void app_crypto_irq_callback(void);

/**
 * @brief Demonstrates AES-128 encryption in ECB mode using CB_CRYPTO_AES_128_ECB.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_ECB` function to encrypt a plaintext
 * message using AES-128 in ECB mode and prints the resulting ciphertext in hexadecimal format.
 */ 
void app_aes_128_ecb_encryption(void);

/**
 * @brief Demonstrates AES-128 decryption in ECB mode using CB_CRYPTO_AES_128_ECB.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_ECB` function to decrypt a ciphertext
 * message using AES-128 in ECB mode and prints the resulting plaintext in hexadecimal format.
 */
void app_aes_128_ecb_decryption(void);

/**
 * @brief Demonstrates AES-128 encryption in CMAC mode using CB_CRYPTO_AES_128_CMAC_Encrypt.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_CMAC_Encrypt` function to calculate
 * the message authentication code of a plaintext message
 * using AES-128 in CMAC mode and prints the resulting mac in hexadecimal format.
 */
void app_aes_128_cmac_encryption(void);

/**
 * @brief Demonstrates AES-128 encryption in CCM mode using CB_CRYPTO_AES_128_CCM.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_CCM` function to encrypt a plaintext
 * message using AES-128 in CCM mode and prints the resulting ciphertext in hexadecimal format.
 */
void app_aes_128_ccm_encryption(void);

/**
 * @brief Demonstrates AES-128 decryption in CCM mode using CB_CRYPTO_AES_128_CCM.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_CCM` function to decrypt a ciphertext
 * message using AES-128 in CCM mode and prints the resulting plaintext in hexadecimal format.
 */
void app_aes_128_ccm_decryption(void);

/**
 * @brief Main function for peripheral cryptographic operations.
 *
 * This function initializes the UART and CRYPTO modules, then performs various AES-128 cryptographic operations.
 */
void app_peripheral_crypto_init(void); 

#endif  // __APP_CRYPTO_H
