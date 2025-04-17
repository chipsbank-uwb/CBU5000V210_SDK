/**
 * @file    CB_flash.c
 * @brief   Flash memory OTA operations for QSPI interface.
 * @details This file contains functions for reading from and writing to flash memory in APP memory regions.
 *          It includes functions for configuring flash commands.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_flash.h"
#include "CB_qspi.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include "CB_Common.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
/* Defines used in timeout mechanism */
extern uint32_t SystemCoreClock;
#define DEF_FLASH_TIMEOUT_MS              1000
#define DEF_FLASH_WIP_TIMEOUT_MS          700
#define DEF_FLASH_TIMEOUT_CPU_CYCLES      (((SystemCoreClock) / 1000U) * DEF_FLASH_TIMEOUT_MS)  // number of CPU cycles 
#define DEF_FLASH_WIP_TIMEOUT_CPU_CYCLES  (((SystemCoreClock) / 1000U) * DEF_FLASH_WIP_TIMEOUT_MS)  // number of CPU cycles 



/*Status Register*/
#define DEF_WIPBIT                                  0x01 /*bit[0] WIP*/


#define DEF_NON_REQUIRED 0

/* Unsupported command/operation */
#define DEF_COMMAND_UNSUPPORTED                     0x00

/* Common read JEDEC ID command */
#define DEF_READ_JEDEC_ID                           0x9F
#define DEF_JEDEC_ID_LENGTH                         3

/* Commond operational commands */
#define DEF_COMMON_SECTOR_ERASE                     0x20
#define DEF_COMMON_BLOCK32K_ERASE                   0x52
#define DEF_COMMON_QUAD_PAGE_PROGRAM                0x32
#define DEF_COMMON_4IO_READ                         0xEB
#define DEF_COMMON_READ_STATUS_REG_1                0x05
#define DEF_COMMON_READ_STATUS_REG_2                0x35
#define DEF_COMMON_WRITE_STATUS_REG_1               0x01
#define DEF_COMMON_WRITE_ENABLE                     0x06

/* PUYA QSPI FLASH USED COMMANDS */
#define DEF_PUYA_PAGE_ERASE                         0x81
#define DEF_PUYA_SECTOR_ERASE                       0x20
#define DEF_PUYA_BLOCK32K_ERASE                     0x52
#define DEF_PUYA_QUAD_PAGE_PROGRAM                  0x32
#define DEF_PUYA_4IO_READ                           0xEB
#define DEF_PUYA_READ_STATUS_REG_1                  0x05
#define DEF_PUYA_READ_STATUS_REG_2                  0x35
#define DEF_PUYA_WRITE_ENABLE                       0x06
#define DEF_PUYA_BURST_READ_COMMAND                 0x77
#define DEF_PUYA_BURST_READ_SET                     0x40
#define DEF_PUYA_BURST_READ_RESET                   0x10
#define DEF_PUYA_BURST_READ_DATA_MODE               EN_QSPI_QuadSPI_Data
#define DEF_PUYA_BURST_READ_DATA_LENGTH             4
#define DEF_PUYA_QUAD_PROGRAM_ADDR_MODE             EN_QSPI_NormalSPI_Addr
#define DEF_PUYA_WRITE_STATUS_REG_1                 0x01
#define DEF_PUYA_WRITE_STATUS_REG_2                 0x31

/* BOYA QSPI FLASH USED COMMANDS */
#define DEF_BOYA_PAGE_ERASE                         0x81
#define DEF_BOYA_SECTOR_ERASE                       0x20
#define DEF_BOYA_BLOCK32K_ERASE                     0x52
#define DEF_BOYA_QUAD_PAGE_PROGRAM                  0x32
#define DEF_BOYA_4IO_READ                           0xEB
#define DEF_BOYA_READ_STATUS_REG_1                  0x05
#define DEF_BOYA_READ_STATUS_REG_2                  0x35
#define DEF_BOYA_WRITE_ENABLE                       0x06
#define DEF_BOYA_BURST_READ_COMMAND                 0x77
#define DEF_BOYA_BURST_READ_SET                     0x40
#define DEF_BOYA_BURST_READ_RESET                   0x10
#define DEF_BOYA_BURST_READ_DATA_MODE               EN_QSPI_QuadSPI_Data
#define DEF_BOYA_BURST_READ_DATA_LENGTH             4
#define DEF_BOYA_QUAD_PROGRAM_ADDR_MODE             EN_QSPI_NormalSPI_Addr

/* WINBOND QSPI FLASH USED COMMANDS */
#define DEF_WINBOND_PAGE_ERASE                      DEF_COMMAND_UNSUPPORTED
#define DEF_WINBOND_SECTOR_ERASE                    0x20
#define DEF_WINBOND_BLOCK32K_ERASE                  0x52
#define DEF_WINBOND_QUAD_PAGE_PROGRAM               0x32
#define DEF_WINBOND_4IO_READ                        0xEB
#define DEF_WINBOND_READ_STATUS_REG_1               0x05
#define DEF_WINBOND_READ_STATUS_REG_2               0x35
#define DEF_WINBOND_WRITE_ENABLE                    0x06
#define DEF_WINBOND_BURST_READ_COMMAND              0x77
#define DEF_WINBOND_BURST_READ_SET                  0x40
#define DEF_WINBOND_BURST_READ_RESET                0x10
#define DEF_WINBOND_BURST_READ_DATA_MODE            EN_QSPI_QuadSPI_Data
#define DEF_WINBOND_BURST_READ_DATA_LENGTH          4
#define DEF_WINBOND_QUAD_PROGRAM_ADDR_MODE          EN_QSPI_NormalSPI_Addr

/* MACRONIX QSPI FLASH USED COMMANDS */
#define DEF_MACRONIX_PAGE_ERASE                     DEF_COMMAND_UNSUPPORTED
#define DEF_MACRONIX_SECTOR_ERASE                   0x20
#define DEF_MACRONIX_BLOCK32K_ERASE                 0x52
#define DEF_MACRONIX_QUAD_PAGE_PROGRAM              0x38 /* THIS IS DIFFERENT FROM THE REST */
#define DEF_MACRONIX_4IO_READ                       0xEB
#define DEF_MACRONIX_READ_STATUS_REG_1              0x05
#define DEF_MACRONIX_READ_STATUS_REG_2              0x35
#define DEF_MACRONIX_WRITE_ENABLE                   0x06
#define DEF_MACRONIX_BURST_READ_COMMAND             0xC0
#define DEF_MACRONIX_BURST_READ_SET                 0x02    /* 32 bytes */
#define DEF_MACRONIX_BURST_READ_RESET               0x10
#define DEF_MACRONIX_BURST_READ_DATA_MODE           EN_QSPI_NormalSPI_Data
#define DEF_MACRONIX_BURST_READ_DATA_LENGTH         1
#define DEF_MACRONIX_QUAD_PROGRAM_ADDR_MODE         EN_QSPI_QuadSPI_Addr

/* BOYA BLOCK PROTECT PARAM */
#define DEF_BOYA_LOCK_MSK_1MB         (0x1F<<2) 
#define DEF_BOYA_LOCK_1MB             (0x07<<2)   //xx1xx             with (CMP=0)
#define DEF_BOYA_UNLOCK_1MB           (0x00<<2)   //xx000             with (CMP=0)

/* PUYA BLOCK PROTECT PARAM */
#define DEF_PUYA_LOCK_MSK_512KB       (0x1F<<2) 
#define DEF_PUYA_LOCK_512KB           (0x07<<2)   //0x1xx             with (WPS=0, CMP=0)
#define DEF_PUYA_UNLOCK_512KB         (0x00<<2)   //xx000             with (WPS=0, CMP=0)

/* MACRONIX BLOCK PROTECT PARAM */
#define DEF_MACRONIX_LOCK_MSK_512KB   (0x0F<<2)
#define DEF_MACRONIX_LOCK_512KB       (0x0F<<2)   //1111              with (TB=0)
#define DEF_MACRONIX_UNLOCK_512KB     (0x00<<2)   //0000              with (TB=0)

#define DEF_MACRONIX_LOCK_MSK_1MB     (0x0F<<2)
#define DEF_MACRONIX_LOCK_1MB         (0x0F<<2)   //1111              with (TB=0)
#define DEF_MACRONIX_UNLOCK_1MB       (0x00<<2)   //0000              with (TB=0)

/* WINBOND BLOCK PROTECT PARAM */
#define DEF_WINBOND_LOCK_MSK_512KB    (0x1F<<2)
#define DEF_WINBOND_LOCK_512KB        (0x07<<2)   //0x1xx             with (CMP=0)
#define DEF_WINBOND_UNLOCK_512KB      (0x00<<2)   //xx000             with (CMP=0)

#define DEF_WINBOND_LOCK_MSK_1MB      (0x1F<<2)
#define DEF_WINBOND_LOCK_1MB          (0x07<<2)   //xx111             with (CMP=0)
#define DEF_WINBOND_UNLOCK_1MB        (0x00<<2)   //xx000             with (CMP=0)

//--------------- Address definitions -------------------//
// TODO: double check all start and end address
#define RESERVED_START_ADDR     0x00000000UL
#define RESERVED_END_ADDR       0x00000FFFUL

#define BOOTLOADER_START_ADDR   0x00001000UL
#define BOOTLOADER_END_ADDR     0x00002FFFUL

#define DEF_FLASH_PAGE_SIZE         256   // page size in bytes
#define DEF_FLASH_SECTOR_SIZE       (DEF_FLASH_PAGE_SIZE * 16)  // sector size in bytes
#define DEF_FLASH_BLOCK32K_SIZE     (DEF_FLASH_SECTOR_SIZE * 8) // block 32k size in bytes
#define DEF_FLASH_BLOCK64K_SIZE     (DEF_FLASH_SECTOR_SIZE * 16) // block 64k size in bytes
#define DEF_FLASH_512KB_SIZE        0x80000   // 512KB flash size in bytes
#define DEF_FLASH_1MB_SIZE          0x100000  // 1MB flash size in bytes

