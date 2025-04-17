/**
 * @file    app_crypto.c
 * @brief   CRYPTO Application Module
 * @details This module provides functions for AES encryption and decryption functions.
 * @author  Chipsbank
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "app_crypto.h"
#include "CB_crypto.h"
#include "CB_scr.h"
#include "ARMCM33_DSP_FP.h"
#include "AppSysIrqCallback.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_CRYPTO_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_CRYPTO_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_crypto(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_crypto(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define KEY_SIZE      16
#define NONCE_SIZE    13
#define ADATA_SIZE    10
#define PAYLOAD_SIZE  16
#define MIC_SIZE       8

#define CRYPTO_SRC_RAMDATA_SIZE    0x00000100
#define CRYPTO_DEST_RAMDATA_SIZE   0x00000100
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static volatile uint8_t id_done;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Initializes the CRYPTO module.
 */
void app_crypto_init(void)
{
    CB_CRYPTO_Init(); 
    app_irq_register_irqcallback(EN_IRQENTRY_CRYPTO_APP_IRQ, app_crypto_irq_callback);
}

/**
 * @brief Interrupt handler for CRYPTO module.
 */
void app_crypto_irq_callback(void)
{
    CRYPTO_TestResult test_result = CB_CRYPTO_GetResults();

    // mark done
    id_done = APP_TRUE;

    // print results
    if (test_result.result == CB_PASS)
    {
      app_crypto("Crypto operation PASSED after  %ld CPU cycles\n", test_result.cpuCylces);
    }
    else 
    {
      app_crypto("Crypto operation FAILED after %ld CPU cycles\n", test_result.cpuCylces);
    }
}

/**
 * @brief Demonstrates AES-128 encryption in ECB mode using CB_CRYPTO_AES_128_ECB.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_ECB` function to encrypt a plaintext
 * message using AES-128 in ECB mode and prints the resulting ciphertext in hexadecimal format.
 */ 
void app_aes_128_ecb_encryption(void) 
{
    // example to encrypt by AES-ECB
    uint8_t key[KEY_SIZE]       = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t plain[PAYLOAD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20};
    uint8_t cipher[100];

    id_done = APP_FALSE;

    /**
     * EN_CRYPTO_ENCRYPT : Encrypt using AES-128-ECB.
     * plain             : plain text to be encrypted.
     * cipher            : Output buffer to store the result (cipher text).
     * key               : Encryption key.
     * Online website    : http://www.ip33.com/crypto/aes128.html
     */                                      
    CB_CRYPTO_AES_128_ECB(EN_CRYPTO_ENCRYPT, plain, cipher, key);

    while (!id_done);  // wait until interrupt is served

    for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
    {
      app_crypto("%02x ",cipher[i]);
    }

    app_crypto(" AES_128_ECB_ENC\n\n");
    /* output is 0x0f 5f 24 0f f8 4e 36 43 ab 85 88 4e 9e 3a 98 3b */
}

/**
 * @brief Demonstrates AES-128 decryption in ECB mode using CB_CRYPTO_AES_128_ECB.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_ECB` function to decrypt a ciphertext
 * message using AES-128 in ECB mode and prints the resulting plaintext in hexadecimal format.
 */
void app_aes_128_ecb_decryption(void) 
{
    // example to decrypt by AES-ECB
    uint8_t key[KEY_SIZE]        = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t cipher[PAYLOAD_SIZE] = { 0x0f, 0x5f, 0x24, 0x0f, 0xf8, 0x4e, 0x36, 0x43, 0xab, 0x85, 0x88, 0x4e, 0x9e, 0x3a, 0x98, 0x3b};
    uint8_t plain[100];

    id_done = APP_FALSE;

    /**
     * EN_CRYPTO_DECRYPT : Encrypt using AES-128-ECB.
     * cipher            : plain text to be encrypted.
     * plain             : Output buffer to store the result (cipher text).
     * key               : Encryption key.
     * Online website    : http://www.ip33.com/crypto/aes128.html
     */                                      
    CB_CRYPTO_AES_128_ECB(EN_CRYPTO_DECRYPT, cipher, plain, key);

    while (!id_done);

    for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
    {
      app_crypto("%02x ",plain[i]);
    }

    app_crypto(" AES_128_ECB_DEC\n\n");
    /* output is 0x01 23 45 67 89 10 11 12 13 14 15 16 17 18 19 20 */
}

/**
 * @brief Demonstrates AES-128 encryption in CMAC mode using CB_CRYPTO_AES_128_CMAC_Encrypt.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_CMAC_Encrypt` function to calculate
 * the message authentication code of a plaintext message
 * using AES-128 in CMAC mode and prints the resulting mac in hexadecimal format.
 */
void app_aes_128_cmac_encryption(void)
{
    // example to calculate mac by AES-CMAC
    uint8_t key[KEY_SIZE]       = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t plain[PAYLOAD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20};
    uint8_t cipher[100];

    id_done = APP_FALSE;

    /**
     * Calculate a message authentication code using AES-128-CMAC.
     * plain          : plain text to be encrypted.
     * PAYLOAD_SIZE   : Length of the input data (plain text).
     * cipher         : Output buffer to store the result (message authentication code).
     * key            : Encryption key.
     * Online website : https://artjomb.github.io/cryptojs-extension/
     */
    CB_CRYPTO_AES_128_CMAC_Encrypt(plain, PAYLOAD_SIZE, cipher, key);

    while(!id_done);

    for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
    {
      app_crypto("%02x ",cipher[i]);
    }
    app_crypto(" AES_128_CMAC_ENC\n\n");
    /* output is 0xcf 0a 3e 5d b9 9e fe 16 be 47 b1 95 1f 2c 7d 24 */
}

