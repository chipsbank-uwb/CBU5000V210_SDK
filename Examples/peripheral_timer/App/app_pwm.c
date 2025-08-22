/**
 * @file    app_pwm.c
 * @brief   [CPU Subsystem] Application timer module implementation.
 * @details This file contains the implementation of the application timer module functions.
 * @author  [Author Name]
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_scr.h"
#include "CB_timer.h"
#include "CB_gpio.h"
#include "CB_iomux.h"
#include "AppSysIrqCallback.h"
#include "NonLIB_sharedUtils.h"


//-------------------------------
// DEFINE SECTION
//-------------------------------
#define PWM_LED1_PIN      EN_GPIO_PIN_2  // Example GPIO pin for an LED
#define PWM_LED2_PIN      EN_GPIO_PIN_4  // Example GPIO pin for an LED

void app_pwm_init(void)
{    
    cb_scr_gpio_module_on();          // Enable the GPIO module
    
    stGPIO_InitTypeDef led1;          // Define a GPIO initialization structure for the LED
    led1.Mode = EN_GPIO_MODE_OUTPUT;  // Set the GPIO mode to output
    led1.Pin= PWM_LED1_PIN ;          // Set the GPIO pin to pin 2
    led1.Pull = EN_GPIO_PULLUP;       // Set the pull-up resistor
    cb_gpio_init(&led1);              // Initialize the GPIO with the specified settings
    
    led1.Pin= PWM_LED2_PIN;          // Set the GPIO pin to pin 4
    cb_gpio_init(&led1);              // Initialize the GPIO with the specified settings
    
    // Configure TIMER0-EVENT0 in FreeRun mode .
    cb_iomux_config(PWM_LED1_PIN,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_EVENT_LIST_0_OUTPUT,(uint8_t)EN_TIMER1_TOE0});
    cb_iomux_config(PWM_LED2_PIN,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_EVENT_LIST_0_OUTPUT,(uint8_t)EN_TIMER1_TOE1});
    
    stTimerSetUp timer_setup;  // Declare a structure to hold timer setup parameters

    timer_setup.Timer                          = EN_TIMER_1;                                            // Set the timer number to TIMER0
    timer_setup.TimerMode                      = EN_TIMER_MODE_FREERUN;                                 // Set the timer mode to run in a loop (or optionally a single run)
    timer_setup.TimeUnit                       = EN_TIMER_US;                                           // Set the time unit to milliseconds
    timer_setup.stTimeOut.timeoutVal[0]        = 200;                                                   // ***Set the duty cycle duration (high level time) to 200 us,duty=(200/1000)=20%
    timer_setup.stTimeOut.timeoutVal[1]        = 1000;                                                  // ***Set the period duration (low level time) to 1000 us
    timer_setup.stTimeOut.timeoutVal[2]        = 0;                                                     // Set the timeout value to 0 ms
    timer_setup.stTimeOut.timeoutVal[3]        = 0;                                                     // Set the timeout value to 0 ms
    timer_setup.stTimeOut.TimerTimeoutEvent    = DEF_TIMER_TIMEOUT_EVENT_0 | DEF_TIMER_TIMEOUT_EVENT_1; // ***Set the timeout event number（The two events have the largest control period, and the other controls the duty cycle）
    timer_setup.TimerEvtComMode                = EN_TIMER_EVTCOM_MODE_01;                               // Set the event communication mode
    timer_setup.AutoStartTimer                 = EN_START_TIMER_ENABLE;                                 // Enable auto start of the timer
    timer_setup.TimerEvtComEnable              = EN_TIMER_EVTCOM_ENABLE;                                // Disable event communication enable
    timer_setup.TimerInterrupt                 = EN_TIMER_INTERUPT_DISABLE;                             // Disable timer interrupt
    timer_setup.stPulseWidth.NumberOfCycles    = 0;                                                     // Set the number of cycles to 0
    timer_setup.stPulseWidth.TimerPulseWidth   = EN_TIMER_PULSEWIDTH_DISABLE;                           // Disable timer pulse width

//    app_irq_register_irqcallback(EN_IRQENTRY_TIMER_1_APP_IRQ, app_timer_1_irq_callback);              // Register the IRQ callback for Timer 0
    cb_timer_init(EN_TIMER_1);                                                                           // Initialize Timer 0
    cb_timer_configure_timer(&timer_setup);                                                              // Configure Timer 0 with the setup parameters
}

/**
 * @brief Controls the breathing LED effect (PWM fade-in and fade-out)
 * 
 * This function gradually increases and then decreases the PWM value to create
 * a smooth "breathing" light effect on an LED. The PWM values are updated in steps,
 * with a delay between each step to control the speed of the transition.

 */
void app_pwm_breathing_led_loop(void)
{
    // LED fade-in (0us ~ 1000us)
    for (uint16_t i = 0; i < 100; i++) 
    {
        if(i == 0)
        {
            // When the value is 0, the duty cycle is 100%, but the output is a high level. Therefore, 
            //it is necessary to set it to the opposite - when the duty cycle is 0, the output is a low level.
          cb_set_timeout_expiration(EN_TIMER_1, EN_TIMER_TIMEOUT_EVENT_0, 1000);        
        }
        else
        {
          // Set PWM duty cycle based on i value, multiplied by 10 to reach up to 1000us
          cb_set_timeout_expiration(EN_TIMER_1, EN_TIMER_TIMEOUT_EVENT_0, i * 10);
        }

        // Delay between each step to control fade speed
        cb_hal_delay_in_ms(20);
    }

    // LED fade-out (1000us ~ 0us)
    for (uint16_t i = 100; i > 0; i--) 
    {
      if(i == 100)
      {
          cb_set_timeout_expiration(EN_TIMER_1, EN_TIMER_TIMEOUT_EVENT_0, 0);
      }
      else
      {
          // Decrease PWM duty cycle gradually
          cb_set_timeout_expiration(EN_TIMER_1, EN_TIMER_TIMEOUT_EVENT_0, i * 10);
      }

        // Delay between each step
        cb_hal_delay_in_ms(20);
    }
}
