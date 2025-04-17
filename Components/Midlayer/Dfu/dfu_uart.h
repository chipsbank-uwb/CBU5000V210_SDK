/**
 * @file    AppCmd.h
 * @brief   
 * @details 
 *          
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __APP_COMMANDER_H
#define __APP_COMMANDER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define OFF_CMD (0u)
#define PERIODIC_RX_CMD (1u)
#define PERIODIC_TX_CMD (2u)

#ifndef APP_TRUE
#define APP_TRUE   1
#endif
#ifndef APP_FALSE
#define APP_FALSE  0
#endif

enum
{
  UART_ROLE_EVK,
  UART_ROLE_67,  
};

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
//void APP_Command_init(uint32_t* p_hdl);
//void APP_Command_Task(void);
//void APP_Command_Respond(uint16_t command, uint8_t *buf, uint8_t len);
//void APP_Cmd_UartInit(uint8_t role);
//void APP_Cmd_UartDeInit(void);
//void app_uart_printf(const char *format, ...);
//uint8_t APP_cmd_get_state(void);


void dfu_uart_init(void);
void dfu_uart_polling(void);
#endif /*__APP_COMMANDER_H*/
