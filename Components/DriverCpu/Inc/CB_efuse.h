/**
 * @file CB_efuse.h
 * @brief Header that contains API for Efuse driver.
 * @details Efuse driver also exposes some helper enums and structures to facilitate usage.
 * @author Chipsbank
 * @date 2024
 */
#ifndef INC_EFUSE_H_
#define INC_EFUSE_H_
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
/**
 * @brief Enumerator for potential error codes returned by Efuse API.
 */
typedef enum {
  EN_EFUSE_OK = 0u,             /*!< Operation successful */
  EN_EFUSE_INVALID_ADDR = 1u,   /*!< Inaccesible efuse array word */
  EN_EFUSE_WRITE_PROTECTED = 2u,/*!< Word cannot be overwritten*/
  EN_EFUSE_READ_PROTECTED = 3u, /*!< Word cannot be read. Probably sensitive information */
  EN_EFUSE_LOCKED = 4u,         /*!< Word is locked from reading and writing*/
  EN_EFUSE_INVALID_BITPOS = 5u, /*!< Setting an invalid bit. Each efuse word is 32 bits */
  EN_EFUSE_UNKNOWN_ERR = 6u,    /*!< Other misc/non-efuse erros */
  EN_EFUSE_SPECIAL_WORDS = 7u,  /*!< Direct writes to word 0,1,2 are limited */
} enEfuseErrCode;
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
 * @brief Enable QSPI AES encryption mode.
 * @details Once enabled, system will no longer 
 * be able to go back to non-encryption mode.
 */
void cb_efuse_qspi_flash_encryption_enable(void);

/**
 * @brief Function to check if QSPI AES encryption is enabled.
 * @details Return value is invalid if QSPI Setting is locked.
 * @see cb_efuse_qspi_flash_encryption_lock()
 * @return uint8_t a value of 1 for aes-enabled, and 0 for none.
 */
uint8_t cb_efuse_qspi_flash_encryption_is_enabled(void);

/**
 * @brief Locks the QSPI encryption setting. No further read/write possible.
 * @details Reads being disabled also means you can't reliably check if
 * aes is enabled anymore.
 * @see cb_efuse_qspi_flash_encryption_is_enabled()
 * @return uint8_t a value of 1 for aes-enabled, and 0 for none.
 */
void cb_efuse_qspi_flash_encryption_lock(void);

/**
 * @brief Write the 128-bit AES key and seed value to eFuse.
 * @details Once written, the set bits in eFuse cannot be cleared anymore; key and seed will be permanently burned.
 * @param secretKey A uint32_t array of size 4 to represent 128-bit AES key.
 * @param seed Auxiliary seed parameter for eFuse key burning operation.
 */
void cb_efuse_qspi_flash_encryption_key_write(uint32_t secretKey[static 4], uint32_t seed);

/**
 * @brief Read the 128-bit AES key.
 * @param outKey A 4-byte unsigned integer array of size 4 to store the
 * read result.
 */
void cb_efuse_qspi_flash_encryption_key_read(uint32_t outKey[static 4]);

/**
 * @brief Forbids any further read/write to the aes key.
 */
void cb_efuse_qspi_flash_encryption_key_lock(void);

/**
 * @brief Retrieve the chip id word3 information.
 * @return uint32_t A 4-byte integer
 */
uint64_t cb_efuse_read_chip_id(void);

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wvla"
#endif

/**
 * @brief Set the specified bits in user config 2.
 * @param bitPosNum The number of bits to be set.
 * @param bitPosArr The array containing the bits to be set, must be of size bitPosNum.
 */
void cb_efuse_userconfig2_set_bits(uint8_t bitPosNum, uint8_t bitPosArr[static bitPosNum]);

/**
 * @brief Read user config 2.
 * @return uint32_t A 4-byte integer that is the user config 2.
 */
uint32_t cb_efuse_userconfig2_read(void);

/**
 * @brief Forbids further read/write to user config 2.
 */
void cb_efuse_userconfig2_lock(void);
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
#endif /** INC_EFUSE_H_ */
