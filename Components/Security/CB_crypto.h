/**
 * @file    CB_crypto.h
 * @brief   Header file for CRYPTO (Security and Protection Accelerator) module
 * @details This file contains the definitions and function prototypes for utilizing
 *          the CRYPTO module for cryptographic operations such as AES encryption and decryption.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef INC_CRYPTO_H_
#define INC_CRYPTO_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum
{
  EN_CRYPTO_ENCRYPT = 0,
  EN_CRYPTO_DECRYPT,  
} enEncryptDecrypt;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
  __IO uint8_t Data[128]; // Address Offset: 0x4000 
} CRYPTO_CIPHER_CONTEXT_TypeDef;

typedef struct
{
  __IO uint8_t Data[128]; // Address Offset: 0x8000 
} CRYPTO_HASH_CONTEXT_TypeDef;

typedef struct
{
  uint32_t result;
  uint32_t cpuCylces;
} CRYPTO_TestResult;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void CB_CRYPTO_Init(void);
CRYPTO_TestResult CB_CRYPTO_GetResults(void);
void cb_crypto_irqhandler(void);
void cb_crypto_app_irq_callback(void);
void CB_CRYPTO_MID_IRQ_Callback(void);
void CB_CRYPTO_AES_128_ECB(enEncryptDecrypt enEncryptDecrypt, uint8_t* in, uint8_t* out, uint8_t* key);
void CB_CRYPTO_AES_128_CMAC_Encrypt(uint8_t* in, uint8_t length, uint8_t* out, uint8_t* key);
void CB_CRYPTO_AES_128_CCM(enEncryptDecrypt enEncryptDecrypt, const uint8_t *key, const uint8_t *nonce, const uint8_t *adata, uint32_t adata_len,
                                           const uint8_t *payload, uint32_t payload_len, uint8_t *mic, uint8_t mic_len, uint8_t *outbuf);
//void CRYPTO_IRQ_Handler(void);

#endif  // INC_CRYPTO_H_

