/**
 * @file    app_spimaster.h
 * @brief   SPI interface functions for initializing and communicating with the IMU.
 * 
 * This header provides the function declarations for initializing the SPI interface
 * and performing read and write operations on an IMU device using SPI communication.
 * 
 * @author  Chipsbank
 * @date    2024
 */
#ifndef APP_CPU_SPIMASTER_H
#define APP_CPU_SPIMASTER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include "CB_spi.h"

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
/**
 * @brief   Initializes the SPI interface and configures the GPIO pins for SPI communication with the IMU.
 * 
 * This function sets up the SPI interface, including the speed, mode, and bit order, 
 * and configures the GPIO pins required for SPI communication (CS, CLK, MISO, MOSI). 
 * The function uses the cb_iomux_config() function to map the SPI pins to their respective functions 
 * and then initializes the SPI communication parameters through cb_spi_init().
*/
void app_spi_master_init(void);

/**
 * @brief   Reads data from an IMU register via SPI.
 * 
 * This function reads data from a specified register of the IMU using SPI communication. 
 * The function selects the appropriate IMU device via the `chips_selct` parameter, specifies 
 * the register address to read from, and stores the read data in the provided buffer.
 * 
 * @param   chips_selct   The chip select signal to choose the IMU device (SPI chip select).
 * @param   imu_register The register address within the IMU to read from.
 * @param   buffer      Pointer to a buffer where the read data will be stored.
 * @param   datalength  The number of bytes to read from the IMU register.
 * 
 * @return  CB_STATUS  The status of the SPI read operation.
 *                      - `CB_PASS`: Operation was successful.
 *                      - `CB_FAIL`: There was an error during the SPI communication.
 */
CB_STATUS app_spi_master_read_imu(enSPIChipSelect chips_selct,uint8_t imu_register,uint8_t* buffer,uint16_t datalength);

/**
 * @brief   Writes data to an IMU register via SPI.
 * 
 * This function writes data to a specified IMU register using SPI communication. It selects the appropriate
 * IMU device via the `chips_selct` parameter, sends the register address to be written, and writes the data from
 * the provided buffer.
 * 
 * @param   chips_selct   The chip select signal to choose the IMU device (SPI chip select).
 * @param   imu_register The register address within the IMU to write to.
 * @param   buffer      Pointer to a buffer containing the data to write to the IMU.
 * @param   datalength  The number of bytes to write to the IMU register.
 * 
 * @return  CB_STATUS  The status of the SPI write operation.
 *                      - `CB_PASS`: Operation was successful.
 *                      - `CB_FAIL`: There was an error during the SPI communication.
 */
CB_STATUS app_spi_master_write_imu(enSPIChipSelect chips_selct,uint8_t imu_register,uint8_t* buffer,uint16_t datalength);

/**
 * @brief   Performs a SPI SDMA (Single Data Multiple Access) write and read operation.
 *
 * This function initiates a SPI SDMA write and read operation using the specified chip select,
 * data length, transmit buffer, and receive buffer. It then prints the contents of the transmit
 * and receive buffers to the UART for debugging purposes.
 *
 * @param   chips_selct   The chip select signal to choose the SPI device (SPI chip select).
 * @param   Length      The number of bytes to transfer (both transmit and receive).
 *
 * @return  None
 */
void app_spi_master_sdma_write_read(enSPIChipSelect chips_selct,uint16_t Length);

/**
 * @brief   Main function for initializing and setting up the SPI master peripheral.
 * 
 * This function initializes the UART for communication, sets up the Timer module with interrupts,
 * configures the Timer parameters, initializes the SPI master, and prints a power-on reset flag.
 */
void app_peripheral_spi_master_init(void);

/**
 * @brief   Main loop function for handling SPI master operations based on UART commands.
 * 
 * This function continuously checks for received UART data. If the receive flag is set, it processes the received data.
 * If the received data starts with the command "CCCC", it reinitializes the SPI master and performs an SDMA write-read operation.
 * Otherwise, it sends the received data in hexadecimal format over UART. After processing, it clears the receive buffer and resets the length.
 */
void app_peripheral_spi_master_loop(void);

/**
 * @brief Callback function for UART0 RX buffer full interrupt.

 * This function is triggered when the UART0 RX buffer is full. It reads a single byte of data from the RX buffer
 * and stores it in the global `RxBuffer`. It also sets the `Rx_flag` to indicate that new data has been received.
 * 
 * @note The function supports both FIFO and SDMA modes for data reception. In this implementation, SDMA mode is used.
 * @return None
 */
void app_uart_0_rxb_full_callback(void);      
#endif //APP_CPU_SPIMASTER_H



