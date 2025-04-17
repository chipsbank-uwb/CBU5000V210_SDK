/**
 * @file    CB_gpio.h
 * @brief   Header file for GPIO driver
 * @details This file contains definitions and declarations for the GPIO driver.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef INC_GPIO_H
#define INC_GPIO_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

/**   
  * @brief  GPIO_InitTypeDef Pin Group
  */
typedef enum
{
  EN_GPIO_PIN_0 = 0U,   /* Pin 0*/
  EN_GPIO_PIN_1     ,   /* Pin 1*/
  EN_GPIO_PIN_2     ,   /* Pin 2*/
  EN_GPIO_PIN_3     ,   /* Pin 3*/
  EN_GPIO_PIN_4     ,   /* Pin 4*/
  EN_GPIO_PIN_5     ,   /* Pin 5*/
  EN_GPIO_PIN_6     ,   /* Pin 6*/
  EN_GPIO_PIN_7     ,   /* Pin 7*/
  EN_GPIO_PIN_8     ,   /* Pin 8*/
  EN_GPIO_PIN_9     ,   /* Pin 9*/
  EN_GPIO_PIN_10    ,   /* Pin 10*/
  EN_GPIO_PIN_11    ,   /* Pin 11*/
  
}enGpioPin;


/** 
  * @brief  GPIO_InitTypeDef Mode Group
  */
typedef enum
{
  EN_GPIO_MODE_INPUT              = 0U,   /*!< GPIO Init Mode as Input  */
  EN_GPIO_MODE_OUTPUT                 ,   /*!< GPIO Init Mode as Ouput   */
  EN_GPIO_MODE_IT_RISING              ,   /*!< GPIO Init Mode as External Interrupt Mode with Rising edge trigger detection  */
  EN_GPIO_MODE_IT_FALLING             ,   /*!< GPIO Init Mode as External Interrupt Mode with Falling edge trigger detection */
  EN_GPIO_MODE_IT_RISING_FALLING          /*!< GPIO Init Mode as External Interrupt Mode with Rising/Falling edge trigger detection  */
}enGpioMode;                       

/** 
  * @brief GPIO_InitTypeDef Pull Group
  * @{
  */

typedef enum
{
  EN_GPIO_PULLUP    = 0U,   /*!< Pull-up                  */
  EN_GPIO_PULLDOWN      ,   /*!< Pull-down                */
  EN_GPIO_NOPULL            /*!< No Pull-up or Pull-down  */
}enGpioPull;               

/**
  * @brief  GPIO Bit DRIVER_SET and Bit RESET enumeration
  */
typedef enum
{
  EN_GPIO_PIN_RESET = 0U,  /**< GPIO pin state: Reset */
  EN_GPIO_PIN_SET          /**< GPIO pin state: Set */
} enGPIO_PinState;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
  enGpioPin    Pin  ;            /* GPIO_PIN Selected to Init. Selection refer to GPIO_InitTypeDef Pin Group  */
  enGpioMode   Mode ;            /* Mode Selected to Init. Selection refer to GPIO_InitTypeDef Mode Group    */
  enGpioPull   Pull ;            /* Configure Pull Up/Down for Output Pin. Selection refer to GPIO_InitTypeDef TrigSel Group */
} stGPIO_InitTypeDef;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief       Initialize GPIO pins.
 * @details     This function initializes GPIO pins based on the provided configuration.
 * @param[in]   cb_gpio_init Pointer to a structure containing GPIO initialization parameters.
 */
void cb_gpio_init(stGPIO_InitTypeDef* GPIO_Init);

/**
 * @brief       Deinitialize GPIO pins.
 * @details     This function deinitializes GPIO pins based on the provided pin mask.
 * @param[in]   GPIO_Pin The pin or pins to be deinitialized.
 */
void cb_gpio_deinit(enGpioPin GPIO_Pin);

/**
 * @brief       Initialize GPIO configuration structure with default values.
 * @details     This function initializes the provided GPIO initialization structure 
 *              with default settings. The default configuration sets the mode to 
 *              output and the pull-up/pull-down resistor to no pull.
 *
 * @param[in]   GPIO_Init Pointer to a GPIO initialization structure that will be 
 *              initialized with default values.
 */
void cb_gpio_structinit(stGPIO_InitTypeDef* GPIO_Init);


/**
 * @brief       Read the input value of a GPIO pin.
 * @details     This function reads the input value of a specified GPIO pin.
 * @param[in]   GPIO_Pin The pin to be read.
 * @return      The input state of the GPIO pin (GPIO_PIN_SET or GPIO_PIN_RESET).
 */
enGPIO_PinState cb_gpio_read_input_pin(enGpioPin GPIO_Pin);

/**
 * @brief   Read the combined input value of all GPIO pins.
 * @details This function reads the combined input value of all GPIO pins and returns it as a single 32-bit value.
 * @return  A 32-bit value representing the input state of all GPIO pins.
 */
uint32_t cb_gpio_read_all_input_pins(void);

/**
 * @brief   Read the output value of a GPIO pin.
 * @details This function reads the output value of a specified GPIO pin and returns 
 * @param[in]   GPIO_Pin The pin to be read.
 * @return  The output state of the GPIO pin (EN_GPIO_PIN_SET for high, EN_GPIO_PIN_RESET for low).
 */
enGPIO_PinState cb_gpio_read_output_pin(enGpioPin GPIO_Pin);

/**
 * @brief   Read the combined output value of all GPIO pins.
 * @details This function reads the combined output value of all GPIO pins and returns it as a single 32-bit value.
 * @return  A 32-bit value representing the output state of all GPIO pins.
 */
uint32_t cb_gpio_read_all_output_pins(void);

/**
 * @brief       Write a value to a GPIO pin.
 * @details     This function writes a specified value to the output of a GPIO pin.
 * @param       GPIO_Pin The pin to be written to.
 * @param[in]   PinState The state to be written to the pin (GPIO_PIN_SET or GPIO_PIN_RESET).
 */
void cb_gpio_write_pin(enGpioPin GPIO_Pin,enGPIO_PinState PinState);

/**
 * @brief   Write a value to all GPIO pins.
 * @details This function writes a specified 32-bit value to the output of all GPIO pins.
 * @param   PinState The 32-bit value to be written to all GPIO pins.
 */
void cb_gpio_write_all_pins(uint32_t PinState);

/**
 * @brief       Toggle the output value of a GPIO pin.
 * @details     This function toggles the output value of a specified GPIO pin.
 * @param[in]   GPIO_Pin The pin to be toggled.
 */
void cb_gpio_toggle_pin(enGpioPin GPIO_Pin);

/**
 * @brief   Weakly defined GPIO interrupt callback function.
 * @details This function is intended to be overridden by the user to handle
 *          GPIO interrupt events in APP layer. By default, it clears the 
 *          trigger of the specified GPIO pin.
 */
void cb_gpio_app_irq_callback(void);

/**
 * @brief   GPIO interrupt handler function.
 * @details This function handles GPIO interrupts by disabling the GPIO IRQ,
 *          retrieving the trigger status, invoking the callback function with
 *          the trigger status as parameter, and then enabling the GPIO IRQ.
 */
void cb_gpio_irqhandler(void);


#endif /*INC_GPIO_H*/


