/**
 * @file CB_efuse.c
 * @brief Implementation file for efuse driver.
 * @details Implements the public API. This file also implements some private
 *          functions exclusively by the driver.
 * @author Chipsbank
 * @date 2024
 */
//-------------------------------
// INCLUDE
//-------------------------------
#include "CB_efuse.h"
#include "CB_scr.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include "string.h"
#include "NonLIB_sharedUtils.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// DEFINE
//-------------------------------
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wvla"
#endif
#define DELAY cb_hal_delay_in_us(88)
#define CB_DUMP 99
#define DEBUG_BUILD (CB_FALSE)
#if (DEBUG_BUILD == CB_TRUE || DEBUG_BUILD == CB_DUMP)
#include "app_uart.h"
#endif
#define AES_ENABLE_BIT (12u)
#define WORD0_LOCK_BIT (0u)
#define CHIP_ID_WORD_3  (3u)
#define CHIP_ID_WORD_10  (10u)
#define USER_CONFIG1_LOCK_BIT (10u)
#define USER_CONFIG2_WORD (11u)
#define USER_CONFIG2_LOCK_BIT (11u)

#define CB_EFUSE_UARTPRINT_ENABLE CB_FALSE
#if (CB_EFUSE_UARTPRINT_ENABLE == CB_TRUE)
  #include "app_uart.h"
  #define CB_EFUSE_PRINT(...) app_uart_printf(__VA_ARGS__)
#else
  #define CB_EFUSE_PRINT(...)
#endif
//-------------------------------
// ENUM
//-------------------------------
/**
 * @brief Enumerator for efuse mode of operation.
 * @details Error correction mechanism such as READ_COMP and PGM are not supported
 */
typedef enum {
  EN_EFUSE_MODE_READ_COMP = (1u << 3), /*!< Reads data from efuse array and compare to reference*/
  EN_EFUSE_MODE_PGM_ONLY = (1u << 2), /*!< Write-only, no redundancy mode */
  EN_EFUSE_MODE_READ_ONLY = (1u << 1), /*!< Read-only, no compare mode */
  EN_EFUSE_MODE_PGM = (1u << 0), /*!< Writes data and self-correct if anything goes wrong */
} enEfuseMode;

/**
 * @brief Enumerator for potential error codes returned by Efuse API.
 * @details Values of each enum doesn't mean much, just for ease of debugging
 */
typedef enum {
  EN_EFUSE_OK = 0u, /*!< Operation successful */
  EN_EFUSE_INVALID_ADDR = 1u, /*!< Inaccesible efuse array word */
  EN_EFUSE_WRITE_PROTECTED = 2u, /*!< Word cannot be overwritten*/
  EN_EFUSE_READ_PROTECTED = 3u, /*!< Word cannot be read. Probably sensitive information */
  EN_EFUSE_LOCKED = 4u, /*!< Word is locked from reading and writing*/
  EN_EFUSE_INVALID_BITPOS = 5u, /*!< Setting an invalid bit. Each efuse word is 32 bits */
  EN_EFUSE_UNKNOWN_ERR = 6u, /*!< Other misc/non-efuse erros */
  EN_EFUSE_SPECIAL_WORDS = 7u, /*!< Direct writes to word 0,1,2 are limited */
} enEfuseErrCode;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief Structure containing status of an Efuse operation.
 */
typedef struct {
  uint8_t numWrongBits; /*!< Number of bits being read/written wrongly */
  uint8_t programFailed; /*!< Writing failed */
  uint8_t prevOpDone; /*!< All previous operations completed */
} stEfuseStatus;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stEfuse_TypeDef *EFUSE = (stEfuse_TypeDef *)EFUSE_BASE_ADDR;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static inline enEfuseErrCode cb_efuse_init(void);
static inline void cb_efuse_configure_mode(enEfuseMode mode);
static inline enEfuseErrCode cb_efuse_configure(uint32_t start, uint32_t end);
static inline enEfuseErrCode cb_efuse_deinit(void);
static inline enEfuseErrCode cb_efuse_check_addr_range(uint32_t addr);
static inline enEfuseErrCode cb_efuse_read_word(uint32_t word, uint32_t *out);
static inline enEfuseErrCode cb_efuse_write_word(uint32_t word, uint32_t data);
static inline enEfuseErrCode cb_efuse_set_bit(uint32_t word, uint32_t bitPos);
static inline enEfuseErrCode cb_efuse_set_bits(uint32_t word, uint8_t* bitPosArr, uint8_t bitPosNum);
static inline stEfuseStatus cb_efuse_get_status(void);
static inline void cb_efuse_dump(void);

