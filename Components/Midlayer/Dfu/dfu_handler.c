/**
 * @file    AppOTA.c
 * @brief   [SYSTEM] UART Commander Application Source File
 * @details This file contains the implementation of functions related to UART command handling.
 *          It defines functions for processing UART receive buffer, command handling, and associated callback functions.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CB_crc.h"
#include "CB_flash.h"
#include "CB_SleepDeepSleep.h"

#include "dfu_handler.h"
//-------------------------------
// DEFINE SECTION
//-------------------------------
#define FW_VERSION "\x01\x00\x00\x00"  // 0x0001
#if defined BOOT
#define BOOT_VERSION "\x00\x00\x00\x00" 
const uint8_t FirmwareVersion[] = BOOT_VERSION;
#else
const uint8_t FirmwareVersion[] __attribute__((section(".ARM.__at_0x6000")))= FW_VERSION;
#endif
//fw_ver = bin[0x6000-0x4000]
//static uint16_t app_fw_ver = 0;
//memcpy(&app_fw_ver,&acCBU5000V210_APP[0x2000],sizeof(app_fw_ver));
#ifndef APP_FACTORY_LOG_ENABLE
#define APP_FACTORY_LOG_ENABLE APP_FALSE 
#endif
#if (APP_FACTORY_LOG_ENABLE == APP_TRUE)
#define LOG(...) dfu_UartPrintf(__VA_ARGS__)
#else
#define LOG(...)   
#endif

#define OTA_PACK_MAX        128
#define APP_BANK_ADDRESS    0x05000
#define BACKUP_BANK_ADDRESS 0x3E800
#define FIRMWARE_BANK_SIZE  0x3A800

#define BOOTSETTING_ADDR_A  0x7A000
#define BOOTSETTING_ADDR_B  0x79000

#define FLASH_SECTOR_SIZE   0x1000
#define FLASH_PAGE_SIZE     0x100

//#define FLASH_WRITE_BUFFER  //if need write in page

#define BigLittleSwap16(A) ((((uint16_t)(A) & 0xff00) >> 8 ) |\
                            (((uint16_t)(A) & 0x00ff) << 8 ))
#define BigLittleSwap32(A) ((((uint32_t)(A) & 0xff000000) >> 24) |\
                            (((uint32_t)(A) & 0x00ff0000) >> 8 ) |\
                            (((uint32_t)(A) & 0x0000ff00) << 8 ) |\
                            (((uint32_t)(A) & 0x000000ff) << 24))
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief
 */

typedef struct {
    uint16_t command;
    dfu_cmdhandler_t handler;
} cmdTab;
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void dfu_read_fw_version(uint16_t command, uint8_t *buf, uint8_t len);
void dfu_reset_chip(uint16_t command, uint8_t *buf, uint8_t len)__attribute__((noreturn));
void dfu_upgrade_start(uint16_t command, uint8_t *buf, uint8_t len);
void dfu_jump_application(uint16_t command, uint8_t *buf, uint8_t len);

void dfu_upgrade_pack(uint16_t command, uint8_t *buf, uint8_t len);
void dfu_upgrade_verify(uint16_t command, uint8_t *buf, uint8_t len);
void dfu_upgrade_finish(uint16_t command, uint8_t *buf, uint8_t len);

