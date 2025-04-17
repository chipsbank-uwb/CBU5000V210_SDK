#ifndef __APP_CPU_I2C_
#define __APP_CPU_I2C_

#include "app_i2c.h"
#include "CB_i2c.h"
#include "CB_iomux.h"
#include "CB_gpio.h"
#include "CB_scr.h"
#include "CB_system.h"
#include "AppSysIrqCallback.h"

/**
 * @brief Initialize I2C for application
 */
void app_i2c_init(void);

/**
 * @brief Reads data from a specified I2C device register.
 *
 * This function reads a specified number of bytes from a given I2C device's register. 
 * It supports both DMA and non-DMA modes based on the `USE_DMA_I2C` configuration.
 *
 * @param address The I2C address of the target device.
 * @param reg The register address within the target device.
 * @param buf Pointer to the buffer where the read data will be stored.
 * @param size Number of bytes to read.
 * @return CB_STATUS Returns `CB_SUCCESS` if the operation is successful, otherwise returns `CB_FAIL`.
 */
CB_STATUS app_i2c_read(uint8_t address, uint8_t reg, uint8_t *buf, uint8_t size);

/**
 * @brief Write data to a specified register of an I2C device.
 * 
 * This function writes a block of data to a specified register of an I2C device. It supports both DMA and non-DMA modes.
 * In DMA mode, the function waits for the DMA transfer to complete within a specified timeout period and checks for any NACK.
 * In non-DMA mode, it directly calls the write function.
 * 
 * @param address The I2C device address.
 * @param reg The register address to write data to.
 * @param buf Pointer to the data buffer.
 * @param size Number of bytes to write.
 * @return CB_STATUS The operation status, indicating success or failure.
 */
CB_STATUS app_i2c_write(uint8_t address, uint8_t reg, uint8_t *buf, uint8_t size);

/**
 * @brief Main function to initialize and start the I2C peripheral.
 * 
 * This function initializes the UART and timer modules, starts the timer, 
 * prints "Power on" via UART, and initializes the IMU 42670 registers.
 */
void app_peripheral_i2c_init(void);

/**
 * @brief Loop function for I2C peripheral operations.
 * 
 * This function checks the I2C flag and retrieves gyroscope and accelerometer 
 * data from the IMU if the flag is set.
 */
void app_peripheral_i2c_loop(void);

/**
 * @brief Timer 0 Interrupt Request (IRQ) Callback Function.
 *
 * This function is called when Timer 0 generates an interrupt. It retrieves gyroscope and accelerometer data from the IMU.
 * The retrieved data can be used for further processing or logging.
 */
void app_timer_0_irq_callback (void);
#endif