#if (DEBUG_BUILD == CB_TRUE)
static inline const char* CB_EFUSE_GetErrString(enEfuseErrCode);
#endif
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Enable QSPI AES encryption mode.
 * @details Once enabled, system will no longer 
 * be able to go back to non-encryption mode.
 */
void cb_efuse_qspi_flash_encryption_enable(void) 
{
	enEfuseErrCode err;
	err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
	CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
	(void)(err);
#endif

	err = cb_efuse_set_bit(0, AES_ENABLE_BIT);
#if (DEBUG_BUILD == CB_TRUE)
	CB_EFUSE_PRINT("  [CB_Efuse] Set Bit %d: %s\n", AES_ENABLE_BIT, CB_EFUSE_GetErrString(err));
#else
	(void)(err);
#endif
  
	err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
	CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
	(void)(err);
#endif
}

/**
 * @brief Function to check if QSPI AES encryption is enabled.
 * @details Return value is invalid if QSPI Setting is locked.
 * @see cb_efuse_qspi_flash_encryption_lock()
 * @return uint8_t a value of 1 for aes-enabled, and 0 for none.
 */
uint8_t cb_efuse_qspi_flash_encryption_is_enabled(void) 
{
    enEfuseErrCode err;
	err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
	CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
	(void)(err);
#endif

    uint32_t result;
	err = cb_efuse_read_word(0, &result);
#if (DEBUG_BUILD == CB_TRUE)
	CB_EFUSE_PRINT("  [CB_Efuse] Read Word 0: %x, %s\n", result, CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
	CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
	(void)(err);
#endif
    return (uint8_t)((result >> 12) & 1u); 
}

/**
 * @brief Locks the QSPI encryption setting. No further read/write possible.
 * @details Reads being disabled also means you can't reliably check if
 * aes is enabled anymore.
 * @see cb_efuse_qspi_flash_encryption_is_enabled()
 * @return uint8_t a value of 1 for aes-enabled, and 0 for none.
 */
void cb_efuse_qspi_flash_encryption_lock(void) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    err = cb_efuse_set_bit(0, WORD0_LOCK_BIT);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Set Bit %d: %s\n", WORD0_LOCK_BIT, CB_EFUSE_GetErrString(err));
    uint32_t dbg = 0;
    err = cb_efuse_read_word(0, &dbg);
    CB_EFUSE_PRINT("  [CB_Efuse] Read Word 0: %x, %s\n", dbg, CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    err = cb_efuse_deinit();
    uint32_t dmp;
    cb_efuse_read_word(0, &dmp);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
}

/**
 * @brief Write the 128-bit AES key.
 * @details Once written, the set bit won't be able to clear anymore.
 * @param secretKey A 4-byte unsigned integer array of size 4 
 * to represent 128-bit AES key.
 */
void cb_efuse_qspi_flash_encryption_key_write(uint32_t secretKey[static 4]) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    for (uint32_t word = 5; word < 9; ++word) 
    {
        err = cb_efuse_write_word(word, secretKey[word-5]);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Write Word %d: %s\n", word, CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
       DELAY;
    }
  
    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
}

/**
 * @brief Read the 128-bit AES key.
 * @param outKey A 4-byte unsigned integer array of size 4 to store the
 * read result.
 */
void cb_efuse_qspi_flash_encryption_key_read(uint32_t outKey[static 4]) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
  
    for (uint32_t word = 5; word < 9; ++word) 
    {
        err = cb_efuse_read_word(word, &outKey[word - 5]);
#if (DEBUG_BUILD == CB_TRUE)
        CB_EFUSE_PRINT("  [CB_Efuse] Read Word %d: %s\n", word, CB_EFUSE_GetErrString(err));
#else
        (void)(err);
#endif
	}

    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
}

/**
 * @brief Forbids any further read/write to the aes key.
 */
