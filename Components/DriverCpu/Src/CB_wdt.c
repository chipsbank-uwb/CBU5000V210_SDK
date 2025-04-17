/**
 * @file    CB_Wdt.c
 * @brief   Implementation of Watchdog Timer.
 * @details This file provides implementations for Watchdog Timer API.
 *          It also includes the implementation for the non-maskable interrupt handler
 *          used by the Watchdog handler, to clear watchdog interrupt and disable it.
 *          Watchdog Timer can be in 2 mode: Reset, which reset cpu upon timeout or Interval, which
 *          simply disables the watchdog (not recommended).
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_wdt.h"
#include "CB_PeripheralPhyAddrDataBase.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

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
static stWDT_TypeDef *pWDT = (stWDT_TypeDef *)WDT_BASE_ADDR;
static WdtCallback_t s_wdtCallback = NULL;

// Define a global variable to store user-set interrupt handlers
static void (*rc_callback_wdt_irq_handler)(void) = NULL;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void cb_wdt_nmi_irq_callback(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Initialize WDT Timer with specified configuration.
 *
 * This function initializes WDT Timer with the provided configuration.
 *
 * @param stWdtConfig Configuration structure for UART initialization.
 */
void cb_wdt_init(const stWdtConfig *const Config) 
{
    pWDT->CTRL &= ~(1u << 1);
    uint32_t config = 0;

    // Protect GraceAfterInt from being set to 0
    uint32_t graceAfterInt = (Config->GraceAfterInt > 0) ? Config->GraceAfterInt : 1;

    config |= (uint32_t)(Config->WdtMode);
    config |= ((uint32_t)(Config->WdtRunInSleep) << 14);
    config |= ((uint32_t)(Config->WdtRunInHalt) << 15);
    config |= (0x3FFE & ((graceAfterInt * WDT_ONE_MSEC_MULT) << 1));  

    pWDT->CFG = config;
    pWDT->TOV = Config->Interval * WDT_ONE_MSEC_MULT;
}

/**
 * @brief De-initializes the Watchdog Timer.
 *
 * This function de-initializes the Watchdog Timer by disabling it, clearing its configuration,
 * and resetting its interrupt settings. It ensures that the Watchdog Timer is in a known
 * initial state after de-initialization.
 */
void cb_wdt_deinit(void)
{
    // Disable the Watchdog Timer to stop it from counting
    cb_wdt_disable();

    // Reset the Control Register (CTRL) to disable the watchdog
    pWDT->CTRL = 0x00000000;

    // Clear the Status Register (CSR) to ensure no pending status bits are set
    pWDT->CSR = 0x00000000;

    // Disable interrupts by setting the Interrupt Enable Register (IER) to 0
    pWDT->IER = 0x00000000;

    // Clear the Interrupt Clear Register (ICR) to clear any pending interrupts
    pWDT->ICR = 0x00000000;

    // Clear the Status Register (SR) to ensure no pending status bits are set
    pWDT->SR = 0x00000000;

    // Clear the Counter Value Register (CVR) to reset the counter value
    pWDT->CVR = 0x00000000;

    // Reset the Timeout Value Register (TOV) to 0 to effectively disable the timeout
    pWDT->TOV = 0x00000000;

    // Clear the Configuration Register (CFG) to reset all configuration settings
    pWDT->CFG = 0x00000000;
}

/**
 * @brief Sets the Watchdog Timer timeout value.
 * @param TimeoutVal Timeout value in milliseconds.
 */
void cb_wdt_set_timeout(uint32_t TimeoutVal)
{
    cb_wdt_disable();

    // Protect TimeoutVal from being set to 0
    uint32_t timeout = (TimeoutVal > 0) ? TimeoutVal : 1;

    // Clear the existing timeout value bits in the configuration register
    pWDT->TOV  = 0x01;

    // Set the new timeout value in the configuration register
    pWDT->TOV = TimeoutVal * WDT_ONE_MSEC_MULT;

    cb_wdt_enable();
}