#define DEF_NUM_OF_PAGE_IN_A_SECTOR         16
#define DEF_NUM_OF_SECTOR_IN_A_BLOCK32K     8
#define DEF_NUM_OF_SECTOR_IN_A_BLOCK64K     16

/* 512KB flash */
#define DEF_NUM_OF_SECTOR_IN_A_512K_FLASH     ((DEF_FLASH_512KB_SIZE) / (DEF_FLASH_SECTOR_SIZE))
#define DEF_NUM_OF_PAGES_IN_A_512K_FLASH      ((DEF_FLASH_512KB_SIZE) / (DEF_FLASH_PAGE_SIZE))
#define DEF_NUM_OF_BLOCK32K_IN_A_512K_FLASH   ((DEF_FLASH_512KB_SIZE) / (DEF_FLASH_BLOCK32K_SIZE))

/* 1MB flash */
#define DEF_NUM_OF_SECTOR_IN_A_1MB_FLASH      ((DEF_FLASH_1MB_SIZE) / (DEF_FLASH_SECTOR_SIZE))
#define DEF_NUM_OF_PAGES_IN_A_1MB_FLASH       ((DEF_FLASH_1MB_SIZE) / (DEF_FLASH_PAGE_SIZE))
#define DEF_NUM_OF_BLOCK32K_IN_A_1MB_FLASH    ((DEF_FLASH_1MB_SIZE) / (DEF_FLASH_BLOCK32K_SIZE))

// Unit: Page numnber
#define DEF_USERCONFIG_SIZE_IN_PAGES          (1)


#define DEF_LIMITED_ACCESS_PAGES_START        (DEF_NUM_OF_PAGE_IN_A_SECTOR)   // 16 pages/4 KB
#define DEF_EXTENDED_ACCESS_PAGES_START       (1)

// Unit: Sector number
#define DEF_USERCONFIG_SIZE_IN_SECTOR          (1)   // 1 sector / 4 KB
#define DEF_BOOTLOADER_SIZE_IN_SECTOR          (3)   // 3 sector / 12 KB
#define DEF_NON_RESTRICTED_SECTOR_START        (DEF_USERCONFIG_SIZE_IN_SECTOR)

// Unit: Block number
#define DEF_NON_RESTRICTED_32KBLOCK_START          (1) // User config page located at 1st sector

// Unit: bytes - address operation limits
#define DEF_MAX_PROGRAM_SIZE                DEF_FLASH_SECTOR_SIZE    // limit program by address by 1 sector size (4KB)
#define DEF_MAX_READ_SIZE                   DEF_FLASH_SECTOR_SIZE    // limit read by address by 1 sector size (4KB)    

#define DEF_LIMITED_ACCESS_ADDR_START       (DEF_USERCONFIG_SIZE_IN_SECTOR * DEF_FLASH_SECTOR_SIZE) /* First sector forbidden for unelevated access */
#define DEF_EXTENDED_ACCESS_ADDR_START      (DEF_USERCONFIG_SIZE_IN_PAGES * DEF_FLASH_PAGE_SIZE)

#define DEF_READ_CHUNK_SIZE                 32                        // limit read chunk to 32 bytes to prevent set and reset of burst read mode

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/* Initialized (flash-specific) command banks */
typedef struct 
{
  /* Identity */
  enFlashVendorID flashVendorID;

  /* Erase command codes */
  uint8_t erasePageCommand;
  uint8_t eraseSectorCommand;
  uint8_t earseBlock32kCommand;

  /* Program command codes */
  uint8_t programPageCommand;
  enAddrMode programAddrMode; /* addressing mode in quad page program */

  /* Read command codes */
  uint8_t readCommand;

  /* Read status register codes */
  uint8_t readStatusReg1;
  uint8_t readStatusReg2;

  /* Write-enable */
  uint8_t writeEnableCommand;

  /* Burst read set and reset */
  uint8_t burstReadCommand;
  uint8_t burstReadSet;
  uint8_t burstReadReset;
  enDataMode burstReadCommandDataMode;
  uint16_t burstReadCommandDataLen;
  
  /* Write status register codes */
  uint8_t writeStatusReg1;
  
} stFlashCommands;

/* Initialized (flash-specific) Block Protect Value */
typedef struct
{
  uint8_t bp_mask;
  uint8_t bp_lock_param;
  uint8_t bp_unlock_param;
} stFlashBPparam;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stQSPI_HandleTypeDef *pQSPI = (stQSPI_HandleTypeDef *) QSPI_BASE_ADDR;

/* CONSTATNS - supported flash vendors */
static const enFlashVendorID CONST_SUPPORTED_VENDOR_ID[] =
{
  EN_FLASH_VENDOR_BOYA,
  EN_FLASH_VENDOR_PUYA,
  EN_FLASH_VENDOR_MACRONIX,
  EN_FLASH_VENDOR_WINBOND,
};

/* CONSTANTS - supported flash capacity */
static const enFlashCapacity CONST_SUPPORTED_FLASH_CAPACITY[] =
{
  EN_FLASH_CAPACITY_512KB,
  EN_FLASH_CAPACITY_1MB,
};

/* Runtime-initialized flash identities */
static enFlashVendorID flashVendorID = EN_FLASH_VENDOR_UNKNOWN;
static enFlashCapacity flashCapacity = EN_FLASH_CAPACITY_UNKNOWN;
static stFlashCommands flashCommands;
static stFlashBPparam flash_bp_param;

/*Runtime-initialized flash limits */
static uint32_t RUNTIME_NON_RESTRICTED_ADDR_START;
static uint32_t RUNTIME_NON_RESTRICTED_ADDR_END;
static uint16_t RUNTIME_NON_RESTRICTED_PAGES_START;
static uint16_t RUNTIME_NON_RESTRICTED_PAGES_END;
static uint16_t RUNTIME_NON_RESTRICTED_SECTOR_START;
static uint16_t RUNTIME_NON_RESTRICTED_SECTOR_END;
static uint8_t RUNTIME_NON_RESTRICTED_BLOCK32K_START;
static uint8_t RUNTIME_NON_RESTRICTED_BLOCK32K_END;