void cb_efuse_qspi_flash_encryption_key_lock(void) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    uint8_t wordsToLock[4] = {5,6,7,8};
    err = cb_efuse_set_bits(0, wordsToLock, 4);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Set Bits: %s\n", CB_EFUSE_GetErrString(err));
    uint32_t dbg = 0;
    err = cb_efuse_read_word(0, &dbg);
    CB_EFUSE_PRINT("  [CB_Efuse] Read 0: %x, %s\n", dbg, CB_EFUSE_GetErrString(err));
    for (int i = 0; i < 4; ++i) 
    {
        err = cb_efuse_read_word(wordsToLock[i], &dbg);
        CB_EFUSE_PRINT("  [CB_Efuse] Read %d: %x, %s\n", wordsToLock[i], dbg, CB_EFUSE_GetErrString(err));
    }
#else
    (void)(err);
#endif

    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
}

/**
 * @brief Retrieve the chip id word3 information.
 * @return uint32_t A 4-byte integer
 */
uint32_t cb_efuse_read_chip_id_w3(void) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    uint32_t chipId;
    err = cb_efuse_read_word(CHIP_ID_WORD_3, &chipId);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Read ChipId: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
  
    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    return chipId;
}

/**
 * @brief Retrieve the chip id word10 information.
 * @return uint32_t A 4-byte integer
 */
uint32_t cb_efuse_read_chip_id_w10(void) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    uint32_t chipId;
    err = cb_efuse_read_word(CHIP_ID_WORD_10, &chipId);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Read ChipId: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    return chipId;
}

/**
 * @brief Retrieve the chip id information.
 * @return uint64_t A 8-byte integer
 */
uint64_t cb_efuse_read_chip_id(void)
{
    uint64_t chipId;
    uint64_t chipID_W3, chipID_W10;

    chipID_W3  = cb_efuse_read_chip_id_w3();
    chipID_W10 = cb_efuse_read_chip_id_w10();

    chipId = chipID_W3 + (chipID_W10<<32);

    #if (DEBUG_BUILD == CB_TRUE)
        CB_EFUSE_PRINT("  [CB_Efuse] Read ChipId W3:  %llx\n", chipID_W3);
        CB_EFUSE_PRINT("  [CB_Efuse] Read ChipId W10: %llx\n", chipID_W10);
        CB_EFUSE_PRINT("  [CB_Efuse] Read ChipId :    %llx\n", chipId);
    #endif

    return chipId;
}

/**
 * @brief Set the specified bits in user config 2.
 * @param bitPosNum The number of bits to be set.
 * @param bitPosArr The array containing the bits to be set, must be of size bitPosNum.
 */
void cb_efuse_userconfig2_set_bits(uint8_t bitPosNum, uint8_t bitPosArr[static bitPosNum]) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    err = cb_efuse_set_bits(USER_CONFIG2_WORD, bitPosArr, bitPosNum);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Set Bits: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
}

/**
 * @brief Read user config 2.
 * @return uint32_t A 4-byte integer that is the user config 2.
 */
uint32_t cb_efuse_userconfig2_read(void) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    uint32_t result;
    err = cb_efuse_read_word(USER_CONFIG2_WORD, &result);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Read Word %d: %s\n", USER_CONFIG2_WORD, CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
    return result; 
}

/**
 * @brief Forbids further read/write to user config 2.
 */
void cb_efuse_userconfig2_lock(void) 
{
    enEfuseErrCode err;
    err = cb_efuse_init();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Init: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif

    uint32_t result;
    err = cb_efuse_set_bit(0, USER_CONFIG2_LOCK_BIT);
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Set Bit: %s\n", USER_CONFIG2_WORD, CB_EFUSE_GetErrString(err));
    uint32_t dbg = 0;
    err = cb_efuse_read_word(0, &dbg);
    CB_EFUSE_PRINT("  [CB_Efuse] Read 0: %x, %s\n", dbg, CB_EFUSE_GetErrString(err));
    err = cb_efuse_read_word(USER_CONFIG2_WORD, &dbg);
    CB_EFUSE_PRINT("  [CB_Efuse] Read %d: %x, %s\n", USER_CONFIG2_WORD, dbg, CB_EFUSE_GetErrString(err));
#else
    (void)(err);
    (void)(result);
#endif

    err = cb_efuse_deinit();
#if (DEBUG_BUILD == CB_TRUE)
    CB_EFUSE_PRINT("  [CB_Efuse] Deinit: %s\n", CB_EFUSE_GetErrString(err));
#else
    (void)(err);
#endif
}

/**
 * @brief Enables the efuse.
 */
static inline enEfuseErrCode cb_efuse_init(void) 
{
    cb_scr_efuse_module_on();
    return EN_EFUSE_OK;
}

