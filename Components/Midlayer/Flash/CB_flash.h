/**
 * @file    CB_flash.h
 * @brief   Header file for Flash (QSPI) operations.
 * @details This file defines structures, constants, and function prototypes for flash operations.
 * @author  Chipsbank
 * @date    2024
 */
#ifndef __CB_FLASH_H_
#define __CB_FLASH_H_


//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_qspi.h"
//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum
{
  EN_FLASH_SUCCESS = 0,
  EN_FLASH_OPERATION_FAILED = 1,
  EN_FLASH_OPERATION_UNSUPPORTED = 2,
  EN_FLASH_INVALID_ADDRESS = 3,
  EN_FLASH_UNINITIALIZED = 4,
} enFlashStatus;

typedef enum 
{
  EN_FLASH_VENDOR_UNKNOWN = 0,
  EN_FLASH_VENDOR_PUYA = 0x85,
  EN_FLASH_VENDOR_BOYA = 0x68,
  EN_FLASH_VENDOR_WINBOND = 0xEF,
  EN_FLASH_VENDOR_MACRONIX = 0xC2,
} enFlashVendorID;


typedef enum
{
  EN_FLASH_CAPACITY_UNKNOWN = 0, 
  EN_FLASH_CAPACITY_512KB = 0x13,
  EN_FLASH_CAPACITY_1MB = 0x14,
} enFlashCapacity;

//-------------------------------
// STRUCT/UNION SECTION

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/* Initialization and Configuration functions *********************************/
enFlashStatus cb_flash_init(void);
enFlashVendorID cb_flash_get_vendor(void);
enFlashCapacity cb_flash_get_capacity(void);

/*enable/disables privileged access functions**********************************/
void cb_flash_enter_elevation(void);
void cb_flash_exit_elevation(void);

/*Sets/resets burst read mode functions****************************************/
enFlashStatus cb_flash_configure_read_mode(enBurstReadAction BurstReadAction);

/* Flash erasing functions ****************************************************/
enFlashStatus cb_flash_erase_page(uint16_t PageNumber);
enFlashStatus cb_flash_erase_sector(uint16_t SectorNumber);
enFlashStatus cb_flash_erase_block32k(uint8_t BlockNumber);

/* Flash programming functions ************************************************/
enFlashStatus cb_flash_program_page(uint16_t PageNumber, uint8_t *data, uint16_t length);
enFlashStatus cb_flash_program_sector(uint16_t SectorNumber, uint8_t *data, uint16_t length);
enFlashStatus cb_flash_program_by_addr(uint32_t address, uint8_t *data, uint16_t length);

/* Flash read functions ******************************************************/
enFlashStatus cb_flash_read_page(uint16_t PageNumber, uint8_t *data, uint16_t length);
enFlashStatus cb_flash_read_sector(uint16_t SectorNumber, uint8_t *data, uint16_t length);
enFlashStatus cb_flash_read_by_addr(uint32_t address, uint8_t *data, uint16_t length);
enFlashStatus cb_flash_read_status_reg1(uint8_t *status_reg1);

#endif
