/**
 * @file    app_gpio.c
 * @brief   GPIO Application Module
 * @details This module provides functions for GPIO initialization, control, reading inputs, and handling interrupts for different applications.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "app_gpio.h"
#include "CB_gpio.h"
#include "CB_scr.h"
#include "AppSysIrqCallback.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_GPIO_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_GPIO_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_gpio_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_gpio_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define LED1_PIN      EN_GPIO_PIN_2  // Example GPIO pin for an LED
#define LED2_PIN      EN_GPIO_PIN_3  // Example GPIO pin for an LED
#define SENSOR_PIN    EN_GPIO_PIN_4  // Example GPIO pin for a sensor (regular reading)
#define BUTTON_PIN    EN_GPIO_PIN_5  // Example GPIO pin for a button (interrupt-driven)

#define LED_ON        1              // Define the state for turning the LED on
#define LED_OFF       0              // Define the state for turning the LED off

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

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Initializes the GPIO module for LED control, button input with interrupt, and sensor input.
 * @details Configures GPIO pins for different functionalities: control an LED, read from a sensor, and handle button press interrupts.
 */
void app_gpio_init(void)
{
    stGPIO_InitTypeDef led1_pin,led2_pin,sensor_pin,button_pin;

    cb_scr_gpio_module_on();

    /*LED1 as GPIO Output Pin*/
    led1_pin.Pin = LED1_PIN;
    led1_pin.Mode = EN_GPIO_MODE_OUTPUT;
    led1_pin.Pull = EN_GPIO_PULLUP;
    cb_gpio_init(&led1_pin);
    cb_gpio_write_pin(LED1_PIN,EN_GPIO_PIN_RESET);

      /*LED2 as GPIO Output Pin*/
    led2_pin.Pin = LED2_PIN;
    led2_pin.Mode = EN_GPIO_MODE_OUTPUT;
    led2_pin.Pull = EN_GPIO_PULLUP;
    cb_gpio_init(&led2_pin);
    cb_gpio_write_pin(LED2_PIN,EN_GPIO_PIN_RESET);

    /*Sensor as GPIO Input Pin*/
    sensor_pin.Pin = SENSOR_PIN;
    sensor_pin.Mode =EN_GPIO_MODE_INPUT;
    sensor_pin.Pull = EN_GPIO_NOPULL;  
    cb_gpio_init(&sensor_pin);

    /*Button as GPIO IRQ Pin*/
    button_pin.Pin = BUTTON_PIN;
    button_pin.Mode = EN_GPIO_MODE_IT_FALLING;  //EN_GPIO_MODE_IT_RISING;        
    button_pin.Pull = EN_GPIO_PULLUP;  
    cb_gpio_init(&button_pin);

    app_irq_register_irqcallback(EN_IRQENTRY_GPIO_APP_IRQ, app_gpio_irq_callback);
    NVIC_EnableIRQ(GPIO_IRQn);
}

/**
 * @brief   Toggles an LED connected to a GPIO pin.
 * @details This function toggles the state of the LED.
 */
void app_gpio_toggle_led(void)
{
    cb_gpio_toggle_pin(LED1_PIN);
}

/**
 * @brief   Sets the LED state.
 * @param   state The desired state of the LED (0 for off, 1 for on).
 */
void app_gpio_set_led(uint8_t state)
{
    if (state != 0)
    {
      cb_gpio_write_pin(LED1_PIN,EN_GPIO_PIN_SET);
    }
    else
    {
      cb_gpio_write_pin(LED1_PIN,EN_GPIO_PIN_RESET);
    } 
}

/**
 * @brief   Sets the LED state.
 * @param   state The desired state of the LED (0 for off, 1 for on).
 */
void app_gpio_set_led2(uint8_t state)
{
    if (state != 0)
    {
      cb_gpio_write_pin(LED2_PIN,EN_GPIO_PIN_SET);
    }
    else
    {
      cb_gpio_write_pin(LED2_PIN,EN_GPIO_PIN_RESET);
    } 
}

/**
 * @brief   Reads the state of a sensor connected to a GPIO pin.
 * @details Reads and returns the current state of the sensor.
 * @return  uint8_t The current sensor state.
 */
uint8_t app_gpio_read_sensor(void)
{
    uint8_t ret;

    enGPIO_PinState pinstate;
    pinstate = cb_gpio_read_input_pin(SENSOR_PIN);
    if (pinstate == EN_GPIO_PIN_SET)
    {
      ret = 1;
    }
    else
    {
      ret = 0;
    }
    return ret;
}

/**
 * @brief   Callback function for GPIO IRQ.
 * @details Called when the button is pressed (interrupt is triggered). 
 */
void app_gpio_irq_callback(void)
{
    app_gpio_print("app_gpio_irq_callback - IRQ\n");
    app_gpio_toggle_led();  
}

/**
 * @brief   Deinitializes the GPIO module.
 * @details Resets the configurations for the GPIOs used for the LED, button, and sensor. Disables interrupts if necessary.
 */
void app_gpio_test_deinit(void)
{
    // Reset the configurations to default (input mode, low state)
    cb_gpio_write_pin(LED1_PIN,EN_GPIO_PIN_RESET);  // Turn off the LED1 explicitly
    cb_gpio_deinit(LED1_PIN);
    cb_gpio_write_pin(LED2_PIN,EN_GPIO_PIN_RESET);  // Turn off the LED2 explicitly

    cb_gpio_deinit(LED2_PIN);
    cb_gpio_deinit(SENSOR_PIN);

    // Disable the GPIO interrupt for the button
    NVIC_DisableIRQ(GPIO_IRQn);
    app_irq_deregister_irqcallback(EN_IRQENTRY_GPIO_APP_IRQ, app_gpio_irq_callback); // Remove the interrupt handler to avoid unexpected behavior
    cb_gpio_deinit(BUTTON_PIN);    
}

/**
 * @brief Initializes UART and GPIO for communication and control.
 */
void app_peripheral_gpio_demo_init(void)
{
    // Initialize UART with the following configuration:
    // GPIO00-RX, GPIO01-TX, Baud Rate = 115200, Stop bit = 1, Bit Order = LSB, Parity = NONE
    app_uart_init();

    // Print power-on reset message
    app_gpio_print("\n\nPower-on reset\n");

    // Initialize GPIO
    app_gpio_init();
}

/**
 * @brief Main loop function to read sensor status and control LED2.
 */
void app_peripheral_gpio_demo_loop(void)
{
    // Read sensor status and set LED2 accordingly
    if (app_gpio_read_sensor() == 1)
    {
      // Turn on LED2 if sensor is active
      app_gpio_set_led2(LED_ON );
    }
    else if (app_gpio_read_sensor() == 0)
    {
      // Turn off LED2 if sensor is inactive
      app_gpio_set_led2(LED_OFF);
    }
}