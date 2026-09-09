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
// GLOBAL VARIABLE SECTION
//-------------------------------
static volatile uint8_t id_done;

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void app_crypto_init(void)
{
    CB_CRYPTO_Init(); 
    app_irq_register_irqcallback(EN_IRQENTRY_CRYPTO_APP_IRQ, app_crypto_irq_callback);
}

void app_crypto_irq_callback(void)
{
    CRYPTO_TestResult test_result = CB_CRYPTO_GetResults();
    id_done = APP_TRUE;

    if (test_result.result == CB_PASS)
    {
        app_crypto("Crypto operation PASSED \n");
    }
    else 
    {
        app_crypto("Crypto operation FAILED \n");
    }
}

// ==============================================
// AES-128-ECB ENCRYPT
// ==============================================
void app_aes_128_ecb_encryption(void) 
{
    uint8_t key[KEY_SIZE]       = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t plain[PAYLOAD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20};
    uint8_t cipher[100];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_128_ECB(EN_CRYPTO_ENCRYPT, plain, cipher, key);
    while (!id_done);

    app_crypto("=== AES-128-ECB Encrypt ===\n");
    app_crypto("Actual  : ");
    for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
        app_crypto("%02x ", cipher[i]);
    app_crypto("\nExpected: 0f 5f 24 0f f8 4e 36 43 ab 85 88 4e 9e 3a 98 3b\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-128-ECB DECRYPT
// ==============================================
void app_aes_128_ecb_decryption(void) 
{
    uint8_t key[KEY_SIZE]        = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t cipher[PAYLOAD_SIZE] = { 0x0f, 0x5f, 0x24, 0x0f, 0xf8, 0x4e, 0x36, 0x43, 0xab, 0x85, 0x88, 0x4e, 0x9e, 0x3a, 0x98, 0x3b};
    uint8_t plain[100];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_128_ECB(EN_CRYPTO_DECRYPT, cipher, plain, key);
    while (!id_done);

    app_crypto("=== AES-128-ECB Decrypt ===\n");
    app_crypto("Actual  : ");
    for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
        app_crypto("%02x ", plain[i]);
    app_crypto("\nExpected: 01 23 45 67 89 10 11 12 13 14 15 16 17 18 19 20\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-128-CMAC
// ==============================================
void app_aes_128_cmac_encryption(void)
{
    uint8_t key[KEY_SIZE]       = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t plain[PAYLOAD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20};
    uint8_t cipher[100];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_128_CMAC_Encrypt(plain, PAYLOAD_SIZE, cipher, key);
    while(!id_done);

    app_crypto("=== AES-128-CMAC ===\n");
    app_crypto("Actual  : ");
    for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
        app_crypto("%02x ", cipher[i]);
    app_crypto("\nExpected: cf 0a 3e 5d b9 9e fe 16 be 47 b1 95 1f 2c 7d 24\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-128-CCM ENCRYPT
// ==============================================
void app_aes_128_ccm_encryption(void) 
{
    uint8_t key[KEY_SIZE]       = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t nonce[NONCE_SIZE]   = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x06 };
    uint8_t adata[ADATA_SIZE]   = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
    uint8_t plain[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20};
    uint8_t cipher[100];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_128_CCM(EN_CRYPTO_ENCRYPT, key, nonce, adata, ADATA_SIZE, plain, sizeof(plain), NULL, MIC_SIZE, cipher);
    while(!id_done);
                                    
    app_crypto("=== AES-128-CCM Encrypt ===\n");
    app_crypto("Actual  : ");
    for (uint8_t i = 0; i < sizeof(plain); i++) 
        app_crypto("%02x ", cipher[i]);
    app_crypto("\nExpected: d4 c6 46 91 fd 21 f5 a5 3a bc 6b 51 ca 28 36 7f\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-128-CCM DECRYPT
// ==============================================
void app_aes_128_ccm_decryption(void) 
{
    uint8_t key[KEY_SIZE]        = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t nonce[NONCE_SIZE]    = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x06 };
    uint8_t adata[ADATA_SIZE]    = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
    uint8_t cipher[PAYLOAD_SIZE] = { 0xd4, 0xc6, 0x46, 0x91, 0xfd, 0x21, 0xf5, 0xa5, 0x3a, 0xbc, 0x6b, 0x51, 0xca, 0x28, 0x36, 0x7f};
    uint8_t mictag[MIC_SIZE] = {0x01, 0xc3, 0xa1, 0x19, 0x97, 0x2b, 0x64, 0xff };
    uint8_t plain[100];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_128_CCM(EN_CRYPTO_DECRYPT, key, nonce, adata, ADATA_SIZE, cipher, sizeof(cipher), mictag, MIC_SIZE, plain);
    while(!id_done);

    app_crypto("=== AES-128-CCM Decrypt ===\n");
    app_crypto("Actual  : ");
    for (uint8_t i = 0; i < sizeof(cipher); i++) 
        app_crypto("%02x ", plain[i]);
    app_crypto("\nExpected: 01 23 45 67 89 10 11 12 13 14 15 16 17 18 19 20\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-256-ECB ENCRYPT
// ==============================================
void app_aes_256_ecb_encryption(void) 
{
    uint8_t key[32] = {  
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };
    uint8_t plain[16]  = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    uint8_t cipher[16];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_256_ECB(EN_CRYPTO_ENCRYPT, plain, cipher, key);  
    while (!id_done);

    app_crypto("=== AES-256-ECB Encrypt ===\n");
    app_crypto("Actual  : ");
    for (uint8_t i = 0; i < 16; i++) 
        app_crypto("%02x ", cipher[i]);
    app_crypto("\nExpected: 8e a2 b7 ca 51 67 45 bf ea fc 49 90 4b 49 60 89\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-256-ECB DECRYPT
// ==============================================
void app_aes_256_ecb_decryption(void)
{
    uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };
    uint8_t cipher[16] = {0x8e,0xa2,0xb7,0xca,0x51,0x67,0x45,0xbf,0xea,0xfc,0x49,0x90,0x4b,0x49,0x60,0x89};
    uint8_t plain[16];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_256_ECB(EN_CRYPTO_DECRYPT, cipher, plain, key);
    while (!id_done);

    app_crypto("=== AES-256-ECB Decrypt ===\n");
    app_crypto("Actual  : ");
    for (int i = 0; i < 16; i++) 
        app_crypto("%02x ", plain[i]);
    app_crypto("\nExpected: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-256-CMAC
// ==============================================
void app_aes_256_cmac_encryption(void)
{
    uint8_t key[32] = {
        0x44, 0x79, 0x6E, 0x61, 0x6D, 0x69, 0x63, 0x53,
        0x54, 0x53, 0x4E, 0x6F, 0x52, 0x6F, 0x74, 0x30,
        0x44, 0x79, 0x6E, 0x61, 0x6D, 0x69, 0x63, 0x53,
        0x54, 0x53, 0x4E, 0x6F, 0x52, 0x6F, 0x74, 0x30
    };
    uint8_t input[32] = {
        0x00,0x00,0x00,0x01, 0x50,0x72,0x69,0x76,0x61,0x63,0x79,0x4B,
        0x08,0x93,0x66,0xBA, 0xFB,0x3B,0x24,0xBF, 0xD2,0x93,0x33,0x77,
        0x61,0xB8,0x8F,0xC3, 0x00,0x00,0x00,0x80
    };
    uint8_t cmac_result[16];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_256_CMAC_Encrypt(input, 32, cmac_result, key);
    while (!id_done);

    app_crypto("=== AES-256-CMAC ===\n");
    app_crypto("Actual  : ");
    for (uint8_t i = 0; i < 16; i++)
        app_crypto("%02x ", cmac_result[i]);
    app_crypto("\nExpected: 13 9C E6 84 4D C6 72 E4 97 EE 37 45 78 70 A3 92\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-256-CCM ENCRYPT
// ==============================================
void app_aes_256_ccm_encryption(void)
{
    uint8_t key[32] = { 
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };
    uint8_t nonce[13] = {0x00,0x00,0x00,0x00,0x00,0x00,0xaa,0xa1,0x00,0x00,0x00,0x00,0x06};
    uint8_t adata[10] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19};
    uint8_t plain[16] = {0x01,0x23,0x45,0x67,0x89,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x20};
    uint8_t cipher_mic_out[24];  

    id_done = APP_FALSE;
    CB_CRYPTO_AES_256_CCM(EN_CRYPTO_ENCRYPT, key, nonce, adata, 10, plain, 16, NULL, 8, cipher_mic_out);
    while(!id_done);

    app_crypto("=== AES-256-CCM Encrypt ===\n");
    app_crypto("Cipher  : ");
    for (int i=0;i<16;i++) 
        app_crypto("%02x ", cipher_mic_out[i]);
    app_crypto("\nMIC     : ");
    for (int i=16;i<24;i++) 
        app_crypto("%02x ", cipher_mic_out[i]);
    app_crypto("\n----------------------------------------\n");
    app_crypto("Exp Cip : cf 27 81 7f 02 be 2f 67 7a bb 3d 06 59 91 96 ba\n");
    app_crypto("Exp MIC : aa 50 a9 c2 bf d3 30 74\n");
    app_crypto("\n\n");
}

// ==============================================
// AES-256-CCM DECRYPT
// ==============================================
void app_aes_256_ccm_decryption(void)
{
    uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };
    uint8_t nonce[13] = {0x00,0x00,0x00,0x00,0x00,0x00,0xaa,0xa1,0x00,0x00,0x00,0x00,0x06};
    uint8_t adata[10] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19};
    uint8_t cipher_mic_input[24] = {
        0xcf,0x27,0x81,0x7f,0x02,0xbe,0x2f,0x67,
        0x7a,0xbb,0x3d,0x06,0x59,0x91,0x96,0xba,
        0xaa,0x50,0xa9,0xc2,0xbf,0xd3,0x30,0x74
    };
    uint8_t *mic_tag = &cipher_mic_input[16];

    id_done = APP_FALSE;
    CB_CRYPTO_AES_256_CCM(EN_CRYPTO_DECRYPT, key, nonce, adata, 10, cipher_mic_input, 16, mic_tag, 8, cipher_mic_input);
    while(!id_done);

    app_crypto("=== AES-256-CCM Decrypt ===\n");
    app_crypto("Actual  : ");
    for (int i = 0; i < 16; i++)
        app_crypto("%02x ", cipher_mic_input[i]);
    app_crypto("\nExpected: 01 23 45 67 89 10 11 12 13 14 15 16 17 18 19 20\n");
    app_crypto("\n\n");
}

// ==============================================
// MAIN TEST ENTRY
// ==============================================
void app_peripheral_crypto_init(void) 
{
    app_uart_init();
    app_uart_printf("\n\nPower-on reset\n");

    app_crypto_init();
	
		app_crypto("======================= AES-128 =======================\n\n");
    
    // ------------------------------
    // AES-128 Test Suite
    // ------------------------------
    app_aes_128_ecb_encryption();
    app_aes_128_ecb_decryption();
    app_aes_128_cmac_encryption();
    app_aes_128_ccm_encryption();
    app_aes_128_ccm_decryption();

    // ------------------------------
    // Switch to AES-256
    // ------------------------------
    app_crypto("======================= AES-256 =======================\n\n");

    // ------------------------------
    // AES-256 Test Suite
    // ------------------------------
    app_aes_256_ecb_encryption();
    app_aes_256_ecb_decryption();
    app_aes_256_cmac_encryption();
    app_aes_256_ccm_encryption();
    app_aes_256_ccm_decryption(); 
}