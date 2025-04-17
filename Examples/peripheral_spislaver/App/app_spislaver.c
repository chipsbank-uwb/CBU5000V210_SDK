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
 * 
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "app_spislaver.h"
#include "CB_spi.h"
#include "CB_iomux.h"
#include "CB_gpio.h"
#include "CB_scr.h"
#include "APP_common.h"
#include "string.h" //to use memmove API

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_SPISLAVE_UARTPRINT_ENABLE APP_TRUE

#if (APP_SPISLAVE_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_spislaver_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_spislaver_print(...)
#endif
//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_EXAMPLE_SPISLAVE_SIZE 10       //Defines the size of data to be received by the SPI slave (10 bytes).
#define DEF_SPI_DUMMY_SIZE 1               // Defines a workaround for SPI slave, requiring 1 dummy byte for clock synchronization.

uint8_t app_spi_salver_rxbuf[DEF_EXAMPLE_SPISLAVE_SIZE + DEF_SPI_DUMMY_SIZE] __attribute__((aligned(4))); //Buffer for storing data received from SPI master. Extra byte reserved for workaround.
uint8_t app_spi_sdma_slaver_mosibuf[DEF_EXAMPLE_SPISLAVE_SIZE + DEF_SPI_DUMMY_SIZE] __attribute__((aligned(4)))={
         1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11}; //Buffer for sending data via SPI SDMA mode. Extra byte reserved for workaround. 

static uint16_t spi_slaver_rxbuf_ptr = 0;  // Pointer to track the position in the receive buffer where the next byte of data will be stored
static uint8_t spi_slaver_rx_done=1;       // Receive done flag
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

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Example demonstrating SPI slave mode using FIFO for data reception and response.
 * 
 * This function configures the SPI slave and sets up a FIFO mode transaction, where the slave 
 * sends 10 bytes of data to the master. The master is expected to provide 1 extra dummy byte for
 * clock synchronization.
 */
void app_spi_slave_main_example_fifo(void) 
{
    /*Clear the app_spi_salver_rxbuf & pointer*/
    memset(&app_spi_salver_rxbuf,0,sizeof(app_spi_salver_rxbuf));
    spi_slaver_rxbuf_ptr = 0;

    /*SPI Configuration & Initialization*/
    app_spi_slave_init();

    /*SPI Slave FIFO Response 10bytes Data to SPI Master. (Required 1byte dummy clock, thus set length to 0x11*/
    /*SPI Master is required to send extra 1byte dummy clock to SPI Slave. (Required extra 1 dummy byte)*/
    cb_spi_slave_fifo_start((DEF_EXAMPLE_SPISLAVE_SIZE + DEF_SPI_DUMMY_SIZE),&app_spi_slave_fifo_mode_response_callback,&app_spi_slave_fifo_receive_complete_callback);
}

/**
 * @brief   Example demonstrating SPI slave mode using SDMA (Scatter-DMA) for data reception.
 * 
 * This function configures the SPI slave for SDMA mode, preparing a buffer for transmitting 
 * 10 bytes of data. The master is expected to send 1 extra dummy byte for clock synchronization.
 */
void app_spi_slave_main_example_sdma(void) 
{
    spi_slaver_rx_done = 0;
    /*Clear the app_spi_salver_rxbuf, app_spi_sdma_slaver_mosibuf & pointer*/
    memset(&app_spi_salver_rxbuf,0,sizeof(app_spi_salver_rxbuf));

    /*SPI Slave SDMA Receive 10bytes Data from SPI Master. (Required 1byte dummy clock, thus set length to 0x11
    *Received 11bytes data, but only 10data is valid data. (10Data + 1 extra dummy byte).
    *Handling the RxBuf data in app_spi_slave_sdma_receive_complete_callback memmove 1 byte forward.
    **/
    cb_spi_slave_sdma_start((DEF_EXAMPLE_SPISLAVE_SIZE + DEF_SPI_DUMMY_SIZE),(uint32_t)&app_spi_sdma_slaver_mosibuf[0], (uint32_t)&app_spi_salver_rxbuf[0],&app_spi_slave_sdma_receive_complete_callback);
}

/**
 * @brief   Initializes the SPI slave peripheral with required settings.
 * 
 * This function configures the IOMUX for SPI pins (CS, CLK, MISO, MOSI) and sets up the SPI 
 * peripheral parameters such as bit order, byte order, speed, and mode.
 */
