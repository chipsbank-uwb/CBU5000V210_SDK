/**
 * @file app_trng.h
 * @brief Header file exposing example functions for Trng usage.
 * @details Contains app_trng's function declarations.
 * @date 2024
 * @author Chipsbank
*/
#ifndef INC_APP_TRNG_H_
#define INC_APP_TRNG_H_
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
 * @brief Generate an RN using noise-seeded TRNG core.
 * @details DRBG runs on AES-256, with no additional input and with prediction resistance disabled.
 */
void app_trng_gen_with_noise(void);

/**
 * @brief Generate an RN using nonce-seeded TRNG core.
 * @details config.Alg can be changed to AES128 to change the algorithm chosen. Fixed seed gives fixed rn output stream.
 */
void app_trng_gen_with_nonce(void);

/**
 * @brief Generate an RN using nonce-seeded TRNG core, with additional input.
 * @detail The additional input will help add more unpredictability to the TRNG outputs.
 * @details One can also run noise-seeded TRNG core with additional input. Just call Snoise instead.
 */
void app_trng_gen_with_addin(void); // seeding can be either nonce or noise

/**
 * @brief Runs HealthCheck on TRNG core.
 */
void app_trng_run_kat(void); 

/**
 * @brief Main function for executing TRNG (True Random Number Generator) peripheral operations.
 */
void app_peripheral_trng_init(void);
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
#endif /** INC_APP_TRNG_H_ */
