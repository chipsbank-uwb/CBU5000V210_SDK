/**
 * @file    cmd_uart.h
 * @brief   
 * @details 
 *          
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __cmd_parser_uart_H
#define __cmd_parser_uart_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_RXMARKER_VAL        0x5A
/*SizeInByte*/
#define DEF_RXMARKER_SIZE       1
#define DEF_CMD_SIZE            2
#define DEF_RESP_SIZE           1
#define DEF_DL_SIZE             1	
#define DEF_HEADER_SIZE         (DEF_RXMARKER_SIZE+DEF_CMD_SIZE+DEF_RESP_SIZE+DEF_DL_SIZE)
#define DEF_CHECKSUM_SIZE       1	

/*Position*/
#define DEF_RXMARKER_POS	    0
#define DEF_CMD_POS             (DEF_RXMARKER_POS + DEF_RXMARKER_SIZE)
#define DEF_RESP_POS            (DEF_CMD_POS + DEF_CMD_SIZE)
#define DEF_DL_POS              (DEF_RESP_POS + DEF_RESP_SIZE)	
#define DEF_DATA_POS	        (DEF_DL_POS + DEF_DL_SIZE)


#define OFF_CMD (0u)
#define PERIODIC_RX_CMD (1u)
#define PERIODIC_TX_CMD (2u)

#ifndef APP_TRUE
#define APP_TRUE   1
#endif
#ifndef APP_FALSE
#define APP_FALSE  0
#endif


#define EVK_UART_RX_PIN  EN_IOMUX_GPIO_0
#define EVK_UART_TX_PIN  EN_IOMUX_GPIO_1

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef void (*cmdhandler_t)(uint16_t, uint8_t*, uint8_t);
typedef uint8_t (*halder_polling_t)(uint16_t , uint8_t *, uint16_t , cmdhandler_t );
void app_uart_printf(const char *format, ...);
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

void cmd_parser_uart_init(void);
uint8_t cmd_parser_uart_pooling_cmd(void);
void cmd_parser_uart_cmd_ready_flag(void);
uint16_t cmd_parser_uart_received_length(void);
uint8_t* cmd_parser_uart_received_buffer(void);
void cmd_parser_uart_rx_restart(void);
void cmd_parser_uart_process_buffer(uint8_t *ptr_buffer, uint16_t len, halder_polling_t halder_func);

void cmd_parser_uart_deinit(void);
void cmd_parser_uart_responder(uint16_t command, uint8_t *buf, uint8_t len);
#endif /*__APP_COMMANDER_H*/
