/**

 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdio.h>
#include <string.h>
#include "CB_iomux.h"
#include "CB_scr.h"
#include "CB_Uart.h"
#include "CB_gpio.h"
#include "AppSysIrqCallback.h"

#if __has_include(<APP_CompileOption.h>)
#include "APP_CompileOption.h"
#endif
//-------------------------------
// DEFINE SECTION
//-------------------------------

#define GPIO_KEY_PIN          EN_GPIO_PIN_0
#define GPIO_LED_PIN          EN_GPIO_PIN_3
#define GPIO_INTERRUPT_PIN    EN_GPIO_PIN_4

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------


//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stUartConfig uartConfig;
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Initializes the UART module for communication.
 * @details This function turns on the UART1 module, configures the I/O multiplexer for UART1 TX pin,
 *          and initializes UART1 with FIFO mode.
 */
void log_uart_init(void)
{
    cb_scr_uart1_module_off();
    cb_scr_uart1_module_on();

    cb_iomux_config(EN_IOMUX_GPIO_1,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART1_TXD});

    uartConfig.uartChannel        = EN_UART_1;
    uartConfig.uartMode           = EN_UART_MODE_FIFO;
    uartConfig.uartBaudrate       = EN_UART_BAUDRATE_921600;
    uartConfig.uartRxMaxBytes   = 1;
    uartConfig.uartRxBufWrap    = EN_UART_RXBUF_WRAP_DISABLE;
    uartConfig.uartStopBits     = EN_UART_STOP_BITS_1;
    uartConfig.uartBitOrder     = EN_UART_BIT_ORDER_LSB_FIRST;
    uartConfig.uartParity       = EN_UART_PARITY_NONE;
    uartConfig.uartFlowControl  = EN_UART_FLOW_CONTROL_DISABLE;
    uartConfig.uartInt          = 0;

    cb_uart_init(uartConfig);  
}

/**
 * @brief Sends data over UART.
 *
 * This function sends the specified data over UART. If FreeRTOS is enabled,
 * it enters a critical section to ensure thread safety during the transmission.
 *
 * @param prtData Pointer to the data to be sent.
 * @param len Length of the data to be sent.
 */
void log_uart_send_port(uint8_t *prtData, uint16_t len)
{
    #if (APP_FREERTOS_ENABLE == APP_TRUE)
    // Enter critical section if FreeRTOS is enabled
    vPortEnterCritical();
    #endif

    // Make sure TX is available
    while ((cb_uart_is_tx_busy(uartConfig)));

    // Transmit the data over UART
    cb_uart_transmit(uartConfig, (uint8_t *) prtData, (uint16_t) len);

    // Wait until the UART transmitter is not busy
    while ((cb_uart_is_tx_busy(uartConfig)));

    #if (APP_FREERTOS_ENABLE == APP_TRUE)
    // Exit critical section if FreeRTOS is enabled
    vPortExitCritical();
    #endif
}

/**
 * @brief Prints a string over UART.
 *
 * This function sends the specified null-terminated string over UART by calling
 * the log_uart_send_port function.
 *
 * @param data Pointer to the null-terminated string to be sent.
 */
void log_uart_print(const char *data)
{
    // Get the length of the string
    size_t len = strlen(data);

    // Send the string over UART
    log_uart_send_port(data, (uint16_t)len);
}

/**
 * @brief Deinitializes the UART module.
 *
 * This function turns off the UART1 module, effectively deinitializing it.
 */
void log_uart_deinit(void)
{
    // Turn off the UART1 module
    cb_scr_uart1_module_off();
}





/**
 * @brief init key gpio
 */

void board_key_init(void)
{
	stGPIO_InitTypeDef key_Pin;

	cb_scr_gpio_module_on();            //open gpio module 

	key_Pin.Pin = GPIO_KEY_PIN;       //select gpio pin
	key_Pin.Mode =EN_GPIO_MODE_INPUT; //set gpio mode
	key_Pin.Pull = EN_GPIO_PULLUP;    //set pull mode

	cb_gpio_init(&key_Pin); //init structure

  
}

/* return key status 1:release 0:press*/
enGPIO_PinState board_key_input_status(void)
{
	return CB_GPIO_ReadPin(GPIO_KEY_PIN);
}




/**
 * @brief init led gpio
 */

void board_led_init(void)
{
    stGPIO_InitTypeDef led_Pin;
	
    cb_scr_gpio_module_on(); 

    led_Pin.Pin = GPIO_LED_PIN;
    led_Pin.Mode =EN_GPIO_MODE_OUTPUT;
    led_Pin.Pull = EN_GPIO_PULLUP;  

	cb_gpio_init(&led_Pin);
 
}

/**
 * @brief select led gpio output status
 */

void board_led_ctrl(unsigned char is_on)
{
    if(is_on)
    {
       cb_gpio_write_pin(GPIO_LED_PIN,EN_GPIO_PIN_SET);  //led gpio output high
	}
    else
    {
       cb_gpio_write_pin(GPIO_LED_PIN,EN_GPIO_PIN_RESET); //led gpio output low
	}
}


/**
 * @brief interrupt callback
 */

void user_gpio_irq_callback(void)
{
  /*

    interrupt,should do something here
  */
}




/**
 * @brief interrupt gpio init
 */

void board_gpio_interrupt_init(void)
{
    stGPIO_InitTypeDef interrupt_Pin;
   /* as GPIO IRQ Pin*/
    interrupt_Pin.Pin = GPIO_INTERRUPT_PIN;
    interrupt_Pin.Mode =EN_GPIO_MODE_IT_FALLING;
    interrupt_Pin.Pull = EN_GPIO_PULLUP;  
    cb_gpio_init(&interrupt_Pin);

    app_irq_deregister_irqcallback(EN_IRQENTRY_GPIO_APP_IRQ, user_gpio_irq_callback); 
	
    NVIC_EnableIRQ(GPIO_IRQn);



}