/* Variables used in timeout mechanism */
static uint32_t FLASH_timeoutStartCPUCycle;
static uint32_t FLASH_timeoutElapsedCPUCycles;
static uint32_t FLASH_timeoutWIPStartCPUCycle;
static uint32_t FLASH_timeoutWIPElapsedCPUCycles;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static CB_STATUS cb_flash_write_enable(void);
static uint8_t cb_flash_check_wip(void);
void cb_flash_block_protect_init(void);
enFlashStatus cb_flash_lock(void);
enFlashStatus cb_flash_unlock(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief This function converts page number to physical address
 * @detail Valid page number range from start address of APP_CURRENT to
 *         end address of APP_NEW
 * @return Physical address if a valid page number, 0xFFFFFFFF if invalid
 */
static uint32_t cb_flash_page_num_to_addr(uint16_t PageNumber)
{ 
  return (uint32_t) (PageNumber*DEF_FLASH_PAGE_SIZE);
}

/**
 * @brief This function converts sector number to physical address
 * @detail Valid sector number range from start address of APP_CURRENT to
 *         end address of APP_NEW
 * @return Physical address if a valid page number, 0xFFFFFFFF if invalid
 */
static uint32_t cb_flash_sector_num_to_addr(uint16_t SectorNumber)
{   
  return (uint32_t) (SectorNumber*DEF_FLASH_SECTOR_SIZE);
}

/**
 * @brief  This function converts sector number to physical address
 * @detail Valid sector number range from start address of APP_CURRENT to
 *         end address of APP_NEW
 *         App address starts inside Block 0 region for block 32k option
 *         
 * @return Physical address if a valid page number, 0xFFFFFFFF if invalid
 */
static uint32_t cb_flash_block32k_num_to_addr(uint8_t BlockNumber)
{ 
  return (uint32_t) (BlockNumber*DEF_FLASH_BLOCK32K_SIZE);
}


/**
 * @brief  Waits for WIP bit to clear helper function
 * @detail This function waits for the WIP bit in the status register
 *         of the QSPI flash chip. Timeout will asserted if the WIP
 *         takes too long to clear
 *
 * @return CB_PASS: WIP has been cleared
 *         CB_FAIL: WIP not cleared and timeout asserted
 */
static CB_STATUS cb_flash_wait_for_wip_clear(void)
{
  uint8_t WIPSet = CB_TRUE;
  uint32_t startCPUCycle = DWT->CYCCNT;

  while (WIPSet == CB_TRUE)
  {
    WIPSet = cb_flash_check_wip();
    uint32_t ellapsedCycles = (DWT->CYCCNT < startCPUCycle) ? 
      (0xFFFFFFFF - startCPUCycle + DWT->CYCCNT + 1) : (DWT->CYCCNT - startCPUCycle);

    int64_t cycleDiff = (int64_t) ((int64_t)(ellapsedCycles) - (int64_t)(DEF_FLASH_TIMEOUT_CPU_CYCLES));
    
    if (cycleDiff > 0)
    {
      return CB_FAIL;
    }
  }
  return CB_PASS;
}

/**
 * @brief This function initializes flash vendor and capacity identity
 * @detail Valid vendor IDs and capacity IDs are in CONST_SUPPORTED_VENDOR_ID
 *         and CONST_SUPPORTED_CAPACITY. Before any FLASH related operation,
 *         user need to call cb_flash_init() once to be able to use the FLASH 
 *         APIs.
 *         The flash upper limit will only be intitialized in this function.
 *         
 * @return EN_FLASH_SUCCESS:          flash chip correctly recognized and initialized
 *         EN_FLASH_UNINITIALIZED:    flash chip not recognized/not supported
 *         EN_FLASH_OPERATION_FALED:  QSPI driver returned error
 */
enFlashStatus cb_flash_init(void)
{
  stQSPI_CmdTypeDef stConfigQSPICommand;
  CB_STATUS ret;
  uint8_t JEDEC_ID[DEF_JEDEC_ID_LENGTH];

  /* First - inialize to unknown */
  flashVendorID = EN_FLASH_VENDOR_UNKNOWN;
  flashCapacity = EN_FLASH_CAPACITY_UNKNOWN;

  /* Configure Command Start------------------------------------------ */
  stConfigQSPICommand.enFlashAcessArea        = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse        = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                 = DEF_READ_JEDEC_ID;
  stConfigQSPICommand.enAddrModeUse           = EN_QSPI_Unused_Addr;
  stConfigQSPICommand.Addr                    = DEF_NON_REQUIRED;
  stConfigQSPICommand.AddrLen                 = 0;
  stConfigQSPICommand.SpecialCommandByte1     = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2     = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles            = 0;
  stConfigQSPICommand.DataLen                 = DEF_JEDEC_ID_LENGTH;
  stConfigQSPICommand.enDataModeUse           = EN_QSPI_NormalSPI_Data;
  /* Configure Command End------------------------------------------ */

  ret = cb_qspi_read_data_without_addr(pQSPI, &stConfigQSPICommand, JEDEC_ID);

  if (ret == CB_FAIL)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  for (uint8_t i = 0; i < (sizeof(CONST_SUPPORTED_VENDOR_ID) / sizeof(enFlashVendorID)); i++)
  {
    if (CONST_SUPPORTED_VENDOR_ID[i] == JEDEC_ID[0])
    {
      flashVendorID = CONST_SUPPORTED_VENDOR_ID[i];
      break;
    }
  }

  for (uint8_t i = 0; i < (sizeof(CONST_SUPPORTED_FLASH_CAPACITY) / sizeof(enFlashCapacity)); i++)
  {
    if (CONST_SUPPORTED_FLASH_CAPACITY[i] == JEDEC_ID[2])
    {
      flashCapacity = CONST_SUPPORTED_FLASH_CAPACITY[i];
      break;
    }
  }

  /* return if identity not found */
  if (flashVendorID == EN_FLASH_VENDOR_UNKNOWN || flashCapacity == EN_FLASH_CAPACITY_UNKNOWN)
  {
    return EN_FLASH_UNINITIALIZED;
  }


  /* Fill-in command banks corresponding to flash vendor */
  flashCommands.flashVendorID         = flashVendorID;
  flashCommands.eraseSectorCommand    = DEF_COMMON_SECTOR_ERASE;
  flashCommands.earseBlock32kCommand  = DEF_COMMON_BLOCK32K_ERASE;
  flashCommands.readCommand           = DEF_COMMON_4IO_READ;
  flashCommands.readStatusReg1        = DEF_COMMON_READ_STATUS_REG_1;
  flashCommands.readStatusReg2        = DEF_COMMON_READ_STATUS_REG_2;
  flashCommands.writeStatusReg1       = DEF_COMMON_WRITE_STATUS_REG_1;
  flashCommands.writeEnableCommand    = DEF_COMMON_WRITE_ENABLE;

  switch (flashVendorID)
  {
    case EN_FLASH_VENDOR_UNKNOWN:
      return EN_FLASH_UNINITIALIZED;

    case EN_FLASH_VENDOR_BOYA:
      flashCommands.erasePageCommand          = DEF_BOYA_PAGE_ERASE;
      flashCommands.burstReadReset            = DEF_BOYA_BURST_READ_RESET;
      flashCommands.burstReadSet              = DEF_BOYA_BURST_READ_SET;
      flashCommands.programPageCommand        = DEF_BOYA_QUAD_PAGE_PROGRAM;
      flashCommands.programAddrMode           = DEF_BOYA_QUAD_PROGRAM_ADDR_MODE;
      flashCommands.burstReadCommandDataMode  = DEF_BOYA_BURST_READ_DATA_MODE;
      flashCommands.burstReadCommandDataLen   = DEF_BOYA_BURST_READ_DATA_LENGTH;
      flashCommands.burstReadCommand          = DEF_BOYA_BURST_READ_COMMAND;
      break;

    case EN_FLASH_VENDOR_PUYA:
      flashCommands.erasePageCommand          = DEF_PUYA_PAGE_ERASE;
      flashCommands.burstReadReset            = DEF_PUYA_BURST_READ_RESET;
      flashCommands.burstReadSet              = DEF_PUYA_BURST_READ_SET;
      flashCommands.programPageCommand        = DEF_PUYA_QUAD_PAGE_PROGRAM;
      flashCommands.programAddrMode           = DEF_PUYA_QUAD_PROGRAM_ADDR_MODE;
      flashCommands.burstReadCommandDataMode  = DEF_PUYA_BURST_READ_DATA_MODE; 
      flashCommands.burstReadCommandDataLen   = DEF_PUYA_BURST_READ_DATA_LENGTH;
      flashCommands.burstReadCommand          = DEF_PUYA_BURST_READ_COMMAND;
      break;

    case EN_FLASH_VENDOR_MACRONIX:
      flashCommands.erasePageCommand          = DEF_MACRONIX_PAGE_ERASE;
      flashCommands.burstReadReset            = DEF_MACRONIX_BURST_READ_RESET;
      flashCommands.burstReadSet              = DEF_MACRONIX_BURST_READ_SET;
      flashCommands.programPageCommand        = DEF_MACRONIX_QUAD_PAGE_PROGRAM;
      flashCommands.programAddrMode           = DEF_MACRONIX_QUAD_PROGRAM_ADDR_MODE;
      flashCommands.burstReadCommandDataMode  = DEF_MACRONIX_BURST_READ_DATA_MODE;
      flashCommands.burstReadCommandDataLen   = DEF_MACRONIX_BURST_READ_DATA_LENGTH;
      flashCommands.burstReadCommand          = DEF_MACRONIX_BURST_READ_COMMAND;
      break;
    
    case EN_FLASH_VENDOR_WINBOND:
      flashCommands.erasePageCommand          = DEF_WINBOND_PAGE_ERASE;
      flashCommands.burstReadReset            = DEF_WINBOND_BURST_READ_RESET;
      flashCommands.burstReadSet              = DEF_WINBOND_BURST_READ_SET;
      flashCommands.programPageCommand        = DEF_WINBOND_QUAD_PAGE_PROGRAM;
      flashCommands.programAddrMode           = DEF_WINBOND_QUAD_PROGRAM_ADDR_MODE;
      flashCommands.burstReadCommandDataMode  = DEF_WINBOND_BURST_READ_DATA_MODE;
      flashCommands.burstReadCommandDataLen   = DEF_WINBOND_BURST_READ_DATA_LENGTH;
      flashCommands.burstReadCommand          = DEF_WINBOND_BURST_READ_COMMAND;
      break;
  }

  /* Initialize runtime flash - upper limit */
  switch (flashCapacity)
  {
    case EN_FLASH_CAPACITY_UNKNOWN:
      return EN_FLASH_UNINITIALIZED;

    case EN_FLASH_CAPACITY_512KB:
      RUNTIME_NON_RESTRICTED_ADDR_END       = (DEF_FLASH_512KB_SIZE - 1);
      RUNTIME_NON_RESTRICTED_PAGES_END      = (DEF_NUM_OF_PAGES_IN_A_512K_FLASH - 1);
      RUNTIME_NON_RESTRICTED_SECTOR_END     = (DEF_NUM_OF_SECTOR_IN_A_512K_FLASH - 1);
      RUNTIME_NON_RESTRICTED_BLOCK32K_END   = (DEF_NUM_OF_BLOCK32K_IN_A_512K_FLASH - 1);
      break;

    case EN_FLASH_CAPACITY_1MB:
      RUNTIME_NON_RESTRICTED_ADDR_END       = (DEF_FLASH_1MB_SIZE - 1);
      RUNTIME_NON_RESTRICTED_PAGES_END      = (DEF_NUM_OF_PAGES_IN_A_1MB_FLASH - 1);
      RUNTIME_NON_RESTRICTED_SECTOR_END     = (DEF_NUM_OF_SECTOR_IN_A_1MB_FLASH - 1);
      RUNTIME_NON_RESTRICTED_BLOCK32K_END   = (DEF_NUM_OF_BLOCK32K_IN_A_1MB_FLASH - 1);
      break;
  }

  /* Initialize block protect parameters */
  cb_flash_block_protect_init();
  

  /* Initialize runtime flash - lower limit */
  RUNTIME_NON_RESTRICTED_ADDR_START     = DEF_LIMITED_ACCESS_ADDR_START;
  RUNTIME_NON_RESTRICTED_PAGES_START    = DEF_LIMITED_ACCESS_PAGES_START;
  RUNTIME_NON_RESTRICTED_SECTOR_START   = DEF_NON_RESTRICTED_SECTOR_START;
  RUNTIME_NON_RESTRICTED_BLOCK32K_START = DEF_NON_RESTRICTED_32KBLOCK_START;

  return EN_FLASH_SUCCESS;
}

/**
 * @brief  This function returns flash vendor ID
 * @detail Valid vendor IDs and capacity IDs are in CONST_SUPPORTED_VENDOR_ID.
 *         
 * @return flash chip vendor ID, indicated on enFlashVendorID enum and CONST_SUPPORTED_VENDOR_ID
 */
enFlashVendorID cb_flash_get_vendor(void)
{
  return flashVendorID;
}

/**
 * @brief  This function returns flash capacity ID
 * @detail Valid capacity IDs are in CONST_SUPPORTED_FLASH_CAPACITY.
 *         
 * @return flash chip capacity ID, indicated on enFlashCapacity enum and CONST_SUPPORTED_FLASH_CAPACITY
 */
enFlashCapacity cb_flash_get_capacity(void)
{
  return flashCapacity;
}

/**
 * @brief  This function enables privileged access to flash chip
 * @detail The lower limit of the flash chip will be extended until
 *         address 0x00000100.
 *         
 * @return None
 */
void cb_flash_enter_elevation(void)
{
  /* Elevate runtime flash - lower limit */
  RUNTIME_NON_RESTRICTED_ADDR_START   = DEF_EXTENDED_ACCESS_ADDR_START;
  RUNTIME_NON_RESTRICTED_PAGES_START  = DEF_EXTENDED_ACCESS_PAGES_START;
}

/**
 * @brief  This function disables privileged access to flash chip
 * @detail The lower limit of the flash chip will be limited until
 *         address 0x00001000.
 *         
 * @return None
 */
void cb_flash_exit_elevation(void)
{
  /* Elevate runtime flash - lower limit */
  RUNTIME_NON_RESTRICTED_ADDR_START   = DEF_LIMITED_ACCESS_ADDR_START;
  RUNTIME_NON_RESTRICTED_PAGES_START  = DEF_LIMITED_ACCESS_PAGES_START;
}

/**
 * @brief This function erase the contents in the corresponding page (256 bytes)
 * (set its bits to '1')
 * @detail Valid page number depending on whether user has entered elevation
 *         mode flash access or not, and depending on the flash capacity
 *
 * @param  PageNumber The page number to erase in flash
 *
 * @return EN_FLASH_UNINITIALIZED:          flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support page erase command
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error
 *         EN_FLASH_INVALID_ADDRESS         input page number is not allowed
 *         EN_FLASH_SUCCESS                 page erase successful
 *
 */
enFlashStatus cb_flash_erase_page(uint16_t PageNumber)
{
  CB_STATUS ret;
  stQSPI_CmdTypeDef  stConfigQSPICommand;

  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }
 
  /* Limit checking */
  if ((PageNumber < RUNTIME_NON_RESTRICTED_PAGES_START) || (PageNumber > RUNTIME_NON_RESTRICTED_PAGES_END))
  {
    return EN_FLASH_INVALID_ADDRESS;
  }

  /* Some flash chips don't support page erase operation. Check them */
  if (flashCommands.erasePageCommand == DEF_COMMAND_UNSUPPORTED)
  {
    return EN_FLASH_OPERATION_UNSUPPORTED;
  }
  
  /* unlock flash */
  if(cb_flash_unlock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }
  
  // feed the start address of the page, since any address within page boundary is valid
  uint32_t PageStartAddr = cb_flash_page_num_to_addr(PageNumber);
    
  /*Write Enable before Chip Erase*/
  cb_flash_write_enable();
  
  /* Configure Command Start------------------------------------------ */
  stConfigQSPICommand.enFlashAcessArea        = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse        = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                 = flashCommands.erasePageCommand; /*PAGE ERASE*/
  stConfigQSPICommand.enAddrModeUse           = EN_QSPI_NormalSPI_Addr;
  stConfigQSPICommand.Addr                    = PageStartAddr;
  stConfigQSPICommand.AddrLen                 = 3;    /*Length in Byte*/
  stConfigQSPICommand.SpecialCommandByte1     = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2     = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles            = 0;
  stConfigQSPICommand.enDataModeUse           = EN_QSPI_Unused_Data;
  stConfigQSPICommand.DataLen                 = 0;  
  /* Configure Command End------------------------------------------ */

  /* Send out configurations to QSPI driver */
  FLASH_timeoutStartCPUCycle = DWT->CYCCNT;
  while (1)  /*Re-try incase of WriteFail (Happened when QSPI is BUSY)*/
  {  
    ret = cb_qspi_write_command_and_addr(pQSPI, &stConfigQSPICommand);
    
    // successful driver operation
    if (ret == CB_PASS)
    {
      break;
    }

    /* Capture the current CPU cycle counter and check for timeout */
    FLASH_timeoutElapsedCPUCycles = (DWT->CYCCNT < FLASH_timeoutStartCPUCycle) ? 
        (0xFFFFFFFF - FLASH_timeoutStartCPUCycle + DWT->CYCCNT + 1) : (DWT->CYCCNT - FLASH_timeoutStartCPUCycle);
    
    int64_t cycleDiff = (int64_t) ((int64_t)(FLASH_timeoutElapsedCPUCycles) - (int64_t)(DEF_FLASH_TIMEOUT_CPU_CYCLES));

    // driver operation timed out
    if (cycleDiff > 0)
    {
      cb_flash_lock();
      return EN_FLASH_OPERATION_FAILED;
    }
  }
  
  /*Wait for Erase Operation Complete*/
  CB_STATUS WIPFlagClear = cb_flash_wait_for_wip_clear();
  if (WIPFlagClear == CB_FAIL)
  {
    cb_flash_lock();
    return EN_FLASH_OPERATION_FAILED;
  }
  
  /* lock flash */
  if(cb_flash_lock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }
  
  return EN_FLASH_SUCCESS;
}

