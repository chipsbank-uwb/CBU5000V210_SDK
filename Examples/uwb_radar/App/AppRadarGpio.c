/**
 * @file    AppRadarGpio.c
 * @brief   GPIO7 input gate - high/floating allows radar + deep-sleep loop; GND holds device awake for UART.
 */

#include "AppRadarGpio.h"
#include "CB_gpio.h"
#include "CB_scr.h"

#define APP_RADAR_GPIO_RUN_PIN  EN_GPIO_PIN_7

/**
 * @brief Configures GPIO7 as the radar run-enable input.
 */
void AppRadarGpio_Init(void)
{
  stGPIO_InitTypeDef run_pin;

  cb_scr_gpio_module_on();
  run_pin.Pin = APP_RADAR_GPIO_RUN_PIN;
  run_pin.Mode = EN_GPIO_MODE_INPUT;
  run_pin.Pull = EN_GPIO_PULLUP;
  cb_gpio_init(&run_pin);
}

/**
 * @brief Returns whether radar operation and auto-resume are allowed.
 * @return 1 if GPIO7 reads high, 0 if grounded low.
 */
uint8_t AppRadarGpio_IsRunAllowed(void)
{
  return (cb_gpio_read_input_pin(APP_RADAR_GPIO_RUN_PIN) == EN_GPIO_PIN_SET) ? 1U : 0U;
}
