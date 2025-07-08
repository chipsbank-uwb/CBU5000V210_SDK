/**
 * @file    CMD_uart.c
 * @brief   [SYSTEM] UART Commander Application Source File
 * @details This file contains the implementation of functions related to UART command handling.
 *          It defines functions for processing UART receive buffer, command handling, and associated callback functions.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "CB_iomux.h"
#include "CB_scr.h"
#include "CB_system.h"
#include "CB_Uart.h"
#include "cmd_parser_uart.h"
#include "dfu_handler.h"
#if __has_include(<APP_CompileOption.h>)
#include "APP_CompileOption.h"
#endif
//-------------------------------
// DEFINE SECTION
//-------------------------------
#ifndef APP_FREERTOS_ENABLE
#define APP_FREERTOS_ENABLE APP_FALSE 
#endif
#if (APP_FREERTOS_ENABLE == APP_TRUE)
#include "FreeRTOS.h"
#endif
#ifndef APP_DFU_LOG_ENABLE
#define APP_DFU_LOG_ENABLE APP_TRUE
#endif

#if (APP_DFU_LOG_ENABLE == APP_TRUE)
#define LOG(...) app_uart_printf(__VA_ARGS__)
#else
#define LOG(...)
#endif
#define UART_RX_BUFFER_SIZE 0x100
#define CMD_RX_BUF_SIZE 256
#define CMD_TX_BUF_SIZE 32
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief Show the current command state.
 */
enum
{
  CMD_IDLE,          /*  Idle            0 */
  CMD_STARTED,       /*  after get started         1 */
  CMD_STOPED,         /*  after get stoped           2 */
};
typedef enum
{
  EN_UartRxWAITING = 0,   /**< Waiting for the start marker (0x5A) */
  EN_UartRxMARKER_DONE ,  /**< Start marker received and validated */
  EN_UartRxHEADER_DONE,   /**< Command header received */
  EN_UartRxCHECKSUM_DONE, /**< Checksum validated */
}en_cmd_state;


//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void cmd_parser_uart_configer(uint8_t tx_pin, uint8_t rx_pin);
static void cmd_parser_uart_send_port(uint8_t *prtData, uint16_t len);
static uint8_t uart_txbuf[UART_RX_BUFFER_SIZE] __attribute__((section("SPECIFIC_UART_TX_SDMA_RAM")));
static uint8_t uart_rxbuf[UART_RX_BUFFER_SIZE] __attribute__((section("SPECIFIC_UART_RX_SDMA_RAM")));
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
en_cmd_state cmd_parser_uart_state = EN_UartRxWAITING;
static uint8_t cmd_ready_flag = APP_FALSE;

static stUartConfig uart_config;
static uint8_t cmd_parser_uart_rxbuf[CMD_RX_BUF_SIZE];


//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Initializes the UART module for communication.
 * @details This function turns on the UART0 module, configures the I/O multiplexer for UART0 RX and TX pins,
 *          enables the UART0 interrupt, and initializes UART0 with SDMA mode.
 */
static void cmd_parser_uart_configer(uint8_t tx_pin, uint8_t rx_pin)
{
  
    cb_scr_uwb_module_on();
    cb_scr_uart0_module_off();
    cb_scr_uart0_module_on();

 
    cb_iomux_config(rx_pin,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART0_RXD});
    cb_iomux_config(tx_pin,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART0_TXD});

     NVIC_EnableIRQ(UART0_IRQn); // Enable UART0 interrupt   

    uart_config.uartChannel        = EN_UART_0;
    uart_config.uartMode           = EN_UART_MODE_SDMA;
    uart_config.uartBaudrate       = EN_UART_BAUDRATE_921600;
    uart_config.uartRxMaxBytes   = 200;
    uart_config.uartRxBufWrap    = EN_UART_RXBUF_WRAP_DISABLE;
    uart_config.uartStopBits     = EN_UART_STOP_BITS_1;
    uart_config.uartBitOrder     = EN_UART_BIT_ORDER_LSB_FIRST;
    uart_config.uartParity       = EN_UART_PARITY_NONE;
    uart_config.uartFlowControl  = EN_UART_FLOW_CONTROL_DISABLE;
    uart_config.uartInt          = 0;

    /* SDMA Buffer address. */
    uart_config.TXbuffer        = (uint32_t)uart_txbuf; // Set TX buffer address
    uart_config.RXbuffer        = (uint32_t)uart_rxbuf; // Set RX buffer address

    /* Callback IRQ register omitted in current SDK to improve IRQ processing time */
    // app_irq_deregister_irqcallback(EN_IRQENTRY_UART_0_RXB_FULL_APP_IRQ, app_uart_0_rxb_full_callback); // Register RX buffer full callback (omitted for performance)
      
    cb_uart_init(uart_config); // Initialize UART with the configured settings  
}

