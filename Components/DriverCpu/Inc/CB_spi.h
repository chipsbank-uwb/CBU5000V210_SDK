 /**
 * @file    CB_spi.h
 * @brief   Spi module driver header.
 * @details This header file provides functions for initializing and using the SPI interface for communication between a master
 * device and one or more slave devices.
 * @author   Chipsbank
 * @date     2024
 */
 
#ifndef __CB_SPI_H
#define __CB_SPI_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
typedef uint8_t (*spi_responsecallbk)(uint8_t); //callback function ptr type for SPI Slave Response
typedef void (*spi_completecallbk)(uint8_t); //callback function ptr type for SPI Slave notify completion

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief Enumeration for SPI Transaction Read/Write Commands.
 */
typedef enum
{
  EN_SPI_TRX_WRITE = 0,
  EN_SPI_TRX_READ
}enTRXCommandReadWriteBit;

/**
 * @brief Enumeration for SPI Transmission Modes.
 */
typedef enum
{
  EN_SPI_IDLE = 0,
  EN_SPI_MASTERFIFOTX,
  EN_SPI_MASTERFIFORX,
  EN_SPI_MASTERFIFOTRX_1BYTECOMMAND, //WRITE 1BYTE COMMAND and following with Write/Read Operation
  EN_SPI_MASTERSDMATX,
  EN_SPI_MASTERSDMARX,
  EN_SPI_MASTERSDMATRX,
  EN_SPI_SLAVEFIFO,
  EN_SPI_SLAVESMDA
}enTransmissonMode;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @enum enSPIChipSelect
 * @brief SPI chip select options.
 */
typedef enum
{
  EN_SPI_USE_CS0=0,
  EN_SPI_USE_CS1,
  EN_SPI_USE_CS2,
  EN_SPI_USE_CS3,
  EN_SPI_USE_CS4
}enSPIChipSelect;

/**
 * @enum enSPIMode
 * @brief SPI mode selection.
 */     
typedef enum
{
  EN_SPI_mode0  =0,
  EN_SPI_mode2  
}enSPIMode;      

/**
 * @enum enSPIBitOrder
 * @brief SPI bit order configuration.
 */  
typedef enum
{
  EN_SPI_bitorder_lsb_first  =0,
  EN_SPI_bitorder_msb_first  
}enSPIBitOrder;      

/**
 * @enum enSPIByteOrder
 * @brief SPI byte order configuration.
 */   
typedef enum
{
  EN_SPI_byteorder_byte0_first =0,
  EN_SPI_byteorder_byte3_first 
}enSPIByteOrder;

/**
 * @enum enSPIClockSpeed
 * @brief SPI clock speed configuration.
 */ 
typedef enum
{
  EN_SPI32MHz=0,
  EN_SPI16MHz,
  EN_SPI8MHz,
  EN_SPI4MHz,
  EN_SPI2MHz,
  EN_SPI1MHz,
  EN_SPI512KHz,
  EN_SPI256KHz  
}enSPIClockSpeed;

/**
  * @brief  SPI Init Structure definition
  */
typedef struct
{
  enSPIMode       SpiMode;
  enSPIBitOrder   BitOrder;
  enSPIByteOrder  ByteOrder;
  enSPIClockSpeed Speed;
}stSPI_InitTypeDef;


/**
 * @enum enSPIINT
 * @brief SPI Interrupt Flags.
 */
typedef enum {
  SPI_FRAME_END      = 0x001,  /**< Frame end interrupt flag */
  SPI_TX_END         = 0x002,  /**< Transmission end interrupt flag */
  SPI_RX_END         = 0x004,  /**< Reception end interrupt flag */
  SPI_TXFIFO_EMPTY   = 0x008,  /**< Transmit FIFO empty interrupt flag */
  SPI_TXFIFO_FULL    = 0x010,  /**< Transmit FIFO full interrupt flag */
  SPI_TXB_EMPTY      = 0x020,  /**< Transmit buffer empty interrupt flag */
  SPI_RXFIFO_EMPTY   = 0x040,  /**< Receive FIFO empty interrupt flag */
  SPI_RXFIFO_FULL    = 0x080,  /**< Receive FIFO full interrupt flag */
  SPI_RXB_FULL       = 0x100,  /**< Receive buffer full interrupt flag */
  SPI_TXFIFO_OVR_ERR = 0x200,  /**< Transmit FIFO overflow error interrupt flag */
  SPI_RXFIFO_OVR_ERR = 0x400,  /**< Receive FIFO overflow error interrupt flag */
  SPI_TXB_RD_ERR     = 0x800,  /**< Transmit buffer read error interrupt flag */
  SPI_RXB_WR_ERR     = 0x1000  /**< Receive buffer write error interrupt flag */
} enSPIINT;


