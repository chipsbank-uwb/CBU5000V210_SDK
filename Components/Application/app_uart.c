/**
 * @file    app_uart.c
 * @brief   [CPU Subsystem] UART Application Module
 * @details This module provides functions for UART initialization and printing formatted output to UART.
 * @author  Chipsbank
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "app_uart.h"
#include "AppSysIrqCallback.h"
#include "CB_scr.h"
#include "CB_iomux.h"
#include "CB_Uart.h"
#include "CB_system.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define UART_RX_BUFFER_SIZE 0x100
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
stUartConfig uart_config;

/**
 * @brief DMA mode transmit and receive buffers.
 * 
 * These buffers are used for DMA mode operations. The address of these buffers
 * needs to be written into the UART configuration structure. The buffers must be
 * aligned to 4 bytes to ensure proper memory access.
 * 
 * @note The buffer size should be set to a value greater than the maximum expected
 *       size of the data to be received or sent to avoid memory overflow.
 * 
 * @note You can also define custom buffer addresses and write them into the structure
 *       for use in subsequent receive operations.
 */
uint8_t uart_txbuf[UART_RX_BUFFER_SIZE] __attribute__((section("SPECIFIC_UART_TX_SDMA_RAM")));
uint8_t uart_rxbuf[UART_RX_BUFFER_SIZE] __attribute__((section("SPECIFIC_UART_RX_SDMA_RAM")));
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Sends a string of data over UART.
 *
 * @param p_dat Pointer to the data buffer containing the bytes to be transmitted.
 * @param len   Number of bytes to transmit.
 */
void app_uart_send_string(uint8_t* p_dat, uint16_t len)
{
    // make sure TX is available
    while ((cb_uart_is_tx_busy(uart_config)));
    cb_uart_transmit(uart_config, (uint8_t *) p_dat, len);
    // make sure TX is finish
    while ((cb_uart_is_tx_busy(uart_config)));
}

/**
 * @brief   Serial echo example.
 * @details This function will send out all UART received data. It is using DMA mode which be able to 
    receive long string data in a hight baud rate.
 */
void app_uart_echo_demo(void)
{
    static uint16_t last_len = 0;
    cb_uart_set_rx_num_of_bytes(EN_UART_0, UART_RX_BUFFER_SIZE); 
    app_uart_printf("%s\n",__func__);
    while(1)
    {
      uint16_t current_len = cb_uart_check_num_received_bytes(EN_UART_0);   
      if(last_len == current_len && current_len > 0)
      {
        current_len = (current_len>UART_RX_BUFFER_SIZE)? UART_RX_BUFFER_SIZE:current_len;
        cb_uart_get_rx_buffer(EN_UART_0,&uart_rxbuf[0],current_len);
        cb_uart_rx_restart(EN_UART_0);
        last_len = 0;
        app_uart_send_string(uart_rxbuf,current_len);
      }
      else{
        last_len = current_len;
        cb_system_delay_in_ms(5);
      }
    }
}

/**
 * @brief   Initializes the UART module for communication.
 * @details This function turns on the UART0 module, configures the I/O multiplexer for UART0 RX and TX pins,
 *          enables the UART0 interrupt, and initializes UART0 with SDMA mode.
 */