void     dfu_command_respond_port(uint16_t command, uint8_t *buf, uint8_t len);
uint32_t dfu_crc_check_port_sw(const void *data, size_t len, uint32_t prev_crc);
uint32_t dfu_crc_check_port(uint8_t *p_data, uint32_t size, uint32_t prev_crc);
uint32_t dfu_flash_erase_port(uint32_t address, uint32_t size);
uint32_t dfu_flash_write_port(uint32_t address, uint8_t *p_data, uint32_t size);
uint32_t dfu_flash_write_page_port(uint32_t address, uint8_t *p_data, uint32_t size);
uint32_t dfu_flash_write_buf_port(uint32_t offset, uint8_t *p_data, uint32_t pack_size);
uint32_t dfu_flash_read_port(uint32_t address, uint8_t *p_data, uint32_t size);
uint32_t dfu_bootsetting_read(bootsetting_info_t *p_info);
uint32_t dfu_bootsetting_write(bootsetting_info_t *p_info);
uint32_t dfu_bootsetting_update(uint32_t version, uint32_t size, uint32_t crc );
uint32_t dfu_firmware_crc_check(uint32_t address, uint32_t size);
void     boot_copy_firmware(uint32_t dest, uint32_t src, uint32_t size );
uint32_t boot_move_backup_to_app(bootsetting_info_t *p_info);
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static dfu_cfg_t g_dfu_cfg;
static uint32_t dfu_addr_offset ;
static uint32_t dfu_fw_ver =0;
static uint8_t dfu_active_flag;

#define CMD_READ_VER  0x0100 
#define CMD_REST_DEV  0x0120 
#define CMD_START     0x0110 
#define CMD_JUMP      0x01F1 

#define CMD_PACK      0x0111 
#define CMD_VERIFY    0x0112 
#define CMD_FINISH    0x0113 

static const cmdTab commandTable[] = 
{
    //Greneral commmand
    {CMD_READ_VER,  dfu_read_fw_version},
    {CMD_REST_DEV,  dfu_reset_chip},
    {CMD_START,     dfu_upgrade_start},
    
    
    {CMD_JUMP,      dfu_jump_application},
};

static const cmdTab otaCommand[] = 
{
    //OTA command
    {CMD_PACK,    dfu_upgrade_pack},
    {CMD_VERIFY,  dfu_upgrade_verify},
    {CMD_FINISH,  dfu_upgrade_finish},
};

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/**
 * @brief OTA command responder.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void dfu_command_respond_port(uint16_t command, uint8_t *buf, uint8_t len)
{
    if(g_dfu_cfg.responder)
        g_dfu_cfg.responder(command,buf,len);
}



uint32_t dfu_crc_check_port_sw(const void *data, size_t len, uint32_t prev_crc)
{
    #define POLY 0xEDB88320  
    const uint8_t *bytes = data;
    uint32_t crc = ~prev_crc;//0xFFFFFFFFUL;
    for (size_t i = 0; i < len; i++) {
      crc ^= bytes[i];  
      for (int j = 0; j < 8; j++) {
          if (crc & 1) {     
              crc = (crc >> 1) ^ POLY;
          } else {  
              crc >>= 1;
          }
      }
    }
    return ~crc;  
}


uint32_t dfu_crc_check_port(uint8_t *p_data, uint32_t size, uint32_t prev_crc)
{
    uint32_t crc;
    enCRCReInit crcreint = EN_CRC_ReInit_Disable;
    if(prev_crc == 0)
    {
        cb_crc_algo_config(EN_CRC32, EN_InitValOne, EN_CRCRefOut_Enable, EN_CRCRefIn_Enable, 0x04C11DB7, 0xFFFFFFFF);  /*CRC-32*/
        crcreint = EN_CRC_ReInit_Enable;
    }
    CB_STATUS status = cb_crc_process_from_input_data(p_data, (uint16_t)size, crcreint);

    if (status == CB_PASS)
    {
        crc =  cb_crc_get_crc_result();
    }
    else{
        crc = 0;
    }
    return crc;
}

uint32_t dfu_flash_erase_port(uint32_t address, uint32_t size)
{
    uint16_t sector = (uint16_t)(size/FLASH_SECTOR_SIZE + 1);
    uint16_t sector_num = (uint16_t)(address/FLASH_SECTOR_SIZE);

    for (uint16_t i = 0; i < sector; i++)
    {
        enFlashStatus FlashStatus = cb_flash_erase_sector(sector_num+i);
        if(FlashStatus != EN_FLASH_SUCCESS)
        {
            return FlashStatus;      
        }
    }
  
    return EN_FLASH_SUCCESS;
}
uint32_t dfu_flash_write_port(uint32_t address, uint8_t *p_data, uint32_t size)
{  
    enFlashStatus FlashStatus;
    FlashStatus = cb_flash_program_by_addr(address,p_data,(uint16_t)size);
    return FlashStatus;  
}

