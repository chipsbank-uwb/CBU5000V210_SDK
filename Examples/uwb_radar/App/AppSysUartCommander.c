/**
 * @file    AppSysUartCommander.c
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
#include <ctype.h>
#include "AppSysUartCommander.h"
#include "AppUwbTRXMemoryPool.h"
#include "CB_system.h"
#include "TaskHandler.h" /**< For task flags */
#include "CB_Uart.h"
#include "CB_uwbframework.h"
#include "AppUwbRadar.h"
#include "AppRadarPersist.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_SYS_UARTCOMMANDER_UARTPRINT_ENABLE APP_TRUE
#if (APP_SYS_UARTCOMMANDER_UARTPRINT_ENABLE == APP_TRUE)
#include "app_uart.h"
#define APP_SYS_UARTCOMMANDER_PRINT(...) app_uart_printf(__VA_ARGS__)
#else
#define APP_SYS_UARTCOMMANDER_PRINT(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void processUartRxBuffer(uint8_t *buf);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint8_t s_receivedByteLength;
static uint8_t s_uartRxBuffer[256];

static app_uart_cmd_st commandTable[] = 
{
    {'c', APP_UART_Func_c},  // RADAR Configuration and Run
    {'f', APP_UART_Func_f},  // RADAR Stop
    // Add more commands and handlers as needed
};
extern uint8_t CB_GetCBLibMajorVersion(void);
extern uint8_t CB_GetCBLibMinorVersion(void);
extern uint8_t CB_GetCBLibPatchVersion(void);
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Callback function for handling UART received bytes.
 * 
 * This function is called whenever a byte is received on UART.
 * It handles backspace, end of command ('\r'), and skips spaces.
 * 
 * @param receivedByte The byte received on UART.
 */
void app_uart_0_rxd_ready_callback(void)
{
  uint8_t received = cb_uart_get_rx_byte(EN_UART_0);
  if (received == '\b' && s_receivedByteLength > 0) 
  {
    s_receivedByteLength--;    // Handle backspace: remove the last character from the buffer
  } 
  else if (received == '\r') 
  {
    // End of command, parse and execute
    s_uartRxBuffer[s_receivedByteLength] = '\0'; // Null-terminate the string
    processUartRxBuffer(s_uartRxBuffer);
    s_receivedByteLength = 0; // Reset buffer
  } 
  else if (!isspace(received)) // Skip spaces
  {
    s_uartRxBuffer[s_receivedByteLength++] = received;  // Add received character to buffer
  }
}


void app_uart_0_rxb_full_callback(void)
{
  uint8_t received;
  cb_uart_get_rx_buffer(EN_UART_0,&received, 1);

  if (received == '\b' && s_receivedByteLength > 0) 
  {
      s_receivedByteLength--;    // Handle backspace: remove the last character from the buffer
  } 
  else if (received == '\r') 
  {
    // End of command, parse and execute
    s_uartRxBuffer[s_receivedByteLength] = '\0'; // Null-terminate the string
    processUartRxBuffer(s_uartRxBuffer);
    s_receivedByteLength = 0; // Reset buffer
  } 
  else if (!isspace(received)) // Skip spaces
  {
    s_uartRxBuffer[s_receivedByteLength++] = received;  // Add received character to buffer
  }
}

/**
 * @brief Process UART receive buffer.
 * 
 * This function processes the UART receive buffer by separating the command
 * and arguments, converting argument strings to integers, and then executing
 * the corresponding command handler function.
 * 
 * @param ptrUartRxBuffer Pointer to the UART receive buffer.
 */
void processUartRxBuffer(uint8_t *ptrUartRxBuffer)
{
  char *command;
  uint32_t args[16]; // reduce number of args to 10
  uint32_t *argPtr;
  uint8_t argCount = 0;
  
  // Separate command and arguments using strtok
  char *token = strtok((char*)ptrUartRxBuffer, ",");
  if (token != NULL) 
  {
    command = token; // Extract the first character as the command
    argPtr = args;
    while ((token = strtok(NULL, ",")) != NULL && argCount < sizeof(args)/sizeof(args[0]))
    {
      int argInt = atoi(token);     // Convert argument string to int
      *argPtr++ = (uint32_t)argInt; // Convert int to uint32_t and store in args
      argCount++;
    }
    
    static uint32_t commandTblSize = sizeof(commandTable) / sizeof(commandTable[0]);
    commandTblSize = sizeof(commandTable) / sizeof(commandTable[0]);
    // Find command in the lookup table
    for (uint8_t i = 0; i < commandTblSize; i++) 
    {
      if (commandTable[i].command == *command) 
      {
        APP_SYS_UARTCOMMANDER_PRINT("\n");
        commandTable[i].handler(argCount, args);
        break;
      }
    }    
  }
  memset(s_uartRxBuffer, 0, sizeof(s_uartRxBuffer));
  memset(args,0,sizeof(args));
  APP_SYS_UARTCOMMANDER_PRINT("\n>");
}


