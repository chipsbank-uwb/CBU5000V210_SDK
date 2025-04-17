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

#if (APP_UWB_RX_PER_ENABLE == APP_TRUE)
#include "AppUwbRxPer.h"
#endif
#if (APP_UWB_RADAR_ENABLE == APP_TRUE)
#include "App/AppUwbRadar.h"
#endif

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
    {'e', APP_UART_Func_e},  // TRX-Periodic
    {'f', APP_UART_Func_f},  // Radar
    {'g', APP_UART_Func_g},  // RXPER
    {'h', APP_UART_Func_h},  // UWB Scanning
    {'s', APP_UART_Func_s},  // General Configuration Setting
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


 /**
 * @brief TRX-Periodic Command parser for customer.
 * 
 * @param args Pointer to the array of arguments.
 */
void APP_UART_Func_e(uint32_t const argc, uint32_t *args)
{
  /* usage: unused  */  
}

/**
 * @brief Radar Command parser for customer.
 * 
 * @param args Pointer to the array of arguments.
 */
void APP_UART_Func_f(uint32_t const argc, uint32_t *args) {
#if (APP_UWB_RADAR_ENABLE == APP_TRUE)
  /* usage: f,arg1,arg2,arg3,arg4
  (arg1) sampling frequency  Typically 10(Hz)
  (arg2) pa code index       [1,31]
  (arg3) scale bit           [0,7]
  (arg4) gain idx            [0,7]
  */  
  uint32_t command = args[0];
  uint32_t pa = args[1];
  uint32_t scale_bit = args[2];
  uint32_t gain_idx = args[3];
  
  if (command == APP_FALSE)
  {
    APP_SYS_UARTCOMMANDER_PRINT("[APP_RADAR_Stop]\n");
    APP_RadarStop();
  }
  else
  {
    APP_RadarInit(command, pa, scale_bit, gain_idx);
    g_task_g_execute = APP_TRUE;
  }
#endif
}

/**
 * @brief RX Packet Error Rate (RX PER) Command parser for internal test.
 * 
 * @param args Pointer to the array of arguments.
 */
void APP_UART_Func_g(uint32_t const argc, uint32_t *args) 
{
#if (APP_UWB_RX_PER_ENABLE == APP_TRUE)
   /* usage: g,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8
  (arg0) rx port               0: Rx0
                               1: Rx1
                               2: Rx2
  (arg1) receive duration      Typically 10000 (ms)
  (arg2) num of cir taps       default 3
  (arg3) cir window            default 11
  (arg4) preamble detection    {1800: m1p2,m3p2}; {3000: others}
         threshold             
  (arg5) preamble detection    {7: m1p2}; {5: m3p2}; {3:others}
          count threshold 
  (arg6) out_hold_cnt         {3: m1p2,m3p2}; {4: others}
  */  
  APP_SYS_UARTCOMMANDER_PRINT("%s\n", __PRETTY_FUNCTION__);

  uint8_t rxPortCommand             = (uint8_t)*(args + 0);
  cb_uwbsystem_rxport_en enRxPort 	= EN_UWB_RX_0;
  switch (rxPortCommand)
  {
	case 0: enRxPort = EN_UWB_RX_0;   break;
	case 1: enRxPort = EN_UWB_RX_1;   break;
	case 2: enRxPort = EN_UWB_RX_2;   break;
	case 3: enRxPort = EN_UWB_RX_ALL; break;
	default: break;
  }  
  uint32_t rxTimeIntervalInMs  = *(args + 1);
  app_uwbtrx_rx_per(enRxPort, rxTimeIntervalInMs);
#endif
}

/**
 * @brief Handles UART command processing for UWB preamble scanning operation mode.
 *
 * This function processes UART commands by configuring UWB preamble scanning
 * based on the given arguments. It initializes the UWB operation mode, sets up 
 * the preamble scanning parameters, and triggers the appropriate UWB operations 
 * based on the mode (RX or TX). 
 *
 * @param[in] argc The number of arguments passed to the function. Minimum value is 1.
 * @param[in] args A pointer to the array of arguments:
 *            - args[0]: UWB operation mode (must be cast to uint8_t).
 *            - args[1]: PSR mode in case of RX
 *            - args[2]: Scan duration in case of RX
 */
void APP_UART_Func_h(uint32_t const argc, uint32_t  *args)
{
	#define PREAMBLE_SCANNING_OPERATION_MODE_Suspend 0
	
}

/**
 * @brief Configures the UWB system parameters via UART command.
 * 
 * This function parses the provided arguments and sets the UWB system 
 * configuration. It also retrieves and prints the updated configuration.
 *
 * @param argc Number of arguments provided.
 * @param args Pointer to the array of arguments.
 * 
 * @details Usage: `s,arg0,arg1,arg2,arg3,arg4,arg5,arg6`
 * - `arg0`: TX Power Code. Refer to the power code table.
 * - `arg1`: TX & RX Packet Mode:
 *   - 0: BPRF_0681  (m5p1)
 *   - 1: BPRF_0850  (m3p2)
 *   - 2: BPRF_0850A (m1p2) - not supported yet
 *   - 3: BPRF_0681A (m5p2) - not supported yet
 *   - 4: HPRF_6810  (m4p1)
 *   - 5: HPRF_7800  (m4p2)
 *   - 6: HPRF_27200 (m4p3)
 *   - 7: HPRF_32100 (m4p4)
 * - `arg2`: TX & RX STS Mode:
 *   - 0: SP0, 1 STS Segment
 *   - 1: SP1, 1 STS Segment
 *   - 2: SP3, 1 STS Segment
 * - `arg3`: TX & RX Preamble Code Index. Range: [9, 32]. Default: {9: BPRF}, {25: HPRF}.
 * - `arg4`: TX & RX SFD Sequence Index. Range: [0, 4]. Default: 2.
 * - `arg5`: RX Operational Mode:
 *   - 0: Sensitivity mode
 *   - 1: Coexist mode
 * - `arg6`: Antenna Index.
 *
 * Usage: ``s`` with no other arguments to view the current configurations
 */
void APP_UART_Func_s(uint32_t const argc, uint32_t  *args){
   /* usage: s,arg0,arg1,arg2,arg3,arg4,arg5,arg6
  (arg0) tx power code                 refer to power code table
  (arg1) tx&rx packet mode             0: BPRF_0681  (m5p1)
                                       1: BPRF_0850  (m3p2)
                                       2: BPRF_0850A (m1p2) - not supported yet
                                       3: BPRF_0681A (m5p2) - not supported yet
                                       4: HPRF_6810  (m4p1)
                                       5: HPRF_7800  (m4p2)
                                       6: HPRF_27200 (m4p3)
                                       7: HPRF_32100 (m4p4)
  (arg2) tx&rx sts mode                0: SP0, 1 STS Segment
                                       1: SP1, 1 STS Segment
                                       2: SP3, 1 STS Segment
  (arg3) tx&rx preamble code index     [9, 32]. Default {9: BPRF}; {25: BPRF}
  (arg4) tx&rx sfd sequence index      [0, 4]. Default 2
  (arg5) rx operational mode           0: Sensitivity mode
                                       1: Coexist mode
  (arg6) antenna index
  */  
  
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
