/**
 * @file CB_trng.h
 * @brief Header that contains API for True Random Number Generator (TRNG) driver.
 * @details TRNG driver also exposes some helper enums and structures to facilitate usage.
 * @author Chipsbank
 * @date 2024
 */
#ifndef INC_TRNG_H_
#define INC_TRNG_H_
//-------------------------------
// INCLUDE
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// DEFINE
//-------------------------------
//-------------------------------
// ENUM
//-------------------------------
typedef enum {
  EN_CB_TRNG_AES128 = 0u,
  EN_CB_TRNG_AES256 = 1u,
} enTrngAlg;

typedef enum {
  EN_CB_TRNG_OK = 0u,       
  EN_CB_TRNG_REJECTED = 1u, /** Either wrong function to call, or params are wrong */
  EN_CB_TRNG_RESEED = 2u,   /** Need to call seed functions again */
  EN_CB_TRNG_BAD = 3u,      /** Alarms were raised, internal lib issue */
} enTrngErrCode;
//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct {
  enTrngAlg Alg;      /** Alg for the DRBG */
  uint8_t UseAddIn;   /** Whether to use additional input */
  uint8_t PredResist; /** Whether to use prediction resistant mode */
} stTrngConfig;
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void CB_TRNG_Init(void);
void CB_TRNG_Deinit(void);
uint32_t CB_TRNG_Reset(void);
uint32_t CB_TRNG_Snoise(const stTrngConfig* const Config);
uint32_t CB_TRNG_Snonce(const stTrngConfig* const Config, uint32_t const Nonce[static 32]);
uint32_t CB_TRNG_ReSnoise(const stTrngConfig* const Config);
uint32_t CB_TRNG_ReSnonce(const stTrngConfig* const Config, uint32_t const Nonce[static 32]);
enTrngErrCode CB_TRNG_GetRng(uint32_t* Output, uint32_t OutputSize);
enTrngErrCode CB_TRNG_GetRngWithAddin(uint32_t* Output, uint32_t OutputSize, uint32_t const AddIn[]);
uint32_t CB_TRNG_RunKat(uint32_t* const Output);
void cb_trng_irqhandler(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
#endif /** INC_TRNG_H_ */
