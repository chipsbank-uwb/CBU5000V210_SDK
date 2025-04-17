/**
 * @file    AppCmd.c
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

#include "dfu_uart.h"
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
#define APP_DFU_LOG_ENABLE APP_FALSE 
#endif

#if (APP_DFU_LOG_ENABLE == APP_TRUE)
#define LOG(...) app_uart_printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#define DUF_RX_BUF_SIZE 256
#define DUF_TX_BUF_SIZE 32

#define UART_RX_BUFFER_SIZE 0x100
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief DMA mode transmit and receive buffers.
 * 
 * These buffers are used for DMA mode operations. The address of these buffers
 * needs to be written into the UART configuration structure. The buffers must be
 * aligned to 4 bytes to ensure proper memory access.
 * 
 * @note The buffer size should be set to a value greater than the maximum expected
 *       size of the data to be received or sent to avoid memory overflow.
 * 
 * @note You can also define custom buffer addresses and write them into the structure
 *       for use in subsequent receive operations.
 */
uint8_t uart_txbuf[UART_RX_BUFFER_SIZE] __attribute__((section("SPECIFIC_UART_TX_SDMA_RAM")));
uint8_t uart_rxbuf[UART_RX_BUFFER_SIZE] __attribute__((section("SPECIFIC_UART_RX_SDMA_RAM")));

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
}en_CmdState;


//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void dfu_uart_configer(uint8_t role);
static void dfu_uart_send_port(uint8_t *prtData, uint16_t len);
static void dfu_uart_deinit(void);
static void dfu_uart_pooling_cmd(void);
static void dfu_uart_process_buffer(uint8_t *ptr_buffer, uint16_t len);
static void dfu_uart_responder(uint16_t command, uint8_t *buf, uint8_t len);
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
en_CmdState dfu_uart_state = EN_UartRxWAITING;
uint8_t dfu_uart_cmd_ready_flag = APP_FALSE;

static stUartConfig uartConfig;
static uint8_t dfu_uart_rxbuf[DUF_RX_BUF_SIZE];


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
static void dfu_uart_configer(uint8_t role)
{
    cb_scr_uwb_module_on();
    cb_scr_uart0_module_off();
    cb_scr_uart0_module_on();

    if(role == UART_ROLE_67)
    {
        cb_iomux_config(EN_IOMUX_GPIO_7,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART0_RXD});
        cb_iomux_config(EN_IOMUX_GPIO_6,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART0_TXD});
    }
    else //if(role == UART_ROLE_EVK)  //EVK
    {
        cb_iomux_config(EN_IOMUX_GPIO_0,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART0_RXD});
        cb_iomux_config(EN_IOMUX_GPIO_1,&(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,(uint8_t)EN_IOMUX_GPIO_AF_UART0_TXD});
    }

    uartConfig.uartChannel        = EN_UART_0;
    uartConfig.uartMode           = EN_UART_MODE_SDMA;
    uartConfig.uartBaudrate       = EN_UART_BAUDRATE_921600;
    uartConfig.uartRxMaxBytes   = 200;
    uartConfig.uartRxBufWrap    = EN_UART_RXBUF_WRAP_DISABLE;
    uartConfig.uartStopBits     = EN_UART_STOP_BITS_1;
    uartConfig.uartBitOrder     = EN_UART_BIT_ORDER_LSB_FIRST;
    uartConfig.uartParity       = EN_UART_PARITY_NONE;
    uartConfig.uartFlowControl  = EN_UART_FLOW_CONTROL_DISABLE;
    uartConfig.uartInt          = 0;

        /*SDMA Buffer address.*/
    uartConfig.TXbuffer           = (uint32_t)uart_txbuf;               // Set transmit buffer address
    uartConfig.RXbuffer           = (uint32_t)uart_rxbuf;               // Set receive buffer address
    cb_uart_init(uartConfig);  

}

static void dfu_uart_send_port(uint8_t *prtData, uint16_t len)
{
    #if (APP_FREERTOS_ENABLE == APP_TRUE)
    vPortEnterCritical();
    #endif
    // make sure TX is available
    while ((cb_uart_is_tx_busy(uartConfig)));

    cb_uart_transmit(uartConfig, (uint8_t *) prtData, (uint16_t) len);
    while ((cb_uart_is_tx_busy(uartConfig)));
    #if (APP_FREERTOS_ENABLE == APP_TRUE)
    vPortExitCritical();
    #endif
}

static void dfu_uart_deinit(void)
{
    cb_scr_uart0_module_off();
}


/**
 * @brief Polls for valid commands in the UART receive buffer using a state machine.
 *        The function waits for a specific marker byte, reads command headers and data, and validates the checksum.
 */