#if defined FLASH_WRITE_BUFFER

uint32_t dfu_flash_write_page_port(uint32_t address, uint8_t *p_data, uint32_t size)
{  
    enFlashStatus FlashStatus;
    uint16_t page_num = address/FLASH_PAGE_SIZE;
    uint16_t page_size;
    if(size <= FLASH_PAGE_SIZE)
    {
        page_size = 1;
    }
    else{
        page_size = size/FLASH_PAGE_SIZE+1;
    }

    if(page_num*FLASH_PAGE_SIZE == address)
    {
        while(page_size > 1)
        {
            FlashStatus=cb_flash_program_page(page_num,p_data,FLASH_PAGE_SIZE);
            if(FlashStatus != EN_FLASH_SUCCESS)
            {
            return FlashStatus;      
            }
            page_size--;
            page_num++;
            p_data += FLASH_PAGE_SIZE;
            size -= FLASH_PAGE_SIZE;
        }
        FlashStatus=cb_flash_program_page(page_num,p_data,size);
        if(FlashStatus != EN_FLASH_SUCCESS)
        {
            return FlashStatus;      
        }
    }
    else{
        FlashStatus = EN_FLASH_INVALID_ADDRESS;
    }
    return FlashStatus;
}

uint32_t dfu_flash_write_buf_port(uint32_t offset, uint8_t *p_data, uint32_t pack_size)
{
    enFlashStatus FlashStatus;
    static uint8_t pack_rec_buf[FLASH_PAGE_SIZE];
    static uint32_t pack_rec_idx = 0;
    static uint32_t dfu_page_offset = 0;

    if(offset == 0)
    {
        pack_rec_idx = 0;
    }
    if(pack_rec_idx == 0 )
    {
        dfu_page_offset = offset;
    }
    memcpy(&pack_rec_buf[pack_rec_idx], p_data, pack_size);
    pack_rec_idx += pack_size;
    if((pack_rec_idx >= FLASH_PAGE_SIZE) || ((pack_size == 0)&&(pack_rec_idx > 0)))
    {
        FlashStatus = dfu_flash_write_page_port(BACKUP_BANK_ADDRESS+dfu_page_offset, &pack_rec_buf, FLASH_PAGE_SIZE); //write page
        pack_rec_idx = 0;
        memset(pack_rec_buf,0,FLASH_PAGE_SIZE);    
    }
    return FlashStatus;
}
#endif

uint32_t dfu_flash_read_port(uint32_t address, uint8_t *p_data, uint32_t size)
{
    enFlashStatus FlashStatus;
    uint16_t page_num = (uint16_t)(address/FLASH_PAGE_SIZE);
    uint16_t page_size;
    if(size <= FLASH_PAGE_SIZE)
    {
        page_size = 1;
    }
    else{
        page_size = (uint16_t)(size/FLASH_PAGE_SIZE+1);
    }

    if(page_num*FLASH_PAGE_SIZE == address)
    {
        while(page_size > 1)
        {
            FlashStatus=cb_flash_read_page(page_num,p_data,FLASH_PAGE_SIZE);
            if(FlashStatus != EN_FLASH_SUCCESS)
            {
            return FlashStatus;      
            }
            page_size--;
            page_num++;
            p_data += FLASH_PAGE_SIZE;
            size -= FLASH_PAGE_SIZE;
        }
        FlashStatus=cb_flash_read_page(page_num,p_data,(uint16_t)size);
        if(FlashStatus != EN_FLASH_SUCCESS)
        {
          return FlashStatus;      
        }
    }
    else{
        FlashStatus = EN_FLASH_INVALID_ADDRESS;
    }
    return FlashStatus;
}