void app_uart_init(void)
{
    cb_scr_uart0_module_on();
    cb_iomux_config(EN_IOMUX_GPIO_0,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART0_RXD});
    cb_iomux_config(EN_IOMUX_GPIO_1,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART0_TXD});
          
    NVIC_EnableIRQ(UART0_IRQn); // Enable Uart0 IRQ   

    // Configure UART settings
    uart_config.uartChannel        = EN_UART_0;                        // Set UART channel to UART0
    uart_config.uartMode           = EN_UART_MODE_SDMA;                // Set UART mode to SDMA (or set EN_UART_MODE_FIFO to FIFO)
    uart_config.uartBaudrate       = EN_UART_BAUDRATE_115200;          // Set baud rate to 115200
    uart_config.uartRxMaxBytes     = 1;                                // Set maximum number of bytes to receive at a time to 1
    uart_config.uartRxBufWrap      = EN_UART_RXBUF_WRAP_DISABLE;       // Disable wrapping of the receive buffer
    uart_config.uartStopBits       = EN_UART_STOP_BITS_1;              // Set number of stop bits to 1
    uart_config.uartBitOrder       = EN_UART_BIT_ORDER_LSB_FIRST;      // Set bit order to LSB (Least Significant Bit) first
    uart_config.uartParity         = EN_UART_PARITY_NONE;              // Set parity to none
    uart_config.uartFlowControl    = EN_UART_FLOW_CONTROL_DISABLE;     // Disable flow control
    uart_config.uartInt            = EN_UART_INT_RXB_FULL;             // Enable RX buffer full interrupt

    /*SDMA Buffer address.*/
    uart_config.TXbuffer           = (uint32_t)uart_txbuf;               // Set transmit buffer address
    uart_config.RXbuffer           = (uint32_t)uart_rxbuf;               // Set receive buffer address

    /* Callback IRQ register omitted in current SDK to improve IRQ processing time */
    // app_irq_register_irqcallback(EN_IRQENTRY_UART_0_RXB_FULL_APP_IRQ, app_uart_0_rxb_full_callback);

    cb_uart_init(uart_config);  
}

/**
 * @brief Changes the baud rate of the UART module.
 * @param baudrate The new baud rate to be set for the UART module.
 */
void app_uart_change_baudrate(enUartBaudrate baudrate)
{
    // Configure UART settings
    uart_config.uartChannel        = EN_UART_0;                        // Set UART channel to UART0
    uart_config.uartMode           = EN_UART_MODE_SDMA;                // Set UART mode to SDMA (or set EN_UART_MODE_FIFO to FIFO)
    uart_config.uartBaudrate       = baudrate;                         // Set baud rate 
    uart_config.uartRxMaxBytes     = 1;                                // Set maximum number of bytes to receive at a time to 1
    uart_config.uartRxBufWrap      = EN_UART_RXBUF_WRAP_DISABLE;       // Disable wrapping of the receive buffer
    uart_config.uartStopBits       = EN_UART_STOP_BITS_1;              // Set number of stop bits to 1
    uart_config.uartBitOrder       = EN_UART_BIT_ORDER_LSB_FIRST;      // Set bit order to LSB (Least Significant Bit) first
    uart_config.uartParity         = EN_UART_PARITY_NONE;              // Set parity to none
    uart_config.uartFlowControl    = EN_UART_FLOW_CONTROL_DISABLE;     // Disable flow control
    uart_config.uartInt            = EN_UART_INT_RXB_FULL;             // Enable RX buffer full interrupt

    /*SDMA Buffer address.*/
    uart_config.TXbuffer           = (uint32_t)uart_txbuf;               // Set transmit buffer address
    uart_config.RXbuffer           = (uint32_t)uart_rxbuf;               // Set receive buffer address
    /* Callback IRQ register omitted in current SDK to improve IRQ processing time */
    // app_irq_register_irqcallback(EN_IRQENTRY_UART_0_RXB_FULL_APP_IRQ, app_uart_0_rxb_full_callback);
      
    cb_uart_init(uart_config);  
}


__WEAK void app_uart_0_rxd_ready_callback(void) 
{
}

__WEAK void app_uart_0_rxb_full_callback(void)
{
}

/**
 * @brief   Prints formatted output to UART.
 * @details This function formats the output string based on the provided format and arguments using vsnprintf,
 *          and transmits each character of the formatted string through UART.
 * @param   format The format string specifying how subsequent arguments are converted for output.
 * @param   ... Additional arguments to substitute into the format string.
 */
void app_uart_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char transmitDataBuffer[256]; // Choose an appropriate buffer size
    vsnprintf((char *)transmitDataBuffer, sizeof(transmitDataBuffer), format, args);

    // make sure TX is available
    while ((cb_uart_is_tx_busy(uart_config)));

    // Transmit each character from the buffer
    size_t len = strlen(transmitDataBuffer);

    while ((cb_uart_is_tx_busy(uart_config) == CB_TRUE));
    cb_uart_transmit(uart_config, (uint8_t *) transmitDataBuffer, (uint16_t) len);
    va_end(args);
}
