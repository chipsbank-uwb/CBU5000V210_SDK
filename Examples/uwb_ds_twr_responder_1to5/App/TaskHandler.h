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
extern uint8_t task_execute;

// Define a task handler
void TaskHandler(void);
#endif