uint32_t dfu_bootsetting_read(bootsetting_info_t *p_info)
{
    dfu_flash_read_port(BOOTSETTING_ADDR_A,(void*)p_info,sizeof(bootsetting_info_t));
    uint32_t crc_check = dfu_crc_check_port((void*)(&p_info->data_crc+1), sizeof(bootsetting_info_t)-4,0);
    if(crc_check != p_info->data_crc)
    {
        dfu_flash_read_port(BOOTSETTING_ADDR_B,(void*)p_info,sizeof(bootsetting_info_t));
        crc_check = dfu_crc_check_port((void*)(&p_info->data_crc+1), sizeof(bootsetting_info_t)-4,0);
        if(crc_check != p_info->data_crc)
        {
            //init default bootsetting ?
            memset(p_info,0,sizeof(bootsetting_info_t));
            p_info->app_bank.fw_start_addr = APP_BANK_ADDRESS;
            dfu_bootsetting_write(p_info);
            return APP_FALSE;
        }
    }
    return APP_TRUE;
}

uint32_t dfu_bootsetting_write(bootsetting_info_t *p_info)
{
    uint32_t rt = 0;
    p_info->data_crc = dfu_crc_check_port((void*)(&p_info->data_crc+1), sizeof(bootsetting_info_t)-4,0);
    rt |= dfu_flash_erase_port(BOOTSETTING_ADDR_B,sizeof(bootsetting_info_t));
    rt |= dfu_flash_write_port(BOOTSETTING_ADDR_B,(void*)p_info,sizeof(bootsetting_info_t));
    rt |= dfu_flash_erase_port(BOOTSETTING_ADDR_A,sizeof(bootsetting_info_t));
    rt |= dfu_flash_write_port(BOOTSETTING_ADDR_A,(void*)p_info,sizeof(bootsetting_info_t));
    return rt;
}


uint32_t dfu_bootsetting_update(uint32_t version, uint32_t size, uint32_t crc )
{
    uint32_t rt = 0;
    bootsetting_info_t bootsetting;
    dfu_bootsetting_read(&bootsetting);

    bootsetting.backup_bank.fw_start_addr = APP_BANK_ADDRESS;
    bootsetting.backup_bank.fw_load_addr  = BACKUP_BANK_ADDRESS;
    bootsetting.backup_bank.fw_size       = size;
    bootsetting.backup_bank.fw_crc        = crc;
    bootsetting.backup_bank.fw_verson     = version;
    bootsetting.backup_bank.fw_active     = APP_TRUE;

    bootsetting.boot_mode = APP_FALSE;
    rt = dfu_bootsetting_write(&bootsetting);
    return rt;
}

void dfu_software_enter_bootmode(void)
{
    bootsetting_info_t bootsetting;
    dfu_bootsetting_read(&bootsetting);
    bootsetting.boot_mode = APP_FALSE;
    dfu_bootsetting_write(&bootsetting);
}


uint32_t dfu_firmware_crc_check(uint32_t address, uint32_t size)
{
    uint8_t readbuf[FLASH_PAGE_SIZE];
    uint32_t crc_check = 0;
    uint32_t offser = 0;
    uint32_t read_size;
    while(offser < size)
    {
        read_size = size - offser;
        if(read_size > FLASH_PAGE_SIZE) 
            read_size = FLASH_PAGE_SIZE;

        dfu_flash_read_port(address + offser, (uint8_t*)&readbuf, read_size);
        crc_check = dfu_crc_check_port(readbuf,read_size,crc_check);
        offser += read_size;
    }

    return crc_check;
}


uint8_t dfu_halder_get_state(void)
{
    return dfu_active_flag;
}

void dfu_halder_init(dfu_cfg_t* p_cfg)
{
    static uint8_t inited = APP_FALSE;
    if(inited)
        return;
    
    inited = APP_TRUE;
    cb_crc_init();
    cb_flash_init();
    if(p_cfg)
    {
        //save cfg
        memcpy(&g_dfu_cfg,p_cfg,sizeof(dfu_cfg_t));
    }
}