/**
 * @brief  This function erase the contents in the corresponding sector (4KB)
 *         (set its bits to '1')
 * @detail Valid sector number will always starts from 1, regardless of whether
 *         user has entered elevated access or not.
 *
 * @param  SectorNumber The sector number to erase in flash
 *
 * @return EN_FLASH_UNINITIALIZED:          flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support sector erase command
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error
 *         EN_FLASH_INVALID_ADDRESS         input sector number is not allowed
 *         EN_FLASH_SUCCESS                 sector erase successful
 *
 */
enFlashStatus cb_flash_erase_sector(uint16_t SectorNumber)
{
  CB_STATUS ret;
  stQSPI_CmdTypeDef stConfigQSPICommand;

  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }

  /* Limit checking */
  if ((SectorNumber < RUNTIME_NON_RESTRICTED_SECTOR_START) || (SectorNumber > RUNTIME_NON_RESTRICTED_SECTOR_END))
  {
    return EN_FLASH_INVALID_ADDRESS;
  }
  
  /* Some flash chips don't support sector erase operation. Check them */
  if (flashCommands.eraseSectorCommand == DEF_COMMAND_UNSUPPORTED)
  {
    return EN_FLASH_OPERATION_UNSUPPORTED;
  }
  
  /* unlock flash */
  if(cb_flash_unlock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  // feed the start address of sector, since any address within sector boundary is valid
  uint32_t SectorStartAddr = cb_flash_sector_num_to_addr(SectorNumber);

  /*Write enable before Chip Erase*/
  cb_flash_write_enable();
  
  /* Configure Command Start --------------------------------------- */
  stConfigQSPICommand.enFlashAcessArea    = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse    = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command             = flashCommands.eraseSectorCommand;
  stConfigQSPICommand.enAddrModeUse       = EN_QSPI_NormalSPI_Addr;
  stConfigQSPICommand.Addr                = SectorStartAddr;
  stConfigQSPICommand.AddrLen             = 3;
  stConfigQSPICommand.SpecialCommandByte1 = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2 = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles        = 0;
  stConfigQSPICommand.enDataModeUse       = EN_QSPI_Unused_Data;
  stConfigQSPICommand.DataLen             = 0;
  /* Configure Command End   --------------------------------------- */
  
  /* Send out configurations to QSPI driver */
  FLASH_timeoutStartCPUCycle = DWT->CYCCNT;
  while (1) /* Retry until erase success */
  {
    ret = cb_qspi_write_command_and_addr(pQSPI, &stConfigQSPICommand);
    if (ret == CB_PASS)
    {
      break;
    }

    /* Capture the current CPU cycle counter and check for timeout */
    FLASH_timeoutElapsedCPUCycles = (DWT->CYCCNT < FLASH_timeoutStartCPUCycle) ? 
        (0xFFFFFFFF - FLASH_timeoutStartCPUCycle + DWT->CYCCNT + 1) : (DWT->CYCCNT - FLASH_timeoutStartCPUCycle);

    int64_t cycleDiff = (int64_t) ((int64_t)(FLASH_timeoutElapsedCPUCycles) - (int64_t)(DEF_FLASH_TIMEOUT_CPU_CYCLES));
    
    // driver operation failed / timed out
    if (cycleDiff > 0)
    {
      cb_flash_lock();
      return EN_FLASH_OPERATION_FAILED;
    }
  }
  
  /* Wait for erase Operation complete */
  CB_STATUS WIPFlagClear = cb_flash_wait_for_wip_clear();
  if (WIPFlagClear == CB_FAIL)
  {
    cb_flash_lock();
    return EN_FLASH_OPERATION_FAILED;
  }
  
  /* lock flash */
  if(cb_flash_lock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  return EN_FLASH_SUCCESS;
}

/**
 * @brief This function erase the contents in the corresponding block (32KB)
 *        (set its bits to '1')
 * @detail Valid block number will always starts from 1, regardless of whether
 *         user has entered elevated access or not.
 * 
 * @param  BlockNumber input block number to erase
 *
 * @return EN_FLASH_UNINITIALIZED:          flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support block erase command
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error
 *         EN_FLASH_INVALID_ADDRESS         the input block number is not allowed
 *         EN_FLASH_SUCCESS                 block erase successful
 */
enFlashStatus cb_flash_erase_block32k(uint8_t BlockNumber)
{
  CB_STATUS ret;
  stQSPI_CmdTypeDef stConfigQSPICommand;

  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }

  /* Limit checking */
  if ((BlockNumber < RUNTIME_NON_RESTRICTED_BLOCK32K_START) || (BlockNumber > RUNTIME_NON_RESTRICTED_BLOCK32K_END))
  {
    return EN_FLASH_INVALID_ADDRESS;
  }

  /* Some flash chips don't support block erase operation. Check them */
  if (flashCommands.earseBlock32kCommand == DEF_COMMAND_UNSUPPORTED)
  {
    return EN_FLASH_OPERATION_UNSUPPORTED;
  }

  /* unlock flash */
  if(cb_flash_unlock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }
  
  uint32_t BlockAddr = cb_flash_block32k_num_to_addr(BlockNumber);


  /*Write enable before Chip Erase*/
  cb_flash_write_enable();
    
  /* Configure Command Start --------------------------------------- */
  stConfigQSPICommand.enFlashAcessArea    = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse    = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command             = flashCommands.earseBlock32kCommand;
  stConfigQSPICommand.enAddrModeUse       = EN_QSPI_NormalSPI_Addr;
  stConfigQSPICommand.Addr                = BlockAddr;
  stConfigQSPICommand.AddrLen             = 3;
  stConfigQSPICommand.SpecialCommandByte1 = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2 = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles        = 0;
  stConfigQSPICommand.enDataModeUse       = EN_QSPI_Unused_Data;
  stConfigQSPICommand.DataLen             = 0;
  /* Configure Command End   --------------------------------------- */
    
  /* Send out configurations to QSPI driver */
  FLASH_timeoutStartCPUCycle = DWT->CYCCNT;
  while (1) /* Retry until erase success */
  {
    ret = cb_qspi_write_command_and_addr(pQSPI, &stConfigQSPICommand);
    if (ret == CB_PASS)
    {
      break;
    }

    /* Capture the current CPU cycle counter and check for timeout */
    FLASH_timeoutElapsedCPUCycles = (DWT->CYCCNT < FLASH_timeoutStartCPUCycle) ? 
        (0xFFFFFFFF - FLASH_timeoutStartCPUCycle + DWT->CYCCNT + 1) : (DWT->CYCCNT - FLASH_timeoutStartCPUCycle);
    
    int64_t cycleDiff = (int64_t) ((int64_t)(FLASH_timeoutElapsedCPUCycles) - (int64_t)(DEF_FLASH_TIMEOUT_CPU_CYCLES));

    // driver operation failed / timed out
    if (cycleDiff > 0)
    {
      cb_flash_lock();
      return EN_FLASH_OPERATION_FAILED;
    }
  }
  
  /* Wait for erase Operation complete */
  CB_STATUS WIPFlagClear = cb_flash_wait_for_wip_clear();
  if (WIPFlagClear == CB_FAIL)
  {
    cb_flash_lock();
    return EN_FLASH_OPERATION_FAILED;
  }

  /* lock flash */
  if(cb_flash_lock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }
  
  return EN_FLASH_SUCCESS;
}

