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
#include "CB_aoa.h"
#include "AppUwbCommTrx.h"
#include "AppUwbDstwr.h"
#include "AppUwbPdoa.h"
#include "AppUwbRngAoa.h"

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
    {'a', APP_UART_Func_a},  // TRX
    {'b', APP_UART_Func_b},  // DSTWR
    {'c', APP_UART_Func_c},  // PDOA
    {'d', APP_UART_Func_d},  // RNGAOA
    {'e', APP_UART_Func_e},  // unused
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

/**
 * @brief Handles UART command processing for UWB TX and RX operation modes.
 *
 * This function processes UART commands to configure and execute UWB 
 * transmissions (TX) or receptions (RX). Depending on the operation mode, 
 * it sets up the required packet configurations, payloads, and ports, and 
 * triggers the appropriate UWB operations.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For TX: Must be 1.
 *                 - For RX: Must be 2.
 * @param[in] args A pointer to the array of arguments:
 *                 - For TX:
 *                   - args[0]: UWB operation mode (`1` for TX).
 *                 - For RX:
 *                   - args[0]: UWB operation mode (`2` for RX).
 */
void APP_UART_Func_a(uint32_t const argc, uint32_t  *args)
{
  /* TX usage: a,1
   * RX usage: a,2
  TX args:
  (arg0) 1
  ------------------------------------------------------
  RX args:
  (arg0) 2
  */
  
  #define DEF_UWB_OPERATION_MODE_TX  1
  #define DEF_UWB_OPERATION_MODE_RX  2
  
  uint8_t uwbOperationMode = (uint8_t)(*(args + 0));
  switch (uwbOperationMode)
  {
    case DEF_UWB_OPERATION_MODE_TX:
    {
      g_task_a_tx_execute = APP_TRUE;
    }
    break;
    case DEF_UWB_OPERATION_MODE_RX:
    {
      g_task_a_rx_execute = APP_TRUE;
    }
    break;
    default:
    break;
  }
}

/**
 * @brief Handles UART command processing for DSTWR (Distance Two-Way Ranging) operation modes.
 *
 * This function processes UART commands to configure and execute DSTWR 
 * operations, including Suspend, Responder, and Initiator modes. It initializes 
 * the configuration for ranging operations and supports extended configurations 
 * for additional parameters.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For Suspend: Must be 1.
 *                 - For Responder or Initiator: Must be 3 or more (extended configuration).
 * @param[in] args A pointer to the array of arguments:
 *                 - args[0]: DSTWR operation mode:
 *                   - `0`: Suspend
 *                   - `1`: Responder
 *                   - `2`: Initiator
 */
void APP_UART_Func_b(uint32_t const argc, uint32_t *args)
{  
  /* usage: b,arg1
  (arg1) Mode           0: Off
                        1: Initiator
                        2: Responder  
  */

  #define DSTWR_OPERATION_MODE_Suspend    0
  #define DSTWR_OPERATION_MODE_Initiator  1
  #define DSTWR_OPERATION_MODE_Responder  2
  
  uint32_t uwbOperationMode = (uint32_t)*(args + 0);

  switch (uwbOperationMode)
  {
    case DSTWR_OPERATION_MODE_Suspend:
    {
      app_dstwr_suspend();
    }
    break;
    case DSTWR_OPERATION_MODE_Initiator:
    {
      g_task_b_ini_execute = APP_TRUE;
    }
    break;
    case DSTWR_OPERATION_MODE_Responder:
    {
      g_task_b_resp_execute = APP_TRUE;
    }
    break;
    default:
    break;
  }
}

/**
 * @brief Handles UART command processing for PDOA operation modes.
 *
 * This function processes UART commands to configure and execute PDOA 
 * operations. Depending on the mode (Suspend, RX, or TX), it sets up the 
 * required configurations and triggers the appropriate operations.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For Suspend: Must be 1.
 *                 - For RX: Must be 2 or 3 (for extended configuration).
 *                 - For TX: Must be 2 or 3 (for extended configuration).
 * @param[in] args A pointer to the array of arguments:
 *                 - args[0]: PDOA operation mode:
 *                   - `0`: Suspend
 *                   - `1`: Initiator
 *                   - `2`: Responder
 */

void APP_UART_Func_c(uint32_t const argc, uint32_t *args)
{
  /* usage: c,arg1
  (arg1) Mode     0: SUSPEND
                  1: INITIATOR  
                  2: RESPONDER
  */  
  #define PDOA_OPERATION_MODE_Suspend    0
  #define PDOA_OPERATION_MODE_INITIATOR  1
  #define PDOA_OPERATION_MODE_RESPONDER  2

  
  uint8_t uwbOperationMode = (uint8_t)(*(args + 0));
  switch (uwbOperationMode)
  {
    case PDOA_OPERATION_MODE_Suspend:
    {
      app_pdoa_suspend();
    }
    break;
    case PDOA_OPERATION_MODE_INITIATOR:
    {
      g_task_c_ini_execute = APP_TRUE;
    }
    break;
    case PDOA_OPERATION_MODE_RESPONDER:
    {
      g_task_c_resp_execute = APP_TRUE;
    }
    break;
    default:
    break;
  }

}

/**
 * @brief Handles UART command processing for RNGAOA (Ranging with Angle of Arrival) operations.
 *
 * This function processes UART commands to configure and execute RNGAOA operations, 
 * including Suspend, Responder, and Initiator modes. It supports configuration of 
 * frequency, packet count, ranging and angle biases, and DS-PDOA activation.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For Suspend: Must be 1.
 *                 - For Responder or Initiator: Must be 1.
 * @param[in] args A pointer to the array of arguments:
 *                 - args[0]: RNGAOA operation mode:
 *                   - `0`: Suspend
 *                   - `1`: Initiator
 *                   - `2`: Responder
 */
void APP_UART_Func_d(uint32_t const argc, uint32_t *args)
{
  /* usage: d,arg1
  (arg1) Device Role    0: Off
                        1: Initiator 
                        2: Responder 
  */

  #define RNGAOA_OPERATION_MODE_Suspend    0
  #define RNGAOA_OPERATION_MODE_Initiator  1
  #define RNGAOA_OPERATION_MODE_Responder  2
  
  
  uint8_t uwbOperationMode = (uint8_t)(*(args + 0));
  switch (uwbOperationMode)
  {
    case RNGAOA_OPERATION_MODE_Suspend:
    {
      app_rngaoa_suspend();
    }
    break;
    case RNGAOA_OPERATION_MODE_Initiator:
    {
      g_task_d_ini_execute = APP_TRUE;
    }
    break;
    case RNGAOA_OPERATION_MODE_Responder:
    {
      g_task_d_resp_execute = APP_TRUE;
    }
    break;
    default:
    break;
  }
}


void APP_UART_Func_e(uint32_t const argc, uint32_t *args)
{
    /* usage: unused  */  
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
  APP_SYS_UARTCOMMANDER_PRINT("CB LIB version: %d.%d.%d\n", CB_GetCBLibMajorVersion(), CB_GetCBLibMinorVersion(), CB_GetCBLibPatchVersion());
  APP_SYS_UARTCOMMANDER_PRINT("\n>");
}

/**
 * @brief   Prints the application name
 * 
 * This function prints name of application example [APP_UWB_EXAMPLE]
 * 
 */
void APP_UART_PrintAppName(void)
{
  APP_SYS_UARTCOMMANDER_PRINT("[APP_UWB_CLI]\n");
}
