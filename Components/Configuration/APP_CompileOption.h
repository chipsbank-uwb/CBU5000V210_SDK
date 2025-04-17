/*
 * @file    APP_CompileOption.h
 * @brief   Header file for the compiler switch.
 * @details This header file declares the compiler swtich that will be used
 *          across the user modules. 
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_COMIPLIE_OPTION_H
#define __APP_COMIPLIE_OPTION_H
#include "APP_common.h"

#define APP_UWB_RADAR_ENABLE          APP_FALSE
#define APP_UWB_RX_PER_ENABLE         APP_FALSE
#define APP_DEMO_ENABLE               APP_FALSE
#define APP_FREERTOS_ENABLE           APP_FALSE
#define APP_COMMTRX_Qmode             APP_TRUE

#define APP_BLE_ENABLE                APP_FALSE
#endif /*__APP_COMIPLIE_OPTION_H*/
