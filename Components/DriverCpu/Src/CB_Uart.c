/**
 * @file    CB_Uart.c
 * @brief   Implementation of UART communication functionality.
 * @details This file provides functions to initialize and manage UART communication
 *          on the specified channels, configure UART modes, and transmit data via UART.
 *          It also includes interrupt handlers for UART0 and UART1 (currently disabled).
 *          UART communication can be configured in FIFO mode or SDMA (Serial Direct Memory Access) mode.
 *          In FIFO mode, data is transmitted/received one byte at a time. In SDMA mode,
 *          a specified size of data is transmitted/received directly between memory and UART.
 *          This module assumes the existence of the main.h, iomux.h, Uart.h, and scr.h header files.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Uart.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include <stdint.h>
#include <string.h>
#include "CB_CompileOption.h"
//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define UART_TX_INTERRUPT_ENABLE  CB_FALSE
//-------------------------------
// DEFINE SECTION
//-------------------------------
#define MAX_NUM_BYTES_SDMA_BUF    256    //Just used as a send buff security limit
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static UART_TypeDef *UART0 = (UART_TypeDef* ) UART0_BASE_ADDR;
static UART_TypeDef *UART1 = (UART_TypeDef* ) UART1_BASE_ADDR;

//global variable
static enUartMode       enUart0ModeConfigured;
static enUartMode       enUart1ModeConfigured;

// for SDMA mode
// static uint8_t transmitDataBuffer0[MAX_NUM_BYTES_SDMA_BUF]  __attribute__((section("SPECIFIC_UART_TX_SDMA_RAM")));
// static uint8_t receiveDataBuffer0[MAX_NUM_BYTES_SDMA_BUF]   __attribute__((section("SPECIFIC_UART_TX_SDMA_RAM")));
// static uint8_t transmitDataBuffer1[MAX_NUM_BYTES_SDMA_BUF]; __attribute__((section("SPECIFIC_UART_TX_SDMA_RAM")));
// static uint8_t receiveDataBuffer1[MAX_NUM_BYTES_SDMA_BUF] ; __attribute__((section("SPECIFIC_UART_RX_SDMA_RAM")));

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void cb_uart_0_rxb_full_irq_callback(void);
void cb_uart_0_txb_empty_irq_callback(void);
void cb_uart_1_rxb_full_irq_callback(void);
void cb_uart_1_txb_empty_irq_callback(void);
//-------------------------------
// INLINE FUNCTION SECTION
//-------------------------------
static inline UART_TypeDef* cb_uart_get_channel(enUartChannel uartChannel)
{
    switch (uartChannel)
    {
      case EN_UART_0: return UART0;
      case EN_UART_1: return UART1;
      default: return NULL; // Invalid channel
    }
}
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Initialize UART communication with specified configuration.
 *
 * This function initializes UART communication with the provided configuration.
 *
 * @param uartConfig Configuration structure for UART initialization.
 */
