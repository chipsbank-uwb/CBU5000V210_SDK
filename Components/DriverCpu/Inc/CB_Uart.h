/**
 * @file    CB_UART.h
 * @brief   Header file for UART communication module.
 * @details This file defines structures, constants, and function prototypes for UART communication.
 *          It includes definitions for UART registers and event masks. It also defines enumeration types
 *          for UART channels and modes. Function prototypes are provided for initializing UART,
 *          reconfiguring UART settings, transmitting data via UART, and handling UART interrupts.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef INC_UART_H_
#define INC_UART_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"
#include <stdint.h>

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief Enumeration defining UART channels.
 */
typedef enum
{
  EN_UART_0 = 0,   /**< UART channel 0 */
  EN_UART_1         /**< UART channel 1 */
}enUartChannel;

/**
 * @brief Enumeration defining UART modes.
 */
typedef enum
{
  EN_UART_MODE_FIFO = 0,  /**< FIFO mode */
  EN_UART_MODE_SDMA        /**< SDMA mode */
}enUartMode;

/**
 * @brief Enumeration defining UART baud rates.
 */
typedef enum
{ 
  EN_UART_BAUDRATE_9600 = 0,  /**< Baud rate: 9600   */
  EN_UART_BAUDRATE_14400,     /**< Baud rate: 14400  */
  EN_UART_BAUDRATE_19200,     /**< Baud rate: 19200  */
  EN_UART_BAUDRATE_38400,     /**< Baud rate: 38400  */
  EN_UART_BAUDRATE_57600,     /**< Baud rate: 57600  */
  EN_UART_BAUDRATE_115200,    /**< Baud rate: 115200 */
  EN_UART_BAUDRATE_230400,    /**< Baud rate: 230400 */
  EN_UART_BAUDRATE_460800,    /**< Baud rate: 460800 */
  EN_UART_BAUDRATE_921600,    /**< Baud rate: 921600 */
  EN_UART_BAUDRATE_1536000    /**< Baud rate: 1536000 */
} enUartBaudrate;


/**
 * @brief Enumeration defining UART RX buffer wrap-around.
 * @note Only applicable in SDMA mode; otherwise, set to disable.
 */
typedef enum {
  EN_UART_RXBUF_WRAP_DISABLE = 0, /**< Disable RX buffer wrap-around */
  EN_UART_RXBUF_WRAP_ENABLE       /**< Enable RX buffer wrap-around */
} enUartRxBufWrap;

/**
 * @brief Enumeration defining the number of stop bits.
 */
typedef enum {
  EN_UART_STOP_BITS_1 = 0,   /**< 1 stop bit */
  EN_UART_STOP_BITS_1_5,     /**< 1.5 stop bits */
  EN_UART_STOP_BITS_2,       /**< 2 stop bits */
  EN_UART_STOP_BITS_3        /**< 3 stop bits */
} enUartStopBits;

/**
 * @brief Enumeration defining UART communication bit order.
 */
typedef enum {
  EN_UART_BIT_ORDER_LSB_FIRST = 0, /**< Least Significant Bit (LSB) first */
  EN_UART_BIT_ORDER_MSB_FIRST      /**< Most Significant Bit (MSB) first */
} enUartBitOrder;

/**
 * @brief Enumeration defining UART parity bit configuration.
 */
typedef enum {
  EN_UART_PARITY_NONE = 0, /**< No parity bit */
  EN_UART_PARITY_EVEN,     /**< Even parity */
  EN_UART_PARITY_ODD       /**< Odd parity */
} enUartParity;

/**
 * @brief Enumeration defining UART flow control configuration.
 */
typedef enum {
  EN_UART_FLOW_CONTROL_DISABLE = 0, /**< Disable flow control */
  EN_UART_FLOW_CONTROL_ENABLE       /**< Enable flow control */
} enUartFlowControl;


/**
 * @brief Enumeration defining UART interrupt types
 */
