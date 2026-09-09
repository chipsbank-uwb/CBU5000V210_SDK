/**
 * @file    AppSysUartCommander.h
 * @brief   [SYSTEM] Header file for the UART Commander Application Module
 * @details This header file declares the UART Commander module and includes necessary
 *          headers for function prototypes, definitions, enums, structs/unions, and global variables.
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __APP_SYS_UART_COMMANDER_H
#define __APP_SYS_UART_COMMANDER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------


//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef void (*ptrFunction)(uint32_t const argc, uint32_t  *args);

typedef struct {
    char command;
    ptrFunction handler;
} app_uart_cmd_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void APP_UART_Func_c(uint32_t const argc, uint32_t  *args);

void APP_UART_Func_f(uint32_t const argc, uint32_t  *args);

/**
 * @brief   Prints the version of the CB Library.
 * 
 * This function prints the version of the CB Library in the format "CB LIB version: X.Y.Z",
 * where X is the major version, Y is the minor version, and Z is the patch version.
 * 
 * @note    This function requires CB_LIB_MAJOR_VERSION, CB_LIB_MINOR_VERSION, and CB_LIB_PATCH_VERSION
 *          to be defined appropriately.
 */
void APP_UART_PrintCBLibVersion(void);

/**
 * @brief   Prints the application name
 * 
 * This function prints name of application example [APP_UWB_EXAMPLE]
 * 
 */
void APP_UART_PrintAppName(void);

#endif /*__APP_SYS_UART_COMMANDER_H*/