void cb_uart_init(stUartConfig uartConfig)
{
    extern uint32_t SystemCoreClock;
    UART_TypeDef *UART = cb_uart_get_channel(uartConfig.uartChannel);

    if (UART == NULL) {
            return; // Invalid channel
    }
    // clear UART CFG register before configuring
    UART->CFG &= 0x00000000;

    uint32_t uartBaudRateCfg = SystemCoreClock/115200;  // default value
    switch (uartConfig.uartBaudrate)
    {
        case EN_UART_BAUDRATE_9600:    uartBaudRateCfg = SystemCoreClock/9600;     break;
        case EN_UART_BAUDRATE_14400:   uartBaudRateCfg = SystemCoreClock/14400;    break;
        case EN_UART_BAUDRATE_19200:   uartBaudRateCfg = SystemCoreClock/19200;    break;
        case EN_UART_BAUDRATE_38400:   uartBaudRateCfg = SystemCoreClock/38400;    break;
        case EN_UART_BAUDRATE_57600:   uartBaudRateCfg = SystemCoreClock/57600;    break;
        case EN_UART_BAUDRATE_115200:  uartBaudRateCfg = SystemCoreClock/115200;   break;  
        case EN_UART_BAUDRATE_230400:  uartBaudRateCfg = SystemCoreClock/230400;   break;
        case EN_UART_BAUDRATE_460800:  uartBaudRateCfg = SystemCoreClock/460800;   break;  
        case EN_UART_BAUDRATE_921600:  uartBaudRateCfg = SystemCoreClock/921600;   break;
        case EN_UART_BAUDRATE_1536000: uartBaudRateCfg = SystemCoreClock/1536000;  break;
        default:  break;
    }
    // set baud rate
    UART->CFG |= UART_CFG_BAUDRATE(uartBaudRateCfg);

    switch (uartConfig.uartRxBufWrap)
    {
        case EN_UART_RXBUF_WRAP_DISABLE:  UART->CFG &= ~(UART_CFG_RXBUF_WRAP);  break;
        case EN_UART_RXBUF_WRAP_ENABLE:   UART->CFG |= UART_CFG_RXBUF_WRAP;     break; 
        default:                          UART->CFG &= ~(UART_CFG_RXBUF_WRAP);  break;  // default to no wrap-around
    }

    switch (uartConfig.uartMode) 
    {
        case EN_UART_MODE_FIFO:   UART->CFG &= ~(UART_CFG_TRXBUF_EN);                  
                                                            UART->TRXD = (UART_TRXD_WRITE_BYTES_UNIT | UART_TRXD_READ_BYTES_UNIT); // setting byte size RW  
                                                            if (UART == UART0) enUart0ModeConfigured = EN_UART_MODE_FIFO;
                                                            else enUart1ModeConfigured = EN_UART_MODE_FIFO;
                                                            break;
        case EN_UART_MODE_SDMA:   UART->CFG |= UART_CFG_TRXBUF_EN; 
                                                            UART->TRXD = (UART_TRXD_WRITE_BYTES_UNIT | UART_TRXD_READ_BYTES_UNIT); // setting byte size RW 
                                                            UART->BUF_SIZE &= ~(0x3UL << 28);         // need to set MIN_BYTES field to 0 for SDMA byte-by-byte transfer
                                                            if (UART == UART0) 
                                                            {
                                                                enUart0ModeConfigured = EN_UART_MODE_SDMA;
                                                                // memset(transmitDataBuffer0, 0, sizeof(transmitDataBuffer0));
                                                                // memset(receiveDataBuffer0, 0, sizeof(receiveDataBuffer0));
                                                            }
                                                            else 
                                                            {
                                                                enUart1ModeConfigured = EN_UART_MODE_SDMA;
                                                                // memset(transmitDataBuffer1, 0, sizeof(transmitDataBuffer1));
                                                                // memset(receiveDataBuffer1, 0, sizeof(receiveDataBuffer1));
                                                            }
                                                            break;
        default:                  UART->CFG &= ~(UART_CFG_TRXBUF_EN); break;    // default to FIFO mode
    }

    switch (uartConfig.uartStopBits) 
    {
        case EN_UART_STOP_BITS_1:   UART->CFG |= UART_CFG_STOP_BITS_1_STOP_BITS;    break;
        case EN_UART_STOP_BITS_1_5: UART->CFG |= UART_CFG_STOP_BITS_1_5_STOP_BITS;  break;
        case EN_UART_STOP_BITS_2:   UART->CFG |= UART_CFG_STOP_BITS_2_STOP_BITS;    break;
        case EN_UART_STOP_BITS_3:   UART->CFG |= UART_CFG_STOP_BITS_3_STOP_BITS;    break;
        default:                    UART->CFG |= UART_CFG_STOP_BITS_1_STOP_BITS;    break;
    }

    switch (uartConfig.uartBitOrder) 
    {
        case EN_UART_BIT_ORDER_LSB_FIRST: UART->CFG &= ~(UART_CFG_BIT_ORDER_MSK);     break;
        case EN_UART_BIT_ORDER_MSB_FIRST: UART->CFG |= UART_CFG_BIT_ORDER_MSB_FIRST;  break;
        default: UART->CFG &= ~(UART_CFG_BIT_ORDER_MSK);                              break;
    }

    switch (uartConfig.uartParity) 
    {
        case EN_UART_PARITY_NONE: UART->CFG |= UART_CFG_PARITY_NO_PARITY;   break;
        case EN_UART_PARITY_EVEN: UART->CFG |= UART_CFG_PARITY_EVEN_PARITY; break;
        case EN_UART_PARITY_ODD:  UART->CFG |= UART_CFG_PARITY_ODD_PARITY;  break;
        default:                  UART->CFG |= UART_CFG_PARITY_NO_PARITY;   break;    // default to no parity
    }

    switch (uartConfig.uartFlowControl) 
    {
        case EN_UART_FLOW_CONTROL_DISABLE:  UART->CFG &= ~(UART_CFG_FLOW);  break;
        case EN_UART_FLOW_CONTROL_ENABLE:   UART->CFG |= UART_CFG_FLOW;     break;
        default:                            UART->CFG &= ~(UART_CFG_FLOW);  break;                            
    }
    // enable tx and rx
    UART->EN |= UART_EN_TXEN | UART_EN_RXEN;

    UART->BUF_SIZE = (uint32_t)(uartConfig.uartRxMaxBytes << 16);

    UART->RXBUF    = (uint32_t)uartConfig.RXbuffer;
    // UART->TXBUF    = (uint32_t) uartConfig.TXbuffer;

    UART->RXCTRL   = UART_RXCTRL_START; // Setup to receive data
    UART->TXCTRL   = UART_TXCTRL_START;

    // interrup enable/disable
    #if (UART_TX_INTERRUPT_ENABLE == CB_TRUE)
        UART->INT_EN   = uartConfig.uartInt;
    #else
        UART->INT_EN     = uartConfig.uartInt;
    #endif
}