/**
 * @brief This function programs the contents in the corresponding page (256 bytes)
 * 
 * @detail Valid page number depending on whether user has entered elevation
 *         mode flash access or not, and depending on the flash capacity
 *         Maximum length can be accepted in page program is 256 bytes
 *
 * @param[in]  PageNumber The page number to program in flash
 * @param[in]  data       Pointer to the data buffer to program
 * @param[in]  length     Number of bytes to program into the page, maximum 256 bytes
 *
 * @return EN_FLASH_UNINITIALIZED:          flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support page program command
 *         EN_FLASH_INVALID_ADDRESS         input page number does not fall into allowed address region
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to program exceeded limit
 *         EN_FLASH_SUCCESS                 page program successful
 */
enFlashStatus cb_flash_program_page(uint16_t PageNumber, uint8_t *data, uint16_t length)
{
  stQSPI_CmdTypeDef  stConfigQSPICommand;
  CB_STATUS ret;

  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }

  // if page number is invalid
  if ((PageNumber < RUNTIME_NON_RESTRICTED_PAGES_START) || (PageNumber > RUNTIME_NON_RESTRICTED_PAGES_END))
  {
    return EN_FLASH_INVALID_ADDRESS;
  }

  /* Some flash chips don't support page program operation. Check them */
  if (flashCommands.programPageCommand == DEF_COMMAND_UNSUPPORTED)
  {
    return EN_FLASH_OPERATION_UNSUPPORTED;
  }

  // feed the start address of the page, since any address within page boundary is valid
  uint32_t PageStartAddr = cb_flash_page_num_to_addr(PageNumber);
  
  // if data length is too long for a page
  if (length > DEF_FLASH_PAGE_SIZE || length == 0)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  /* unlock flash */
  if(cb_flash_unlock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }
  
  /*Write Enable before Page Program*/
  cb_flash_write_enable();   

  /* Configure Command for Quad Page Program  -------------------------------------------- */
  stConfigQSPICommand.enFlashAcessArea        = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse        = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                 = flashCommands.programPageCommand;
  stConfigQSPICommand.enAddrModeUse           = flashCommands.programAddrMode;
  stConfigQSPICommand.Addr                    = PageStartAddr;  //Add offset from UserConfigSector
  stConfigQSPICommand.AddrLen                 = 3; /*Addr Len in byte*/
  stConfigQSPICommand.SpecialCommandByte1     = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2     = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles            = 0;
  stConfigQSPICommand.enDataModeUse           = EN_QSPI_QuadSPI_Data;
  stConfigQSPICommand.DataLen                 = (uint16_t) length;
  /* Configure Command for Quad Page Program End------------------------------------------ */

  ret = cb_qspi_write_data_with_addr(pQSPI, &stConfigQSPICommand,&data[0]);

  if (ret == CB_FAIL)
  {
    cb_flash_lock();
    return EN_FLASH_OPERATION_FAILED;
  }

  /* Timeout for polling WIP bit */
  CB_STATUS WIPFlagClear = cb_flash_wait_for_wip_clear();
  if (WIPFlagClear == CB_FAIL)
  {
    cb_flash_lock();
    return EN_FLASH_OPERATION_FAILED;
  }
  
  /* lock flash */
  if(cb_flash_lock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  return EN_FLASH_SUCCESS;
}

/**
 * @brief This function programs the contents in the corresponding sector (4096 bytes)
 * 
 * @detail Valid sector number to program will always be 1, regardless of user has 
 *         entered privileged mode access or not
 *         Maximum length can be accepted in sector program is 4096 bytes
 *
 * @param[in]  SectorNumber The sector number to program in flash
 * @param[in]  data         Pointer to the data buffer to program
 * @param[in]  length       Number of bytes to program into the sector, maximum 4096 bytes
 *
 * @return EN_FLASH_UNINITIALIZED:            flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support page program command
 *         EN_FLASH_INVALID_ADDRESS         input sector number does not fall into allowed address region
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to program exceeded limit
 *         EN_FLASH_SUCCESS                 sector program successful
 */
enFlashStatus cb_flash_program_sector(uint16_t SectorNumber, uint8_t *data, uint16_t length)
{
  /* Limit checking */
  if ((SectorNumber < RUNTIME_NON_RESTRICTED_SECTOR_START) || (SectorNumber > RUNTIME_NON_RESTRICTED_SECTOR_END))
  {
    return EN_FLASH_INVALID_ADDRESS;
  }
  
  // data size larger than sector size
  if (length > DEF_FLASH_SECTOR_SIZE || length == 0)
  {
    return EN_FLASH_OPERATION_FAILED;
  }
  
  // program page by page
  uint16_t bytesProgrammed = 0;
  uint16_t startPage = (uint16_t) SectorNumber * (DEF_FLASH_SECTOR_SIZE / DEF_FLASH_PAGE_SIZE);
  enFlashStatus pageProgramStatus;

  while (length > bytesProgrammed)
  {
    uint16_t bytesToProgram = ((length - bytesProgrammed) > DEF_FLASH_PAGE_SIZE) ? (DEF_FLASH_PAGE_SIZE) : (length - bytesProgrammed);
    pageProgramStatus = cb_flash_program_page(startPage, &data[bytesProgrammed], bytesToProgram);

    if (pageProgramStatus != EN_FLASH_SUCCESS)
    {
      return pageProgramStatus;
    }

    bytesProgrammed += bytesToProgram;
    startPage++;
  }

  return EN_FLASH_SUCCESS;
}

/**
 * @brief This function programs the contents from any start address in the flash
 * 
 * @detail Valid start program address depends on whether user has entered elevation
 *         mode flash access or not, and on both the flash capacity and the
 *         program length in bytes.
 *         Maximum length can be accepted in address program is 4096 bytes
 *
 * @param[in]  address      The start address to program in flash
 * @param[in]  data         Pointer to the data buffer to program
 * @param[in]  length       Number of bytes to program into the flash, maximum 4096 bytes
 *
 * @return EN_FLASH_UNINITIALIZED:          flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support page program command
 *         EN_FLASH_INVALID_ADDRESS         input address does not fall into allowed address region
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to program exceeded limit
 *         EN_FLASH_SUCCESS                 program successful
 */