void APP_UART_Func_c(uint32_t const argc, uint32_t *args)
{

}

void APP_UART_Func_f(uint32_t const argc, uint32_t *args)
{
		uint32_t mode_PRF = args[0];
		uint32_t power_code = args[1];
		uint32_t scale_bit = args[2];
		uint32_t gain_idx = args[3];
		uint32_t CIR_tap_start = args[4];
		uint32_t CIR_tap_end = args[5];
		uint32_t sensing_mode = args[6];
		uint32_t sensing_burst_num = args[7];
		uint32_t sensing_interval_ms = args[8];
		uint32_t frame_interval_ms = args[9];
		uint32_t num_cir_per_frame = args[10];
		uint32_t deep_sleep_en = args[11];
		uint32_t num_rx_mode = args[12];
		uint32_t preamble_index = args[13];
	
    if (argc != 14 && mode_PRF == 0)
    {
        APP_RadarStop();
        g_task_f_execute = APP_FALSE;
        AppRadarPersist_DisablePersistMode();
        return;
    }
		
    if (argc != 14)
    {
        APP_SYS_UARTCOMMANDER_PRINT("Error: Wrong number of arguments. Expected 14, got %d\n", argc);
        return;
    }

    // f,mode_PRF,power_code,scale_bit,gain_idx,CIR_tap_start,CIR_tap_end,sensing_mode,sensing_burst_num,sensing_interval_ms,frame_interval_ms,num_cir_per_frame,deep_sleep_en,num_rx_mode,preamble_index
		// f,0,40,6,4,0,20,1,1,1,500,8,0,2,9
		uint8_t init_ok = APP_RadarInit(mode_PRF, power_code, scale_bit, gain_idx, CIR_tap_start, CIR_tap_end,
		                                sensing_mode, sensing_burst_num, sensing_interval_ms, frame_interval_ms,
		                                num_cir_per_frame, deep_sleep_en, num_rx_mode, preamble_index);
		
		// continuous example
		// f,0,40,6,4,0,50,0,1,100,500,8,0,2,9
		
		// f,0,40,4,5,0,240,0,1,250,500,8,0,2,9

		if (init_ok == APP_TRUE)
		{
			if (deep_sleep_en == 1U)
			{
				AppRadarPersist_SaveFromUartArgs(mode_PRF, power_code, scale_bit, gain_idx, CIR_tap_start, CIR_tap_end,
				                                 sensing_mode, sensing_burst_num, sensing_interval_ms, frame_interval_ms,
				                                 num_cir_per_frame, deep_sleep_en, num_rx_mode, preamble_index);
			}
			else
			{
				AppRadarPersist_DisablePersistMode();
			}
		}

    g_task_f_execute = init_ok;
}

/**
 * @brief   Prints the version of the CB Library.
 * 
 * This function prints the version of the CB Library in the format "CB LIB version: X.Y.Z",
 * where X is the major version, Y is the minor version, and Z is the patch version.
 * 
 * @note    This function requires CB_LIB_MAJOR_VERSION, CB_LIB_MINOR_VERSION, and CB_LIB_PATCH_VERSION
 *          to be defined appropriately.
 */
void APP_UART_PrintCBLibVersion(void)
{
//  APP_SYS_UARTCOMMANDER_PRINT("CB LIB version: %d.%d.%d\n", CB_GetCBLibMajorVersion(), CB_GetCBLibMinorVersion(), CB_GetCBLibPatchVersion());
//  APP_SYS_UARTCOMMANDER_PRINT("\n>");
}

/**
 * @brief   Prints the application name
 * 
 * This function prints name of application example [APP_UWB_EXAMPLE]
 * 
 */
void APP_UART_PrintAppName(void)
{
//  APP_SYS_UARTCOMMANDER_PRINT("[APP_UWB_RADAR]\n");
}

void cb_uart_0_rxb_full_app_irq_callback(void){ app_uart_0_rxb_full_callback();  } /* Direct callback here for improved IRQ processing time in current SDK */