/**
 * @brief Disables the efuse.
 * @return enEfuseErrCode error code of the de-init operation.
 */
static inline enEfuseErrCode cb_efuse_deinit(void) 
{
    cb_scr_efuse_module_off();
    return EN_EFUSE_OK;
}


/**
 * @brief Read from a specific word within efuse.
 * @param word Word to read from.
 * @param out Output to store the read result.
 * @return enEfuseErrCode the error code of the read operation.
 */
static inline enEfuseErrCode cb_efuse_read_word(uint32_t word, uint32_t *out) 
{
    if (word > 11) 
    {
        return EN_EFUSE_INVALID_ADDR;
    }

    cb_efuse_configure_mode(EN_EFUSE_MODE_READ_ONLY);
    EFUSE->ctrl |= (1u << 0);
    EFUSE->regAddr = word;
    *out = EFUSE->regRd;
    stEfuseStatus status = cb_efuse_get_status();
    while (status.prevOpDone != CB_TRUE) 
    {
        status = cb_efuse_get_status();
    }
    return EN_EFUSE_OK;
}

/**
 * @brief Write to a specific efuse operation.
 * @param word Word to write to.
 * @param data Data to write.
 * @return enEfuseErrCode the error code of the write operation.
 */
static inline enEfuseErrCode cb_efuse_write_word(uint32_t word, uint32_t data) 
{
    enEfuseErrCode err;
    stEfuseStatus stat;
    cb_efuse_configure_mode(EN_EFUSE_MODE_PGM_ONLY);

    err = cb_efuse_configure(word, word);
    if(err != EN_EFUSE_OK) return err;

    err = cb_efuse_check_addr_range(word);
    if (err != EN_EFUSE_OK) 
    {
        return err;
    }

    EFUSE->regAddr = word;
    EFUSE->regWr = data;
    EFUSE->ctrl = 1;
    stat = cb_efuse_get_status();
    while (stat.prevOpDone != 1) 
    {
        stat = cb_efuse_get_status();
    }
    return EN_EFUSE_OK;
}

/**
 * @brief Set 1 specific bit in a specific word in the efuse.
 * @param word Word to operate on.
 * @param bitPos Bit position of the word specified to be set.
 * @return enEfuseErrCode the error code of the set-bit operation.
 */
static inline enEfuseErrCode cb_efuse_set_bit(uint32_t word, uint32_t bitPos) 
{
    if (word > 11) 
    {
        return EN_EFUSE_INVALID_ADDR;
    }
    if (bitPos > 31) 
    {
        return EN_EFUSE_INVALID_BITPOS;
    }
    EFUSE->pgmBit |= (1u << 1);
    EFUSE->pgmBit = (EFUSE->pgmBit & ~0x3Cu)| ((word & 0xF) << 2);
    EFUSE->pgmBit = (EFUSE->pgmBit & ~0x7C0u) | ((bitPos & 0x1F) << 6);
    EFUSE->pgmBit |= (1u << 0);
    stEfuseStatus stat = cb_efuse_get_status();
    while (stat.prevOpDone != 1) 
    {
        stat = cb_efuse_get_status();
    }
    return EN_EFUSE_OK;
}

/**
 * @brief Set a number of bits in a specific word in the efuse.
 * @param word Word to operate on.
 * @param bitPosArr Bit positions of the word specified to be set.
 * @param bitPosNum Number of bit positions specified.
 * @details bitPosNum needs to be equal to the size of bitPosArr.
 * @return enEfuseErrCode the error code of the set-bits operation.
 */
static inline enEfuseErrCode cb_efuse_set_bits(uint32_t word, uint8_t* bitPosArr, uint8_t bitPosNum) 
{
    if (word > 11) 
    {
        return EN_EFUSE_INVALID_ADDR;
    }
    for (uint8_t i = 0; i < bitPosNum; ++i) 
    {
        if (bitPosArr[i] > 31) return EN_EFUSE_INVALID_BITPOS;
    }
    EFUSE->pgmBit |= (1u << 1);
    for (uint8_t i = 0; i < bitPosNum; ++i) 
    {
#if (DEBUG_BUILD == CB_TRUE)
        CB_EFUSE_PRINT("Setting bit %d \n", bitPosArr[i]);
#endif
        EFUSE->pgmBit = (EFUSE->pgmBit & ~0x3Cu)| ((word & 0xF) << 2);
        EFUSE->pgmBit = (EFUSE->pgmBit & ~0x7C0u) | ((bitPosArr[i] & 0x1Fu) << 6);
        EFUSE->pgmBit |= (1u << 0);
        stEfuseStatus stat = cb_efuse_get_status();
        while (stat.prevOpDone != 1) 
        {
            stat = cb_efuse_get_status();
        }
    }
    return EN_EFUSE_OK;
}