/**
 * @brief Initialize WDT Timer.
 * @detail This function enables the watchdog timer. Watchdog starts counting after this function is called
 */
void cb_wdt_enable(void) 
{
    pWDT->IER = 0x00000001;
    pWDT->CTRL = 0x00000003;
    pWDT->CSR = 0xAC624A73;
}

/**
 * @brief Disable WDT Timer.
 *
 * @detail This function disables the watchdog timer. Watchdog stops counting immediately.
 */
void cb_wdt_disable(void) 
{
    pWDT->IER = 0;
    pWDT->CTRL = 0x00000000;
    pWDT->CFG = 0;
    s_wdtCallback = NULL;
}

/**
 * @brief Configure WDT IRQ.
 * @param enable Pass 1 to enable the WDT IRQ, 0 to disable it.
 */
void cb_wdt_irq_config(uint8_t enable) 
{
    if (enable) 
    {
        pWDT->IER = 0x00000001; // Enable WDT IRQ
    }
    else
    {
        pWDT->IER = 0x00000000; // Disable WDT IRQ
    }
}

/**
 * @brief Pets the WDT Timer.
 *
 * @detail This function pets the watchdog timer. Counter will restart.
 */
void cb_wdt_refresh(void) 
{ 
    pWDT->CSR = 0xAC624A73; 
}

/**
 * @brief Use the WDT Timer to force a cpu reset.
 *
 * @detail This function resets the cpu. Watchdog will be disabled implicitly.
 */
void cb_wdt_force_reset(void) 
{
    cb_wdt_disable();
    pWDT->CFG = 0x1;
    pWDT->TOV = 0x1;
    cb_wdt_enable();
}

/**
 * @brief Check if the watchdog timer is running
 *
 * @return Whether the watchdog timer is running or not
 */
uint8_t cb_wdt_is_running(void) 
{ 
    return (pWDT->SR & 0x1); 
}

/**
 * @brief Retrieves the watchdog timer's internal counter.
 *
 * @return The current counter value of the watchdog timer.
 */
uint32_t cb_wdt_get_current_count(void) 
{ 
    return pWDT->CVR; 
}

/**
 * @brief Retrieves the watchdog configured irq status.
 *
 * @return The irq setting of the watchdog timer.
 */
uint32_t cb_wdt_get_irq_setting(void) 
{ 
    return pWDT->IER; 
}

/**
 * @brief Clears the Watchdog Timer interrupt.
 *  This function clears the interrupt flag of the Watchdog Timer, ensuring that any pending
 * interrupt is reset. This is typically done after handling the interrupt to prevent it from
 * triggering again immediately.
 */
void cb_wdt_clear_irq(void)
{
    pWDT->ICR = 0x00000001;
}

__WEAK void cb_wdt_nmi_app_irq_callback(void)
{
}

/**
 * @brief Set the WDT interrupt handler.
 * @param handler The callback function to be called on WDT interrupt.
 */
void cb_wdt_nmi_rc_irq_callback(void(*handler)(void))
{
    rc_callback_wdt_irq_handler = handler;
}

/**
 * @brief Clear the WDT interrupt handler.
 */
void cb_wdt_nmi_clear_irq_handler(void)
{
    rc_callback_wdt_irq_handler = NULL;
}


/**
 * @brief Non-maskable interrupt handler. Used by watchdog.
 *
 * @detail Clears the WDT interrupt signal and disables it.
 */
void NMI_Handler(void) 
{
    cb_wdt_clear_irq();

    //Call the callback function
    cb_wdt_nmi_irq_callback();
}


/**
 * @brief   Weak callback function for TIMER 3 interrupt.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.

 * @param   TimeOutEventNumber: The timeout event number triggering the interrupt.
 */
void cb_wdt_nmi_irq_callback(void)
{
    cb_wdt_nmi_app_irq_callback();

    if(rc_callback_wdt_irq_handler != NULL)
    {
        rc_callback_wdt_irq_handler();
    }
}
