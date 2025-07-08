/**
 * @file    AppFactoryCmd.h
 * @brief   
 * @details 
 *          
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __mp_factory_COMMANDER_H
#define __mp_factory_COMMANDER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"
#include "cmd_parser_uart.h"
//-------------------------------
// DEFINE SECTION
//-------------------------------
#define OFF_CMD (0u)
#define PERIODIC_RX_CMD (1u)
#define PERIODIC_TX_CMD (2u)

//-------------------------------
// ENUM SECTION
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
void ftm_handler_init(void);
uint8_t ftm_halder_polling(uint16_t command, uint8_t *prtData, uint16_t len, cmdhandler_t responder);
uint8_t ftm_halder_get_state(void);
#endif /*__APP_SYS_UART_COMMANDER_H*/