/**
 * @brief Configures the start and end range of writable words.
 * @start Start of range.
 * @end End of range.
 * @return enEfuseErrCode error code for configuration operation.
 */
static inline enEfuseErrCode cb_efuse_configure(uint32_t start, uint32_t end) 
{
    if (start > 11 || end > 11 || start > end) 
    {
        return EN_EFUSE_INVALID_ADDR;
    }

    EFUSE->addr = (EFUSE->addr & ~0xFu) | (start & 0xF);
    EFUSE->addr = (EFUSE->addr & ~0xF0u) | ((end & 0xF) << 4);
    return EN_EFUSE_OK;
}

/**
 * @brief Check if the received word is within valid ranges
 * @param addr the word to be checked.
 * @return enEfuseErrCode error code for range check operation.
 */
static inline enEfuseErrCode cb_efuse_check_addr_range(uint32_t addr) 
{
    uint32_t lowerBound;
    uint32_t upperBound;

    if (addr > 11) 
    {
        return EN_EFUSE_INVALID_ADDR;
    }

    lowerBound = EFUSE->addr & 0xF;
    upperBound = (EFUSE->addr >> 4) & 0xF;

    if (addr > upperBound || addr < lowerBound) 
    {
        return EN_EFUSE_INVALID_ADDR;
    }
    return EN_EFUSE_OK;
}

/**
 * @brief Get the status of efuse.
 * @return stEfuseStatus the status structure.
 */
static inline stEfuseStatus cb_efuse_get_status(void) 
{
    return (stEfuseStatus){.programFailed = ((EFUSE->status >> 1) & 0x1),
                                                 .numWrongBits = ((EFUSE->status >> 2) & 0xF),
                                                 .prevOpDone = (EFUSE->status & 0x1)};
}

/**
 * @brief Configure the mode of efuse operation.
 * @details Can either be PGM (not working), READ_CMP (not working), PGM_ONLY, READ_ONLY
 * @param mode Efuse mode of choice.
 */
static inline void cb_efuse_configure_mode(enEfuseMode mode) 
{
    if (mode > 8) 
    {
        return;
    }
    EFUSE->mode &= ~(0xFu);
    EFUSE->mode |= mode;
}

#if (DEBUG_BUILD == CB_TRUE)
static inline const char *CB_EFUSE_GetErrString(enEfuseErrCode errCode) 
{
    switch (errCode) 
    {
        case EN_EFUSE_OK: {
            return "OK";
        }
        case EN_EFUSE_INVALID_ADDR: {
            return "ACCESSING INVALID ADDRESS, PLEASE RECONFIGURE";
        }
        case EN_EFUSE_WRITE_PROTECTED: {
            return "WORD IS WRITE-PROTECTED";
        }
        case EN_EFUSE_READ_PROTECTED: {
            return "WORD IS READ-PROTECTED";
        }
        case EN_EFUSE_LOCKED: {
            return "WORD IS LOCKED (WRITE+READ PROTECTED)";
        }
        case EN_EFUSE_UNKNOWN_ERR: {
            return "SOMETHING MIGHT BE NULL";
        }
        case EN_EFUSE_INVALID_BITPOS: {
            return "ACCESSING INVALID BIT POSITION, VALID IS [31:0]";
        }
        case EN_EFUSE_SPECIAL_WORDS: {
            return "WORD SHOULDN'T BE WRITTEN TO DIRECTLY DUE TO SPECIAL EFFECTS";
        }
    }
    return "UNHANDLED";
}
#endif /** Definition for GetErrString */

void cb_efuse_dump(void) 
{
#if (DEBUG_BUILD == CB_DUMP)
    for (int i = 0; i < 12; ++i) 
    {
       uint32_t dbg = 0;
       cb_efuse_read_word(i, &dbg);
       CB_EFUSE_PRINT("Word %d: %x\n", i, dbg);
    }
#endif
}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