/**
 * @brief Deinitialize UART communication for a specified UART channel.
 *
 * This function deinitializes the UART communication by disabling the UART module,
 * clearing configuration registers, and disabling interrupts for the specified UART channel.
 *
 * @param uartChannel The UART channel to deinitialize. Use EN_UART_0 or EN_UART_1.
 */
void cb_uart_deinit(enUartChannel uartChannel)
{
    UART_TypeDef *UART;
    enUartMode *enUartModeConfigured;

    UART = cb_uart_get_channel(uartChannel);

    if (UART == NULL) {
            return; // Invalid channel
    }

    if (uartChannel == EN_UART_0) {
            enUartModeConfigured = &enUart0ModeConfigured;
    } else if (uartChannel == EN_UART_1) {
            enUartModeConfigured = &enUart1ModeConfigured;
    } else {
            return; // Invalid channel
    }

    // Disable UART TX and RX
    UART->EN &= ~(UART_EN_TXEN | UART_EN_RXEN);

    // Clear all configuration settings
    UART->CFG &= 0x00000000;

    // Disable all interrupts
    UART->INT_EN = 0x00000000;

    // Clear interrupt flags
    UART->INT_CLR = UART_INT_CLR_INT_CLEAR;

    // Stop receiving and transmitting
    UART->RXCTRL = UART_RXCTRL_STOP;
    UART->TXCTRL = UART_TXCTRL_STOP;

    // Reset mode configuration
    *enUartModeConfigured = EN_UART_MODE_FIFO;
}

/**
 * @brief Initialize a stUartConfig structure with default values.
 *
 * This function initializes a stUartConfig structure with default UART configuration values.
 * It sets all the configuration parameters to their default values, ensuring that the UART
 * is configured with a standard set of settings when initialized.
 *
 * @param uartConfig Pointer to the stUartConfig structure to be initialized.
 */
void cb_uart_struct_init(stUartConfig *uartConfig) 
{
    if (uartConfig == NULL) 
    {
            return; // Return if the pointer is NULL
    }
    // uartConfig->uartChannel        = EN_UART_0;                 // Default to UART0
    uartConfig->uartMode           = EN_UART_MODE_FIFO;            // Default to FIFO mode
    uartConfig->uartBaudrate       = EN_UART_BAUDRATE_115200;      // Default baud rate 115200
    uartConfig->uartRxMaxBytes     = 1;                            // Default RX buffer size
    uartConfig->uartRxBufWrap      = EN_UART_RXBUF_WRAP_DISABLE;   // Default to no wrap-around
    uartConfig->uartStopBits       = EN_UART_STOP_BITS_1;          // Default to 1 stop bit
    uartConfig->uartBitOrder       = EN_UART_BIT_ORDER_LSB_FIRST;  // Default to LSB first
    uartConfig->uartParity         = EN_UART_PARITY_NONE;          // Default to no parity
    uartConfig->uartFlowControl    = EN_UART_FLOW_CONTROL_DISABLE; // Default to no flow control
    uartConfig->uartInt            = EN_UART_INT_RXB_FULL;         // Default to no interrupts    
}

/**
 * @brief Enable or disable the specified UART channel.
 *
 * This function enables or disables the specified UART channel for transmission and reception.
 *
 * @param uartChannel The UART channel to enable or disable. Use EN_UART_0 or EN_UART_1.
 * @param newState New state of the UART channel. Use ENABLE or DISABLE.
 */
