/**
 * @file app_pka.h
 * @brief Header file exposing example functions for Pka usage.
 * @details Contains app_pka's function declarations.
 * @date 2024
 * @author Chipsbank
*/
#ifndef INC_APP_PKA_H_
#define INC_APP_PKA_H_
//-------------------------------
// INCLUDE
//-------------------------------
#include "APP_common.h"
//-------------------------------
// DEFINE
//-------------------------------
//-------------------------------
// ENUM
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
 * @brief Example usage of signing api.
 * @detail 
 * This example hashed the binary with SHA-256, and generated an rsa-1024 keypair with openssl. These are outputted
 * in big-endian, so the reverse API is used to make them little-endian friendly. The output signature is copied to a
 * global in this example so the verification example can run properly. Remember to ALWAYS wrap the PKA API around
 * a Init() and Deinit(). This is compulsory.
 */
void app_pka_encrypt_rsa1024(void);

/**
 * @brief Example usage of verification API.
 * @detail Get the public key from the generated private key used for signing. Reverse them if they are big-endian
 * originally. The verification API will spit out the "decrypted" hash, which should be the same as the original if
 * the signature was uncorrupted or unchanged (high integrity). We use memcmp here to verify this.
 */
void app_pka_verify_rsa1024(void);

/**
 * @brief Signing and verification using API that takes precomputed values.
 * @detail Normal APIs don't require the precomputed values, but they take much more cpu cycles. Since precomputation
 * only use public components of keypairs and they stay constant so long as the keypair is the same, we provided the 
 * choice to save these cpu cycles.
 */
void app_pka_rsa_sign_and_verify_precompute(void);

/**
 * @brief Main function to execute PKA-related operations and tests.
 *
 * This function initializes the UART for debugging purposes and then proceeds 
 * to perform RSA encryption, verification, and precomputation of signing and verification 
 * using the PKA (Public Key Accelerator) module. These operations serve as examples 
 * of how to use the PKA module at the application level.
 */
void app_peripheral_pka_init(void);
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
#endif /** INC_APP_TRNG_H_ */
