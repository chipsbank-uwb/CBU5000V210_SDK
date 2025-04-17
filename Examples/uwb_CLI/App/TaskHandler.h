/*
 * @file    Task.h
 * @brief   Header file for the task handler module.
 * @details This header file declares the TaskHandler function, which is responsible
 *          for handling tasks in the system. It includes necessary headers and
 *          provides function prototype for TaskHandler.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __TASK_H
#define __TASK_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

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
// GLOBAL VARIABLE SECTION
//-------------------------------
extern uint8_t g_task_a_tx_execute;
extern uint8_t g_task_a_rx_execute;
extern uint8_t g_task_b_ini_execute;
extern uint8_t g_task_b_resp_execute;
extern uint8_t g_task_c_ini_execute;
extern uint8_t g_task_c_resp_execute;
extern uint8_t g_task_d_ini_execute;
extern uint8_t g_task_d_resp_execute;
extern uint8_t g_task_e_execute;
extern uint8_t g_task_f_execute;
extern uint8_t g_task_g_execute;

// Define a task handler
void TaskHandler(void);
#endif