/**
 * @brief Sends data through the UART port.
 *
 * This function sends data through the UART port. It ensures that the UART
 * transmitter is available before sending the data. If FreeRTOS is enabled,
 * it enters a critical section to prevent context switching during the
 * transmission.
 *
 * @param prtData Pointer to the data to be sent.
 * @param len Length of the data to be sent.
 */
static void cmd_parser_uart_send_port(uint8_t *prtData, uint16_t len)
{
    #if (APP_FREERTOS_ENABLE == APP_TRUE)
    vPortEnterCritical();
    #endif
    // make sure TX is available
    while ((cb_uart_is_tx_busy(uart_config)));

    cb_uart_transmit(uart_config, (uint8_t *) prtData, (uint16_t) len);
    while ((cb_uart_is_tx_busy(uart_config)));
    #if (APP_FREERTOS_ENABLE == APP_TRUE)
    vPortExitCritical();
    #endif
}
/**
 * @brief Deinitializes the UART driver.
 *
 * This function turns off the UART0 module, effectively deinitializing
 * the UART driver and freeing any associated resources.
 */
void cmd_parser_uart_deinit(void)
{
   cb_scr_uart0_module_off();
}

/**
 * @brief Polls for valid commands in the UART receive buffer using a state machine.
 *        The function waits for a specific marker byte, reads command headers and data, and validates the checksum.
 */
uint8_t cmd_parser_uart_pooling_cmd(void)
{
    uint16_t received_len = 0;
    uint16_t expected_len = 0;
    uint16_t checksum_pos = 0;

    cmd_parser_uart_state = EN_UartRxWAITING;    
    cmd_ready_flag = APP_FALSE;
    cmd_parser_uart_state = EN_UartRxWAITING;
    expected_len = DEF_RXMARKER_SIZE;

    while (cmd_ready_flag != APP_TRUE)
    {
        received_len = cb_uart_check_num_received_bytes(EN_UART_0);
       
        if (received_len >= expected_len )
        {
            cb_uart_get_rx_buffer(EN_UART_0,&cmd_parser_uart_rxbuf[0],expected_len); //updating the Buffer
            switch (cmd_parser_uart_state)
            {
                case EN_UartRxWAITING:
                    if (cmd_parser_uart_rxbuf[DEF_RXMARKER_POS] == DEF_RXMARKER_VAL)
                    {
                        //Marker Matched
                        expected_len = DEF_HEADER_SIZE; //ReadUntilLength
                        cmd_parser_uart_state = EN_UartRxMARKER_DONE;
                    }
                    else
                    {
                        //Marker Mismtached
                        cb_uart_rx_restart(EN_UART_0);
                    }
                break;
                case EN_UartRxMARKER_DONE:
                    if (cmd_parser_uart_rxbuf[DEF_RXMARKER_POS] == DEF_RXMARKER_VAL)
                    { 
                        {
                            checksum_pos = DEF_DL_POS+DEF_DL_SIZE;
                            checksum_pos += cmd_parser_uart_rxbuf[DEF_DL_POS];
                            //Marker Matched
                            expected_len = (checksum_pos + DEF_CHECKSUM_SIZE); //Read Based on DataLength
                            cmd_parser_uart_state = EN_UartRxHEADER_DONE;
                            break;
                        }
                    }

                    //Marker Mismtached
                    cb_uart_rx_restart(EN_UART_0);
                break;

                case EN_UartRxHEADER_DONE:
                    cb_uart_rx_stop(EN_UART_0); //Terminate the UART Rx.
                    checksum_pos = DEF_DL_POS+DEF_DL_SIZE;
                    checksum_pos += cmd_parser_uart_rxbuf[DEF_DL_POS];
                    uint8_t checksun = 0;
                    for (uint8_t i = 1; i < checksum_pos; i++)
                    {
                        checksun += cmd_parser_uart_rxbuf[i];
                    }
                    if(checksun == cmd_parser_uart_rxbuf[checksum_pos])
                    {
                        cmd_ready_flag = APP_TRUE;
                    }
                    else{
                        expected_len = DEF_RXMARKER_SIZE;
                        cmd_parser_uart_state = EN_UartRxWAITING;
                        cb_uart_rx_restart(EN_UART_0);
                    }
                break;
                case EN_UartRxCHECKSUM_DONE:
                default: //unexpected behaviour
                   cb_uart_rx_restart(EN_UART_0);
                break;
            }
        }
        else if(received_len == 0)
        {
            return cmd_ready_flag;
        }
    }
    return cmd_ready_flag;
}