/**
 * @brief Process command.
 * 
 * This function processes the UART receive buffer by separating the command
 * and arguments, converting argument strings to integers, and then executing
 * the corresponding command handler function.
 * 
 * @param command   the command id.
 * @param prtData   Pointer to the data buffer.
 * @param len       The data len of buffer.
 */

uint8_t dfu_halder_polling(uint16_t command, uint8_t *prtData, uint16_t len, dfu_cmdhandler_t responder)
{
    uint8_t cb_done = APP_FALSE;
    uint32_t commandTblSize;
    cmdTab* table;

    commandTblSize = sizeof(commandTable) / sizeof(commandTable[0]);
    table = commandTable;
    g_dfu_cfg.responder = responder;
    // Find command in the lookup table
    for (uint8_t i = 0; i < commandTblSize; i++) 
    {
        if (table[i].command == command) 
        {
            table[i].handler(command,prtData,(uint8_t)len);
            cb_done = APP_TRUE;
            break;
        }
    }
    //OTA command
    if((cb_done == APP_FALSE) && (dfu_active_flag == APP_TRUE))
    {
        commandTblSize = sizeof(otaCommand) / sizeof(otaCommand[0]);
        table = otaCommand;
        // Find command in the lookup table
        for (uint8_t i = 0; i < commandTblSize; i++) 
        {
            if (table[i].command == command) 
            {
                table[i].handler(command,prtData,(uint8_t)len);
                cb_done = APP_TRUE;
                break;
            }
        }
    }

    return cb_done;
}



