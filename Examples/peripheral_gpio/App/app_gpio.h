/**
 * @file    app_gpio.h
 * @brief   GPIO Application Module Header
 * @details This module provides functions for GPIO initialization, control, reading inputs, and handling interrupts for different applications.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_CPU_GPIO_H
#define __APP_CPU_GPIO_H

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
void app_gpio_init(void);
void app_gpio_toggle_led(void);
void app_gpio_set_led2(uint8_t state);
void app_gpio_set_led(uint8_t state);
uint8_t app_gpio_read_sensor(void);
void app_gpio_test_deinit(void);

void app_gpio_irq_callback(void);
void app_peripheral_gpio_demo_init(void);
void app_peripheral_gpio_demo_loop(void);

#endif  