/**
 * @brief Process UART receive buffer.
 * 
 * This function processes the UART receive buffer by separating the command
 * and arguments, converting argument strings to integers, and then executing
 * the corresponding command handler function.
 * 
 * @param ptr_buffer Pointer to the UART receive buffer.
 * @param len             The data len of buffer.
 */
void cmd_parser_uart_process_buffer(uint8_t *ptr_buffer, uint16_t len, halder_polling_t halder_func)
{
    uint16_t command;
    uint8_t  cmdType;
    uint16_t  lenData;
    uint8_t  *prtData;

    if(len > DEF_HEADER_SIZE)
    {
        command = (uint16_t)(ptr_buffer[DEF_CMD_POS]<<8 | ptr_buffer[DEF_CMD_POS+1]);
        cmdType = ptr_buffer[DEF_RESP_POS]; //Req:0x00 | Resp:0x01
        lenData = cmd_parser_uart_rxbuf[DEF_DL_POS];
        prtData = &ptr_buffer[DEF_DATA_POS];
        if(cmdType != 0x00)
        {
            LOG("command type error\n");
            return; //command Type error
        }
        else
        { 
        
           LOG("right command\r\n");
        }
        uint8_t cb_done = APP_FALSE;
        //OTA command
        cb_done = halder_func(command,prtData,lenData, cmd_parser_uart_responder);
    }
}
/**
 * @brief Command responder.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void cmd_parser_uart_responder(uint16_t command, uint8_t *buf, uint8_t len)
{
    static uint8_t cmd_parser_uart_txbuf[CMD_TX_BUF_SIZE];
    static uint8_t seq = 0;
    //memset(cmd_parser_uart_txbuf,0,CMD_TX_BUF_SIZE);
    cmd_parser_uart_txbuf[DEF_RXMARKER_POS] = DEF_RXMARKER_VAL;
    cmd_parser_uart_txbuf[DEF_DL_POS] = len;
    cmd_parser_uart_txbuf[DEF_CMD_POS] = command>>8;
    cmd_parser_uart_txbuf[DEF_CMD_POS+1] = command&0xff;
    cmd_parser_uart_txbuf[DEF_RESP_POS] = 0x01; //Req:0x00 | Resp:0x01
    if(len > 0)
    {
            memcpy(&cmd_parser_uart_txbuf[DEF_DATA_POS],buf,len);
    }
    uint8_t checksun = 0;
    for (uint8_t i = 1; i < (len+DEF_HEADER_SIZE); i++)
    {
        checksun += cmd_parser_uart_txbuf[i];
    }
    len = len+DEF_HEADER_SIZE;
    cmd_parser_uart_txbuf[len] = checksun;
    len++;
    cmd_parser_uart_send_port(cmd_parser_uart_txbuf,len);
}

uint16_t cmd_parser_uart_received_length(void)
{
    uint16_t received_length = cb_uart_check_num_received_bytes(EN_UART_0); 
    return received_length;
}
uint8_t* cmd_parser_uart_received_buffer(void)
{
    return &cmd_parser_uart_rxbuf[0];
}

void cmd_parser_uart_rx_restart(void)
{
    cb_uart_rx_restart(EN_UART_0);
}
/**
 * @brief Initializes the UART driver.
 *
 * This function configures the UART driver for the specified role and
 * initializes the DFU (Device Firmware Update) handler with the UART
 * configuration. It sets up the responder and deinitialization functions
 * for the DFU handler.
 */
void cmd_parser_uart_init(void)
{
    cmd_parser_uart_configer(EVK_UART_TX_PIN, EVK_UART_RX_PIN);
}


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