enFlashStatus cb_flash_program_by_addr(uint32_t address, uint8_t *data, uint16_t length)
{
  stQSPI_CmdTypeDef  stConfigQSPICommand;
  CB_STATUS ret;

  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }
  
  // GUARD CHECK: if address falls into user config region
  if (address > RUNTIME_NON_RESTRICTED_ADDR_END || address < RUNTIME_NON_RESTRICTED_ADDR_START)
  {
    return EN_FLASH_INVALID_ADDRESS;
  }
  
  // GUARD CHECK: if the address falls into the last available sector, and length exceeds
  // the flash memory limit (address + length > DEF_NON_RESTRICTED_ADDR_END)
  // only allow for 4KB (1 SECTOR) program by address at a time
  if (((length > DEF_MAX_PROGRAM_SIZE) || (length == 0)) || ((uint32_t) length > (RUNTIME_NON_RESTRICTED_ADDR_END - address + 1)))
  {
    // return operation failed
    return EN_FLASH_OPERATION_FAILED;
  }
  
  /* Check to see if page program command is supported */
  if (flashCommands.programPageCommand == DEF_COMMAND_UNSUPPORTED)
  {
    return EN_FLASH_OPERATION_UNSUPPORTED;
  }
  
  /* unlock flash */
  if(cb_flash_unlock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  // do some math to divide and program at page correctly
  uint32_t startProgramAddr = address;
  uint16_t bytesProgrammed = 0;

  /* Configure Command for Quad Page Program  -------------------------------------------- */
  stConfigQSPICommand.enFlashAcessArea        = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse        = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                 = flashCommands.programPageCommand;
  stConfigQSPICommand.enAddrModeUse           = flashCommands.programAddrMode;
  stConfigQSPICommand.AddrLen                 = 3; /*Addr Len in byte*/
  stConfigQSPICommand.SpecialCommandByte1     = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2     = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles            = 0;
  stConfigQSPICommand.enDataModeUse           = EN_QSPI_QuadSPI_Data;
  /* Configure Command for Quad Page Program End------------------------------------------ */

  while (length > bytesProgrammed)
  {
    uint16_t bytesToProgram;

    if ((startProgramAddr % DEF_FLASH_PAGE_SIZE) != 0)
    {
      bytesToProgram = (uint16_t) ((((startProgramAddr / DEF_FLASH_PAGE_SIZE) + 1) * DEF_FLASH_PAGE_SIZE) - startProgramAddr);

      if (bytesToProgram > length)
      {
        bytesToProgram = length;
      }
    }

    else 
    {
      bytesToProgram = ((length - bytesProgrammed) > DEF_FLASH_PAGE_SIZE) ? (DEF_FLASH_PAGE_SIZE) : (length - bytesProgrammed);
    }

    stConfigQSPICommand.Addr = startProgramAddr;
    stConfigQSPICommand.DataLen = bytesToProgram;

    cb_flash_write_enable();
    ret = cb_qspi_write_data_with_addr(pQSPI, &stConfigQSPICommand, &data[bytesProgrammed]);

    if (ret == CB_FAIL)
    {
      cb_flash_lock();
      return EN_FLASH_OPERATION_FAILED;
    }

    /* Timeout for polling WIP bit */
    CB_STATUS WIPFlagClear = cb_flash_wait_for_wip_clear();
    if (WIPFlagClear == CB_FAIL)
    {
      cb_flash_lock();
      return EN_FLASH_OPERATION_FAILED;
    }

    bytesProgrammed += bytesToProgram;
    startProgramAddr += bytesToProgram;
  }
  
  /* lock flash */
  if(cb_flash_lock() != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }
  
  return EN_FLASH_SUCCESS;
}

/**
 * @brief This function reads the contents in the corresponding page (256 bytes)
 * 
 * @detail Valid page number depending on whether user has entered elevation
 *         mode flash access or not, and depending on the flash capacity
 *         Maximum length can be accepted in page read is 256 bytes
 *
 * @param[in]  PageNumber The page number to read in flash
 * @param[in]  data       Pointer to the data buffer to read
 * @param[in]  length     Number of bytes to read from the page, maximum 256 bytes
 *
 * @return EN_FLASH_UNINITIALIZED:            flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support read command
 *         EN_FLASH_INVALID_ADDRESS         input page number does not fall into allowed address region
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to read exceeded limit
 *         EN_FLASH_SUCCESS                 page read successful
 */
enFlashStatus cb_flash_read_page(uint16_t PageNumber, uint8_t *data, uint16_t length)
{
  stQSPI_CmdTypeDef  stConfigQSPICommand;
  CB_STATUS ret;
  uint16_t bytesRead = 0;

  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }

  // if page number is invalid
  if ((PageNumber < RUNTIME_NON_RESTRICTED_PAGES_START) || (PageNumber > RUNTIME_NON_RESTRICTED_PAGES_END))
  {
    return EN_FLASH_INVALID_ADDRESS;
  }

  /* Some flash chips don't support read operation. Check them */
  if (flashCommands.readCommand == DEF_COMMAND_UNSUPPORTED)
  {
    return EN_FLASH_OPERATION_UNSUPPORTED;
  }

  // feed the start address of the page, since any address within page boundary is valid
  uint32_t readStartAddr = cb_flash_page_num_to_addr(PageNumber);
  
  // if data length exceeds page length
  if (length > DEF_FLASH_PAGE_SIZE || length == 0)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  /* Configure Command for Quad READ Start------------------------------------------ */
  stConfigQSPICommand.enFlashAcessArea        = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse        = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                 = flashCommands.readCommand; /*QUAD Read*/
  stConfigQSPICommand.enAddrModeUse           = EN_QSPI_QuadSPI_Addr;
  stConfigQSPICommand.AddrLen                 = 4; /*Addr Len in byte*/
  stConfigQSPICommand.SpecialCommandByte1     = (uint8_t)0x00; /*Perfomace Byte*/
  stConfigQSPICommand.SpecialCommandByte2     = 0;//no required.
  stConfigQSPICommand.nDummyCycles            = 4;
  stConfigQSPICommand.enDataModeUse           = EN_QSPI_QuadSPI_Data;
  /* Configure Command for Quad READ end------------------------------------------ */

  // read out 32-byte chunk each time to prevent set and reset of burst read mode
  while (length > bytesRead)
  {
    uint16_t bytesToRead = ((length - bytesRead) > DEF_READ_CHUNK_SIZE) ? (DEF_READ_CHUNK_SIZE) : (length - bytesRead);

    stConfigQSPICommand.Addr = readStartAddr;
    stConfigQSPICommand.DataLen = bytesToRead;

    ret = cb_qspi_read_data_with_addr(pQSPI, &stConfigQSPICommand, &data[bytesRead]);

    if (ret != CB_PASS)
    {
      return EN_FLASH_OPERATION_FAILED;
    }

    bytesRead += bytesToRead;
    readStartAddr += bytesToRead;
  }

  return EN_FLASH_SUCCESS;
}

/**
 * @brief This function reads the contents in the corresponding sector (4096 bytes)
 * 
 * @detail Valid sector number to read will always be 1, regardless of user has 
 *         entered privileged mode access or not
 *         Maximum length can be accepted in sector read is 4096 bytes
 *
 * @param[in]  SectorNumber The sector number to read in flash
 * @param[in]  data         Pointer to the data buffer to read
 * @param[in]  length       Number of bytes to read from the sector, maximum 4096 bytes
 *
 * @return EN_FLASH_UNINITIALIZED:            flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support read command
 *         EN_FLASH_INVALID_ADDRESS         input sector number does not fall into allowed address region
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to read exceeded limit
 *         EN_FLASH_SUCCESS                 sector read successful
 */
enFlashStatus cb_flash_read_sector(uint16_t SectorNumber, uint8_t *data, uint16_t length)
{
  /* Limit checking */
  if ((SectorNumber < RUNTIME_NON_RESTRICTED_SECTOR_START) || (SectorNumber > RUNTIME_NON_RESTRICTED_SECTOR_END))
  {
    return EN_FLASH_INVALID_ADDRESS;
  }
  
  // data size larger than sector size
  if (length > DEF_FLASH_SECTOR_SIZE || length == 0)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  // read page by page
  uint16_t bytesRead = 0;
  uint16_t startPage = (uint16_t) SectorNumber * (DEF_FLASH_SECTOR_SIZE / DEF_FLASH_PAGE_SIZE);
  enFlashStatus pageReadStatus;

  while (length > bytesRead)
  {
    uint16_t bytesToRead = ((length - bytesRead) > DEF_FLASH_PAGE_SIZE) ? (DEF_FLASH_PAGE_SIZE) : (length - bytesRead);
    pageReadStatus = cb_flash_read_page(startPage, &data[bytesRead], bytesToRead);

    if (pageReadStatus != EN_FLASH_SUCCESS)
    {
      return pageReadStatus;
    }

    bytesRead += bytesToRead;
    startPage++;
  }

  return EN_FLASH_SUCCESS;
}


/**
 * @brief This function reads the contents from any start address in the flash
 * 
 * @detail Valid start read address depends on whether user has entered elevation
 *         mode flash access or not, and on both the flash capacity and the
 *         read length in bytes.
 *         Maximum length can be accepted in address read is 4096 bytes
 *
 * @param[in]  address      The start address to read in flash
 * @param[in]  data         Pointer to the data buffer to read
 * @param[in]  length       Number of bytes to read from the flash, maximum 4096 bytes
 *
 * @return EN_FLASH_UNINITIALIZED:            flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support read command
 *         EN_FLASH_INVALID_ADDRESS         input address does not fall into allowed address region
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to read exceeded limit
 *         EN_FLASH_SUCCESS                 read successful
 */
enFlashStatus cb_flash_read_by_addr(uint32_t address, uint8_t *data, uint16_t length)
{
  stQSPI_CmdTypeDef  stConfigQSPICommand;
  CB_STATUS ret;

  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }
  
  // only allow for 4KB (1 SECTOR) read by address at a time
  if ((length > DEF_MAX_READ_SIZE) || (length == 0))
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  // GUARD CHECK: if address falls into user config region
  if ((address > RUNTIME_NON_RESTRICTED_ADDR_END) || (address < RUNTIME_NON_RESTRICTED_ADDR_START))
  {
    return EN_FLASH_INVALID_ADDRESS;
  }
  
  // GUARD CHECK: if the address falls into the last available sector, and length exceeds
  // the flash memory limit (address + length > DEF_NON_RESTRICTED_ADDR_END)
  if ((uint32_t)length > (RUNTIME_NON_RESTRICTED_ADDR_END - address + 1))
  {
    // return operation failed
    return EN_FLASH_OPERATION_FAILED;
  }

  // read out data by 32-byte chunks each time, to prevent burst read set and reset
  uint32_t startReadAddr = address;
  uint16_t bytesRead = 0;

  /* Configure read command start */
  stConfigQSPICommand.enFlashAcessArea        = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse        = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                 = flashCommands.readCommand; /*QUAD Read*/
  stConfigQSPICommand.enAddrModeUse           = EN_QSPI_QuadSPI_Addr;
  stConfigQSPICommand.AddrLen                 = 4; /*Addr Len in byte*/
  stConfigQSPICommand.SpecialCommandByte1     = (uint8_t)0x00; /*Perfomace Byte*/
  stConfigQSPICommand.SpecialCommandByte2     = 0;//no required.
  stConfigQSPICommand.nDummyCycles            = 4;
  stConfigQSPICommand.enDataModeUse           = EN_QSPI_QuadSPI_Data;
  /* Configure read command end */

  while (length > bytesRead)
  {
    uint16_t bytesToRead;

    if ((startReadAddr % DEF_READ_CHUNK_SIZE) != 0)
    {
      bytesToRead = (uint16_t) ((((startReadAddr / DEF_READ_CHUNK_SIZE) + 1) * DEF_READ_CHUNK_SIZE) - startReadAddr);

      if (bytesToRead > length)
      {
        bytesToRead = length;
      }
    }

    else 
    {
      bytesToRead = ((length - bytesRead) > DEF_READ_CHUNK_SIZE) ? (DEF_READ_CHUNK_SIZE) : (length - bytesRead);
    }

    stConfigQSPICommand.Addr = startReadAddr;
    stConfigQSPICommand.DataLen = bytesToRead;

    ret = cb_qspi_read_data_with_addr(pQSPI, &stConfigQSPICommand, &data[bytesRead]);

    if (ret == CB_FAIL)
    {
      return EN_FLASH_OPERATION_FAILED;
    }

    startReadAddr += bytesToRead;
    bytesRead += bytesToRead;
  }  

  return EN_FLASH_SUCCESS;
}


