/**
 * @file    CB_i2c.h
 * @brief   Header file for I2C communication module.
 * @details This file defines structures, constants, and function prototypes for I2C communication.
 *          Function prototypes are provided for initializing I2C, reconfiguring I2C settings,
 *          transmitting data via I2C, and handling I2C interrupts.
 * @author  Chipsbank
 * @date    2024
 */
#ifndef INC_I2C_H_
#define INC_I2C_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"
#include "CB_dma.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief   I2C interrupt types.
 * @details Enumerates different types of I2C interrupts.
 */
typedef enum 
{
  EN_I2C_INT_DISABLE      = 0x00,  /**< Disable all I2C interrupts. */
  EN_I2C_INT_RX_FULL      = 0x01,  /**< Enable interrupt when receive buffer is full. */
  EN_I2C_INT_TX_EMPTY     = 0x02,  /**< Enable interrupt when transmit buffer is empty. */
  EN_I2C_INT_READY        = 0x04,  /**< Enable interrupt when I2C is ready. */
  EN_I2C_INT_NACK         = 0x08,  /**< Enable interrupt when NACK is received. */
  EN_I2C_INT_ALL          = 0x0f   /**< Enable all interrupt */
} enI2CInt;

/**
 * @brief   I2C DMA settings.
 * @details Enumerates DMA settings for I2C.
 */
typedef enum 
{
  EN_I2C_DMA_DISABLE  = 0,  /**< Disable DMA for I2C. */
  EN_I2C_DMA_ENABLE   = 1   /**< Enable DMA for I2C. */
} enI2CDMA;

///**
// * @brief   Functional state enumeration.
// * @details Enumerates functional states for enabling or disabling features.
// */
//typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief   I2C configuration structure.
 * @details Structure to hold I2C configuration settings.
 */
typedef struct 
{
  enI2CInt I2CInt;            /**< I2C interrupt configuration. */
  uint32_t I2CSpeed;          /**< I2C speed in Hz. */
} stI2CConfig;
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief I2C Interrupt Handler.
 * @detail Abort I2C Transmission if neccessary.
 */
void cb_i2c_irqhandler (void);

/**
 * @brief Weakly defined I2C Interrupt Handler at MID level.
 */
void cb_i2c_app_irq_callback(void);

/**
 * @brief                 Init the I2C communication module with specified speed.
 * @detail                Set the correct speed for I2C and also registers I2C interrupt system-wise.
 * @param[in] I2CConfig   Pointer to the I2C configuration structure
 */
void cb_i2c_init(stI2CConfig *I2CConfig);

/**
 * @brief DeInit the I2C communication module.
 * @detail Disable I2C interrupts and turn off its enable bits.
 */
void cb_i2c_deinit(void);

/**
 * @brief   Initialize the I2C configuration structure with default values.
 * @details This function initializes the provided I2C configuration structure with default settings.
 *          The default configuration sets the I2C speed to 100 kHz and disables all interrupts.
 * @param   I2CConfig Pointer to the I2C configuration structure that will be initialized with default values.
 */
void cb_i2c_struct_init(stI2CConfig *I2CConfig);

/**
* @brief  Enable the I2C communication module.
*/
void cb_i2c_enable(void);

/**
* @brief  Disable the I2C communication module and suspend any ongoing transaction.
*/
void cb_i2c_disable(void);

/**
 * @brief  Checks for transaction status of the I2C (done/busy).
 * @return CB_TRUE if I2C ready, CB_FALSE otherwise 
 */
uint8_t cb_i2c_is_ready(void);

/**
 * @brief   Returns the staus of NACK flag
 *          This function is useful to check if the previous transmission has been successful
 *          or not.
 *
 * @return  CB_TRUE          NACK flag is SET
 *          CB_FAIL          NACK flag is CLEARED
 */
uint8_t cb_i2c_is_nack(void);

/**
 * @brief      Write as Master using I2C.
 * @param[in]  PeripheralAddr   7 bits address of Slave
 * @param[in]  txbuf            buffer containing data to be written
 * @param[in]  DataLen          Length of data to be written. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission successful
 *             CB_FAIL          NACK flag received from slave device or transmission timed out
 */
CB_STATUS cb_i2c_master_write(uint8_t PeripheralAddr, uint8_t *txbuf, uint16_t DataLen);

/**
 * @brief      Write as Master using I2C, to a specific register.
 * @param[in]  PeripheralAddr   7 bits address of Slave
 * @param[in]  RegAddr          8 bits address of Register
 * @param[in]  txbuf            buffer containing data to be written
 * @param[in]  DataLen          Length of data to be written. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission successful
 *             CB_FAIL          NACK flag received from slave device or transmission timed out
 */
