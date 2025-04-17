/**
 * @file    AppOTA.h
 * @brief   [SYSTEM] Header file for the UART Commander Application Module
 * @details This header file declares the UART Commander module and includes necessary
 *          headers for function prototypes, definitions, enums, structs/unions, and global variables.
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __APP_OTA_H
#define __APP_OTA_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>


  
  
//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_RXMARKER_VAL        0x5A
/*SizeInByte*/
#define DEF_RXMARKER_SIZE       1
#define DEF_CMD_SIZE            2
#define DEF_RESP_SIZE           1
#define DEF_DL_SIZE             1	
#define DEF_HEADER_SIZE         (DEF_RXMARKER_SIZE+DEF_CMD_SIZE+DEF_RESP_SIZE+DEF_DL_SIZE)
#define DEF_CHECKSUM_SIZE       1	

/*Position*/
#define DEF_RXMARKER_POS	    0
#define DEF_CMD_POS             (DEF_RXMARKER_POS + DEF_RXMARKER_SIZE)
#define DEF_RESP_POS            (DEF_CMD_POS + DEF_CMD_SIZE)
#define DEF_DL_POS              (DEF_RESP_POS + DEF_RESP_SIZE)	
#define DEF_DATA_POS	        (DEF_DL_POS + DEF_DL_SIZE)

//-------------------------------
// ENUM SECTION
//-------------------------------
#ifndef APP_TRUE
#define APP_TRUE   1
#endif
#ifndef APP_FALSE
#define APP_FALSE  0
#endif

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef void (*dfu_cmdhandler_t)(uint16_t, uint8_t*, uint8_t);
typedef void (*dfu_reinit_t)(void);
typedef struct {
    dfu_cmdhandler_t responder;
    dfu_reinit_t      reinit;
} dfu_cfg_t;



typedef struct {
    uint32_t fw_start_addr;
    uint32_t fw_load_addr;
    uint32_t fw_size;
    uint32_t fw_crc;
    uint32_t fw_verson;
    uint32_t fw_active;
} bank_info_t;

typedef struct {
    uint32_t      data_crc;
    uint32_t      boot_mode;
    bank_info_t   app_bank;
    uint32_t      reserve0[4];
    bank_info_t   backup_bank;
    uint32_t      reserve1[4];
    uint32_t      ecc_public_key[64];
} bootsetting_info_t;
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void    dfu_halder_init(dfu_cfg_t* p_cfg);
uint8_t dfu_halder_get_state(void);
uint8_t dfu_halder_polling(uint16_t command, uint8_t *prtData, uint16_t len, dfu_cmdhandler_t responder);
void     dfu_software_enter_bootmode(void);
uint32_t dfu_boot_startup(void);
#endif /*__APP_OTA_H*/