typedef enum {
  EN_UART_INT_TXD_READY          = 0x0001,   /**< enable interrupt by txd_ready event */
  EN_UART_INT_TXFIFO_EMPTY       = 0x0002,   /**< enable interrupt by txfifo_ready event*/
  EN_UART_INT_TXFIFO_FULL        = 0x0004,   /**< enable interrupt by txfifo_full event*/
  EN_UART_INT_TXB_EMPTY          = 0x0008,   /**< enable interrupt by txb_empty event*/
  EN_UART_INT_RXD_READY          = 0x0010,   /**< enable interrupt by rxd_ready event*/
  EN_UART_INT_RXFIFO_READY       = 0x0020,   /**< enable interrupt by rxfifo_ready event*/
  EN_UART_INT_RXFIFO_EMPTY       = 0x0040,   /**< enable interrupt by rxfifo_empty event*/
  EN_UART_INT_RXFIFO_FULL        = 0x0080,   /**< enable interrupt by rxfifo_full event*/
  EN_UART_INT_RXB_FULL           = 0x0100,   /**< enable interrupt by rxb_full event*/
  EN_UART_INT_CTS                = 0x0200,   /**< enable interrupt by cts event*/
  EN_UART_INT_RXFIFO_OVF_ERR     = 0x0400,   /**< enable interrupt by rxfifo_ovf_err event*/
  EN_UART_INT_PARITY_ERR         = 0x0800,   /**< enable interrupt by parity_err event*/
  EN_UART_INT_FRAME_ERR          = 0x1000,   /**< enable interrupt by frame_err event*/
  EN_UART_INT_BREAK_ERR          = 0x2000,   /**< enable interrupt by break_err event*/
  EN_UART_INT_TXB_RD_ERR         = 0x4000,   /**< enable interrupt by txb_rd_err event*/
  EN_UART_INT_RXB_WR_ERR         = 0x8000,    /**< enable interrupt by rxb_wr_err event*/
  EN_UART_INT_ALL                = 0xFFFF    /**< All  interrupt combined */
} enUartInt;

/**
 * @brief Enumeration defining UART flags.
 * @details Each flag is represented by a unique bit in a 16-bit value.
 */
typedef enum {
  EN_UART_FLAG_TX_ON              = 0x0001,   /**< UART transmitter is on */
  EN_UART_FLAG_RX_ON              = 0x0002,   /**< UART receiver is on */
  EN_UART_FLAG_TXD_READY          = 0x0004,   /**< UART TX data register is ready for transmission */
  EN_UART_FLAG_TXFIFO_EMPTY       = 0x0008,   /**< UART TX FIFO is empty */
  EN_UART_FLAG_TXFIFO_FULL        = 0x0010,   /**< UART TX FIFO is full */
  EN_UART_FLAG_TXB_EMPTY          = 0x0020,   /**< UART TX buffer is empty */
  EN_UART_FLAG_RXD_READY          = 0x0040,   /**< UART RX data register has data ready */
  EN_UART_FLAG_RXFIFO_READY       = 0x0080,   /**< UART RX FIFO has data ready */
  EN_UART_FLAG_RXFIFO_EMPTY       = 0x0100,   /**< UART RX FIFO is empty */
  EN_UART_FLAG_RXFIFO_FULL        = 0x0200,   /**< UART RX FIFO is full */
  EN_UART_FLAG_RXB_FULL           = 0x0400,   /**< UART RX buffer is full */
  EN_UART_FLAG_CTS                = 0x0800,   /**< Clear to Send signal is active */
  EN_UART_FLAG_RXFIFO_OVF_ERR     = 0x1000,   /**< UART RX FIFO overflow error occurred */
  EN_UART_FLAG_PARITY_ERR         = 0x2000,   /**< Parity error detected */
  EN_UART_FLAG_FRAME_ERR          = 0x4000,   /**< Frame error detected */
  EN_UART_FLAG_BREAK_ERR          = 0x8000,   /**< Break error detected */
  EN_UART_FLAG_TXB_RD_ERR         = 0x10000,  /**< UART TX buffer read error occurred */
  EN_UART_FLAG_RXB_WR_ERR         = 0x20000,  /**< UART RX buffer write error occurred */
  EN_UART_FLAG_ALL                = 0x3FFFF   /**< All UART flags combined */
} enUartFlag;
//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

/**
 * @brief Structure defining UART configuration.
 */