/**
 * @brief Checks the Write In Progress (WIP) status of the flash memory.
 *
 * This function reads the status register of the flash memory to determine
 * if a write operation is currently in progress. It continuously attempts to
 * read the status until successful. If the WIP bit is set, it indicates that
 * the memory is busy writing; otherwise, it indicates that the memory is ready.
 *
 * @return CB_TRUE   a write operation is in progress.
 *         CB_FALSE  no write operation is occurring.
 */
uint8_t cb_flash_check_wip(void)
{
  uint8_t result = CB_TRUE;
  CB_STATUS ret = CB_FAIL;
  stQSPI_CmdTypeDef  stConfigQSPICommand;
  uint8_t ReadBuf[1];

  /* Configure Command Start------------------------------------------ */
  stConfigQSPICommand.enFlashAcessArea        = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse        = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                 = flashCommands.readStatusReg1;/*Read Status Register*/
  stConfigQSPICommand.enAddrModeUse           = EN_QSPI_NormalSPI_Addr;
  stConfigQSPICommand.Addr                    = DEF_NON_REQUIRED;
  stConfigQSPICommand.AddrLen                 = 0;
  stConfigQSPICommand.SpecialCommandByte1     = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2     = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles            = 0;
  stConfigQSPICommand.DataLen                 = 1;
  stConfigQSPICommand.enDataModeUse           = EN_QSPI_NormalSPI_Data;
 /* Configure Command End------------------------------------------ */

  /* Poll for WIP bit */
  FLASH_timeoutWIPStartCPUCycle = DWT->CYCCNT;
  while (1)  /*Re-try incase of ReadFail (Happened when QSPI is BUSY)*/
  {  
    ret = cb_qspi_read_data_without_addr(pQSPI, &stConfigQSPICommand,&ReadBuf[0]);
    if (ret == CB_PASS)
    {
      ReadBuf[0] &= DEF_WIPBIT;
      if (ReadBuf[0] == DEF_WIPBIT)
      {
        result = CB_TRUE;
      }
      else
      {
        result = CB_FALSE;
      }
      break;
    }

    // /* Capture the current CPU cycle counter and check for timeout */
    FLASH_timeoutWIPElapsedCPUCycles = (DWT->CYCCNT < FLASH_timeoutWIPStartCPUCycle) ? 
      (0xFFFFFFFF - FLASH_timeoutWIPStartCPUCycle + DWT->CYCCNT + 1) : (DWT->CYCCNT - FLASH_timeoutWIPStartCPUCycle);
    
    int64_t cycleDiff = (int64_t) ((int64_t)(FLASH_timeoutElapsedCPUCycles) - (int64_t)(DEF_FLASH_TIMEOUT_CPU_CYCLES));

    if (cycleDiff > 0)
    {
      return CB_TRUE;
    }
  }

  return result;
}


/**
 * @brief   Enables write operations on the flash memory.
 * @details This function configures the QSPI command structure to send a Write Enable (WREN) 
 * command to the flash memory. It retries the command in case of failure due to QSPI 
 * being busy and confirms the operation by checking the Write Enable Latch (WEL).
 * @return  CB_PASS  if the Write Enable operation is successful.
 *          CB_FAIL  if the Write Enable operation is not successful.
 */
CB_STATUS cb_flash_write_enable(void)
{
  CB_STATUS ret = CB_FAIL;
  stQSPI_CmdTypeDef  stConfigQSPICommand;

  /* Configure Command Start------------------------------------------ */
  stConfigQSPICommand.enFlashAcessArea       = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse       = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                = flashCommands.writeEnableCommand; /*Write Enable*/
  stConfigQSPICommand.enAddrModeUse          = EN_QSPI_NormalSPI_Addr;
  stConfigQSPICommand.Addr                   = DEF_NON_REQUIRED;
  stConfigQSPICommand.AddrLen                = 0;
  stConfigQSPICommand.SpecialCommandByte1    = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2    = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles           = 0;
  stConfigQSPICommand.DataLen                = 0;
  stConfigQSPICommand.enDataModeUse          = EN_QSPI_NormalSPI_Data;
  /* Configure Command End------------------------------------------ */
  
  FLASH_timeoutStartCPUCycle = DWT->CYCCNT;
  while (1)  /*Re-try incase of WriteFail (Happened when QSPI is BUSY)*/
  {  
    ret = cb_qspi_write_single_command(pQSPI, &stConfigQSPICommand);
    if (ret == CB_PASS)
    {
      break;
    }

    // /* Capture the current CPU cycle counter and check for timeout */
    FLASH_timeoutElapsedCPUCycles = (DWT->CYCCNT < FLASH_timeoutStartCPUCycle) ? 
      (0xFFFFFFFF - FLASH_timeoutStartCPUCycle + DWT->CYCCNT + 1) : (DWT->CYCCNT - FLASH_timeoutStartCPUCycle);
    
    int64_t cycleDiff = (int64_t) ((int64_t)(FLASH_timeoutElapsedCPUCycles) - (int64_t)(DEF_FLASH_TIMEOUT_CPU_CYCLES));

    if (cycleDiff > 0)
    {
      return CB_FAIL;
    }
  }

  return ret;
}

/**
 * @brief    Sets/resets the burst read mode with burst wrap length of 32 bytes for the QSPI flash chip.
 * @detail   This function is used to help I-cache to read data from QSPI chip (during normal operation
 *           or deep sleep). After this function is set, any read operation is constrainted within 32 bytes
 *           of data, aligned with whole QSPI flash. If address exceeds current 32-byte region, the contents
 *           will be a wrap-around (from the start of the region)
 *           
 * @param[in] BurstReadAction: EN_BURSTREAD_SET or EN_BURSTREAD_RESET
 *
 * @return EN_FLASH_UNINITIALIZED:            flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support burst read set/reset command
 *         EN_FLASH_SUCCESS                 set/reset successful
 */
enFlashStatus cb_flash_configure_read_mode(enBurstReadAction BurstReadAction)
{
  stQSPI_CmdTypeDef stConfigQSPICommand;

  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }

  /* Some flash chips don't support burst read operation. Check them */
  if (flashCommands.burstReadReset == DEF_COMMAND_UNSUPPORTED || \
      flashCommands.burstReadSet == DEF_COMMAND_UNSUPPORTED || \
      flashCommands.burstReadCommand == DEF_COMMAND_UNSUPPORTED)
  {
    return EN_FLASH_OPERATION_UNSUPPORTED;
  }

  stConfigQSPICommand.enFlashAcessArea       = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse       = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                = flashCommands.burstReadCommand; /* Burst read command */
  stConfigQSPICommand.enAddrModeUse          = EN_QSPI_Unused_Addr;
  stConfigQSPICommand.Addr                   = DEF_NON_REQUIRED;
  stConfigQSPICommand.AddrLen                = 0;
  stConfigQSPICommand.SpecialCommandByte1    = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2    = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles           = 0;
  stConfigQSPICommand.DataLen                = flashCommands.burstReadCommandDataLen;
  stConfigQSPICommand.enDataModeUse          = flashCommands.burstReadCommandDataMode;

  /* Send out read mode through QSPI driver */
  if (BurstReadAction == EN_BURSTREAD_SET)
  {
    cb_qspi_send_read_mode_command(EN_BURSTREAD_SET, &stConfigQSPICommand, flashCommands.burstReadSet);
  }

  else 
  {
    cb_qspi_send_read_mode_command(EN_BURSTREAD_RESET, &stConfigQSPICommand, flashCommands.burstReadReset);
  }

  return EN_FLASH_SUCCESS;
}



/**
 * @brief This function reads the status register1 of the flash memory
 *
 * @return EN_FLASH_OPERATION_FAILED    QSPI driver returned error, or length to read exceeded limit
 *         EN_FLASH_SUCCESS             read successful
 */