//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Handles the SPI interrupt events.
 */
void cb_spi_irqhandler(void);

/**
 * @brief Initializes the SPI module with the specified parameters.
*/
void cb_spi_init(stSPI_InitTypeDef *InitParameters);

/**
 * @brief Deinitializes the SPI module.
 */
void cb_spi_deinit(void);

/**
 * @brief Initializes the SPI configuration structure with default values.
 *
 * This function sets the default configuration parameters for the SPI interface.
 *
 * @param InitParameters Pointer to the SPI initialization structure to be initialized.
 */
void cb_spi_struct_init(stSPI_InitTypeDef *InitParameters);

/**
 * @brief Enables or disables the SPI module.
 *
 * This function enables or disables the SPI module based on the provided command.
 *
 * @param Cmd Command to enable or disable the SPI module.
 *            - SPI_DISABLE: Disable the SPI module.
 *            - SPI_ENABLE: Enable the SPI module.
 */
void cb_spi_cmd(uint8_t Cmd);

/**
 * @brief Stops the ongoing SPI operation.
 */
void cb_spi_stop(void);

/**
 * @brief Transmits data over SPI in FIFO mode.
 * @param[in] ChipSelct Chip select for SPI device.
 * @param[in] txdata Pointer to the data buffer to be transmitted.
 * @param[in] Length Number of bytes to transmit.
 * @return CB_STATUS Status of the operation, CB_PASS if successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_fifo_write(enSPIChipSelect ChipSelct,uint8_t* txdata,uint16_t Length);

/**
 * @brief      Reads data from SPI in FIFO mode.
 * @param[in]  ChipSelct Chip select for SPI device.
 * @param[in]  Length Number of bytes to read.
 * @param[out] rxdata Pointer to the buffer where received data will be stored.
 * @return     CB_STATUS Status of the operation, CB_PASS if successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_fifo_read(enSPIChipSelect ChipSelct,uint8_t* rxdata,uint16_t Length);

/**
 * @brief         Performs single-byte read or write transaction on SPI using FIFO.
 * @param[in]     ChipSelct The SPI chip select to use for the transaction.
 * @param[in]     ReadOrWrite Specifies whether the transaction is read or write.
 * @param[in]     command Command byte to send in the transaction.
 * @param[in]     Length Length of data to read or write.
 * @param[in,out] trxbuffer Pointer to the buffer for read/write data.
 * @return        CB_STATUS CB_PASS if the transaction is successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_fifo_write_read(enSPIChipSelect ChipSelct,enTRXCommandReadWriteBit ReadOrWrite,uint8_t command,uint8_t* trxbuffer,uint16_t Length);

/**
 * @brief     Writes data over SPI in SDMA mode.
 * @param[in] ChipSelct Chip select for SPI device.
 * @param[in] Length Number of bytes to transmit.
 * @param[in] StartAddr Starting address of the transmit buffer.
 * @return    CB_STATUS Status of the operation, CB_PASS if successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_sdma_write(enSPIChipSelect ChipSelct,uint16_t Length, uint32_t StartAddr);

/**
 * @brief     Reads data from SPI in SDMA mode.
 * @param[in] ChipSelct Chip select for SPI device.
 * @param[in] Length Number of bytes to read.
 * @param[out] StartAddr Starting address of the receive buffer.
 * @return    CB_STATUS Status of the operation, CB_PASS if successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_sdma_read(enSPIChipSelect ChipSelct,uint16_t Length, uint32_t StartAddr);

/**
 * @brief      Performs DMA-based write and read transaction on SPI.
 * @param[in]  ChipSelct The SPI chip select to use for the transaction.
 * @param[in]  Length Length of data to transfer.
 * @param[in]  TxBufAddr DMA buffer address for transmit data.
 * @param[out] RxBufAddr DMA buffer address for receive data.
 * @return     CB_STATUS CB_PASS if the transaction is successfully initiated, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_sdma_write_read(enSPIChipSelect ChipSelct,uint16_t Length, uint32_t TxBufAddr, uint32_t RxBufAddr);

/**
 * @brief     Starts SPI operation in slave mode using FIFO.
 * @param[in] Length Length of data to be handled by SPI FIFO.
 * @param[in] p_ResponseCB Pointer to the callback function.
 * @param[in] p_CompletionCB Pointer to the callback function for completion notification.
 * @return    CB_STATUS CB_PASS if SPI operation is successfully started, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_slave_fifo_start(uint16_t Length,spi_responsecallbk p_ResponseCB, spi_completecallbk p_CompletionCB);

/**
 * @brief Starts SPI operation in slave mode using SDMA.
 *
 *                           !!!!!!!!!!!!!!!!!  Attention  !!!!!!!!!!!!!!!!!!
 * In SDMA mode, the first byte received by the slave is a clock alignment byte and should be ignored.
 * The actual valid data starts from the second byte. Therefore, the slave to receive n bytes of valid data, 
 * the master should send n+1 bytes. After the slave receives n+1 bytes, the valid data will be stored in the buffer
 * starting from the second byte (i.e., indices 1 to n+1), while the first byte (index 0) should be ignored.
 * 
 *  The host also needs to remove the first byte received when connecting to the slave,
 *  that is, the host sends n+1 and can only receive the slave nbyte
 *
 * @param[in] Length Length of data to be handled by SPI DMA. The actual valid data starts with the second byte,
 *               so to receive n bytes of valid data, the Length should be set to n+1.
 * @param[in] TxBufAddr Memory address of the TX buffer for DMA.
 * @param[in] RxBufAddr Memory address of the RX buffer for DMA.
 * @param[in] p_CompletionCB Pointer to the callback function for completion notification.
 * @return CB_STATUS CB_PASS if SPI operation is successfully started, otherwise CB_FAIL.
 */
