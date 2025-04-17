/**
 * @file    app_timer.h
 * @brief   [CPU Subsystem] Header file for the application timer module.
 * @details This file contains declarations and prototypes for the application timer module.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_CPU_TIMER_H
#define __APP_CPU_TIMER_H

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

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_timer_main(void);

void app_timer_0_irq_callback(void);
void app_timer_1_irq_callback(void);
void app_timer_2_irq_callback(void);
void app_timer_3_irq_callback(void);
void app_peripheral_timer_init(void);

#endif  // __APP_CPU_TIMER_H
