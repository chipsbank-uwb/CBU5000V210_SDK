/**
 * @file    app_wdt.h
 * @brief   [CPU Subsystem] Wdt Application Module Header
 * @details This header file contains function prototypes for Watchdog usage examples.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef INC_APP_WDT_H_
#define INC_APP_WDT_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
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
void app_wdt_demo_with_tasks(void);
void app_wdt_task_delay50ms_example(void);
void app_wdt_task_delay350ms_example(void);
void app_wdt_demo_with_force_reset(void);
void app_wdt_test(uint16_t timeout);
void app_peripheral_wdt_init(void);

#endif /* INC_APP_WDT_H_*/