/* Register a callback function for completion notification. In SDMA mode, the first byte in the RxBuffer
 * is a clock alignment byte and should be ignored. The valid data starts from the second byte. */
CB_STATUS cb_spi_slave_sdma_start(uint16_t Length,uint32_t TxBufAddr, uint32_t RxBufAddr, spi_completecallbk p_CompletionCB);

/**
 * @brief Retrieves the current transmission mode of the SPI interface.
 */
enTransmissonMode cb_spi_get_current_transmission_mode(void);

/**
 * @brief Configures the SPI interrupts.
 *
 * This function configures the specified SPI interrupt based on the provided command.
 *
 * @param SPI_IT Specifies the SPI interrupt to be configured.
 *               - SPI_FRAME_END: Frame end interrupt.
 *               - SPI_TX_END: Transmission end interrupt.
 *               - SPI_RX_END: Reception end interrupt.
 *               - SPI_TXFIFO_EMPTY: TX FIFO empty interrupt.
 *               - SPI_TXFIFO_FULL: TX FIFO full interrupt.
 *               - SPI_TXB_EMPTY: TX buffer empty interrupt.
 *               - SPI_RXFIFO_EMPTY: RX FIFO empty interrupt.
 *               - SPI_RXFIFO_FULL: RX FIFO full interrupt.
 *               - SPI_RXB_FULL: RX buffer full interrupt.
 * @param NewState Command to enable or disable the specified interrupt.
 *            - SPI_IT_DISABLE: Disable the specified interrupt.
 *            - SPI_IT_ENABLE: Enable the specified interrupt.
 */
void cb_spi_irq_config(enSPIINT SPI_IT, uint8_t NewState);

/**
 * @brief Retrieves the current SPI interrupt flags.
 *
 * This function returns the current state of the SPI interrupt flags.
 *
 * @return uint32_t The current SPI interrupt flags.
 */
uint32_t cb_spi_get_irq_flags(void);


#endif //__CB_SPI_H