CB_STATUS cb_i2c_master_write_to_register(uint8_t PeripheralAddr, uint8_t RegAddr, uint8_t *txbuf, uint16_t DataLen);

/**
 * @brief       Read as Master using I2C from a slave.
 * @param[in]   PeripheralAddr  7 bits address of Slave.
 * @param[out]  rxbuf           buffer to contain read data.
 * @param[in]   DataLen         Length of data to be read. Maximum is 4095 bytes at a time.
 * @return      CB_PASS         I2C Transmission successful
 *              CB_FAIL         NACK flag received from slave device or transmission timed out
 */
CB_STATUS cb_i2c_master_read(uint8_t PeripheralAddr, uint8_t *rxbuf, uint16_t DataLen);

/**
 * @brief      Read as Master using I2C from a specific register
 * @param[in]  PeripheralAddr   7 bits address of Slave.
 * @param[in]  RegAddr          8 bits address of Register.
 * @param[out] rxbuf            buffer to contain read data.
 * @param[in]  DataLen          Length of data to be read. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission successful
 *             CB_FAIL          NACK flag received from slave device or transmission timed out
 */
CB_STATUS cb_i2c_master_read_from_register(uint8_t PeripheralAddr, uint8_t RegAddr, uint8_t *rxbuf, uint16_t DataLen);

/**
 * @brief      Write as Master using I2C+DMA to a specific register.
 * @param[in]  DMAChannel       DMA channel to use for I2C writes
 * @param[in]  PeripheralAddr   8 bits address of Slave.
 * @param[in]  RegAddr          7 bits address of Register.
 * @param[in]  BufAddr          address of buffer containing data to be written.
 * @param[in]  DataLen          Length of data to be written. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission setup successful
 *             CB_FAIL          I2C module not ready or data length exceeded maximum or NACK received
 */
CB_STATUS cb_i2c_master_write_to_register_dma (enDMAChannel DMAChannel, uint8_t PeripheralAddr, uint8_t RegAddr, uint32_t BufAddr, uint32_t DataLen);

/**
 * @brief  Read as Master using I2C+DMA from a specific register.
 * @param[in]  DMAChannel       DMA channel to use for I2C writes
 * @param[in]  PeripheralAddr   8 bits address of Slave.
 * @param[in]  RegAddr          7 bits address of Register.
 * @param[in]  BufAddr          address of buffer containing data to be read.
 * @param[in]  DataLen          Length of data to be read. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission setup successful
 *             CB_FAIL          I2C module not ready or data length exceeded maximum or NACK received
 */
CB_STATUS cb_i2c_master_read_from_register_dma (enDMAChannel DMAChannel, uint8_t PeripheralAddr, uint8_t RegAddr, uint32_t BufAddr, uint32_t DataLen);

/**
 * @brief   Configure I2C interrupts.
 * @param[in] I2C_INT     Specifies the I2C interrupts sources to be enabled or disabled.
 *                      This parameter can be any combination of the following values:
 *                      - @arg EN_I2C_INT_RX_FULL: Enable interrupt when the receive buffer is full.
 *                      - @arg EN_I2C_INT_TX_EMPTY: Enable interrupt when the transmit buffer is empty.
 *                      - @arg EN_I2C_INT_READY: Enable interrupt when the I2C is ready.
 *                      - @arg EN_I2C_INT_NACK: Enable interrupt when a NACK is received.
 *                      - @arg EN_I2C_INT_ALL: Check and clear all I2C interrupts.
 * @param[in] NewState   The new state for the interrupt. Use ENABLE to enable the interrupt and DISABLE to disable it.
 */
void cb_i2c_int_config(uint16_t I2C_INT, uint8_t NewState);

/**
 * @brief   Get and clear the status of I2C interrupts.
 * @param[in] I2C_INT     Specifies the I2C interrupts sources to check and clear.
 *                      This parameter can be any combination of the following values:
 *                      - @arg EN_I2C_INT_RX_FULL: Check and clear the receive buffer full interrupt.
 *                      - @arg EN_I2C_INT_TX_EMPTY: Check and clear the transmit buffer empty interrupt.
 *                      - @arg EN_I2C_INT_READY: Check and clear the I2C ready interrupt.
 *                      - @arg EN_I2C_INT_NACK: Check and clear the NACK received interrupt.
 *                      - @arg EN_I2C_INT_ALL: Check and clear all I2C interrupts.
 * @return              The status of the specified I2C interrupts. Each bit corresponds to the status of one interrupt source.
 */
uint8_t cb_i2c_get_and_clear_int_status(uint16_t I2C_INT);


#endif /*INC_I2C_H_*/
