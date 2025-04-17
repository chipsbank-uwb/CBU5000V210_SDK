/**
 * @file    app_spislaver.c
 * @brief   SPI Slave API Example Code for demonstration of SPI Slave mode using FIFO and SDMA.
 * 
 * This file contains implementation examples for configuring and using SPI Slave mode in both 
 * FIFO and SDMA (Scatter-DMA) modes, demonstrating data transmission and reception with a master 
 * device. The provided functions initialize the SPI slave, handle data callbacks, and print received 
 * data to the UART interface if enabled.
 * 
 * @note    This code assumes specific hardware configurations and dependencies on the Chipsbank SDK.
 * @author  Chipsbank
 * @date    2024
 */
#ifndef APP_CPU_SPISLAVE_H
#define APP_CPU_SPISLAVE_H

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
 * @brief   Example demonstrating SPI slave mode using FIFO for data reception and response.
 * 
 * This function configures the SPI slave and sets up a FIFO mode transaction, where the slave 
 * sends 10 bytes of data to the master. The master is expected to provide 1 extra dummy byte for
 * clock synchronization.
 */
void app_spi_slave_main_example_fifo(void);

/**
 * @brief   Example demonstrating SPI slave mode using SDMA (Scatter-DMA) for data reception.
 * 
 * This function configures the SPI slave for SDMA mode, preparing a buffer for transmitting 
 * 10 bytes of data. The master is expected to send 1 extra dummy byte for clock synchronization.
 */
void app_spi_slave_main_example_sdma(void);

/**
 * @brief   Initializes the SPI slave peripheral with required settings.
 * 
 * This function configures the IOMUX for SPI pins (CS, CLK, MISO, MOSI) and sets up the SPI 
 * peripheral parameters such as bit order, byte order, speed, and mode.
 */
void app_spi_slave_init(void);

/**
 * @brief   Deinitializes the SPI slave peripheral.
 * 
 * This function deinitializes the SPI peripheral, freeing any allocated resources.
 */
void app_spi_slave_deinit(void);

/**
 * @brief   SPI Slave FIFO mode callback for handling MOSI data reception and MISO response.
 * 
 * This callback function is called when a byte is received from the master in FIFO mode. The received 
 * byte is stored in the receive buffer, and the slave responds by sending the same byte back to the master.
 * 
 * @param   mosidata    The data received from the SPI master.
 * @return  The data to be sent back to the SPI master.
 */
uint8_t app_spi_slave_fifo_mode_response_callback(uint8_t mosidata);

/**
 * @brief   Completion callback for SPI Slave FIFO mode transaction.
 * 
 * This function is called when the SPI slave has finished receiving data in FIFO mode. 
 * If the transaction is successful, it prints the received data to the UART.
 * 
 * @param   completionstatus    The status of the SPI transaction (e.g., success or failure).
 */
void app_spi_slave_fifo_receive_complete_callback(uint8_t completionstatus);

/**
 * @brief   Completion callback for SPI Slave SDMA mode transaction.
 * 
 * This function is called when the SPI slave has completed receiving data in SDMA mode. 
 * If the transaction is successful, it processes the received data by removing the unwanted 
 * first byte (used for clock synchronization) and shifts the valid data one byte forward in 
 * the receive buffer. The received data is then printed to the UART for debugging purposes.
 * 
 * @param   completionstatus    The status of the SPI SDMA transaction (e.g., success or failure).
 */
void app_spi_slave_sdma_receive_complete_callback(uint8_t completionstatus);

/**
 * @brief Main function for SPI slaver peripheral mode
 * 
 * This function initializes the UART and prints the power-on reset message for the SPI slaver.
 */
void app_peripheral_spi_slaver_init(void);

/**
 * @brief Main loop function for handling SPI Slave received data.
 *
 * This function continuously checks for completed SPI Slave data reception and processes the received data.
 * Key steps include:
 * 1. Checking if the receive flag (`spi_slaver_rx_done`) is set, indicating that data reception has completed.
 * 2. Invoking the `app_spi_slave_main_example_sdma` function to handle the received data.
 * 3. Resetting the receive flag (`spi_slaver_rx_done`) to indicate that the data has been processed.
 *
 * @note This function is designed to be called repeatedly in the main loop to ensure timely processing of received data.
 */
void app_peripheral_spi_slave_loop(void);

#endif //APP_CPU_SPISLAVE_H