void app_spi_slave_init(void)
{
    stGPIO_InitTypeDef spi_pins;
    cb_scr_gpio_module_on();

    stSPI_InitTypeDef hspi;
    cb_iomux_config(EN_IOMUX_GPIO_5, &(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS, (uint8_t)EN_IOMUX_GPIO_AF_SPIS_CS}); /* CS   */
    cb_iomux_config(EN_IOMUX_GPIO_6, &(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS, (uint8_t)EN_IOMUX_GPIO_AF_SPIS_CLK}); /* CLK  */ 
    cb_iomux_config(EN_IOMUX_GPIO_3, &(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS, (uint8_t)EN_IOMUX_GPIO_AF_SPIS_MISO}); /* MISO */ 
    cb_iomux_config(EN_IOMUX_GPIO_7, &(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS, (uint8_t)EN_IOMUX_GPIO_AF_SPIS_MOSI}); /* MOSI */ 

    // Initialize SPI configuration parameters
    hspi.BitOrder = EN_SPI_bitorder_msb_first;
    hspi.ByteOrder = EN_SPI_byteorder_byte0_first;
    hspi.Speed = EN_SPI512KHz;    // Set to 32MHz if required. Ensure a 500¦¸ pull-up resistor is connected to the MISO line for stable communication at high speeds.
    hspi.SpiMode = EN_SPI_mode0;  
    cb_spi_init(&hspi); 
}

/**
 * @brief   Deinitializes the SPI slave peripheral.
 * 
 * This function deinitializes the SPI peripheral, freeing any allocated resources.
 */
void app_spi_slave_deinit(void)
{
    cb_spi_deinit();
}

/**
 * @brief   SPI Slave FIFO mode callback for handling MOSI data reception and MISO response.
 * 
 * This callback function is called when a byte is received from the master in FIFO mode. The received 
 * byte is stored in the receive buffer, and the slave responds by sending the same byte back to the master.
 * 
 * @param   mosidata    The data received from the SPI master.
 * @return  The data to be sent back to the SPI master.
 */
uint8_t app_spi_slave_fifo_mode_response_callback(uint8_t mosidata)
{
    uint8_t misodata;
    app_spi_salver_rxbuf[spi_slaver_rxbuf_ptr++]=mosidata;
    if (spi_slaver_rxbuf_ptr >= (DEF_EXAMPLE_SPISLAVE_SIZE + DEF_SPI_DUMMY_SIZE))
    {
      spi_slaver_rxbuf_ptr = 0;
    }
    /*Example Response Data: Write MISO with same data of MOSI received.*/
    misodata = mosidata;

    return misodata;
}

/**
 * @brief   Completion callback for SPI Slave FIFO mode transaction.
 * 
 * This function is called when the SPI slave has finished receiving data in FIFO mode. 
 * If the transaction is successful, it prints the received data to the UART.
 * 
 * @param   completionstatus    The status of the SPI transaction (e.g., success or failure).
 */
void app_spi_slave_fifo_receive_complete_callback(uint8_t completionstatus)
{
    if (completionstatus != CB_PASS)
    {
      app_spislaver_print ("Failed\n");	
    }
    else
    {
      /*Rx Buffer PrintOut*/
      app_spislaver_print("[SPI SlaveFIFO]Received:");	    
      for (uint16_t printcnt=0;printcnt<(DEF_EXAMPLE_SPISLAVE_SIZE);printcnt++)
      {
        app_spislaver_print("0x%x,",app_spi_salver_rxbuf[printcnt]);	    
      }
      app_spislaver_print("\n");	  
    }
}

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
void app_spi_slave_sdma_receive_complete_callback(uint8_t completionstatus)
{
    spi_slaver_rx_done = 1;
    if (completionstatus != CB_PASS)
    {
      app_spislaver_print ("Failed\n");	
    }
    else
    { 
        
      // /*Modify the received Rxbuffer content by discarding 1st unwanted byte*/
      memmove(&app_spi_salver_rxbuf[0],&app_spi_salver_rxbuf[1],sizeof(app_spi_salver_rxbuf)-1);
        
      /*Tx Buffer PrintOut*/
      app_spislaver_print("\n[SPI SlaveSDMA]Send:\n");	
      for (uint16_t i=0;i<(DEF_EXAMPLE_SPISLAVE_SIZE + DEF_SPI_DUMMY_SIZE);i++)
      {
          app_spislaver_print("%d ",app_spi_sdma_slaver_mosibuf[i]);
      }  

      /*Rx Buffer PrintOut*/
      app_spislaver_print("\n[SPI SlaveSDMA]Received:\n");	    
      for (uint16_t printcnt=0;printcnt<(DEF_EXAMPLE_SPISLAVE_SIZE);printcnt++)
      {
        app_spislaver_print("%d ",app_spi_salver_rxbuf[printcnt]);	    
      }
      app_spislaver_print("\n");	    

      /* Clear the Rx buffer for the next transaction */
      memset(&app_spi_salver_rxbuf, 0x00, sizeof(app_spi_salver_rxbuf));
    }  
}

/**
 * @brief Main function for SPI slaver peripheral mode
 * 
 * This function initializes the UART and prints the power-on reset message for the SPI slaver.
 */
void app_peripheral_spi_slaver_init(void)
{
    app_uart_init();                                           // UART initialization: GPIO00-RX, GPIO01-TX, BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
    app_spi_slave_init();                                      //SPI Configuration & Initialization
    app_spislaver_print("\n\nSPI Slaver Power-on reset\n");    // Print power-on reset message
}

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
void app_peripheral_spi_slave_loop(void)
{   
    if(spi_slaver_rx_done)
    {
      app_spi_slave_main_example_sdma();           // Handle the received data using the example function
    }
}