void cb_uart_cmd(enUartChannel uartChannel, uint8_t newState)
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);

    if (UART == NULL) {
        return; // Invalid channel
    }
    // Enable or disable the UART channel
    if (newState == CB_TRUE) {
            UART->EN |= (UART_EN_TXEN | UART_EN_RXEN); // Enable TX and RX
    } else if (newState == CB_FAIL) {
            UART->EN &= ~(UART_EN_TXEN | UART_EN_RXEN); // Disable TX and RX
    }
}

/**
 * @brief Transmit data via UART.
 * 
 * This function transmits a specified number of bytes over the configured UART channel.
 * It supports both FIFO and SDMA modes for data transmission.
 * 
 * @param uartConfig Configuration structure for UART initialization.
 * @param data Pointer to the data buffer containing the bytes to be transmitted.
 * @param size Number of bytes to transmit.
 */
void cb_uart_transmit(stUartConfig uartConfig, uint8_t *data, uint16_t size)  
{
    UART_TypeDef *UART = cb_uart_get_channel(uartConfig.uartChannel);

    if (UART == NULL) {
            return; // Invalid channel
    }
    UART->TXBUF = uartConfig.TXbuffer;

    // Set the number of bytes to transmit
    UART->BUF_SIZE = (UART->BUF_SIZE & ~(0xFFFUL)) | (uint32_t)size;

    if (uartConfig.uartMode == EN_UART_MODE_FIFO)                //FIFO mode
    {
        UART->TXCTRL = UART_TXCTRL_START;
        while (size > 0) 
        {
            UART->TXD = *(data++);
            while ((UART->EVENT & UART_EVENT_TXFIFO_EMPTY_MSK) != UART_EVENT_TXFIFO_EMPTY);
            size -= 1;
        }
        while ((UART->EVENT & UART_EVENT_TXB_EMPTY_MSK) != UART_EVENT_TXB_EMPTY);
    } 
    else                                                        //SDMA mode
    {
        // SDMA mode: Copy data to the configured TXbuffer
        if (uartConfig.TXbuffer == NULL || size > MAX_NUM_BYTES_SDMA_BUF) 
        {
            return; // Invalid TXbuffer or data size exceeds limit
        }
        memcpy((void *) uartConfig.TXbuffer, data, size);

        // Start transmission
        UART->TXCTRL = UART_TXCTRL_START;
    }
}

/**
 * @brief Check if the UART transmission (TXD) is not available 
 *
 * This function returns true if the TX is not available yet
 *
 * @param uartConfig Configuration structure for UART initialization.
 * @return true if TX is busy, false otherwise.  
 */
uint8_t cb_uart_is_tx_busy(stUartConfig uartConfig) 
{
    UART_TypeDef *UART = cb_uart_get_channel(uartConfig.uartChannel);

    if (UART == NULL)
    {
            return CB_TRUE; // Invalid channel, consider TX as busy
    }
    return ((UART->EVENT & UART_EVENT_TXB_EMPTY_MSK) != UART_EVENT_TXB_EMPTY) && ((UART->EVENT & UART_EVENT_TX_ON_MSK) == UART_EVENT_TX_ON);
}

/**
 * @brief Set the number of bytes to be received on RXD for a specified UART channel.
 *
 * This function configures the UART to expect a specified number of bytes to be received
 * on the RXD line for the next data transfer. It updates the buffer size configuration
 * and restarts the receive operation.
 *
 * @param uartChannel The UART channel for which to set the number of bytes to receive.
 *                    Use EN_UART_0 or EN_UART_1.
 * @param maxBytes    The number of bytes expected to be received on the RXD line.
 *                    This value should be within the valid range supported by the UART.
 */
void cb_uart_set_rx_num_of_bytes(enUartChannel uartChannel, uint16_t maxBytes) 
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);

    if (UART == NULL) 
    {
            return; // Invalid channel
    }
    // Set the number of bytes to be received
    UART->BUF_SIZE  = (UART->BUF_SIZE & ~(0xFFFUL << 16)) | (((uint32_t) maxBytes) << 16);
    UART->RXCTRL    = UART_RXCTRL_START;
}

/**
 * @brief Gets one byte from the specified UART RXD register
 *
 * This function returns a byte read from the specified UART RXD register.
 * It is primarily intended for use in FIFO mode, where data is received one byte at a time.
 *
 * @param uartChannel The UART channel to read from. Use EN_UART_0 or EN_UART_1.
 * @return One byte read from the specified UART RXD register.
 */
