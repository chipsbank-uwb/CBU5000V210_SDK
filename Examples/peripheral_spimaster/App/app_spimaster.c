/**
 * @file    app_spimaster.c
 * @brief   SPI initialization and communication functions for IMU.
 * 
 * This file contains the initialization and read/write functions to communicate with an IMU using SPI protocol.
 * It sets up the GPIO pins required for SPI communication, configures the SPI interface, and provides read and write
 * functions to access the IMU registers.
 * 
 * @note The specific SPI pins and parameters such as speed and mode are defined within the functions.
 * 
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "app_spimaster.h"
#include "CB_spi.h"
#include "CB_iomux.h"
#include "CB_gpio.h"
#include "CB_scr.h"
#include "APP_common.h"
#include "CB_system.h"
#include "string.h" 

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_SPIMASTER_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_SPIMASTER_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_spimaster_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_spimaster_print(...)
#endif

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
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static uint8_t spi_txbuff[11] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20,21};
static uint8_t spi_rxbuff[11] = {0};
static uint8_t spi_master_rx_done;                // Receive done flag

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/* imu register setting reference :https://product.tdk.com/system/files/dam/doc/product/sensor/mortion-inertial/imu/data_sheet/ds-000347-icm-42688-p-v1.6.pdf*/

/**
 * @brief   Initializes the SPI interface and configures the GPIO pins for SPI communication with the IMU.
 * 
 * This function sets up the SPI interface, including the speed, mode, and bit order, 
 * and configures the GPIO pins required for SPI communication (CS, CLK, MISO, MOSI). 
 * The function uses the cb_iomux_config() function to map the SPI pins to their respective functions 
 * and then initializes the SPI communication parameters through cb_spi_init().
*/
void app_spi_master_init(void)
{
    stGPIO_InitTypeDef spi_pins;
    cb_scr_gpio_module_on();

    //This block sets up the GPIO pins for SPI communication
    cb_iomux_config(EN_IOMUX_GPIO_5,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_SPIM_CS0});/* CS   */
    cb_iomux_config(EN_IOMUX_GPIO_6,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_SPIM_CLK}); /* CLK  */
    cb_iomux_config(EN_IOMUX_GPIO_3,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_SPIM_MISO});/* MISO */ 
    cb_iomux_config(EN_IOMUX_GPIO_7,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_SPIM_MOSI});/* MOSI */ 

    // Initialize SPI configuration parameters
    stSPI_InitTypeDef hspi;
    hspi.BitOrder=EN_SPI_bitorder_msb_first;
    hspi.ByteOrder=EN_SPI_byteorder_byte0_first;
    hspi.Speed=EN_SPI2MHz;
    hspi.SpiMode=EN_SPI_mode0;  
    cb_spi_init(&hspi); 
}

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
CB_STATUS app_spi_master_read_imu(enSPIChipSelect chips_selct,uint8_t imu_register,uint8_t* buffer,uint16_t datalength)
{ 
    uint32_t delay;
    CB_STATUS status = cb_spi_master_fifo_write_read(chips_selct,EN_SPI_TRX_READ,imu_register,buffer,datalength);
    delay=(SystemCoreClock/1000);//1ms

    do{
        delay--;
        if( delay == 0)
        {
        status=CB_FAIL; break;
        }
    }while(cb_spi_get_current_transmission_mode() != EN_SPI_IDLE); //Wait SPI ready.
    return status;
}

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
CB_STATUS app_spi_master_write_imu(enSPIChipSelect chips_selct,uint8_t imu_register,uint8_t* buffer,uint16_t datalength)
{ 
    uint32_t delay;
    CB_STATUS status = cb_spi_master_fifo_write_read(chips_selct,EN_SPI_TRX_WRITE,imu_register,buffer,datalength);
    delay=(SystemCoreClock/1000);//1ms

    do{
        delay--;
        if( delay == 0)
        {
        status=CB_FAIL; break;
        }
    }while(cb_spi_get_current_transmission_mode() != EN_SPI_IDLE); //Wait SPI ready.

    return status;
}

/**
 * @brief   Performs a SPI SDMA (Single Data Multiple Access) write and read operation.
 *
 * This function initiates a SPI SDMA write and read operation using the specified chip select,
 * data length, transmit buffer, and receive buffer. It then prints the contents of the transmit
 * and receive buffers to the UART for debugging purposes.
 *
 * @param   chips_selct   The chip select signal to choose the SPI device (SPI chip select).
 * @param   length      The number of bytes to transfer (both transmit and receive).
 *
 * @return  None
 */
void app_spi_master_sdma_write_read(enSPIChipSelect chips_selct, uint16_t length)
{
    // Perform the SPI SDMA write and read operation
    cb_spi_master_sdma_write_read(EN_SPI_USE_CS0, length, (uint32_t)&spi_txbuff[0], (uint32_t)&spi_rxbuff[0]);
    while(cb_spi_get_current_transmission_mode() != EN_SPI_IDLE); //Wait SPI ready.

    // Print the contents of the transmit buffer
    app_spimaster_print("\r\n1-SPI SDMA Send: \r\n");
    for (uint16_t i = 0; i < length; i++)
    {
        app_spimaster_print("%d", spi_txbuff[i]);
        app_spimaster_print(" ");
    }
    app_spimaster_print("\r\n");

    // Print the contents of the receive buffer
    app_spimaster_print("2-SPI SDMA Receive:\r\n");
    for (uint16_t i = 0; i < length; i++)
    {
        app_spimaster_print("%d", spi_rxbuff[i]);
        app_spimaster_print(" ");
    }
    app_spimaster_print("\r\n");
    
    memset(&spi_rxbuff[0],0,sizeof(spi_rxbuff));
}

/**
 * @brief   Main function for initializing and setting up the SPI master peripheral.
 * 
 * This function initializes the UART for communication, sets up the Timer module with interrupts,
 * configures the Timer parameters, initializes the SPI master, and prints a power-on reset flag.
 */
void app_peripheral_spi_master_init(void)
{
    app_uart_init();                                           // Initialize UART: GPIO00-RX, GPIO01-TX, BPR=115200, Stop bit = 1, Bit Order = LSB, Parity = NONE
    app_spi_master_init();                                     // Initialize SPI
    app_spimaster_print("\n\nSPI Master Power-on reset\n");    // Power-on reset flag
}

/**
 * @brief   Main loop function for handling SPI master operations based on UART commands.
 * 
 * This function continuously checks for received UART data. If the receive flag is set, it processes the received data.
 * If the received data starts with the command "CCCC", it reinitializes the SPI master and performs an SDMA write-read operation.
 * Otherwise, it sends the received data in hexadecimal format over UART. After processing, it clears the receive buffer and resets the length.
 */
void app_peripheral_spi_master_loop(void)
{
    app_spi_master_sdma_write_read(EN_SPI_USE_CS0, 11);          // Perform SDMA write-read operation for 11 bytes
    cb_system_delay_in_ms(2000);    
}



