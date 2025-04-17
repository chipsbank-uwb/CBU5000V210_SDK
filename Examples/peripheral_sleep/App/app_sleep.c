/**
 * @file     app_sleep.c
 * @brief    [Application] Source file for managing CPU sleep and deep sleep modes.
 * @details  This file provides implementations for managing CPU sleep and deep sleep modes, including functions for entering
 * sleep and deep sleep modes for specified durations.
 * @author   Chipsbank
 * @date     2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "app_sleep.h"
#include "CB_SleepDeepSleep.h"
#include "CB_gpio.h"
#include "CB_system.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_SLEEP_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_SLEEP_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_sleep_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_sleep_print(...)
#endif

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

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Enters sleep mode for a specified duration.
 * @param slpduration_in_ms The duration to sleep in milliseconds.
 * @note This function delegates the sleep control to cb_sleep_control.
 */
void app_enter_sleep(uint32_t slpduration_in_ms)
{
    cb_sleep_control(slpduration_in_ms);
}

/**
 * @brief Enters deep sleep mode for a specified duration.
 * @param slpduration_in_ms The duration to enter deep sleep mode in milliseconds.
 * @note This function delegates the deep sleep control to cb_deep_sleep_control.
 */
void app_enter_deep_sleep(uint32_t slpduration_in_ms)
{
    cb_deep_sleep_control(slpduration_in_ms);
}

/**
 * @brief Main function for the peripheral sleep demo.
 *
 * This function initializes the UART, prints relevant messages, 
 * enters sleep mode for a specified duration, and then exits sleep mode.
 *
 */
void app_peripheral_sleep_demo_init(void)
{
    app_uart_init();  // Initialize UART with specific settings

    // /* Print power-on reset message */
    app_sleep_print("\nPower-on reset\n");  
    cb_uart_clear_irq(EN_UART_0);

}

/**
 * @brief This function demonstrates the peripheral sleep functionality by running a loop 
 *        and putting the system into sleep mode after a specified number of iterations.
 * 
 * @param sleep_time The duration in milliseconds for which the system will sleep.
 * 
 * The function increments a static counter `cnt` on each call. After the counter reaches 30, 
 * it enters sleep mode for the specified duration (`sleep_time`). Before entering sleep mode, 
 * it checks the UART interrupt flags to ensure no active interrupts are pending.
 * 
 * @note The function uses `cb_system_delay_in_ms` for delay and `app_enter_sleep` to enter sleep mode.
 */
void app_peripheral_sleep_demo_loop(uint16_t sleep_time)
{
    static uint16_t cnt = 0;
    cb_system_delay_in_ms(100);
    app_sleep_print("run cnt = %d\n",cnt);
    
    if(cnt++ == 30)
    {
        app_sleep_print("enter sleep %d ms.....\n",sleep_time);
        
        /* Enter sleep mode for the specified duration */
        while((cb_uart_get_irq_flags(EN_UART_0) & 0x01) == 0x01);
        app_enter_sleep(sleep_time);        
        
        // /* Confirm exit from sleep mode */
        app_sleep_print("\nExit sleep mode\n");
        app_sleep_print("keep runing\n");
    }
}