/**
 * @brief 
 * 
 * @param command read firmware version.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void dfu_read_fw_version(uint16_t command, uint8_t *buf, uint8_t len)
{
    LOG("%s\r\n",__func__);
    uint32_t current_fw_ver = 0;//*(__IO uint32_t*)FirmwareVersion;
    memcpy(&current_fw_ver,&FirmwareVersion,sizeof(current_fw_ver));
    uint16_t rsp_ver = (uint16_t)BigLittleSwap16(current_fw_ver);

    uint8_t respondLen = sizeof(rsp_ver);
    dfu_command_respond_port(command,(uint8_t*)&rsp_ver,respondLen);
}


/**
 * @brief 
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void dfu_reset_chip(uint16_t command, uint8_t *buf, uint8_t len)
{
    LOG("%s\r\n",__func__);
    uint8_t statuscode = 0;
    uint8_t respondLen = sizeof(statuscode);
    dfu_command_respond_port(command,&statuscode,respondLen);

    #ifndef BOOT
    #include "CB_ble.h" 
    BLE_DeInit();
    #endif
    NVIC_SystemReset();
}



/**
 * @brief 
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void dfu_upgrade_start(uint16_t command, uint8_t *buf, uint8_t len)
{
    LOG("%s\r\n",__func__);
    uint8_t statuscode = 0;
    uint32_t current_fw_ver = 0;//*(__IO uint32_t*)FirmwareVersion;
    memcpy(&current_fw_ver,&FirmwareVersion,sizeof(current_fw_ver));
    //cmp fw version
    uint16_t new_fw_ver = (uint16_t)(buf[0]<<8|buf[1]);
    if(new_fw_ver >= current_fw_ver) // debug
    {
        dfu_active_flag = APP_TRUE;  
        dfu_addr_offset = 0;
        dfu_fw_ver = new_fw_ver;
    }
    else if(new_fw_ver == current_fw_ver)
    {
        statuscode = 0x01; // the same version
    }
    else{
        statuscode = 0x02; // lower version
    }

    uint8_t respondLen = sizeof(statuscode);
    dfu_command_respond_port(command,&statuscode,respondLen);
}


/**
 * @brief 
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void dfu_upgrade_pack(uint16_t command, uint8_t *buf, uint8_t len)
{
    LOG("%s\r\n",__func__);

    uint8_t statuscode = 0;
    uint32_t offser = (uint32_t)(buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3]);
    uint8_t  pack_size = buf[4];
    uint8_t *p_data =  &buf[5];
    uint8_t idx_crc = 5+pack_size;
    uint32_t pack_crc = (uint32_t)(buf[idx_crc+0]<<24|buf[idx_crc+1]<<16|buf[idx_crc+2]<<8|buf[idx_crc+3]);

    if(dfu_active_flag != APP_TRUE)
        return;

    if(dfu_addr_offset == 0)
    {
        //erase backup bank
        dfu_flash_erase_port(BACKUP_BANK_ADDRESS, FIRMWARE_BANK_SIZE);
    }

    if(dfu_addr_offset != offser)
    {
        statuscode = 0x01; // offset err
    }
    else if(pack_size > 0 && pack_size <= 128)
    {
        uint32_t crc_check = dfu_crc_check_port(p_data,pack_size,0);
        if(crc_check == pack_crc)
        {
            #if defined FLASH_WRITE_BUFFER
            dfu_flash_write_buf_port(dfu_addr_offset, p_data, (uint32_t)pack_size);
            #else
            dfu_flash_write_port(BACKUP_BANK_ADDRESS+dfu_addr_offset, p_data, (uint32_t)pack_size);
            #endif
            dfu_addr_offset += pack_size;
        }
        else{
            statuscode = 0x03; //crc err
        }
    }
    else{
        statuscode = 0x02; //len err
    }
    uint8_t respondLen = sizeof(statuscode);
    dfu_command_respond_port(command,&statuscode,respondLen);
}

/**
 * @brief 
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void dfu_upgrade_verify(uint16_t command, uint8_t *buf, uint8_t len)
{
    LOG("%s\r\n",__func__);
    uint8_t statuscode = 0;
    uint32_t fw_crc = (uint32_t)(buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3]);
    uint32_t read_size = 0;
    uint32_t read_addr = 0;
    uint8_t readbuf[OTA_PACK_MAX];

    #if defined FLASH_WRITE_BUFFER
    //comfirm the last pack has been write
    dfu_flash_write_buf_port(dfu_addr_offset, readbuf, 0);
    #endif

    uint32_t crc_check = dfu_firmware_crc_check(BACKUP_BANK_ADDRESS,dfu_addr_offset);
    if(crc_check == fw_crc)
    {
        //verify pass, update boot setting
        dfu_bootsetting_update(dfu_fw_ver,dfu_addr_offset,fw_crc);
    }
    else{
        statuscode = 0x01;
    }
    uint8_t respondLen = sizeof(statuscode);
    dfu_command_respond_port(command,&statuscode,respondLen);
}

/**
 * @brief 
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void dfu_upgrade_finish(uint16_t command, uint8_t *buf, uint8_t len)
{
    LOG("%s\r\n",__func__);
    uint8_t statuscode = 0;
    dfu_active_flag = APP_FALSE; 
    uint8_t respondLen = sizeof(statuscode);
    dfu_command_respond_port(command,&statuscode,respondLen);
}


static void boot_enterApp(__IO uint32_t pc, __IO uint32_t sp) __attribute__((optnone))
{
    __asm("           \n\
          msr msp, r1 /* load r1 into MSP */\n\
          bx r0       /* branch to the address at r0 */\n\
    ");
}

static void boot_jumpAddress(uint32_t address)__attribute__((optnone))
{
    if(g_dfu_cfg.reinit)
    g_dfu_cfg.reinit();

    address -= 0x1000; // FLASH_ADDR to MCU_ADDR
    __IO uint32_t pc = *(__IO uint32_t *)(address+4);
    __IO uint32_t sp = *(__IO uint32_t *)address;
    boot_enterApp(pc,sp);
}
void dfu_jump_application(uint16_t command, uint8_t *buf, uint8_t len)
{
    LOG("%s\r\n",__func__);
    uint8_t statuscode = 0;
    uint32_t fw_addr = (uint32_t)(buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3]);
    uint8_t respondLen = sizeof(statuscode);
    dfu_command_respond_port(command,&statuscode,respondLen);

    boot_jumpAddress(fw_addr); 
}