typedef struct 
{
  enUartChannel      uartChannel;    /**< UART channel */
  enUartMode         uartMode;      /**< UART mode */
  enUartBaudrate     uartBaudrate;  /**< UART baud rate */
  enUartRxBufWrap    uartRxBufWrap;   /**< UART RX buffer wrap-around */
  enUartStopBits     uartStopBits;  /**< UART number of stop bits */
  enUartBitOrder     uartBitOrder;   /**< UART Communication bit order  */
  enUartParity       uartParity;   /** UART configure of parity bits */
  enUartFlowControl  uartFlowControl;  

  uint16_t  uartInt;
  
  uint16_t  uartRxMaxBytes;    

  uint32_t  TXbuffer;
  uint32_t  RXbuffer;

} stUartConfig; 

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
 * @brief Initialize UART communication with specified configuration.
 *
 * This function initializes UART communication with the provided configuration.
 *
 * @param[in] uartConfig Configuration structure for UART initialization.
 */
void cb_uart_init(stUartConfig uartConfig);

/**
 * @brief Deinitialize UART communication for a specified UART channel.
 *
 * This function deinitializes the UART communication by disabling the UART module,
 * clearing configuration registers, and disabling interrupts for the specified UART channel.
 *
 * @param[in] uartChannel The UART channel to deinitialize. Use EN_UART_0 or EN_UART_1.
 */
void cb_uart_deinit(enUartChannel uartChannel);

/**
 * @brief Initialize a stUartConfig structure with default values.
 *
 * This function initializes a stUartConfig structure with default UART configuration values.
 * It sets all the configuration parameters to their default values, ensuring that the UART
 * is configured with a standard set of settings when initialized.
 *
 * @param[in] uartConfig Pointer to the stUartConfig structure to be initialized.
 */
void cb_uart_struct_init(stUartConfig *uartConfig); 

/**
 * @brief Enable or disable the specified UART channel.
 *
 * This function enables or disables the specified UART channel for transmission and reception.
 *
 * @param[in] uartChannel The UART channel to enable or disable. Use EN_UART_0 or EN_UART_1.
 * @param[in] newState New state of the UART channel. Use ENABLE or DISABLE.
 */
void cb_uart_cmd(enUartChannel uartChannel, uint8_t newState);

/**
 * @brief Transmit multiple bytes of data
 *
 * This function transfer multiple bytes of data, in either SDMA or FIFO mode
 *
 * @param[in] uartConfig Configuration structure for UART initialization.
 * @param[in] data  The data array to be sent to UART TXD register
 * @param[in] size  Size of the data array in bytes
 */
 void cb_uart_transmit(stUartConfig uartConfig, uint8_t *data, uint16_t size);

/**
 * @brief Check if the UART transmission (TXD) is not available 
 *
 * This function returns true if the TX is not available yet
 *
 * @param[in] uartConfig Configuration structure for UART initialization.
 * @return    true if TX is busy, false otherwise.  
 */
uint8_t cb_uart_is_tx_busy(stUartConfig uartConfig);

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
void cb_uart_set_rx_num_of_bytes(enUartChannel uartChannel, uint16_t maxBytes); 

/**
 * @brief Gets one byte from the specified UART RXD register
 *
 * This function returns a byte read from the specified UART RXD register.
 * It is primarily intended for use in FIFO mode, where data is received one byte at a time.
 *
 * @param[in] uartChannel The UART channel to read from. Use EN_UART_0 or EN_UART_1.
 * @return One byte read from the specified UART RXD register.
 */
uint8_t cb_uart_get_rx_byte(enUartChannel uartChannel);

/**
 * @brief Retrieves data from the UART receive buffer.
 *
 * This function copies a specified number of bytes from the UART receive buffer into a destination array.
 * The primary use case is for SDMA mode where data is directly transferred from the UART to a memory buffer.
 *
 * @param uartChannel[in] The UART channel from which to retrieve data. Use EN_UART_0 or EN_UART_1.
 * @param dest[out]        Pointer to the destination array where the received data will be stored.
 * @param numBytes[in]    The number of bytes to read from the receive buffer and copy to the destination array.
 */