static void dfu_uart_pooling_cmd(void)
{
    uint16_t received_len = 0;
    uint16_t expected_len = 0;
    uint16_t checksum_pos = 0;

    dfu_uart_state = EN_UartRxWAITING;    
    dfu_uart_cmd_ready_flag = APP_FALSE;
    dfu_uart_state = EN_UartRxWAITING;
    expected_len = DEF_RXMARKER_SIZE;

    while (dfu_uart_cmd_ready_flag != APP_TRUE)
    {
        received_len = cb_uart_check_num_received_bytes(EN_UART_0);
        if (received_len >= expected_len )
        {
            cb_uart_get_rx_buffer(EN_UART_0,&dfu_uart_rxbuf[0],expected_len); //updating the Buffer
            switch (dfu_uart_state)
            {
                case EN_UartRxWAITING:
                    if (dfu_uart_rxbuf[DEF_RXMARKER_POS] == DEF_RXMARKER_VAL)
                    {
                        //Marker Matched
                        expected_len = DEF_HEADER_SIZE; //ReadUntilLength
                        dfu_uart_state = EN_UartRxMARKER_DONE;
                    }
                    else
                    {
                        //Marker Mismtached
                        cb_uart_rx_restart(EN_UART_0);
                    }
                break;
                case EN_UartRxMARKER_DONE:
                    if (dfu_uart_rxbuf[DEF_RXMARKER_POS] == DEF_RXMARKER_VAL)
                    { 
                        {
                            checksum_pos = DEF_DL_POS+DEF_DL_SIZE;
                            checksum_pos += dfu_uart_rxbuf[DEF_DL_POS];
                            //Marker Matched
                            expected_len = (checksum_pos + DEF_CHECKSUM_SIZE); //Read Based on DataLength
                            dfu_uart_state = EN_UartRxHEADER_DONE;
                            break;
                        }
                    }

                    //Marker Mismtached
                    cb_uart_rx_restart(EN_UART_0);
                break;

                case EN_UartRxHEADER_DONE:
                    cb_uart_rx_stop(EN_UART_0); //Terminate the UART Rx.
                    checksum_pos = DEF_DL_POS+DEF_DL_SIZE;
                    checksum_pos += dfu_uart_rxbuf[DEF_DL_POS];
                    uint8_t checksun = 0;
                    for (uint8_t i = 1; i < checksum_pos; i++)
                    {
                        checksun += dfu_uart_rxbuf[i];
                    }
                    if(checksun == dfu_uart_rxbuf[checksum_pos])
                    {
                        dfu_uart_cmd_ready_flag = APP_TRUE;
                    }
                    else{
                        expected_len = DEF_RXMARKER_SIZE;
                        dfu_uart_state = EN_UartRxWAITING;
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
            return;
        }
    }
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
static void dfu_uart_process_buffer(uint8_t *ptr_buffer, uint16_t len)
{
    uint16_t command;
    uint8_t  cmdType;
    uint16_t  lenData;
    uint8_t  *prtData;
    uint8_t checksun = 0;

    if(len > DEF_HEADER_SIZE)
    {
        command = (uint16_t)(ptr_buffer[DEF_CMD_POS]<<8 | ptr_buffer[DEF_CMD_POS+1]);
        cmdType = ptr_buffer[DEF_RESP_POS]; //Req:0x00 | Resp:0x01
        lenData = dfu_uart_rxbuf[DEF_DL_POS];
        prtData = &ptr_buffer[DEF_DATA_POS];
        if(cmdType != 0x00)
        {
            LOG("command type error\n");
            return; //command Type error
        }
        uint8_t cb_done = APP_FALSE;
        //OTA command
        cb_done = dfu_halder_polling(command,prtData,lenData, dfu_uart_responder);
    }
}
/**
 * @brief Command responder.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
static void dfu_uart_responder(uint16_t command, uint8_t *buf, uint8_t len)
{
    static uint8_t dfu_uart_txbuf[DUF_TX_BUF_SIZE];
    static uint8_t seq = 0;
    //memset(dfu_uart_txbuf,0,DUF_TX_BUF_SIZE);
    dfu_uart_txbuf[DEF_RXMARKER_POS] = DEF_RXMARKER_VAL;
    dfu_uart_txbuf[DEF_DL_POS] = len;
    dfu_uart_txbuf[DEF_CMD_POS] = command>>8;
    dfu_uart_txbuf[DEF_CMD_POS+1] = command&0xff;
    dfu_uart_txbuf[DEF_RESP_POS] = 0x01; //Req:0x00 | Resp:0x01
    if(len > 0)
    {
            memcpy(&dfu_uart_txbuf[DEF_DATA_POS],buf,len);
    }
    uint8_t checksun = 0;
    for (uint8_t i = 1; i < (len+DEF_HEADER_SIZE); i++)
    {
        checksun += dfu_uart_txbuf[i];
    }
    len = len+DEF_HEADER_SIZE;
    dfu_uart_txbuf[len] = checksun;
    len++;
    dfu_uart_send_port(dfu_uart_txbuf,len);
}


void dfu_uart_polling(void)
{
    dfu_uart_pooling_cmd();
    if(dfu_uart_cmd_ready_flag)
    {
        uint16_t received_length = cb_uart_check_num_received_bytes(EN_UART_0); 
        //command ready to send
        dfu_uart_process_buffer(&dfu_uart_rxbuf[0],received_length);
        memset(dfu_uart_rxbuf, 0, sizeof(dfu_uart_rxbuf));// Reset buffer 
        cb_uart_rx_restart(EN_UART_0);
    }

}

/**
 * @brief init .
 * 
 */
void dfu_uart_init(void)
{
    dfu_uart_configer(UART_ROLE_EVK);
    dfu_cfg_t uart_cfg = {0};
    uart_cfg.responder = dfu_uart_responder;
    uart_cfg.reinit = dfu_uart_deinit;
    dfu_halder_init(&uart_cfg);
}


