/**
 * @file CB_pka.h
 * @brief Header that contains API for Public Key Accelerator (PKA) driver.
 * @details PKA driver also exposes some helper enums and structures to facilitate usage.
 * @author Chipsbank
 * @date 2024
 */
#ifndef INC_PKA_H_
#define INC_PKA_H_
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
//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct {
  uint8_t Modulus[128];
  uint8_t Exp[128];
  uint32_t ModSize;
} stPkaRsaKey;
typedef struct {
  uint8_t RInv[128];
  uint8_t Mp[128];
  uint8_t RSqr[128];
} stPkaPrecompVals;
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void CB_PKA_Init(void);
void CB_PKA_Deinit(void);
void cb_pka_irqhandler(void);
void CB_PKA_RsaSign(const uint8_t *const InputHash, uint32_t const HashSize, uint8_t *const SignedOutput,
                    const stPkaRsaKey *const PrivateKey);
void CB_PKA_RsaSignP(const uint8_t *const InputHash, uint32_t const HashSize, const stPkaPrecompVals *const Precomputed,
                     uint8_t *const SignedOutput, const stPkaRsaKey *const PrivateKey);
void CB_PKA_RsaVerify(const uint8_t *const Signature, uint32_t const SigSize, uint8_t *const Output,
                      const stPkaRsaKey *const PublicKey);
void CB_PKA_RsaVerifyP(const uint8_t *const Signature, uint32_t const SigSize, const stPkaPrecompVals *Precomputed,
                       uint8_t *const Output, const stPkaRsaKey *PublicKey);
void CB_PKA_Reverse(uint8_t *const Arr, uint32_t const ArrSize);
uint32_t CB_PKA_CalcRInv(const uint8_t *const Modulus, uint32_t const ModSize, uint8_t *const RInv);
uint32_t CB_PKA_CalcMp(const uint8_t *const Modulus, uint32_t const ModSize, const uint8_t *const RInv,
                       uint8_t *const Mp);
uint32_t CB_PKA_CalcRSqr(const uint8_t *const Modulus, uint32_t const ModSize, const uint8_t *const RInv,
                         uint8_t *const RSqr);
uint32_t CB_PKA_ModMult(const uint8_t *const X, uint32_t const XSize, const uint8_t *const Y, uint32_t const YSize,
                        const uint8_t *const Modulus, uint32_t const ModSize, const uint8_t *const RInv,
                        const uint8_t *const Mp, const uint8_t *const RSqr, uint8_t *const Out);
uint32_t CB_PKA_ModAdd(const uint8_t *const X, uint32_t const XSize, const uint8_t *const Y, uint32_t const YSize,
                       const uint8_t *const Modulus, uint32_t const ModSize, uint8_t *const Out);
uint32_t CB_PKA_ModSub(const uint8_t *const X, uint32_t const XSize, const uint8_t *const Y, uint32_t const YSize,
                       const uint8_t *const Modulus, uint32_t const ModSize, uint8_t *const Out);
uint32_t CB_PKA_Reduce(const uint8_t *const X, const uint32_t XSize, const uint8_t *const Modulus,
                       uint32_t const ModSize, uint8_t *const Out);
uint32_t CB_PKA_ModDiv(const uint8_t *const X, uint32_t const XSize, const uint8_t *const Y, uint32_t const YSize,
                       const uint8_t *const Modulus, uint32_t const ModSize, uint8_t *const Out);
uint32_t CB_PKA_ModInv(const uint8_t *const X, const uint32_t XSize, const uint8_t *const Modulus,
                       uint32_t const ModSize, uint8_t *const Out);

// F0 flag (timing attack resistance) thingy not handled
uint32_t CB_PKA_ModExp(const uint8_t *const X, uint32_t const XSize, const uint8_t *const Y, uint32_t const YSize,
                       const uint8_t *const Modulus, uint32_t const ModSize, const uint8_t *const Mp,
                       const uint8_t *const RSqr, uint8_t *const Out);
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
#endif /** INC_PKA_H_ */
