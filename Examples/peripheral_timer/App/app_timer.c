/**
 * @file    app_timer.c
 * @brief   [CPU Subsystem] Application timer module implementation.
 * @details This file contains the implementation of the application timer module functions.
 * @author  [Author Name]
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "app_timer.h"
#include "CB_scr.h"
#include "CB_timer.h"
#include "CB_gpio.h"
#include "AppSysIrqCallback.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_TIMER_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_TIMER_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_timer_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_timer_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define TIMER_LED1_PIN      EN_GPIO_PIN_2  // Example GPIO pin for an LED

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint32_t timer_0_counter;
static uint32_t timer_1_counter;
static uint32_t timer_2_counter;
static uint32_t timer_3_counter;
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Main function for managing application timer setup.
 * @details This function sets up the timers based on the provided configurations.
 */
void app_timer_main(void)
{
    // Configure TIMER0-EVENT0 in FreeRun mode that timeout every 1000ms.
    stTimerSetUp timer_setup;  // Declare a structure to hold timer setup parameters

    timer_setup.Timer                          = EN_TIMER_0;                // Set the timer number to TIMER0
    timer_setup.TimerMode                      = EN_TIMER_MODE_FREERUN;     // Set the timer mode to run in a loop (or optionally a single run)
    timer_setup.TimeUnit                       = EN_TIMER_MS;               // Set the time unit to milliseconds
    timer_setup.stTimeOut.timeoutVal[0]        = 1000;                      // Set the timeout value to 1000 ms
    timer_setup.stTimeOut.timeoutVal[1]        = 0;                         // Set the timeout value to 0 ms
    timer_setup.stTimeOut.timeoutVal[2]        = 0;                         // Set the timeout value to 0 ms
    timer_setup.stTimeOut.timeoutVal[3]        = 0;                         // Set the timeout value to 0 ms
    timer_setup.stTimeOut.TimerTimeoutEvent    = DEF_TIMER_TIMEOUT_EVENT_0; // Set the timeout event number
    timer_setup.TimerEvtComMode                = EN_TIMER_EVTCOM_MODE_00;   // Set the event communication mode
    timer_setup.AutoStartTimer                 = EN_START_TIMER_ENABLE;     // Enable auto start of the timer
    timer_setup.TimerEvtComEnable              = EN_TIMER_EVTCOM_DISABLE;   // Disable event communication enable
    timer_setup.TimerInterrupt                 = EN_TIMER_INTERUPT_ENABLE;  // Enable timer interrupt
    timer_setup.stPulseWidth.NumberOfCycles    = 0;                         // Set the number of cycles to 0
    timer_setup.stPulseWidth.TimerPulseWidth   = EN_TIMER_PULSEWIDTH_DISABLE; // Disable timer pulse width

    app_irq_register_irqcallback(EN_IRQENTRY_TIMER_0_APP_IRQ, app_timer_0_irq_callback); // Register the IRQ callback for Timer 0
    cb_timer_init(EN_TIMER_0);                                                          // Initialize Timer 0
    cb_timer_configure_timer(&timer_setup);                                             // Configure Timer 0 with the setup parameters
}

/**
 * @brief   Callback function for Timer 0 IRQ.
 */
__WEAK void app_timer_0_irq_callback (void)
{
    timer_0_counter++;
    app_timer_print("app_timer_0_irq_callback - %d\n", timer_0_counter);
    cb_gpio_toggle_pin(EN_GPIO_PIN_2);
}

/**
 * @brief   Callback function for Timer 1 IRQ.
 */
void app_timer_1_irq_callback (void)
{
    timer_1_counter++;
    app_timer_print("app_timer_1_irq_callback - %d\n", timer_0_counter);
}

/**
 * @brief   Callback function for Timer 2 IRQ.
 */
void app_timer_2_irq_callback (void)
{
    timer_2_counter++;
    app_timer_print("app_timer_2_irq_callback - %d\n", timer_2_counter);
}

/**
 * @brief   Callback function for Timer 3 IRQ.
 */
void app_timer_3_irq_callback (void)
{
    timer_3_counter++;
    app_timer_print("app_timer_3_irq_callback - %d\n", timer_3_counter);
}

/**
 * @brief Main function for initializing peripherals and starting the timer.
 *
 * This function performs the following operations in sequence:
 * 1. Initializes UART communication parameters.
 * 2. Initializes the Timer module and enables interrupts.
 * 3. Configures and starts the Timer.
 * 4. Prints a power-on reset message via UART.
 */
void app_peripheral_timer_init(void)
{
    app_uart_init();                            // Initialization of UART: GPIO00-RX, GPIO01-TX, BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
    app_timer_main();                           // Configure and start the Timer
    app_uart_printf("\n\nPower-on reset\n");    // Power-on reset indicator
    
    cb_scr_gpio_module_on();        // Enable the GPIO module
    
    stGPIO_InitTypeDef LED;         // Define a GPIO initialization structure for the LED
    LED.Mode = EN_GPIO_MODE_OUTPUT; // Set the GPIO mode to output
    LED.Pin = TIMER_LED1_PIN;        // Set the GPIO pin to pin 2
    LED.Pull = EN_GPIO_PULLUP;      // Set the pull-up resistor
    cb_gpio_init(&LED);             // Initialize the GPIO with the specified settings
    
}

