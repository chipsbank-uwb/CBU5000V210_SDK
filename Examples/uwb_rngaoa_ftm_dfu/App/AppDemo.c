/**
 * @file    AppDemo.c
 * @brief   Customer-Specified FTM Use Case Example
 * @details 
 * 
 * @author  Chipsbank
 * @date    2026
 */
#include <string.h>
#include "AppDemo.h"
#include "cmd_parser_uart.h"
#include "ftm_handler.h"
#include "dfu_handler.h"
#include "AppUwbRngAoa.h"

void APP_DEMO_Init(void)
{
   #if APP_RNGAOA_ROLE
      app_rngaoa_initiator();
   #else
      app_rngaoa_responder();
   #endif
}