void cb_uart_get_rx_buffer(enUartChannel uartChannel,uint8_t *dest, uint16_t numBytes);

/**
 * @brief Checks the number of bytes received in the specified UART channel.
 *
 * This function checks the number of bytes that have been received in the specified UART channel buffer.
 * It reads the relevant status register and extracts the number of received bytes.
 *
 * @param[in] uartChannel The UART channel to check. Use EN_UART_0 or EN_UART_1.
 * @return The number of bytes currently available in the specified UART receive buffer.
 */
uint16_t cb_uart_check_num_received_bytes(enUartChannel uartChannel);

/**
 * @brief Stops the receive operation for the specified UART channel.
 *
 * This function stops the receiving process for the specified UART channel.
 *
 * @param[in] uartChannel The UART channel to stop. Use EN_UART_0 or EN_UART_1.
 */
void cb_uart_rx_stop(enUartChannel uartChannel);

/**
 * @brief Restarts the receive operation for the specified UART channel.
 *
 * This function restarts the receive process for the specified UART channel.
 *
 * @param[in] uartChannel The UART channel to restart. Use EN_UART_0 or EN_UART_1.
 */
void cb_uart_rx_restart(enUartChannel uartChannel);

/**
 * @brief Configure UART interrupts for a specified UART channel.
 * @details This function configures the specified UART channel's interrupts based on the provided parameters.
 * It enables or disables the specified interrupt type for the given UART channel.

 * @param[in] uartChannel The UART channel to configure. Use EN_UART_0 or EN_UART_1.
 * @param[in] uart_int    Specifies the UART interrupts sources to be enabled or disabled.
 *                        This parameter can be any combination of the values defined in the "enUartInt" enumeration.
 * @param[in] newState    The new state for the interrupt. Use ENABLE to enable the interrupt and DISABLE to disable it.
 */
void cb_usart_irq_config(enUartChannel uartChannel, uint16_t uart_int, uint8_t newState);

/**
 * @brief Get the current UART flags for a specified UART channel.
 *
 * This function reads the UART status register and returns the current flags.
 * The flags indicate the status of various UART events and errors.
 *
 * @param uartChannel The UART channel to query. Use EN_UART_0 or EN_UART_1.
 * @return The current UART flags as a bitmask. Refer to the enUartFlag enumeration for flag definitions.
 */
uint32_t cb_uart_get_irq_flags(enUartChannel uartChannel);

/**
 * @brief Clear interrupt flags for a specified UART channel.
 *
 * This function clears the interrupt flags for the specified UART channel.
 * It ensures that the interrupt flags are reset to avoid repeated interrupt triggers.
 *
 * @param uartChannel The UART channel for which to clear the interrupt flags. Use EN_UART_0 or EN_UART_1.
 */
void cb_uart_clear_irq(enUartChannel uartChannel); 

/**
 * @brief   UART0 interrupt request handler.
 * @details This function handles various UART0 interupts and calls corresponding callback.
 *
 */
void cb_uart_0_irqhandler(void);

/**
 * @brief   UART1 interrupt request handler.
 * @details This function handles various UART1 interupts and calls corresponding callback.
 *
 */
void cb_uart_1_irqhandler(void);

/**
 * @brief IRQ callback for UART0 RX buffer full at APP layer
 * This callback is executed when the number of bytes received matches RXB_MAX_BYTES in UART->BUF_SIZE register
 */
void cb_uart_0_rxb_full_app_irq_callback(void);

/**
 * @brief IRQ callback for UART0 TXB empty at APP layer.
 * Last TX byte has been sent out
 */
void cb_uart_0_txb_empty_app_irq_callback(void);

/**
 * @brief IRQ callback for UART1 RX buffer full at APP layer
 * This callback is executed when the number of bytes received matches RXB_MAX_BYTES in UART->BUF_SIZE register
 */
void cb_uart_1_rxb_full_app_irq_callback(void);

/**
 * @brief IRQ callback for UART1 TXB empty at APP layer.
 * Last TX byte has been sent out
 */
void cb_uart_1_txb_empty_app_irq_callback(void);

#endif /* INC_UART_H_ */