/**
 * @brief Demonstrates AES-128 encryption in CCM mode using CB_CRYPTO_AES_128_CCM.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_CCM` function to encrypt a plaintext
 * message using AES-128 in CCM mode and prints the resulting ciphertext in hexadecimal format.
 */
void app_aes_128_ccm_encryption(void) 
{
    // example to encrypt by AES-CCM
    uint8_t key[KEY_SIZE]       = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t nonce[NONCE_SIZE]   = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x06 };
    uint8_t adata[ADATA_SIZE]   = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
    uint8_t plain[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20};
    uint8_t cipher[100];

    id_done = APP_FALSE;

    /**
     * EN_CRYPTO_ENCRYPT : Encrypt using AES-128-CCM.
     * key               : Encryption key.
     * nonce             : nonce value.
     * adata             : Additional authenticated data.
     * ADATA_SIZE        : adata_len Length of the additional authenticated data.                              
     * plain             : plain text to be encrypted.
     * PAYLOAD_SIZE      : Length of the plain text. 
     * MIC_SIZE          : Length of the message integrity code (authentification tag).
     * cipher            : Output buffer to store the result (cipher text).
     * Online website    : https://const.net.cn/tool/aes/aes-ccm/
     */                                
    CB_CRYPTO_AES_128_CCM(EN_CRYPTO_ENCRYPT, key, nonce, adata, ADATA_SIZE, plain, sizeof(plain), NULL, MIC_SIZE, cipher);

    while(!id_done);
                                    
    for (uint8_t i = 0; i < sizeof(plain); i++) 
    {
      app_crypto("%02x ",cipher[i]);
    }
    app_crypto(" AES_128_CCM_ENC\n\n");
    /* output is 0xd4 c6 46 91 fd 21 f5 a5 3a bc 6b 51 ca 28 36 7f */
}

/**
 * @brief Demonstrates AES-128 decryption in CCM mode using CB_CRYPTO_AES_128_CCM.
 *
 * This function mocks the usage of the `CB_CRYPTO_AES_128_CCM` function to decrypt a ciphertext
 * message using AES-128 in CCM mode and prints the resulting plaintext in hexadecimal format.
 */
void app_aes_128_ccm_decryption(void) 
{
    // example to decrypt by AES-CCM
    uint8_t key[KEY_SIZE]        = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t nonce[NONCE_SIZE]    = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x06 };
    uint8_t adata[ADATA_SIZE]    = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
    uint8_t cipher[PAYLOAD_SIZE] = { 0xd4, 0xc6, 0x46, 0x91, 0xfd, 0x21, 0xf5, 0xa5, 0x3a, 0xbc, 0x6b, 0x51, 0xca, 0x28, 0x36, 0x7f};
    uint8_t mictag[MIC_SIZE] = {0x01, 0xc3, 0xa1, 0x19, 0x97, 0x2b, 0x64, 0xff };
    uint8_t plain[100];

    id_done = APP_FALSE;

    /**
     * EN_CRYPTO_DECRYPT : Decrypt using AES-128-CCM.
     * key               : Encryption key.
     * nonce             : nonce value.
     * adata             : Additional authenticated data.
     * ADATA_SIZE        : adata_len Length of the additional authenticated data.                              
     * plain             : plain text to be encrypted.
     * PAYLOAD_SIZE      : Length of the plain text. 
     * MIC_SIZE          : Length of the message integrity code (authentification tag).
     * cipher            : Output buffer to store the result (cipher text).
     * Online website    : https://const.net.cn/tool/aes/aes-ccm/
     */                                     
    CB_CRYPTO_AES_128_CCM(EN_CRYPTO_DECRYPT, key, nonce, adata, ADATA_SIZE, cipher, sizeof(cipher), mictag, MIC_SIZE, plain);

    while(!id_done);

    for (uint8_t i = 0; i < sizeof(cipher); i++) 
    {
      app_crypto("%02x ",plain[i]);
    }

    app_crypto(" AES_128_CCM_DEC\n\n");
    /* output is 0x01 23 45 67 89 10 11 12 13 14 15 16 17 18 19 20 */
}

/**
 * @brief Main function for peripheral cryptographic operations.
 *
 * This function initializes the UART and CRYPTO modules, then performs various AES-128 cryptographic operations.
 */
void app_peripheral_crypto_init(void) 
{
    // Initialize UART with specific configuration and print power-on reset message
    app_uart_init();
    app_uart_printf("\n\nPower-on reset\n");

    // Initialize the CRYPTO module
    app_crypto_init();
    
    app_crypto("Raw data:\n");
    uint8_t plain[PAYLOAD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20};
    for (uint8_t i = 0; i < sizeof(plain); i++) 
    {
      app_crypto("%02x ",plain[i]);
    }
    app_crypto("\n\n");

    // Perform AES-128 ECB encryption and decryption
    app_aes_128_ecb_encryption();
    app_aes_128_ecb_decryption();

    // Perform AES-128 CMAC encryption
    app_aes_128_cmac_encryption();

    // Perform AES-128 CCM encryption and decryption
    app_aes_128_ccm_encryption();
    app_aes_128_ccm_decryption();
}