enFlashStatus cb_flash_read_status_reg1(uint8_t *status_reg1)
{
  uint8_t result = EN_FLASH_OPERATION_FAILED;
  CB_STATUS ret = CB_FAIL;
  stQSPI_CmdTypeDef  stConfigQSPICommand;

  /* Configure Command Start------------------------------------------ */
  stConfigQSPICommand.enFlashAcessArea        = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse        = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                 = flashCommands.readStatusReg1;/*Read Status Register*/
  stConfigQSPICommand.enAddrModeUse           = EN_QSPI_NormalSPI_Addr;
  stConfigQSPICommand.Addr                    = DEF_NON_REQUIRED;
  stConfigQSPICommand.AddrLen                 = 0;
  stConfigQSPICommand.SpecialCommandByte1     = DEF_NON_REQUIRED;
  stConfigQSPICommand.SpecialCommandByte2     = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles            = 0;
  stConfigQSPICommand.DataLen                 = 1;
  stConfigQSPICommand.enDataModeUse           = EN_QSPI_NormalSPI_Data;
 /* Configure Command End------------------------------------------ */

  /* Poll for WIP bit */
  FLASH_timeoutWIPStartCPUCycle = DWT->CYCCNT;
  while (1)  /*Re-try incase of ReadFail (Happened when QSPI is BUSY)*/
  {  
    ret = cb_qspi_read_data_without_addr(pQSPI, &stConfigQSPICommand,status_reg1);

    if (ret == CB_PASS)
    {
      result = EN_FLASH_SUCCESS;
      break;
    }

    // /* Capture the current CPU cycle counter and check for timeout */
    FLASH_timeoutWIPElapsedCPUCycles = (DWT->CYCCNT < FLASH_timeoutWIPStartCPUCycle) ? 
      (0xFFFFFFFF - FLASH_timeoutWIPStartCPUCycle + DWT->CYCCNT + 1) : (DWT->CYCCNT - FLASH_timeoutWIPStartCPUCycle);
    
    int64_t cycleDiff = (int64_t) ((int64_t)(FLASH_timeoutElapsedCPUCycles) - (int64_t)(DEF_FLASH_TIMEOUT_CPU_CYCLES));

    if (cycleDiff > 0)
    {
      break;
    }
  }

  return result;
}

/**
 * @brief  This function initialize flash Block Protect parameters
 * @detail The parameters depend on the specific flash 
 *
 * @return None
 */
void cb_flash_block_protect_init(void)
{
  switch (flashVendorID)
  {
    case EN_FLASH_VENDOR_UNKNOWN:
        break;
    
    case EN_FLASH_VENDOR_BOYA:
        if(flashCapacity == EN_FLASH_CAPACITY_1MB)
        {
          flash_bp_param.bp_mask                  = DEF_BOYA_LOCK_MSK_1MB;
          flash_bp_param.bp_lock_param            = DEF_BOYA_LOCK_1MB;
          flash_bp_param.bp_unlock_param          = DEF_BOYA_UNLOCK_1MB;
        }
      break;

    case EN_FLASH_VENDOR_PUYA:
        if(flashCapacity == EN_FLASH_CAPACITY_512KB)
        {
          flash_bp_param.bp_mask                  = DEF_PUYA_LOCK_MSK_512KB;
          flash_bp_param.bp_lock_param            = DEF_PUYA_LOCK_512KB;
          flash_bp_param.bp_unlock_param          = DEF_PUYA_UNLOCK_512KB;
        }
      break;

    case EN_FLASH_VENDOR_MACRONIX:
        if(flashCapacity == EN_FLASH_CAPACITY_512KB)
        {
          flash_bp_param.bp_mask                  = DEF_MACRONIX_LOCK_MSK_512KB;
          flash_bp_param.bp_lock_param            = DEF_MACRONIX_LOCK_512KB;
          flash_bp_param.bp_unlock_param          = DEF_MACRONIX_UNLOCK_512KB;
        }
        else if(flashCapacity == EN_FLASH_CAPACITY_1MB)
        {
          flash_bp_param.bp_mask                  = DEF_MACRONIX_LOCK_MSK_1MB;
          flash_bp_param.bp_lock_param            = DEF_MACRONIX_LOCK_1MB;
          flash_bp_param.bp_unlock_param          = DEF_MACRONIX_UNLOCK_1MB;
        }
      break;
    
    case EN_FLASH_VENDOR_WINBOND:
        if(flashCapacity == EN_FLASH_CAPACITY_512KB)
        {
          flash_bp_param.bp_mask                  = DEF_WINBOND_LOCK_MSK_512KB;
          flash_bp_param.bp_lock_param            = DEF_WINBOND_LOCK_512KB;
          flash_bp_param.bp_unlock_param          = DEF_WINBOND_UNLOCK_512KB;
        }
        else if(flashCapacity == EN_FLASH_CAPACITY_1MB)
        {
          flash_bp_param.bp_mask                  = DEF_WINBOND_LOCK_MSK_1MB;
          flash_bp_param.bp_lock_param            = DEF_WINBOND_LOCK_1MB;
          flash_bp_param.bp_unlock_param          = DEF_WINBOND_UNLOCK_1MB;
        }
      break;
  }
}

/**
 * @brief    Block Protecton setting.
 * @detail   This function is used to provide block protection in Software Protected mode(SRP1=0, SRP0=0).
 *           with CMP=0.
 *           
 * @param[in] BlockParam:  flash_bp_param.bp_lock_param
 *
 * @return EN_FLASH_UNINITIALIZED:            flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support burst read set/reset command
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to read exceeded limit
 *         EN_FLASH_SUCCESS                 set/reset successful
 */
enFlashStatus cb_flash_block_protect(uint8_t BlockParam)
{
  stQSPI_CmdTypeDef stConfigQSPICommand;

  uint8_t status_reg1_rd, status_reg1_wr;

  uint8_t ret = CB_FAIL;
  uint8_t result = EN_FLASH_OPERATION_FAILED;
    
  /* Check if flash has been initialized */
  if ((flashVendorID == EN_FLASH_VENDOR_UNKNOWN) || (flashCapacity == EN_FLASH_CAPACITY_UNKNOWN))
  {
    return EN_FLASH_UNINITIALIZED;
  }

  /* Some flash chips don't support burst read operation. Check them */
  if (flashCommands.burstReadReset == DEF_COMMAND_UNSUPPORTED || \
      flashCommands.burstReadSet == DEF_COMMAND_UNSUPPORTED || \
      flashCommands.burstReadCommand == DEF_COMMAND_UNSUPPORTED)
  {
    return EN_FLASH_OPERATION_UNSUPPORTED;
  }


  /*Write enable before Status Register Write*/
  cb_flash_write_enable();
  
  /* Read status register1*/
  ret = cb_flash_read_status_reg1(&status_reg1_rd);
  if(ret != EN_FLASH_SUCCESS)
  {
    return EN_FLASH_OPERATION_FAILED;
  }
  
  /* Configure write value*/
  BlockParam &= flash_bp_param.bp_mask;
  
  if(BlockParam == (status_reg1_rd & flash_bp_param.bp_mask) )
  {
    return EN_FLASH_SUCCESS;
  }
  
  status_reg1_rd &= ~(0x1F<<2);
  status_reg1_wr = (uint8_t)(status_reg1_rd | BlockParam);
  
  stConfigQSPICommand.enFlashAcessArea       = EN_VendorConfigArea;
  stConfigQSPICommand.enCommandModeuse       = EN_QSPI_NormalSPI_Command;
  stConfigQSPICommand.Command                = flashCommands.writeStatusReg1; /* write status register1 command */
  stConfigQSPICommand.enAddrModeUse          = EN_QSPI_NormalSPI_Addr;
  stConfigQSPICommand.Addr                   = DEF_NON_REQUIRED;
  stConfigQSPICommand.AddrLen                = 0;
  stConfigQSPICommand.SpecialCommandByte1    = status_reg1_wr;
  stConfigQSPICommand.SpecialCommandByte2    = DEF_NON_REQUIRED;
  stConfigQSPICommand.nDummyCycles           = 0;
  stConfigQSPICommand.DataLen                = 1;
  stConfigQSPICommand.enDataModeUse          = EN_QSPI_NormalSPI_Data;
  
  FLASH_timeoutStartCPUCycle = DWT->CYCCNT;
  while (1)  /*Re-try incase of WriteFail (Happened when QSPI is BUSY)*/
  {
    ret = cb_qspi_write_command_with_param(pQSPI, &stConfigQSPICommand);
    if (ret == CB_PASS)
    {
      result = EN_FLASH_SUCCESS;
      break;
    }

    // /* Capture the current CPU cycle counter and check for timeout */
    FLASH_timeoutElapsedCPUCycles = (DWT->CYCCNT < FLASH_timeoutStartCPUCycle) ? 
      (0xFFFFFFFF - FLASH_timeoutStartCPUCycle + DWT->CYCCNT + 1) : (DWT->CYCCNT - FLASH_timeoutStartCPUCycle);
    
    int64_t cycleDiff = (int64_t) ((int64_t)(FLASH_timeoutElapsedCPUCycles) - (int64_t)(DEF_FLASH_TIMEOUT_CPU_CYCLES));

    if (cycleDiff > 0)
    {
      return EN_FLASH_OPERATION_FAILED;
    }
  }
  
  /*Wait for Erase Operation Complete*/
  CB_STATUS WIPFlagClear = cb_flash_wait_for_wip_clear();
  if (WIPFlagClear == CB_FAIL)
  {
    return EN_FLASH_OPERATION_FAILED;
  }

  return result;
}

/**
 * @brief  Lock FLASH with BLOCK PROTECT
 *
 * @return EN_FLASH_UNINITIALIZED           flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support burst read set/reset command
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to read exceeded limit
 *         EN_FLASH_SUCCESS                 set/reset successful
 */
enFlashStatus cb_flash_lock(void)
{
  enFlashStatus result = EN_FLASH_OPERATION_FAILED;

  result = cb_flash_block_protect(flash_bp_param.bp_lock_param);

  return result;
}

/**
 * @brief  Unlock FLASH with BLOCK PROTECT
 *
 * @return EN_FLASH_UNINITIALIZED           flash chip not initialized/recognized
 *         EN_FLASH_OPERATION_UNSUPPORTED   flash chip does not support burst read set/reset command
 *         EN_FLASH_OPERATION_FAILED        QSPI driver returned error, or length to read exceeded limit
 *         EN_FLASH_SUCCESS                 set/reset successful
 */
enFlashStatus cb_flash_unlock(void)
{
  enFlashStatus result = EN_FLASH_OPERATION_FAILED;
    
  result = cb_flash_block_protect(flash_bp_param.bp_unlock_param);

  return result;
}
