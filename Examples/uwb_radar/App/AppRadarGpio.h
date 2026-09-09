/**
 * @file    AppRadarGpio.h
 * @brief   GPIO gate for radar run and deep-sleep auto-resume.
 * @details GPIO7 input with pull-up: high or floating allows radar operation and
 *          deep-sleep auto-resume; grounding GPIO7 holds the device awake for UART
 *          configuration.
 */
#ifndef APP_RADAR_GPIO_H
#define APP_RADAR_GPIO_H
#include "APP_common.h"
/**
 * @brief Configures GPIO7 as the radar run-enable input.
 */
void AppRadarGpio_Init(void);
/**
 * @brief Returns whether radar operation and auto-resume are allowed.
 * @return 1 if GPIO7 reads high, 0 if grounded low.
 */
uint8_t AppRadarGpio_IsRunAllowed(void);
#endif /* APP_RADAR_GPIO_H */