uint8_t cb_uart_get_rx_byte(enUartChannel uartChannel)
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);
        
    if (UART == NULL) 
    {
        return 0; // Invalid channel, return 0 or handle error appropriately
    }
    return (uint8_t)UART->RXD;
}

/**
 * @brief Retrieves data from the UART receive buffer.
 *
 * This function copies a specified number of bytes from the UART receive buffer into a destination array.
 * The primary use case is for SDMA mode where data is directly transferred from the UART to a memory buffer.
 *
 * @param uartChannel The UART channel from which to retrieve data. Use EN_UART_0 or EN_UART_1.
 * @param dest        Pointer to the destination array where the received data will be stored.
 * @param numBytes    The number of bytes to read from the receive buffer and copy to the destination array.
 */
void cb_uart_get_rx_buffer(enUartChannel uartChannel,uint8_t *dest, uint16_t numBytes) 
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);
        
    if (UART == NULL) 
    {
        return; // Invalid channel
    }

    uint8_t *rxBuffer = (uint8_t *)UART->RXBUF;

    for (int i = 0; (i < numBytes); i++) /* Should have a boundary check here - omitted in current SDK for faster processing */
    {
        dest[i] = rxBuffer[i];
    }
}

/**
 * @brief Checks the number of bytes received in the specified UART channel.
 *
 * This function checks the number of bytes that have been received in the specified UART channel buffer.
 * It reads the relevant status register and extracts the number of received bytes.
 *
 * @param uartChannel The UART channel to check. Use EN_UART_0 or EN_UART_1.
 * @return The number of bytes currently available in the specified UART receive buffer.
 */
uint16_t cb_uart_check_num_received_bytes(enUartChannel uartChannel)
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);
        
    if (UART == NULL)
    {
        return 0; // Invalid channel, return 0 or handle error appropriately
    }

    // Return the number of bytes currently available in the receive buffer
    return (uint16_t)(((UART->TRX & UART_RXB_NBYTE_MSK) >> UART_RXB_NBYTE_POS));
}

/**
 * @brief Stops the receive operation for the specified UART channel.
 *
 * This function stops the receiving process for the specified UART channel.
 *
 * @param uartChannel The UART channel to stop. Use EN_UART_0 or EN_UART_1.
 */
void cb_uart_rx_stop(enUartChannel uartChannel)
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);
        
    if (UART == NULL)
    {
        return; // Invalid channel
    }

    // Stop the receive operation
    UART->RXCTRL |= UART_RXCTRL_STOP; // Stop Pulse
}

/**
 * @brief Restarts the receive operation for the specified UART channel.
 *
 * This function restarts the receive process for the specified UART channel.
 *
 * @param uartChannel The UART channel to restart. Use EN_UART_0 or EN_UART_1.
 */
void cb_uart_rx_restart(enUartChannel uartChannel)
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);
        
    if (UART == NULL) 
    {
        return; // Invalid channel
    }

    // Stop and then start the receive operation
    UART->RXCTRL |= UART_RXCTRL_STOP;  // Stop Pulse
    UART->RXCTRL |= UART_RXCTRL_START; // Start Pulse
}

/**
 * @brief Configure UART interrupts for a specified UART channel.
 *
 * This function configures the specified UART channel's interrupts based on the provided parameters.
 * It enables or disables the specified interrupt type for the given UART channel.
 *
 * @param[in] uartChannel The UART channel to configure. Use EN_UART_0 or EN_UART_1.
 * @param[in] uart_int    Specifies the UART interrupts sources to be enabled or disabled.
 *                        This parameter can be any combination of the values defined in the "enUartInt" enumeration.
 * @param[in] newState    The new state for the interrupt. Use ENABLE to enable the interrupt and DISABLE to disable it.
 */
void cb_usart_irq_config(enUartChannel uartChannel, uint16_t uart_int, uint8_t newState)
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);

    if (UART == NULL) {
        return; // Invalid channel
    }

    // Check if the newState is valid
    if (newState != CB_TRUE && newState != CB_FAIL) {
            return; // Invalid state
    }

    // Enable or disable the specified interrupt
    if (newState == CB_TRUE) {
            UART->INT_EN |= uart_int; // Enable the specified interrupt
    } else if (newState == CB_FAIL) {
            UART->INT_EN &= ~uart_int; // Disable the specified interrupt
    }
}

