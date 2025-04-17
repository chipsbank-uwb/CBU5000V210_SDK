/**
 * @file    app_wdt.c
 * @brief   [CPU Subsystem] Wdt Application Module
 * @details This module provides implementation for Watchdog Usage Examples
 * @author  Chipsbank
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "app_wdt.h"
#include "APP_common.h"
#include "AppSysIrqCallback.h"
#include "CB_wdt.h"
#include "CB_system.h"
#include "string.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_WDT_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_WDT_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_wdt_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_wdt_print(...)
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
static const stWdtConfig default_config = {.WdtMode = EN_WDT_RESET,       //Resets EN_WDT_RESET = 1/ interrupts EN_WDT_INTERVAL = 0     
                                           .Interval = 2000,              //Interval time 2s
                                           .WdtRunInHalt = 0,             //Whether to run in debug mode
                                           .WdtRunInSleep = 0,            //Whether it is operating in sleep mode
                                           .GraceAfterInt = 0};           //Delay reset after timeout
          
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_wdt_irq_callback(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
* @brief    Demonstrates a simplified version of how the main function can use the Watchdog Timer (WDT).
*
* @details  Initializes the WDT using a default configuration, enables it, and performs example tasks.
*           Registers an IRQ callback for handling WDT timeout events and disables the WDT at the end.
*
* @pre      Default configuration for the WDT must be initialized. Watchdog should be off initially.
*
* @post     Watchdog is disabled at the end of the function.
*
* @return   None
*/
void app_wdt_demo_with_tasks(void) 
{
    // Configuring the watchdog and start it
    stWdtConfig myconfig;
    memcpy(&myconfig, &default_config, sizeof(myconfig));
    myconfig.GraceAfterInt = 100;
    cb_wdt_init(&myconfig);

    app_irq_register_irqcallback(EN_IRQENTRY_WDT_APP_IRQ, app_wdt_irq_callback);
    cb_wdt_enable();

    // Call a potentially lengthy task as an exampl
    app_wdt_task_delay50ms_example();

    // Another lenghty task. Takes ~400ms
    app_wdt_task_delay350ms_example();

    app_wdt_task_delay350ms_example();

    app_irq_deregister_irqcallback(EN_IRQENTRY_WDT_APP_IRQ, app_wdt_irq_callback);
    cb_wdt_disable();
}

/**
* @brief    Shows an example task that processes for approximately 100ms.
*
* @details  This function simulates a task that takes about 100ms to complete by using a delay function.
*
* @return   None
*/
void app_wdt_task_delay50ms_example(void) 
{
    app_wdt_print("app_wdt_task_delay50ms_example Running...\n");
    cb_system_delay_in_ms(50);
    app_wdt_print("app_wdt_task_delay50ms_example Done!!!\n");
}


/** 
* @brief    Shows an example task that processes for approximately 400ms.
*
* @details  This function simulates a task that takes about 400ms to complete by using a delay function.
*
* @return   None
*/
void app_wdt_task_delay350ms_example(void) 
{
    app_wdt_print("app_wdt_task_delay350ms_example Running...\n");
    cb_system_delay_in_ms(350);
    app_wdt_print("app_wdt_task_delay350ms_example Done!!!\n");
}

/**
* @brief    Demonstrates how to use the Watchdog Timer (WDT) with custom configurations.
*
* @details  This function showcases how to configure, check the status, and force reset the WDT.
*           It also demonstrates how to register an IRQ callback for handling WDT timeout events.

* @return   None
*/
void app_wdt_demo_with_force_reset(void) 
{
    // Example of how to make our own config
    stWdtConfig config = {
      .WdtMode = EN_WDT_RESET,                 //Resets EN_WDT_RESET = 1/ interrupts EN_WDT_INTERVAL = 0
      .WdtRunInHalt = 0,                       //Interval time
      .WdtRunInSleep = 0,                      //Whether to run in debug mode
      .Interval = 1000,                        //Whether it is operating in sleep mode
      .GraceAfterInt = 100,                    //Delay reset after timeout
    };
    // Configure and start the watchdog
    cb_wdt_init(&config);
    cb_wdt_enable();

    // Check if Wachdog is running
    app_wdt_print("Watchdog is running: ");
    if (cb_wdt_is_running() == APP_TRUE) 
    {
      app_wdt_print("True\n");
    } 
    else 
    {
      app_wdt_print("False\n");
    }

    // Example if program state ever reaches forbidden, we can force reset
    uint32_t this_value_is_for_bidden = 42;
    for (uint32_t i = 0; i < 100; ++i) 
    {
      if (i == this_value_is_for_bidden) 
      {
        // ForceReset implicitly disables watchdog
        app_wdt_print("\n");
        cb_wdt_force_reset();
      }
      app_wdt_print("Val: %d ", i);
    }
}

/** 
* @brief    Initializes and starts the Watchdog Timer (WDT) with a specified timeout.
*
* @details  Configures the WDT to reset the CPU if the timeout occurs. Registers an IRQ callback
*           to handle watchdog timeout events.
*
* @param[in] timeout The timeout period in milliseconds after which the WDT will trigger a reset.

* @return   None
*/
void app_wdt_test(uint16_t timeout)
{
    stWdtConfig myconfig;
    myconfig.WdtMode = 1;                   //Resets EN_WDT_RESET = 1/ interrupts EN_WDT_INTERVAL = 0
    myconfig.Interval = timeout;            //Interval time
    myconfig.WdtRunInHalt = 0;              //Whether to run in debug mode
    myconfig.WdtRunInSleep = 0;             //Whether it is operating in sleep mode
    myconfig.GraceAfterInt = 1000;          //Delay reset after timeout

    cb_wdt_init(&myconfig);
    app_wdt_print("WDT start,wait for 5s timeout.....\n");

    app_irq_register_irqcallback(EN_IRQENTRY_WDT_APP_IRQ, app_wdt_irq_callback);

    cb_wdt_enable();
}


/**
 * @brief Main function to initialize peripherals and configure the Watchdog Timer (WDT)
 *
 * This function initializes the UART for debugging purposes.
 * It then configures the Watchdog Timer (WDT) by calling app_wdt_test with a timeout of 5000 milliseconds.
 * The WDT is set up to ensure the system resets automatically in case of a fault.
 */
void app_peripheral_wdt_init(void)
{
    // Initialize UART for debugging
    app_uart_init();                          
    
    // Print power-on reset message
    app_wdt_print("\n\nPower-on reset\n");  

    // Configure WDT with a timeout of 5000 milliseconds
    app_wdt_test(5000);                      

}


void app_wdt_irq_callback(void) 
{
    app_wdt_print("Watchdog timeout!\n");
}