void boot_copy_firmware(uint32_t dest, uint32_t src, uint32_t size )
{
    uint32_t read_size = 0;
    uint32_t offset = 0;
    uint8_t readbuf[FLASH_PAGE_SIZE];
    //erase dest 
    dfu_flash_erase_port(dest, size);

    //copy src to dest
    while(offset < size)
    {
        read_size = size-offset;
        if(read_size > FLASH_PAGE_SIZE) read_size = FLASH_PAGE_SIZE;
        dfu_flash_read_port(src+offset, (uint8_t*)&readbuf, read_size);
        dfu_flash_write_port(dest+offset, (uint8_t*)&readbuf, read_size);
        offset += read_size;
    }
}

uint32_t boot_move_backup_to_app(bootsetting_info_t *p_info)
{
    uint8_t copy_repeat = 3;
    uint32_t crc_check;
    while (--copy_repeat)
    {
        //copy firmware dato from backup to APP
        boot_copy_firmware(p_info->backup_bank.fw_start_addr,p_info->backup_bank.fw_load_addr,p_info->backup_bank.fw_size);
        //check crc after copy
        crc_check = dfu_firmware_crc_check(p_info->backup_bank.fw_start_addr,p_info->backup_bank.fw_size);
        if(crc_check == p_info->backup_bank.fw_crc)
        {
            //copy bootsetting info
            memcpy(&p_info->app_bank,&p_info->backup_bank,sizeof(bank_info_t)); 
            //disable backup bank
            p_info->backup_bank.fw_active = APP_FALSE;
            p_info->boot_mode = APP_FALSE;
            //write bootsetting
            dfu_bootsetting_write(p_info);

            // reply software reset command
            uint8_t statuscode = 0;
            dfu_command_respond_port(CMD_REST_DEV,&statuscode,1);
            //reload from flash and reset
            cb_deep_sleep_control(10);
            return APP_TRUE;   
        }
    }
    return APP_FALSE;
}

uint32_t dfu_boot_startup(void)
{
    uint32_t crc_check;
    bootsetting_info_t bootsetting;
    uint32_t rt = dfu_bootsetting_read(&bootsetting);
    if(rt == APP_TRUE)
    {
        if(bootsetting.boot_mode != APP_TRUE)
        {
            if(bootsetting.backup_bank.fw_active != APP_TRUE)
            {
                #if 1
                //check app crc  
                crc_check = dfu_firmware_crc_check(bootsetting.app_bank.fw_start_addr,bootsetting.app_bank.fw_size);
                if(crc_check == bootsetting.app_bank.fw_crc && bootsetting.app_bank.fw_start_addr >= 0x1000 && bootsetting.app_bank.fw_size)
                #else
                if(bootsetting.app_bank.fw_start_addr >= 0x1000)
                #endif
                {
                    boot_jumpAddress(bootsetting.app_bank.fw_start_addr); //jump to APP
                }
                else{
                    //check backup bank
                    crc_check = dfu_firmware_crc_check(bootsetting.backup_bank.fw_load_addr,bootsetting.backup_bank.fw_size);
                    if(crc_check == bootsetting.backup_bank.fw_crc && bootsetting.backup_bank.fw_start_addr >= 0x1000 && bootsetting.backup_bank.fw_size)
                    {
                        //copy backup to app
                        if(boot_move_backup_to_app(&bootsetting)== APP_TRUE)
                        {
                            NVIC_SystemReset();// reset to app
                        }
                    }
                }
            }
            else{
                //check backup bank
                crc_check = dfu_firmware_crc_check(bootsetting.backup_bank.fw_load_addr,bootsetting.backup_bank.fw_size);
                if(crc_check == bootsetting.backup_bank.fw_crc && bootsetting.backup_bank.fw_start_addr >= 0x1000)
                {
                    //copy backup to app
                    if(boot_move_backup_to_app(&bootsetting)== APP_TRUE)
                    {
                    NVIC_SystemReset();// reset to app
                    }
                }
            }
        }
    }

    // if jump app fail, enter boot mode
    return APP_FALSE;
} 