/**
 * @brief Get the current UART flags for a specified UART channel.
 *
 * This function reads the UART status register and returns the current flags.
 * The flags indicate the status of various UART events and errors.
 *
 * @param uartChannel The UART channel to query. Use EN_UART_0 or EN_UART_1.
 * @return The current UART flags as a bitmask. Refer to the enUartFlag enumeration for flag definitions.
 */
uint32_t cb_uart_get_irq_flags(enUartChannel uartChannel)
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);

    if (UART == NULL) 
    {
            return 0; // Invalid channel
    }

    // Read and return the current UART flags
    return UART->EVENT ;
}

/**
 * @brief Clear interrupt flags for a specified UART channel.
 *
 * This function clears the interrupt flags for the specified UART channel.
 * It ensures that the interrupt flags are reset to avoid repeated interrupt triggers.
 *
 * @param uartChannel The UART channel for which to clear the interrupt flags. Use EN_UART_0 or EN_UART_1.
 */
void cb_uart_clear_irq(enUartChannel uartChannel) 
{
    UART_TypeDef *UART = cb_uart_get_channel(uartChannel);

    if (UART == NULL) 
    {
            return; // Invalid channel
    }

    // Clear interrupt flags
    UART->INT_CLR = UART_INT_CLR_INT_CLEAR;

    // Restart the receive operation
    UART->RXCTRL = UART_RXCTRL_START;
}

/**
 * @brief   UART0 interrupt request handler.
 * @details This function handles various UART0 interupts and calls corresponding callback.
 */
void cb_uart_0_irqhandler(void) 
{
    if ((UART0->EVENT & UART_EVENT_RXB_FULL_MSK) == UART_EVENT_RXB_FULL && (UART0->INT_EN & EN_UART_INT_RXB_FULL)) 
    {
        cb_uart_0_rxb_full_irq_callback();
    }

    if ((UART0->EVENT & UART_EVENT_TXB_EMPTY_MSK) == UART_EVENT_TXB_EMPTY && (UART0->INT_EN & EN_UART_INT_TXB_EMPTY))
    {
        cb_uart_0_txb_empty_irq_callback();
    }

    UART0->INT_CLR  = UART_INT_CLR_INT_CLEAR;
    UART0->RXCTRL   = UART_RXCTRL_START;
}

/**
 * @brief   UART1 interrupt request handler.
 * @details This function handles various UART1 interupts and calls corresponding callback.
 */
void cb_uart_1_irqhandler(void) 
{
    if ((UART1->EVENT & UART_EVENT_RXB_FULL_MSK) == UART_EVENT_RXB_FULL && (UART1->INT_EN & EN_UART_INT_RXB_FULL)) 
    {
        cb_uart_1_rxb_full_irq_callback();
    }

    if ((UART1->EVENT & UART_EVENT_TXB_EMPTY_MSK) == UART_EVENT_TXB_EMPTY && (UART1->INT_EN & EN_UART_INT_TXB_EMPTY))
    {
        cb_uart_1_txb_empty_irq_callback();
    }

    UART1->INT_CLR  = UART_INT_CLR_INT_CLEAR;
    UART1->RXCTRL   = UART_RXCTRL_START;
}

/**
 * @brief IRQ callback for UART0 RX buffer full
 * This callback is executed when the number of bytes received matches RXB_MAX_BYTES in UART->BUF_SIZE register
 */
void cb_uart_0_rxb_full_irq_callback(void)
{
    cb_uart_0_rxb_full_app_irq_callback();
}

__WEAK void cb_uart_0_rxb_full_app_irq_callback(void)
{
}

/**
 * @brief IRQ callback for UART0 TXB empty.
 * Last TX byte has been sent out
 */
void cb_uart_0_txb_empty_irq_callback(void)
{
    cb_uart_0_txb_empty_app_irq_callback();
}

__WEAK void cb_uart_0_txb_empty_app_irq_callback(void)
{
}

/**
 * @brief IRQ callback for UART1 RX buffer full
 * This callback is executed when the number of bytes received matches RXB_MAX_BYTES in UART->BUF_SIZE register
 */
void cb_uart_1_rxb_full_irq_callback(void)
{
    cb_uart_1_rxb_full_app_irq_callback();
}

__WEAK void cb_uart_1_rxb_full_app_irq_callback(void)
{
}

/**
 * @brief IRQ callback for UART1 TXB empty.
 * Last TX byte has been sent out
 */
void cb_uart_1_txb_empty_irq_callback(void)
{
    cb_uart_1_txb_empty_app_irq_callback();
}

__WEAK void cb_uart_1_txb_empty_app_irq_callback(void)
{
}

