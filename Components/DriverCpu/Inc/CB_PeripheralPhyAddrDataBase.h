#ifndef __CB_NONLIB_PERIPHERAL_PHY_ADDRESS_DATABASE_H
#define __CB_NONLIB_PERIPHERAL_PHY_ADDRESS_DATABASE_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

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

//-------------------------------
// DEFINE SECTION
//-------------------------------

// Section: UART __________________________________________________________________
#define UART0_BASE_ADDR  (0x40025000)
#define UART1_BASE_ADDR  (0x40026000) 

/*****************  Bit definition for uart_en register     *******************/
#define UART_EN_TXEN_POS                (1U)
#define UART_EN_TXEN_MSK                (0x1UL << UART_EN_TXEN_POS)
#define UART_EN_TXEN                    UART_EN_TXEN_MSK
#define UART_EN_RXEN_POS                (0U)
#define UART_EN_RXEN_MSK                (0x1UL << UART_EN_RXEN_POS)
#define UART_EN_RXEN                    UART_EN_RXEN_MSK

/*****************  Bit definition for uart_txctrl register *******************/
#define UART_TXCTRL_STOP_POS            (1U)
#define UART_TXCTRL_STOP_MSK            (0x1UL << UART_TXCTRL_STOP_POS)
#define UART_TXCTRL_STOP                UART_TXCTRL_STOP_MSK
#define UART_TXCTRL_START_POS           (0U)
#define UART_TXCTRL_START_MSK           (0x1UL << UART_TXCTRL_START_POS)
#define UART_TXCTRL_START               UART_TXCTRL_START_MSK

/*****************  Bit definition for uart_rxctrl register *******************/
#define UART_RXCTRL_STOP_POS            (1U)
#define UART_RXCTRL_STOP_MSK            (0x1UL << UART_RXCTRL_STOP_POS)
#define UART_RXCTRL_STOP                UART_RXCTRL_STOP_MSK
#define UART_RXCTRL_START_POS           (0U)
#define UART_RXCTRL_START_MSK           (0x1UL << UART_RXCTRL_START_POS)
#define UART_RXCTRL_START               UART_RXCTRL_START_MSK

/*****************  Bit definition for uart_int_en register *******************/
#define UART_INT_EN_RXB_WR_ERR_POS      (15U)
#define UART_INT_EN_RXB_WR_ERR_MSK      (0x1UL << UART_INT_EN_RXB_WR_ERR_POS)
#define UART_INT_EN_RXB_WR_ERR          UART_INT_EN_RXB_WR_ERR_MSK
#define UART_INT_EN_TXB_RD_ERR_POS      (14U)
#define UART_INT_EN_TXB_RD_ERR_MSK      (0x1UL << UART_INT_EN_TXB_RD_ERR_POS)
#define UART_INT_EN_TXB_RD_ERR          UART_INT_EN_TXB_RD_ERR_MSK
#define UART_INT_EN_BREAK_ERR_POS       (13U)
#define UART_INT_EN_BREAK_ERR_MSK       (0x1UL << UART_INT_EN_BREAK_ERR_POS)
#define UART_INT_EN_BREAK_ERR           UART_INT_EN_BREAK_ERR_MSK
#define UART_INT_EN_FRAME_ERR_POS       (12U)
#define UART_INT_EN_FRAME_ERR_MSK       (0x1UL << UART_INT_EN_FRAME_ERR_POS)
#define UART_INT_EN_FRAME_ERR           UART_INT_EN_FRAME_ERR_MSK
#define UART_INT_EN_PARITY_ERR_POS      (11U)
#define UART_INT_EN_PARITY_ERR_MSK      (0x1UL << UART_INT_EN_PARITY_ERR_POS)
#define UART_INT_EN_PARITY_ERR          UART_INT_EN_PARITY_ERR_MSK
#define UART_INT_EN_RXFIFO_OVF_ERR_POS  (10U)
#define UART_INT_EN_RXFIFO_OVF_ERR_MSK  (0x1UL << UART_INT_EN_RXFIFO_OVF_ERR_POS)
#define UART_INT_EN_RXFIFO_OVF_ERR      UART_INT_EN_RXFIFO_OVF_ERR_MSK
#define UART_INT_EN_CTS_POS             (9U)
#define UART_INT_EN_CTS_MSK             (0x1UL << UART_INT_EN_CTS_POS)
#define UART_INT_EN_CTS                 UART_INT_EN_CTS_MSK
#define UART_INT_EN_RXB_FULL_POS        (8U)
#define UART_INT_EN_RXB_FULL_MSK        (0x1UL << UART_INT_EN_RXB_FULL_POS)
#define UART_INT_EN_RXB_FULL            UART_INT_EN_RXB_FULL_MSK
#define UART_INT_EN_RXFIFO_FULL_POS     (7U)
#define UART_INT_EN_RXFIFO_FULL_MSK     (0x1UL << UART_INT_EN_RXFIFO_FULL_POS)
#define UART_INT_EN_RXFIFO_FULL         UART_INT_EN_RXFIFO_FULL_MSK
#define UART_INT_EN_RXFIFO_EMPTY_POS    (6U)
#define UART_INT_EN_RXFIFO_EMPTY_MSK    (0x1UL << UART_INT_EN_RXFIFO_EMPTY_POS)
#define UART_INT_EN_RXFIFO_EMPTY        UART_INT_EN_RXFIFO_EMPTY
#define UART_INT_EN_RXFIFO_READY_POS    (5U)
#define UART_INT_EN_RXFIFO_READY_MSK    (0x1UL << UART_INT_EN_RXFIFO_READY_POS)
#define UART_INT_EN_RXFIFO_READY        UART_INT_EN_RXFIFO_READY_MSK
#define UART_INT_EN_RXD_READY_POS       (4U)
#define UART_INT_EN_RXD_READY_MSK       (0x1UL << UART_INT_EN_RXD_READY_POS)
#define UART_INT_EN_RXD_READY           UART_INT_EN_RXD_READY_MSK
#define UART_INT_EN_TXB_EMPTY_POS       (3U)
#define UART_INT_EN_TXB_EMPTY_MSK       (0x1UL << UART_INT_EN_TXB_EMPTY_POS)
#define UART_INT_EN_TXB_EMPTY           UART_INT_EN_TXB_EMPTY_MSK
#define UART_INT_EN_TXFIFO_FULL_POS     (2U)
#define UART_INT_EN_TXFIFO_FULL_MSK     (0x1UL << UART_INT_EN_TXFIFO_FULL_POS)
#define UART_INT_EN_TXFIFO_FULL         UART_INT_EN_TXFIFO_FULL_MSK
#define UART_INT_EN_TXFIFO_EMPTY_POS    (1U)
#define UART_INT_EN_TXFIFO_EMPTY_MSK    (0x1UL << UART_INT_EN_TXFIFO_EMPTY_POS)
#define UART_INT_EN_TXFIFO_EMPTY        UART_INT_EN_TXFIFO_EMPTY_MSK
#define UART_INT_EN_TXD_READY_POS       (0U)
#define UART_INT_EN_TXD_READY_MSK       (0x1UL << UART_INT_EN_TXD_READY_POS)
#define UART_INT_EN_TXD_READY           UART_INT_EN_TXD_READY_MSK

/*****************  Bit definition for uart_int_clr register  *******************/
#define UART_INT_CLR_INT_CLEAR_POS      (0U)
#define UART_INT_CLR_INT_CLEAR_MSK      (0x1UL << UART_INT_CLR_INT_CLEAR_POS)
#define UART_INT_CLR_INT_CLEAR          UART_INT_CLR_INT_CLEAR_MSK

/*****************  Bit definition for uart_cfg register    *******************/
#define UART_CFG_BAUDRATE_POS           (12U)
#define UART_CFG_BAUDRATE_MSK           (0xFFFFFUL << UART_CFG_BAUDRATE_POS)
#define UART_CFG_BAUDRATE(x)            ((x << UART_CFG_BAUDRATE_POS) & UART_CFG_BAUDRATE_MSK)  // input x is 64Mhz/baudrate
#define UART_CFG_RXBUF_WRAP_POS         (8U)
#define UART_CFG_RXBUF_WRAP_MSK         (0x1UL << UART_CFG_RXBUF_WRAP_POS)
#define UART_CFG_RXBUF_WRAP             UART_CFG_RXBUF_WRAP_MSK
#define UART_CFG_TRXBUF_EN_POS          (7U)
#define UART_CFG_TRXBUF_EN_MSK          (0x1UL << UART_CFG_TRXBUF_EN_POS)
#define UART_CFG_TRXBUF_EN              UART_CFG_TRXBUF_EN_MSK
#define UART_CFG_STOP_BITS_POS          (5U)
#define UART_CFG_STOP_BITS_MSK          (0x3UL << UART_CFG_STOP_BITS_POS)
#define UART_CFG_STOP_BITS(x)           ((x << UART_CFG_STOP_BITS_POS) & UART_CFG_STOP_BITS_MSK)  // x=0 - 1 stop bit; x=1 - 1.5 stop bits; x=2 - 2 stop bits; x=3 - 3 stop bits
#define UART_CFG_STOP_BITS_1_STOP_BITS  (UART_CFG_STOP_BITS(0UL))
#define UART_CFG_STOP_BITS_1_5_STOP_BITS  (UART_CFG_STOP_BITS(1UL))
#define UART_CFG_STOP_BITS_2_STOP_BITS  (UART_CFG_STOP_BITS(2UL))
#define UART_CFG_STOP_BITS_3_STOP_BITS  (UART_CFG_STOP_BITS(3UL))
#define UART_CFG_BYTE_ORDER_POS         (4U)
#define UART_CFG_BYTE_ORDER_MSK         (0x1UL << UART_CFG_BYTE_ORDER_POS)
#define UART_CFG_BYTE_ORDER                       // *unused - reserved*             
#define UART_CFG_BIT_ORDER_POS          (3U)
#define UART_CFG_BIT_ORDER_MSK          (0x1UL << UART_CFG_BIT_ORDER_POS)
#define UART_CFG_BIT_ORDER_LSB_FRIST    ((0x0UL << UART_CFG_BIT_ORDER_POS) & UART_CFG_BIT_ORDER_MSK) // may be unutilised
#define UART_CFG_BIT_ORDER_MSB_FIRST    ((0x1UL << UART_CFG_BIT_ORDER_POS) & UART_CFG_BIT_ORDER_MSK) // may be unutilised
#define UART_CFG_PARITY_POS             (1U)
#define UART_CFG_PARITY_MSK             (0x3UL << UART_CFG_PARITY_POS)
#define UART_CFG_PARITY(x)              ((x << UART_CFG_PARITY_POS) & UART_CFG_PARITY_MSK) // x=0 - no parity; x=1 - even parity; x=2 - odd parity
#define UART_CFG_PARITY_NO_PARITY       (UART_CFG_PARITY(0UL))
#define UART_CFG_PARITY_EVEN_PARITY     (UART_CFG_PARITY(1UL))
#define UART_CFG_PARITY_ODD_PARITY      (UART_CFG_PARITY(2UL))
#define UART_CFG_FLOW_POS               (0U)
#define UART_CFG_FLOW_MSK               (0x1UL << UART_CFG_FLOW_POS)
#define UART_CFG_FLOW                   UART_CFG_FLOW_MSK

/*****************  Bit definition for uart_TRXD register *******************/
#define UART_TRXD_READ_BYTES_POS         (0U)
#define UART_TRXD_READ_BYTES_MSK         (0x3UL << UART_TRXD_READ_BYTES_POS)
#define UART_TRXD_WRITE_BYTES_POS        (2U)
#define UART_TRXD_WRITE_BYTES_MSK        (0x3UL << UART_TRXD_WRITE_BYTES_POS)

#define UART_TRXD_READ_BYTES_UNIT        ((0x0UL<<UART_TRXD_READ_BYTES_POS ) & UART_TRXD_READ_BYTES_MSK )  //Read in byte
#define UART_TRXD_WRITE_BYTES_UNIT       ((0x0UL<<UART_TRXD_WRITE_BYTES_POS) & UART_TRXD_WRITE_BYTES_MSK)  //Write in byte
#define UART_TRXD_READ_WORD_UNIT         ((0x3UL<<UART_TRXD_READ_BYTES_POS ) & UART_TRXD_READ_BYTES_MSK )  //Read in word
#define UART_TRXD_WRITE_WORD_UNIT        ((0x3UL<<UART_TRXD_WRITE_BYTES_POS) & UART_TRXD_WRITE_BYTES_MSK)  //Read in word

/*****************  Bit definition for uart_trx register    *******************/
#define UART_TXB_NBYTE_POS              (0U)
#define UART_TXB_NBYTE_MSK              (0xFFFUL << UART_TXB_NBYTE_POS)
#define UART_TXB_NBYTE                  UART_TXB_NBYTE_MSK
#define UART_RXB_NBYTE_POS              (16U)
#define UART_RXB_NBYTE_MSK              (0xFFFUL << UART_RXB_NBYTE_POS)
#define UART_RXB_NBYTE                  UART_RXB_NBYTE_MSK
#define UART_RXFIFO_NBYTE_POS           (28U)
#define UART_RXFIFO_NBYTE_MSK           (0xFFFUL << UART_RXFIFO_NBYTE_POS)
#define UART_RXFIFO_NBYTE               UART_RXFIFO_NBYTE_MSK

/*****************  Bit definition for uart_event register  *******************/
#define UART_EVENT_RXB_WR_ERR_POS       (19U)
#define UART_EVENT_RXB_WR_ERR_MSK       (0x1UL << UART_EVENT_RXB_WR_ERR_POS)  // bit 19
#define UART_EVENT_RXB_WR_ERR           UART_EVENT_RXB_WR_ERR_MSK
#define UART_EVENT_TXB_RD_ERR_POS       (18U)
#define UART_EVENT_TXB_RD_ERR_MSK       (0x1UL << UART_EVENT_TXB_RD_ERR_POS)  // bit 18
#define UART_EVENT_TXB_RD_ERR           UART_EVENT_TXB_RD_ERR_MSK
#define UART_EVENT_BREAK_ERR_POS        (17U)
#define UART_EVENT_BREAK_ERR_MSK        (0x1UL << UART_EVENT_BREAK_ERR_POS)
#define UART_EVENT_BREAK_ERR            UART_EVENT_BREAK_ERR_MSK
#define UART_EVENT_FRAME_ERR_POS        (16U)
#define UART_EVENT_FRAME_ERR_MSK        (0x1UL << UART_EVENT_FRAME_ERR_POS)    // bit 16
#define UART_EVENT_FRAME_ERR            UART_EVENT_FRAME_ERR_MSK
#define UART_EVENT_PARITY_ERR_POS       (15U)
#define UART_EVENT_PARITY_ERR_MSK       (0x1UL << UART_EVENT_PARITY_ERR_POS) // bit 15
#define UART_EVENT_PARITY_ERR           UART_EVENT_PARITY_ERR_MSK
#define UART_EVENT_RXFIFO_OVF_ERR_POS   (14U)
#define UART_EVENT_RXFIFO_OVF_ERR_MSK   (0x1UL << UART_EVENT_RXFIFO_OVF_ERR_POS)  // bit 14
#define UART_EVENT_RXFIFO_OVF_ERR       UART_EVENT_RXFIFO_OVF_ERR_MSK
#define UART_EVENT_CTS_POS              (11U)
#define UART_EVENT_CTS_MSK              (0x1UL << UART_EVENT_CTS_POS)  // bit 11
#define UART_EVENT_CTS                  UART_EVENT_CTS_MSK
#define UART_EVENT_RXB_FULL_POS         (10U)
#define UART_EVENT_RXB_FULL_MSK         (0x1UL << UART_EVENT_RXB_FULL_POS)  // bit 10
#define UART_EVENT_RXB_FULL             UART_EVENT_RXB_FULL_MSK
#define UART_EVENT_RXFIFO_FULL_POS      (9U)
#define UART_EVENT_RXFIFO_FULL_MSK      (0x1UL << UART_EVENT_RXFIFO_FULL_POS)  // bit  9
#define UART_EVENT_RXFIFO_FULL          UART_EVENT_RXFIFO_FULL_MSK
#define UART_EVENT_RXFIFO_EMPTY_POS     (8U)
#define UART_EVENT_RXFIFO_EMPTY_MSK     (0x1UL << UART_EVENT_RXFIFO_EMPTY_POS)  // bit  8
#define UART_EVENT_RXFIFO_EMPTY         UART_EVENT_RXFIFO_EMPTY_MSK
#define UART_EVENT_RXFIFO_READY_POS     (7U)
#define UART_EVENT_RXFIFO_READY_MSK     (0x1UL << UART_EVENT_RXFIFO_READY_POS)  // bit  7
#define UART_EVENT_RXFIFO_READY         UART_EVENT_RXFIFO_READY_MSK
#define UART_EVENT_RXD_READY_POS        (6U)
#define UART_EVENT_RXD_READY_MSK        (0x1UL << UART_EVENT_RXD_READY_POS)  // bit  6
#define UART_EVENT_RXD_READY            UART_EVENT_RXD_READY_MSK
#define UART_EVENT_TXB_EMPTY_POS        (5U)
#define UART_EVENT_TXB_EMPTY_MSK        (0x1UL << UART_EVENT_TXB_EMPTY_POS)  // bit  5
#define UART_EVENT_TXB_EMPTY            UART_EVENT_TXB_EMPTY_MSK
#define UART_EVENT_TXFIFO_FULL_POS      (4U)
#define UART_EVENT_TXFIFO_FULL_MSK      (0x1UL << UART_EVENT_TXFIFO_FULL_POS)  // bit  4
#define UART_EVENT_TXFIFO_FULL          UART_EVENT_TXFIFO_FULL_MSK
#define UART_EVENT_TXFIFO_EMPTY_POS     (3U)
#define UART_EVENT_TXFIFO_EMPTY_MSK     (0x1UL << UART_EVENT_TXFIFO_EMPTY_POS)  // bit  3
#define UART_EVENT_TXFIFO_EMPTY         UART_EVENT_TXFIFO_EMPTY_MSK
#define UART_EVENT_TXD_READY_POS        (2U)
#define UART_EVENT_TXD_READY_MSK        (0x1UL << UART_EVENT_TXD_READY_POS)  // bit  2
#define UART_EVENT_TXD_READY            UART_EVENT_TXD_READY_MSK
#define UART_EVENT_RX_ON_POS            (1U)
#define UART_EVENT_RX_ON_MSK            (0x1UL << UART_EVENT_RX_ON_POS)  // bit  1
#define UART_EVENT_RX_ON                UART_EVENT_RX_ON_MSK
#define UART_EVENT_TX_ON_POS            (0U)
#define UART_EVENT_TX_ON_MSK            (0x1UL << UART_EVENT_TX_ON_POS)  // bit  0
#define UART_EVENT_TX_ON                UART_EVENT_TX_ON_MSK

typedef struct
{
  __IO uint32_t EN;           /* Address offset: 0x00      */
  __IO uint32_t TXCTRL;       /* Address offset: 0x04      */
  __IO uint32_t RXCTRL;       /* Address offset: 0x08      */
  __IO uint32_t INT_EN;       /* Address offset: 0x0C      */
  __IO uint32_t INT_CLR;      /* Address offset: 0x10      */
  __IO uint32_t CFG;          /* Address offset: 0x14      */
  __IO uint32_t TXD;          /* Address offset: 0x18      */
  __IO uint32_t RXD;          /* Address offset: 0x1C      */
  __IO uint32_t TRXD;         /* Address offset: 0x20      */
  __IO uint32_t TXBUF;        /* Address offset: 0x24      */
  __IO uint32_t RXBUF;        /* Address offset: 0x28      */
  __IO uint32_t BUF_SIZE;     /* Address offset: 0x2C      */
  __IO uint32_t TRX;          /* Address offset: 0x30      */
  __IO uint32_t EVENT;        /* Address offset: 0x34      */
} UART_TypeDef;

// Section: TIMER __________________________________________________________________
#define TIMER0_BASE_ADDR    (0x40029000)
#define TIMER1_BASE_ADDR    (0x4002A000)
#define TIMER2_BASE_ADDR    (0x4002B000)
#define TIMER3_BASE_ADDR    (0x4002C000)

#define DEF_POS_BIT_RUN_TYPE            0
#define DEF_POS_BIT_EVT_TYPE            1
#define DEF_POS_BIT_AUTO_CLEAR          11
#define DEF_POS_BIT_EVT_COM             8
#define DEF_POS_BIT_TM_TOE_COM          4
#define DEF_MAX_CYCLE_OF_PULSE_WIDTH    63
#define DEF_POS_BIT_EVT_WIDTH           2
#define DEF_POS_BIT_PAUSE               0

typedef struct
{
  __IO uint32_t EN;              /* Address 0ffset: 0x0  */
  __IO uint32_t START;           /* Address 0ffset: 0x4  */
  __IO uint32_t CTRL;            /* Address 0ffset: 0x8  */
  __IO uint32_t TVL;             /* Address 0ffset: 0xc  */
  __IO uint32_t INT_EN;          /* Address 0ffset: 0x10 */
  __IO uint32_t INT_CLR;         /* Address 0ffset: 0x14 */
  __IO uint32_t INT_ST;          /* Address 0ffset: 0x18 */
  __IO uint32_t PS;              /* Address 0ffset: 0x1c */
  __IO uint32_t MODE;            /* Address 0ffset: 0x20 */
  __IO uint32_t TO_EN;           /* Address 0ffset: 0x24 */
  __IO uint32_t TOV0;            /* Address 0ffset: 0x28 */
  __IO uint32_t TOV1;            /* Address 0ffset: 0x2c */
  __IO uint32_t TOV2;            /* Address 0ffset: 0x30 */
  __IO uint32_t TOV3;            /* Address 0ffset: 0x34 */
  __IO uint32_t TOE;             /* Address 0ffset: 0x38 */
  __IO uint32_t VER;             /* Address 0ffset: 0x3c */
} TIMER_TypeDef;

// Section: GPIO __________________________________________________________________
#define GPIO_BASE_ADDR      (0x40022000)

typedef struct
{
  __IO  uint32_t  GPIO_MODE ;      /* Address offset: 0x00      */
  __IO  uint32_t  GPIO      ;      /* Address offset: 0x04      */
  __IO  uint32_t  GPIO_OUT  ;      /* Address offset: 0x08      */
  __I   uint32_t  GPIO_IN   ;      /* Address offset: 0x0C      */
  __IO  uint32_t  TRIG      ;      /* Address offset: 0x10      */
  __IO  uint32_t  TRIG_CFG  ;      /* Address offset: 0x14      */
  __I   uint32_t  TRIG_VAL  ;      /* Address offset: 0x18      */
  __IO  uint32_t  TRIG_CLEAR;      /* Address offset: 0x1C      */
  __IO  uint32_t  TRIG_OUT  ;      /* Address offset: 0x20      */
  __IO  uint32_t  GPIO_PIN0 ;      /* Address offset: 0x24      */
  __IO  uint32_t  GPIO_PIN1 ;      /* Address offset: 0x28      */
  __IO  uint32_t  GPIO_PIN2 ;      /* Address offset: 0x2C      */
  __IO  uint32_t  GPIO_PIN3 ;      /* Address offset: 0x30      */
  __IO  uint32_t  GPIO_PIN4 ;      /* Address offset: 0x34      */
  __IO  uint32_t  GPIO_PIN5 ;      /* Address offset: 0x38      */
  __IO  uint32_t  GPIO_PIN6 ;      /* Address offset: 0x3C      */
  __IO  uint32_t  GPIO_PIN7 ;      /* Address offset: 0x40      */
  __IO  uint32_t  GPIO_PIN8 ;      /* Address offset: 0x44      */
  __IO  uint32_t  GPIO_PIN9 ;      /* Address offset: 0x48      */
  __IO  uint32_t  GPIO_PIN10;      /* Address offset: 0x4C      */
  __IO  uint32_t  GPIO_PIN11;      /* Address offset: 0x50      */    
} stGPIO_TypeDef;

/*****************  Bit definition for GPIO_MODE register    *******************/
#define GPIO_mode_en_Pos              (0U)
#define GPIO_mode_en_Msk              (0x1UL << GPIO_mode_en_Pos)              /*!< 0x00000001 */
#define GPIO_mode_en                  GPIO_mode_en_Msk                         /*!< GPIO Module Enable */

/*****************  Bit definition for GPIO register        *******************/
#define GPIO_out_Pos                  (0U)
#define GPIO_out_Msk                  (0xFFFUL << GPIO_out_Pos)                /*!< 0x00000FFF */
#define GPIO_out                      GPIO_out_Msk                             /*!< output register to GPIO port  */

/*****************  Bit definition for GPIO_OUT register    *******************/
#define GPIO_out_en_Pos               (0U)
#define GPIO_out_en_Msk               (0xFFFUL << GPIO_out_en_Pos)             /*!< 0x00000FFF */
#define GPIO_out_en                   GPIO_out_en_Msk                          /*!< GPIO output enable bit N =1: output enable for pin N  */

/*****************  Bit definition for GPIO_IN register     *******************/
#define GPIO_data_Pos                 (0U)
#define GPIO_data_Msk                 (0xFFFUL << GPIO_data_Pos)               /*!< 0x00000FFF */
#define GPIO_data                     GPIO_data_Msk                            /*!< Read GPIO input port  */

/*****************  Bit definition for TRIG register        *******************/
#define GPIO_trig_en_Pos              (0U)
#define GPIO_trig_en_Msk              (0xFFFUL << GPIO_trig_en_Pos)            /*!< 0x00000FFF */
#define GPIO_trig_en                  GPIO_trig_en_Msk                         /*!< Trigger mode enable  */

/*****************  Bit definition for TRIG_CFG register     *******************/
#define GPIO_trig_set_Pos             (0U)
#define GPIO_trig_set_Msk             (0xFFFFFFUL << GPIO_trig_set_Pos)        /*!< 0x00FFFFFF */
#define GPIO_trig_set                 GPIO_trig_set_Msk                        /*!< TSn[1:0] for each pin; 00: no trig 01: rising edge 10: falling edge 11: rising or falling edge  */

/*****************  Bit definition for TRIG_VAL register     *******************/
#define GPIO_trig_status_Pos          (0U)
#define GPIO_trig_status_Msk          (0xFFFUL << GPIO_trig_status_Pos)        /*!< 0x00000FFF */
#define GPIO_trig_status              GPIO_trig_status_Msk                     /*!< Trigger status of the pin   */

/*****************  Bit definition for TRIG_CLEAR register  *******************/
#define GPIO_trig_clr_Pos             (0U)                                    
#define GPIO_trig_clr_Msk             (0xFFFUL << GPIO_trig_clr_Pos)           /*!< 0x00000FFF */
#define GPIO_trig_clr                 GPIO_trig_clr_Msk                        /*!< Clear the trigger value   */

/*****************  Bit definition for TRIG_OUT register    *******************/
#define GPIO_trig_out_sel_Pos         (0U)
#define GPIO_trig_out_sel_Msk         (0x1UL << GPIO_trig_out_sel_Pos)         /*!< 0x00000001 */
#define GPIO_trig_out_sel             GPIO_trig_out_sel_Msk                    /*!< trigger output selection; 0:Pass Through Register 1: Direct IRQ*/

/*****************  Bit definition for GPIO_PIN(0 to 11) register    ***********/
#define GPIO_pin_cfg_Pos              (0U)                                    
#define GPIO_pin_cfg_Msk              (0x1FUL << GPIO_pin_cfg_Pos)             /*!< 0x00000001 */
#define GPIO_pin_cfg                  GPIO_pin_cfg_Msk                         /*!< GPIO Pin config (PE, PS, DS1, DS0, SL);PE(Pull Up/Down Enable) 0:enable, PS (Pull Up/Down Select) 0:Pull Up/1:Pull Down*/

/** @defgroup TRIG_CFG
  * @{
  */
#define GPIO_TRIG_SET_NOTRIG              ((uint32_t)(0x0UL << GPIO_trig_set_Pos))          /*!< GPIO Trigger Sel: No Trigger*/
#define GPIO_TRIG_SET_RISING              ((uint32_t)(0x1UL << GPIO_trig_set_Pos))          /*!< GPIO Trigger Sel: Rising Edge Trigger*/
#define GPIO_TRIG_SET_FALLING             ((uint32_t)(0x2UL << GPIO_trig_set_Pos))          /*!< GPIO Trigger Sel: Falling Edge Trigger*/
#define GPIO_TRIG_SET_RISINGFALLING       ((uint32_t)(0x3UL << GPIO_trig_set_Pos))          /*!< GPIO Trigger Sel: Rising and Falling Edge Trigger*/

#define GPIO_TRIG_CLEAR_MSK               ((uint32_t)(0x3UL << GPIO_trig_set_Pos))          /*!< GPIO Trigger Sel: Use for Mask Clear Purpose*/


/** @defgroup GPIO_MODE
  * @{
  */
#define GPIO_MODULE_DISABLE              ((uint32_t)(0x0UL << GPIO_mode_en_Pos))          /*!< GPIO Module Disable*/
#define GPIO_MODULE_ENABLE              ((uint32_t)(0x1UL << GPIO_mode_en_Pos))           /*!< GPIO Module Enable*/

/**   
  * @brief  GPIO_InitTypeDef Pin Group
  */
#define GPIO_PIN_0                     (0x00000001UL)  /* Pin 0 selected    */
#define GPIO_PIN_1                     (0x00000002UL)  /* Pin 1 selected    */
#define GPIO_PIN_2                     (0x00000004UL)  /* Pin 2 selected    */
#define GPIO_PIN_3                     (0x00000008UL)  /* Pin 3 selected    */
#define GPIO_PIN_4                     (0x00000010UL)  /* Pin 4 selected    */
#define GPIO_PIN_5                     (0x00000020UL)  /* Pin 5 selected    */
#define GPIO_PIN_6                     (0x00000040UL)  /* Pin 6 selected    */
#define GPIO_PIN_7                     (0x00000080UL)  /* Pin 7 selected    */
#define GPIO_PIN_8                     (0x00000100UL)  /* Pin 8 selected    */
#define GPIO_PIN_9                     (0x00000200UL)  /* Pin 9 selected    */
#define GPIO_PIN_10                    (0x00000400UL)  /* Pin 10 selected   */
#define GPIO_PIN_11                    (0x00000800UL)  /* Pin 11 selected   */
#define GPIO_PIN_All                   (0x00000FFFUL)  /* All pins selected */
#define GPIO_PIN_MASK                  (0x00000FFFUL)       /* PIN mask for assert test */

/** 
  * @brief  GPIO_InitTypeDef Mode Group
  */
#define  GPIO_MODE_INPUT              (0x00000000UL)   /*!< GPIO Init Mode as Input  */
#define  GPIO_MODE_OUTPUT             (0x00000001UL)   /*!< GPIO Init Mode as Ouput   */
#define  GPIO_MODE_IT_RISING          (0x00000002UL)   /*!< GPIO Init Mode as External Interrupt Mode with Rising edge trigger detection  */
#define  GPIO_MODE_IT_FALLING         (0x00000003UL)   /*!< GPIO Init Mode as External Interrupt Mode with Falling edge trigger detection */
#define  GPIO_MODE_IT_RISING_FALLING  (0x00000004UL)   /*!< GPIO Init Mode as External Interrupt Mode with Rising/Falling edge trigger detection  */

/** 
  * @brief GPIO_InitTypeDef Pull Group
  * @{
  */
#define  GPIO_PULLUP                  (0x00000003UL)   /*!< Pull-up                  */
#define  GPIO_PULLDOWN                (0x00000001UL)   /*!< Pull-down                */
#define  GPIO_NOPULL                  (0x00000000UL)   /*!< No Pull-up or Pull-down  */

// Section: IOMUX __________________________________________________________________
#define IOMUX_BASE_ADDR      (0x4002D000)

typedef struct
{
  __IO uint32_t DEBUG_IO;      /* Address offset: 0x00      */
  __IO uint32_t GPIO00;        /* Address offset: 0x04      */
  __IO uint32_t GPIO01;        /* Address offset: 0x08      */
  __IO uint32_t GPIO02;        /* Address offset: 0x0C      */
  __IO uint32_t GPIO03;        /* Address offset: 0x10      */
  __IO uint32_t GPIO04;        /* Address offset: 0x14      */
  __IO uint32_t GPIO05;        /* Address offset: 0x18      */
  __IO uint32_t GPIO06;        /* Address offset: 0x1C      */
  __IO uint32_t GPIO07;        /* Address offset: 0x20      */
  __IO uint32_t GPIO08;        /* Address offset: 0x24      */
  __IO uint32_t GPIO09;        /* Address offset: 0x28      */
  __IO uint32_t GPIO10;        /* Address offset: 0x2C      */
  __IO uint32_t GPIO11;        /* Address offset: 0x30      */
  __IO uint32_t SPI_TRG;       /* Address offset: 0x34      */
  __IO uint32_t UART0_TRG;     /* Address offset: 0x38      */
  __IO uint32_t UART1_TRG;     /* Address offset: 0x3C      */
  __IO uint32_t TIME0_TRG;     /* Address offset: 0x40      */
  __IO uint32_t TIME1_TRG;     /* Address offset: 0x44      */
  __IO uint32_t TIME2_TRG;     /* Address offset: 0x48      */
  __IO uint32_t TIME3_TRG;     /* Address offset: 0x4C      */
  __IO uint32_t I2C_TRIG;      /* Address offset: 0x50      */
  __IO uint32_t UWB_TRIG;      /* Address offset: 0x54      */
} stIOMUX_TypeDef;

// Section: CRC __________________________________________________________________
#define CRC_BASE_ADDR      (0x4002E000)

typedef struct
{

  __IO  uint32_t  crc_en;         /* Address offset: 0x00      */
  __IO  uint32_t  crc_cfg;        /* Address offset: 0x04      */
  __IO  uint32_t  crc_poly;       /* Address offset: 0x08      */
  __IO  uint32_t  crc_xor;        /* Address offset: 0x0C      */
  __IO  uint32_t  crc_in;         /* Address offset: 0x10      */
  __IO  uint32_t  crc_out;        /* Address offset: 0x14      */
  __IO  uint32_t  crc_data_sel;   /* Address offset: 0x18      */
  __IO  uint32_t  crc_ahb_addr;   /* Address offset: 0x1C      */
  __IO  uint32_t  crc_data_len;   /* Address offset: 0x20      */
  __IO  uint32_t  crc_start;      /* Address offset: 0x24      */
  __IO  uint32_t  crc_irq;        /* Address offset: 0x28      */
  __IO  uint32_t  crc_end;        /* Address offset: 0x2C      */  
} stCRC_TypeDef;

/*****************  Bit definition for crc_en register       *******************/
#define CRC_en_Pos                (0U)
#define CRC_en_Msk                (0x1UL << CRC_en_Pos)             /*!< 0x00000001 */
#define CRC_en                    CRC_en_Msk                        /*!< CRC Module Enable */

/*****************  Bit definition for crc_cfg register     *******************/
#define CRC_refin_Pos             (0U)
#define CRC_refin_Msk             (0x1UL << CRC_refin_Pos)          /*!< 0x00000001 */
#define CRC_refin                 CRC_refin_Msk                     /*!< input reverse 0:enable (LSB of byte first) 1:disable (MSB first in) */
#define CRC_refout_Pos            (1U)
#define CRC_refout_Msk            (0x1UL << CRC_refout_Pos)         /*!< 0x00000002 */
#define CRC_refout                CRC_refout_Msk                    /*!< output reverse 0:enable 1:disable */
#define CRC_order_sel_Pos         (2U)
#define CRC_order_sel_Msk         (0x3UL << CRC_order_sel_Pos)      /*!< 0x0000000C */
#define CRC_order_sel             CRC_order_sel_Msk                 /*!< crc_order 00:8bit 01:16bit 10:32bit */
#define CRC_init_value_Pos        (4U)
#define CRC_init_value_Msk        (0x1UL << CRC_init_value_Pos)     /*!< 0x00000010 */
#define CRC_init_value            CRC_init_value_Msk                /*!< crc_initial value 1 or 0. 0:00000000 1:FFFFFFFF */
#define CRC_byte_order_Pos        (5U)
#define CRC_byte_order_Msk        (0x1UL << CRC_byte_order_Pos)     /*!< 0x00000020 */
#define CRC_byte_order            CRC_byte_order_Msk                /*!< byte order: 0: bit[31:24] first in. 1: bit[7:0] first in. */

/*****************  Bit definition for crc_poly register     *******************/
#define CRC_poly_Pos              (0U)
#define CRC_poly_Msk              (0xFFFFFFFFUL << CRC_poly_Pos)    /*!< 0xFFFFFFFF */
#define CRC_poly                  CRC_poly_Msk                      /*!< crc_poly */

/*****************  Bit definition for crc_xor register     *******************/
#define CRC_xor_Pos                (0U)
#define CRC_xor_Msk                (0xFFFFFFFFUL << CRC_xor_Pos)      /*!< 0xFFFFFFFF */
#define CRC_xor                    CRC_xor_Msk                        /*!< crc_output_xor_value */

/*****************  Bit definition for crc_in register       *******************/
#define CRC_in_data_Pos            (0U)
#define CRC_in_data_Msk            (0xFFFFFFFFUL << CRC_in_data_Pos)  /*!< 0xFFFFFFFF */
#define CRC_in_data                CRC_in_data_Msk                    /*!< crc_input_data. */

/*****************  Bit definition for crc_result register   *******************/
#define CRC_result_Pos            (0U)
#define CRC_result_Msk            (0xFFFFFFFFUL << CRC_result_Pos)  /*!< 0xFFFFFFFF */
#define CRC_result                CRC_result_Msk                    /*!< crc_output_data. */

/*****************  Bit definition for crc_data_sel register  *****************/
#define CRC_data_sel_Pos          (0U)
#define CRC_data_sel_Msk          (0x1UL << CRC_data_sel_Pos)       /*!< 0x00000001 */
#define CRC_data_sel              CRC_data_sel_Msk                  /*!< crc_data_source. 0: apb  1: ahb */

/*****************  Bit definition for crc_ahb_addr register ******************/
#define CRC_ahb_addr_Pos          (0U)
#define CRC_ahb_addr_Msk          (0xFFFFFFFFUL << CRC_ahb_addr_Pos) /*!< 0xFFFFFFFF */
#define CRC_ahb_addr              CRC_ahb_addr_Msk                    /*!< crc_ahb_addr. */

/*****************  Bit definition for crc_data_len register  *****************/
#define CRC_data_len_Pos          (0U)
#define CRC_data_len_Msk          (0x0000FFFFUL << CRC_data_len_Pos)/*!< 0x0000FFFF */
#define CRC_data_len              CRC_data_len_Msk                  /*!< number of bytes input to crc.. */

/*****************  Bit definition for crc_start register   *******************/
#define CRC_start_Pos             (0U)
#define CRC_start_Msk             (0x1UL << CRC_start_Pos)           /*!< 0x00000001 */
#define CRC_start                 CRC_start_Msk                      /*!< number of bytes input to crc */
#define CRC_init_en_Pos           (1U)
#define CRC_init_en_Msk           (0x1UL << CRC_init_en_Pos)         /*!< 0x00000002 */
#define CRC_init_en               CRC_init_en_Msk                    /*!< enable initialize crc reg*/

/*****************  Bit definition for crc_irq register     *******************/
#define CRC_irq_clear_Pos         (0U)
#define CRC_irq_clear_Msk         (0x1UL << CRC_irq_clear_Pos)      /*!< 0x00000001 */
#define CRC_irq_clear             CRC_irq_clear_Msk                 /*!< crc_irq_clear*/
#define CRC_irq_en_Pos            (1U)
#define CRC_irq_en_Msk            (0x1UL << CRC_irq_en_Pos)         /*!< 0x00000002 */
#define CRC_irq_en                CRC_irq_en_Msk                    /*!< enable crc irq */


/*****************  Bit definition for crc_end register     *******************/
#define CRC_end_Pos               (0U)
#define CRC_end_Msk               (0x1UL << CRC_end_Pos)            /*!< 0x00000001 */
#define CRC_end                   CRC_end_Msk                       /*!< CRC result ready event to indicate crc end. Write 0 to clear it. */

/** @defgroup CRC_en. CRC Module Enable or Disable
  * @{
  */
#define CRC_MODULE_DISABLE            ((uint32_t)(0x0UL << CRC_en_Pos))            /*!< CRC Module Disable*/
#define CRC_MODULE_ENABLE             ((uint32_t)(0x1UL << CRC_en_Pos))           /*!< CRC Module Enable*/

/** @defgroup CRC_byte_order. CRC byte order setting
  * @{
  */
#define CRC_ByteOrder_BigEndian       ((uint32_t)(0x0UL << CRC_byte_order_Pos))   /*!< CRC byte order setting.Big endian*/
#define CRC_ByteOrder_LittleEndian    ((uint32_t)(0x1UL << CRC_byte_order_Pos))   /*!< CRC byte order setting.Little endian*/

/** @defgroup CRC_data_sel. To select input data from APB or AHB
  * @{
  */
#define CRC_datasel_APB               ((uint32_t)(0x0UL << CRC_data_sel_Pos))     /*!< CRC Data input select from APB*/
#define CRC_datasel_AHB               ((uint32_t)(0x1UL << CRC_data_sel_Pos))     /*!< CRC Data input select from AHB*/

/** @defgroup CRC_irq. CRC irq enable control
  * @{
  */
#define CRC_IRQ_DISABLE               ((uint32_t)(0x0UL << CRC_irq_en_Pos))        /*!< CRC irq enable control setting */
#define CRC_IRQ_ENABLE                ((uint32_t)(0x1UL << CRC_irq_en_Pos))       /*!< CRC irq enable control setting */

/** @defgroup CRC_end. Clear Result Ready Event
  * @{
  */
#define CRC_CompleteEventClr          ((uint32_t)(0x0UL << CRC_end_Pos))          /*!< CRC module clear event complete signal.Write 0 to clear */

// Section: WDT  __________________________________________________________________
#define WDT_BASE_ADDR     (0x40028000)
#define WDT_ONE_SEC_MULT  (0x7D00)
#define WDT_ONE_MSEC_MULT (WDT_ONE_SEC_MULT / 1000)
typedef struct
{
  __IO uint32_t CTRL;          /* Address 0ffset: 0x0  */
  __IO uint32_t CSR;           /* Address 0ffset: 0x4  */
  __IO uint32_t IER;           /* Address 0ffset: 0x8  */
  __IO uint32_t ICR;           /* Address 0ffset: 0xc  */
  __IO uint32_t SR;            /* Address 0ffset: 0x10 */
  __IO uint32_t CVR;           /* Address 0ffset: 0x14 */
  __IO uint32_t TOV;           /* Address 0ffset: 0x18 */
  __IO uint32_t CFG;           /* Address 0ffset: 0x1c */
  __IO uint32_t VER;           /* Address 0ffset: 0x20 */
} stWDT_TypeDef;

// Section: I2C  __________________________________________________________________
#define I2C_BASE_ADDR       (0x40027000)
#define I2C_MAX_DATA_LENGTH 4095
/*****************  Bit definition for cr register       *******************/
#define I2C_DEV_ADDR_POS                (0U)
#define I2C_DEV_ADDR_MSK                (0x7FUL << I2C_DEV_ADDR_POS)            /*!< 0x0000007F */
#define I2C_DEV_ADDR                    I2C_DEV_ADDR_MSK                        /*!< Device Addr*/
#define I2C_RW_POS                      (7U)
#define I2C_RW_MSK                      (0x1UL << I2C_RW_POS)                   /*!< 0x00000080 */
#define I2C_RW                          I2C_RW_MSK                              /*!< R/W bit */
#define I2C_REG_ADDR_POS                (8U)
#define I2C_REG_ADDR_MSK                (0xFFUL << I2C_REG_ADDR_POS)            /*!< 0x0000FF00 */
#define I2C_REG_ADDR                    I2C_REG_ADDR_MSK                        /*!< Device Addr*/
#define I2C_INCL_RA_POS                 (16U)
#define I2C_INCL_RA_MSK                 (0x1UL << I2C_INCL_RA_POS)               /*!< 0x00010000 */
#define I2C_INCL_RA                     I2C_INCL_RA_MSK                          /*!< Include Regsiter Address */
#define I2C_NACK_LAST_BYTE_POS          (17U)
#define I2C_NACK_LAST_BYTE_MSK          (0x1UL << I2C_NACK_LAST_BYTE_POS)       /*!< 0x00020000 */
#define I2C_NACK_LAST_BYTE              I2C_NACK_LAST_BYTE_MSK                  /*!< Nack last byte */
#define I2C_NO_STOP_POS                 (18U)
#define I2C_NO_STOP_MSK                 (0x1UL << I2C_NO_STOP_POS)               /*!< 0x00040000 */
#define I2C_NO_STOP                     I2C_NO_STOP_MSK                         /*!< no stop bit */
#define I2C_BYTE_COUNT_POS              (20U)
#define I2C_BYTE_COUNT_MSK              (0xFFFUL << I2C_BYTE_COUNT_POS)         /*!< 0xFFF00000 */
#define I2C_BYTE_COUNT                  I2C_BYTE_COUNT_MSK                      /*!< data length in byte */
/*****************  Bit definition for fr register     *******************/
#define I2C_TWSCL_POS                   (0U)
#define I2C_TWSCL_MSK                   (0xFFFUL << I2C_TWSCL_POS)               /*!< 0x00000FFF */
#define I2C_TWSCL                       I2C_TWSCL_MSK                            /*!< clock peroid.*/
#define I2C_PROP_MODE_POS               (12U)
#define I2C_PROP_MODE_MSK               (0x1UL << I2C_PROP_MODE_POS)             /*!< 0x00001000 */
#define I2C_PROP_MODE                   I2C_PROP_MODE_MSK                        /*!< proprietary mode.1:Enable*/
#define I2C_CR_I2C_ENABLE_POS           (13U)
#define I2C_CR_I2C_ENABLE_MSK           (0x1UL << I2C_CR_I2C_ENABLE_POS)         /*!< 0x00002000 */
#define I2C_CR_I2C_ENABLE               I2C_CR_I2C_ENABLE_MSK                    /*!< cr enable.1:Enable*/
#define I2C_TWDRIVE_POS                 (16U)
#define I2C_TWDRIVE_MSK                 (0xFFFUL << I2C_TWDRIVE_POS)             /*!< 0x0FFF0000 */
#define I2C_TWDRIVE                     I2C_TWDRIVE_MSK                          /*!< drive period.*/
#define I2C_DMA_SEL_POS                 (28U)
#define I2C_DMA_SEL_MSK                 (0xFFFUL << I2C_DMA_SEL_POS)             /*!< 0x10000000 */
#define I2C_DMA_SEL                      I2C_DMA_SEL_MSK                          /*!< DMA Select.0:Toggle on RxFull 1: Toggle on TxEmpty*/
/*****************  Bit definition for i2c_rxdata register     *******************/
#define I2C_RXDATA_POS                  (0U)
#define I2C_RXDATA_MSK                  (0xFFFFFFFFUL << I2C_RXDATA_POS)     /*!< 0xFFFFFFFF */
#define I2C_RXDATA                      I2C_RXDATA_MSK                       /*!< rxdata.*/
/*****************  Bit definition for i2c_txdata register     *******************/
#define I2C_TXDATA_POS                  (0U)
#define I2C_TXDATA_MSK                  (0xFFFFFFFFUL << I2C_TXDATA_POS)     /*!< 0xFFFFFFFF */
#define I2C_TXDATA                      I2C_TXDATA_MSK                       /*!< txdata.*/
/*****************  Bit definition for status register     *******************/
#define I2C_RXDATA_FULL_POS             (0U)
#define I2C_RXDATA_FULL_MSK             (0x1UL << I2C_RXDATA_FULL_POS)        /*!< 0x00000001 */
#define I2C_RXDATA_FULL                 I2C_RXDATA_FULL_MSK                   /*!< rxdata_full:set on rising edge srx_data_full, reset when read*/
#define I2C_TXDATA_EMPTY_POS            (1U)
#define I2C_TXDATA_EMPTY_MSK            (0x1UL << I2C_TXDATA_EMPTY_POS)       /*!< 0x00000002 */
#define I2C_TXDATA_EMPTY                I2C_TXDATA_EMPTY_MSK                  /*!< txdata_empty:set on rising edge stx_data_empty, reset when read*/
#define I2C_READY_POS                   (2U)
#define I2C_READY_MSK                   (0x1UL << I2C_READY_POS)              /*!< 0x00000004 */
#define I2C_READY                       I2C_READY_MSK                         /*!< i2c ready:set when transaction finishes, reset when IRQ stat is read..*/
/*****************  Bit definition for i2c_irq register     *******************/
#define I2C_EN_RXDATA_IRQ_POS           (0U)
#define I2C_EN_RXDATA_IRQ_MSK           (0x1UL << I2C_EN_RXDATA_IRQ_POS)     /*!< 0x00000001 */
#define I2C_EN_RXDATA_IRQ               I2C_EN_RXDATA_IRQ_MSK                /*!< rxdata_full:set on rising edge srx_data_full, reset when read*/
#define I2C_EN_TXDATA_IRQ_POS           (1U)
#define I2C_EN_TXDATA_IRQ_MSK           (0x1UL << I2C_EN_TXDATA_IRQ_POS)     /*!< 0x00000002 */
#define I2C_EN_TXDATA_IRQ               I2C_EN_TXDATA_IRQ_MSK                /*!< txdata_empty:set on rising edge stx_data_empty, reset when read*/
#define I2C_EN_READY_IRQ_POS            (2U)
#define I2C_EN_READY_IRQ_MSK            (0x1UL << I2C_EN_READY_IRQ_POS)      /*!< 0x00000004 */
#define I2C_EN_READY_IRQ                I2C_EN_READY_IRQ_MSK                 /*!< i2c ready:set when transaction finishes, reset when IRQ stat is read*/
#define I2C_EN_NACK_IRQ_POS             (3U)
#define I2C_EN_NACK_IRQ_MSK             (0x1UL << I2C_EN_NACK_IRQ_POS)       /*!< 0x00000008 */
#define I2C_EN_NACK_IRQ                 I2C_EN_NACK_IRQ_MSK                  /*!< i2c_nack :IRQ NACK error set at receiption of NACK, reset when IRQ stat is read*/
/*****************  Bit definition for irq_reg register     *******************/
#define I2C_RXDATA_IRQ_POS              (0U)
#define I2C_RXDATA_IRQ_MSK              (0x1UL << I2C_RXDATA_IRQ_POS)        /*!< 0x00000001 */
#define I2C_RXDATA_IRQ                  I2C_RXDATA_IRQ_MSK                   /*!< rxdata_full:set on rising edge srx_data_full, reset when read*/
#define I2C_TXDATA_IRQ_POS              (1U)                                
#define I2C_TXDATA_IRQ_MSK              (0x1UL << I2C_TXDATA_IRQ_POS)        /*!< 0x00000002 */
#define I2C_TXDATA_IRQ                  I2C_TXDATA_IRQ_MSK                   /*!< txdata_empty:set on rising edge stx_data_empty, reset when read*/
#define I2C_READY_IRQ_POS               (2U)                                
#define I2C_READY_IRQ_MSK               (0x1UL << I2C_READY_IRQ_POS)         /*!< 0x00000004 */
#define I2C_READY_IRQ                   I2C_READY_IRQ_MSK                    /*!< I2C ready:set when transaction finishes, reset when IRQ stat is read*/
#define I2C_NACK_IRQ_POS                (3U)                                
#define I2C_NACK_IRQ_MSK                (0x1UL << I2C_NACK_IRQ_POS)          /*!< 0x00000008 */
#define I2C_NACK_IRQ                    I2C_NACK_IRQ_MSK                     /*!< I2C_nack :IRQ NACK error set at receiption of NACK, reset when IRQ stat is read*/
/*****************  I2C READ/WRITE SELECT *******************/
#define I2C_WRITEMODE                   ((uint32_t)(0x0UL << I2C_RW_POS))        /*!< Configure to I2C Write Mode */
#define I2C_READMODE                    ((uint32_t)(0x1UL << I2C_RW_POS))        /*!< Configure to I2C Read Mode */
/*****************  CONFIG INCLUDE REGISTER ADDRESS *******************/
#define I2C_NO_INCLUDE_REG_ADDR         ((uint32_t)(0x0UL << I2C_INCL_RA_POS))      /*!< Configure to not include regsiter address in transmision */
#define I2C_INCLUDE_REG_ADDR            ((uint32_t)(0x1UL << I2C_INCL_RA_POS))      /*!< Configure to include regsiter address in transmision */
/*****************  CONFIG DMA SELECT TO RX_FULL or TX_EMPTY *******************/
#define I2C_DMA_SEL_RX                  ((uint32_t)(0x0UL << I2C_DMA_SEL_POS))      /*!< Configure to DMA select on RxFull */
#define I2C_DMA_SEL_TX                  ((uint32_t)(0x1UL << I2C_DMA_SEL_POS))      /*!< Configure to DMA select on TxEmpty*/
typedef struct
{

  __IO  uint32_t  cr;           /*Address offset: 0x00*/
  __IO  uint32_t  fr;           /*Address offset: 0x04*/
  __I   uint32_t  rxdata;       /*Address offset: 0x08*/
  __IO  uint32_t  txdata;       /*Address offset: 0x0C*/
  __I   uint32_t  status;       /*Address offset: 0x10*/
  __IO  uint32_t  irq_en;       /*Address offset: 0x14*/
  __I   uint32_t  irq_reg;      /*Address offset: 0x18*/
  
} stI2C_TypeDef;

// Section: DMA  __________________________________________________________________
#define DMA_BASE_ADDR            (0x40021000)
#define DMA_CHANNEL0_BASE_ADDR  (DMA_BASE_ADDR+0x100)
#define DMA_CHANNEL1_BASE_ADDR  (DMA_CHANNEL0_BASE_ADDR+0x20)
#define DMA_CHANNEL2_BASE_ADDR  (DMA_CHANNEL1_BASE_ADDR+0x20)
/*****************  Bit definition for dma_en register       *******************/
#define DMA_ENABLE_POS                  (0U)
#define DMA_ENABLE_MSK                  (0x1UL << DMA_ENABLE_POS)             /*!< 0x00000001 */
#define DMA_ENABLE                      DMA_ENABLE_MSK                        /*!< DMA Module Enable */
/*****************  Bit definition for irq_en register       *******************/
#define DMA_CH_READY_IRQ_EN_POS         (0U)
#define DMA_CH_READY_IRQ_EN_MSK         (0xFUL << DMA_CH_READY_IRQ_EN_POS)         /*!< 0x0000000F */
#define DMA_CH_READY_IRQ_EN             DMA_CH_READY_IRQ_EN_MSK                    /*!< Channel Ready IRQ Enable*/
#define DMA_CH_ERR_IRQ_EN_POS           (16U)
#define DMA_CH_ERR_IRQ_EN_MSK           (0xFUL << DMA_CH_ERR_IRQ_EN_POS)           /*!< 0x000F0000 */
#define DMA_CH_ERR_IRQ_EN               DMA_CH_ERR_IRQ_EN_MSK                      /*!< Error IRQ Enable*/
/*****************  Bit definition for irq_status register       *******************/
#define DMA_CH_READY_IRQ_STAT_POS       (0U)
#define DMA_CH_READY_IRQ_STAT_MSK       (0xFUL << DMA_CH_READY_IRQ_STAT_POS)         /*!< 0x0000000F */
#define DMA_CH_READY_IRQ_STAT           DMA_CH_READY_IRQ_STAT_MSK                    /*!< Channel Ready IRQ Enable*/
#define DMA_CH_READY_IRQ_CH_0_MSK       (0x1UL << DMA_CH_READY_IRQ_STAT_POS)
#define DMA_CH_READY_IRQ_CH_0           DMA_CH_READY_IRQ_CH_0_MSK
#define DMA_CH_READY_IRQ_CH_1_MSK       (0x2UL << DMA_CH_READY_IRQ_STAT_POS)
#define DMA_CH_READY_IRQ_CH_1           DMA_CH_READY_IRQ_CH_1_MSK
#define DMA_CH_READY_IRQ_CH_2_MSK       (0x4UL << DMA_CH_READY_IRQ_STAT_POS)
#define DMA_CH_READY_IRQ_CH_2           DMA_CH_READY_IRQ_CH_2_MSK
#define DMA_CH_ERR_IRQ_STAT_POS         (16U)
#define DMA_CH_ERR_IRQ_STAT_MSK         (0xFUL << DMA_CH_ERR_IRQ_STAT_POS)           /*!< 0x000F0000 */
#define DMA_CH_ERR_IRQ_STAT             DMA_CH_ERR_IRQ_STAT_MSK                      /*!< Error IRQ Enable*/
/*****************  Bit definition for ChFcWs register       *******************/
#define DMA_CH_FC_WS_POS                (0U)
#define DMA_CH_FC_WS_MSK                (0x3UL << DMA_CH_FC_WS_POS)                 /*!< 0x00000003 */
#define DMA_CH_FC_WS                    DMA_CH_FC_WS_MSK                            /*!< Flow Control Wait State*/
/*****************  Bit definition for dma_req_reg register       *******************/
#define DMA_CH0_FLOW_CONTROL_POS        (0U)
#define DMA_CH0_FLOW_CONTROL_MSK        (0xFFUL << DMA_CH0_FLOW_CONTROL_POS)         /*!< 0x000000FF */
#define DMA_CH0_FLOW_CONTROL            DMA_CH0_FLOW_CONTROL_MSK                    /*!< Flow Control Channel 0*/
#define DMA_CH1_FLOW_CONTROL_POS        (8U)
#define DMA_CH1_FLOW_CONTROL_MSK        (0xFFUL << DMA_CH1_FLOW_CONTROL_POS)         /*!< 0x0000FF00 */
#define DMA_CH1_FLOW_CONTROL            DMA_CH1_FLOW_CONTROL_MSK                    /*!< Flow Control Channel 1*/
#define DMA_CH2_FLOW_CONTROL_POS        (16U)
#define DMA_CH2_FLOW_CONTROL_MSK        (0xFFUL << DMA_CH2_FLOW_CONTROL_POS)         /*!< 0x00FF0000 */
#define DMA_CH2_FLOW_CONTROL            DMA_CH2_FLOW_CONTROL_MSK                    /*!< Flow Control Channel 2*/
#define DMA_CH3_FLOW_CONTROL_POS        (24U)
#define DMA_CH3_FLOW_CONTROL_MSK        (0xFFUL << DMA_CH3_FLOW_CONTROL_POS)         /*!< 0xFF000000 */
#define DMA_CH3_FLOW_CONTROL            DMA_CH3_FLOW_CONTROL_MSK                    /*!< Flow Control Channel 3*/
/*****************  DMA CHANNEL THINGS *******************/
/*****************  Bit definition for ChSrcAddr register       *******************/
#define DMA_CH_SRC_ADDR_POS             (0U)
#define DMA_CH_SRC_ADDR_MSK             (0xFFFFFFFFUL << DMA_CH_SRC_ADDR_POS)       /*!< 0xFFFFFFFF */
#define DMA_CH_SRC_ADDR                 DMA_CH_SRC_ADDR_MSK                          /*!< Source Address */
/*****************  Bit definition for ChSrcAddr register       *******************/
#define DMA_CH_DEST_ADDR_POS            (0U)
#define DMA_CH_DEST_ADDR_MSK            (0xFFFFFFFFUL << DMA_CH_DEST_ADDR_POS)      /*!< 0xFFFFFFFF */
#define DMA_CH_DEST_ADDR                DMA_CH_DEST_ADDR_MSK                        /*!< Destination Address */
/*****************  Bit definition for ChLLIAddr register       *******************/
#define DMA_CH_LLI_ADDR_POS             (0U)
#define DMA_CH_LLI_ADDR_MSK             (0xFFFFFFFFUL << DMA_CH_LLI_ADDR_POS)        /*!< 0xFFFFFFFF */
#define DMA_CH_LLI_ADDR                 DMA_CH_LLI_ADDR_MSK                          /*!< LLI Address */
/*****************  Bit definition for ChCtrl register       *******************/
#define DMA_TRANSFER_SIZE_POS           (0U)
#define DMA_TRANSFER_SIZE_MSK           (0x3FFFFUL << DMA_TRANSFER_SIZE_POS)       /*!< 0x0003FFFF */
#define DMA_TRANSFER_SIZE               DMA_TRANSFER_SIZE_MSK                      /*!< Indicate the number of byte of transfer data */
#define DMA_TYPE_POS                    (18U)
#define DMA_TYPE_MSK                    (0x3UL << DMA_TYPE_POS)                   /*!< 0x000C0000 */
#define DMA_TYPE                        DMA_TYPE_MSK                              /*!< Indicate the whether the LLI is delay or not (2'h1: delay) */
#define DMA_SWIDTH_POS                  (20U)
#define DMA_SWIDTH_MSK                  (0x3UL << DMA_SWIDTH_POS)                 /*!< 0x00300000 */
#define DMA_SWIDTH                      DMA_SWIDTH_MSK                            /*!< Indicate width of data moved from source once( 2'b00: byte access;2'b01: hword access;2'b10: word access)*/
#define DMA_DWIDTH_POS                  (22U)
#define DMA_DWIDTH_MSK                  (0x3UL << DMA_DWIDTH_POS)                 /*!< 0x00C00000 */
#define DMA_DWIDTH                      DMA_DWIDTH_MSK                            /*!< Indicate width of data moved to destination once( 2'b00: byte access;2'b01: hword access;2'b10: word access)*/
#define DMA_IFC_SEL_POS                 (24U)
#define DMA_IFC_SEL_MSK                 (0x1UL << DMA_IFC_SEL_POS)                 /*!< 0x01000000 */
#define DMA_IFC_SEL                     DMA_IFC_SEL_MSK                            /*!< Indicate select apb or ahb transfer data(1'b1: apb; 1'b0: ahb) - can only use ahb in UW1000*/
#define DMA_SINC_POS                    (25U)
#define DMA_SINC_MSK                    (0x1UL << DMA_SINC_POS)                   /*!< 0x02000000 */
#define DMA_SINC                        DMA_SINC_MSK                              /*!< Indicate whether needs to update source address(1'b1: need update; 1'b0: not need update)*/
#define DMA_DINC_POS                    (26U)
#define DMA_DINC_MSK                    (0x1UL << DMA_DINC_POS)                   /*!< 0x04000000 */
#define DMA_DINC                        DMA_DINC_MSK                              /*!< Indicate whether needs to update destination address(1'b1: need update; 1'b0: not need update)*/


#define DMA_FC_SRC_POSEDGE_POS          (27U)
#define DMA_FC_SRC_POSEDGE_MSK          (0x1UL << DMA_FC_SRC_POSEDGE_POS)
#define DMA_FC_SRC_POSEDGE              DMA_FC_SRC_POSEDGE_MSK

#define DMA_FC_DEST_POSEDGE_POS         (28U)
#define DMA_FC_DEST_POSEDGE_MSK         (0x1UL << DMA_FC_DEST_POSEDGE_POS)
#define DMA_FC_DEST_POSEDGE             DMA_FC_DEST_POSEDGE_MSK

#define DMA_CONTINUOUS_POS              (31U)
#define DMA_CONTINUOUS_MSK              (0x1UL << DMA_CONTINUOUS_POS)             /*!< 0x80000000 */
#define DMA_CONTINUOUS                  DMA_CONTINUOUS_MSK                        /*!< Indicate data buffer is empty or not(1'b1: empty; 1'b0: not empty)*/
/*****************  Bit definition for ChFcSrcSel register       *******************/
#define DMA_CH_FC_SRC_SEL_POS           (0U)
#define DMA_CH_FC_SRC_SEL_MSK           (0xFUL << DMA_CH_FC_SRC_SEL_POS)             /*!< 0x0000000F */
#define DMA_CH_FC_SRC_SEL               DMA_CH_FC_SRC_SEL_MSK                        /*!< Indicate value of flow control input that source selected*/

#define DMA_CH_FC_SRC_SEL_0             (0x1UL << DMA_CH_FC_SRC_SEL_POS)
#define DMA_CH_FC_SRC_SEL_1             (0x2UL << DMA_CH_FC_SRC_SEL_POS)
#define DMA_CH_FC_SRC_SEL_2             (0x3UL << DMA_CH_FC_SRC_SEL_POS)
#define DMA_CH_FC_SRC_SEL_3             (0x4UL << DMA_CH_FC_SRC_SEL_POS)
#define DMA_CH_FC_SRC_SEL_4             (0x5UL << DMA_CH_FC_SRC_SEL_POS)
#define DMA_CH_FC_SRC_SEL_5             (0x6UL << DMA_CH_FC_SRC_SEL_POS)
#define DMA_CH_FC_SRC_SEL_6             (0x7UL << DMA_CH_FC_SRC_SEL_POS)
#define DMA_CH_FC_SRC_SEL_7             (0x8UL << DMA_CH_FC_SRC_SEL_POS)

#define DMA_CH_FC_DEST_SEL_POS          (8U)
#define DMA_CH_FC_DEST_SEL_MSK          (0xFUL << DMA_CH_FC_DEST_SEL_POS)             /*!< 0x00000F00 */
#define DMA_CH_FC_DEST_SEL              DMA_CH_FC_DEST_SEL_MSK                        /*!< Indicate value of flow control input that destination selected*/

#define DMA_CH_FC_DEST_SEL_0            (0x1UL << DMA_CH_FC_DEST_SEL_POS)
#define DMA_CH_FC_DEST_SEL_1            (0x2UL << DMA_CH_FC_DEST_SEL_POS)
#define DMA_CH_FC_DEST_SEL_2            (0x3UL << DMA_CH_FC_DEST_SEL_POS)
#define DMA_CH_FC_DEST_SEL_3            (0x4UL << DMA_CH_FC_DEST_SEL_POS)
#define DMA_CH_FC_DEST_SEL_4            (0x5UL << DMA_CH_FC_DEST_SEL_POS)
#define DMA_CH_FC_DEST_SEL_5            (0x6UL << DMA_CH_FC_DEST_SEL_POS)
#define DMA_CH_FC_DEST_SEL_6            (0x7UL << DMA_CH_FC_DEST_SEL_POS)
#define DMA_CH_FC_DEST_SEL_7            (0x8UL << DMA_CH_FC_DEST_SEL_POS)

#define DMA_CH_FC_LLI_SEL_POS           (16U)
#define DMA_CH_FC_LLI_SEL_MSK           (0xFUL << DMA_CH_FC_LLI_SEL_POS)             /*!< 0x000F0000 */
#define DMA_CH_FC_LLI_SEL               DMA_CH_FC_LLI_SEL_MSK                        /*!< Indicate value of flow control input that Lli selected*/
/*****************  Bit definition for Chcfg register       *******************/
#define DMA_CH_CFG_EN_POS               (0U)
#define DMA_CH_CFG_EN_MSK               (0x1UL << DMA_CH_CFG_EN_POS)               /*!< 0x00000001 */
#define DMA_CH_CFG_EN                   DMA_CH_CFG_EN_MSK                          /*!< Indicate enable channel configure*/
#define DMA_CH_CFG_PAUSE_POS            (1U)
#define DMA_CH_CFG_PAUSE_MSK            (0x1UL << DMA_CH_CFG_PAUSE_POS)             /*!< 0x00000002 */
#define DMA_CH_CFG_PAUSE                DMA_CH_CFG_PAUSE_MSK                        /*!< Indicate pause channel configure*/
#define DMA_CH_CFG_PRIO_POS             (2U)
#define DMA_CH_CFG_PRIO_MSK             (0x1UL << DMA_CH_CFG_PRIO_POS)             /*!< 0x00000004 */
#define DMA_CH_CFG_PRIO                 DMA_CH_CFG_PRIO_MSK                        /*!< Indicate whether has priority or not */
#define DMA_CH_CFG_HALTONERR_POS        (3U)
#define DMA_CH_CFG_HALTONERR_MSK        (0x1UL << DMA_CH_CFG_HALTONERR_POS)         /*!< 0x00000008 */
#define DMA_CH_CFG_HALTONERR            DMA_CH_CFG_HALTONERR_MSK                    /*!< Indicate having error response of system*/
#define DMA_CH_CFG_IRQONNEXTLLI_POS     (4U)
#define DMA_CH_CFG_IRQONNEXTLLI_MSK     (0x1UL << DMA_CH_CFG_IRQONNEXTLLI_POS)     /*!< 0x00000010 */
#define DMA_CH_CFG_IRQONNEXTLLI         DMA_CH_CFG_IRQONNEXTLLI_MSK                /*!< Indicate whether next LLI is ready or not*/


typedef struct
{
  __IO  uint32_t dma_en;          /*Address offset: 0x00*/
  __IO  uint32_t irq_en;          /*Address offset: 0x04*/
  __IO  uint32_t irq_status;      /*Address offset: 0x08*/
  __IO  uint32_t ChFcWs;          /*Address offset: 0x0c*/
  __IO  uint32_t ch_idle;       /*Address offset: 0x10*/
  __IO  uint32_t dma_req_reg;     /*Address offset: 0x14*/
} stDMA_TypeDef;

typedef struct
{
  __IO  uint32_t  ChannelSrcAddr;    /*Address offset: 0x00*/
  __IO  uint32_t  ChannelDestAddr;   /*Address offset: 0x04*/
  __IO  uint32_t  ChannelLLIAddr;    /*Address offset: 0x08*/
  __IO  uint32_t  ChannelCtrl;       /*Address offset: 0x0c*/
  __IO  uint32_t  ChannelFcSrcSel;   /*Address offset: 0x10*/
  __IO  uint32_t  Channelcfg;        /*Address offset: 0x14*/
}stDMAChannel_TypeDef;               /*Address offset: 0x100,0x120,0x140,0x160*/

// Section: EFUSE  __________________________________________________________________
#define EFUSE_BASE_ADDR (0x4002F000)
typedef struct
{
  __IO uint32_t mode;       /* Address offset: 0x00      */
  __IO uint32_t ctrl;       /* Address offset: 0x04      */
  __IO uint32_t addr;       /* Address offset: 0x08      */
  __IO uint32_t regAddr;    /* Address offset: 0x0C      */
  __IO uint32_t regWr;      /* Address offset: 0x10      */
  __IO uint32_t regRd;      /* Address offset: 0x14      */
  __IO uint32_t pgmBit;     /* Address offset: 0x18      */
  __I  uint32_t status;     /* Address offset: 0x1C      */
  __I  uint32_t failBit[8]; /* Address offset: 0x20->0x3C */
} stEfuse_TypeDef;

// Section: SPI______________________________________________________________________
#define pSPI ((stSPI_TypeDef *) 0x40023000)   /*APB Base Address of SPI Module*/
/*****************  Bit definition for SPI_EN register       *******************/
#define spi_en_Pos                        (0U)        
#define spi_en_Msk                        (0x1UL << spi_en_Pos)                     /*!< 0x00000001 */
#define spi_en                            spi_en_Msk                                /*!< SPI module enable*/
/*****************  Bit definition for SPI_START register     *******************/
#define spi_start_Pos                     (0U)        
#define spi_start_Msk                     (0x1UL << spi_start_Pos)                   /*!< 0x00000001 */       
#define spi_start                         spi_start_Msk                              /*!< start spi transmission. Write 1 to start spi */
#define spi_stop_Pos                      (1U)        
#define spi_stop_Msk                      (0x1UL << spi_stop_Pos)                   /*!< 0x00000002 */       
#define spi_stop                          spi_stop_Msk                              /*!< stop spi frame. Write 1 to stop spi */
/*****************  Bit definition for SPI_INT_EN register       *******************/
#define SPI_FRAME_END_Pos                 (0U)        
#define SPI_FRAME_END_Msk                 (0x1UL << SPI_FRAME_END_Pos)                   /*!< 0x00000001 */
#define SPI_FRAME_END                     SPI_FRAME_END_Msk                              /*!< Enable Interrupt to FRAME_END Event */
#define SPI_TX_END_Pos                    (1U)        
#define SPI_TX_END_Msk                    (0x1UL << SPI_TX_END_Pos)                     /*!< 0x00000002 */
#define SPI_TX_END                        SPI_TX_END_Msk                                /*!< Enable Interrupt to TX_END Event */
#define SPI_RX_END_Pos                    (2U)        
#define SPI_RX_END_Msk                    (0x1UL << SPI_RX_END_Pos)                     /*!< 0x00000004 */
#define SPI_RX_END                        SPI_RX_END_Msk                                /*!< Enable Interrupt to RX_END Event */
#define SPI_TXFIFO_EMPTY_Pos              (3U)        
#define SPI_TXFIFO_EMPTY_Msk              (0x1UL << SPI_TXFIFO_EMPTY_Pos)               /*!< 0x00000008 */
#define SPI_TXFIFO_EMPTY                  SPI_TXFIFO_EMPTY_Msk                          /*!< Enable Interrupt to TXFIFO_EMPTY Event */
#define SPI_TXFIFO_FULL_Pos               (4U)        
#define SPI_TXFIFO_FULL_Msk               (0x1UL << SPI_TXFIFO_FULL_Pos)                 /*!< 0x00000010 */
#define SPI_TXFIFO_FULL                   SPI_TXFIFO_FULL_Msk                            /*!< Enable Interrupt to TXFIFO_FULL Event */
#define SPI_TXB_EMPTY_Pos                 (5U)        
#define SPI_TXB_EMPTY_Msk                 (0x1UL << SPI_TXB_EMPTY_Pos)                   /*!< 0x00000020 */
#define SPI_TXB_EMPTY                     SPI_TXB_EMPTY_Msk                              /*!< Enable Interrupt to TXB_EMPTY Event (TX Buffer),tx buffer is empty, or all data have sent out */
#define SPI_RXFIFO_EMPTY_Pos              (6U)        
#define SPI_RXFIFO_EMPTY_Msk              (0x1UL << SPI_RXFIFO_EMPTY_Pos)               /*!< 0x00000040 */
#define SPI_RXFIFO_EMPTY                  SPI_RXFIFO_EMPTY_Msk                          /*!< Enable Interrupt to RXFIFO_EMPTY Event */
#define SPI_RXFIFO_FULL_Pos               (7U)        
#define SPI_RXFIFO_FULL_Msk               (0x1UL << SPI_RXFIFO_FULL_Pos)                 /*!< 0x00000080 */
#define SPI_RXFIFO_FULL                   SPI_RXFIFO_FULL_Msk                            /*!< Enable Interrupt to RXFIFO_FULL Event,*/
#define SPI_RXB_FULL_Pos                  (8U)        
#define SPI_RXB_FULL_Msk                  (0x1UL << SPI_RXB_FULL_Pos)                   /*!< 0x00000100 */
#define SPI_RXB_FULL                      SPI_RXB_FULL_Msk                              /*!< Enable Interrupt to RXB_FULL Event (RX Buffer),rx buffer is full, or all expected data received */
#define SPI_TXFIFO_OVR_ERR_Pos            (9U)        
#define SPI_TXFIFO_OVR_ERR_Msk            (0x1UL << SPI_TXFIFO_OVR_ERR_Pos)             /*!< 0x00000200 */
#define SPI_TXFIFO_OVR_ERR                SPI_TXFIFO_OVR_ERR_Msk                        /*!< Enable Interrupt to TXFIFO_OVR_ERR, txfifo overrun */
#define SPI_RXFIFO_OVF_ERR_Pos            (10U)        
#define SPI_RXFIFO_OVF_ERR_Msk            (0x1UL << SPI_RXFIFO_OVF_ERR_Pos)             /*!< 0x00000400 */
#define SPI_RXFIFO_OVF_ERR                SPI_RXFIFO_OVF_ERR_Msk                        /*!< Enable Interrupt to RXFIFO_OVF_ERR,rxfifo overflow */
#define SPI_TXB_RD_ERR_Pos                (11U)        
#define SPI_TXB_RD_ERR_Msk                (0x1UL << SPI_TXB_RD_ERR_Pos)                 /*!< 0x00000800 */
#define SPI_TXB_RD_ERR                    SPI_TXB_RD_ERR_Msk                            /*!< Enable Interrupt to TXB_RD_ERR, tx buffer read error  */
#define SPI_RXB_WR_ERR_Pos                (12U)        
#define SPI_RXB_WR_ERR_Msk                (0x1UL << SPI_RXB_WR_ERR_Pos)                 /*!< 0x00001000 */
#define SPI_RXB_WR_ERR                    SPI_RXB_WR_ERR_Msk                            /*!< Enable Interrupt to RXB_WR_ERR, rxbuffer write error  */
/*****************  Bit definition for SPI_INT_CLR register       *******************/
#define SPI_int_clr_Pos                   (0U)        
#define SPI_int_clr_Msk                   (0x1UL << SPI_int_clr_Pos)                     /*!< 0x00000001 */
#define SPI_int_clr                       SPI_int_clr_Msk                                /*!< clear interrupt. 1: write 1 to clear interrupt ,  0: no action*/
/*****************  Bit definition for SPI_CFG register           *******************/
#define SPI_mode_Pos                      (0U)        
#define SPI_mode_Msk                      (0x3UL << SPI_mode_Pos)                       /*!< 0x00000003 */
#define SPI_spi_mode                      SPI_mode_Msk                                  /*!< SPI mode. Define SPI clock pol and phase. 0: mode0, 2: mode2 */
#define SPI_type_Pos                      (2U)        
#define SPI_type_Msk                      (0x1UL << SPI_type_Pos)                       /*!< 0x00000004 */
#define SPI_type                          SPI_type_Msk                                  /*!< SPI type. 1: SPI Master , 0: SPI Slave */
#define SPI_bitorder_Pos                  (3U)        
#define SPI_bitorder_Msk                  (0x1UL << SPI_bitorder_Pos)                   /*!< 0x00000008 */
#define SPI_bitorder                      SPI_bitorder_Msk                              /*!< bit order in a byte. 0: lsb first, 1: msb first*/
#define SPI_byteorder_Pos                 (4U)        
#define SPI_byteorder_Msk                 (0x1UL << SPI_byteorder_Pos)                   /*!< 0x00000010 */
#define SPI_byteorder                     SPI_byteorder_Msk                              /*!< byte order in a word [byte3 byte2 byte1 byte0] =0.  Byte 0 first  1: Byte 3 first.*/
#define SPI_clk_sel_Pos                   (5U)        
#define SPI_clk_sel_Msk                   (0x7UL << SPI_clk_sel_Pos)                     /*!< 0x000000E0 */
#define SPI_clk_sel                       SPI_clk_sel_Msk                                /*!< clock selection for spi master. 64MHz/2^(n+1). 0: 32MHz,1: 16MHz,2: 8MHz,3: 4MHz,4: 2MHz,5: 1MHz,6: 512KHz,7: 256KHz*/
#define SPI_cs0_sel_Pos                   (8U)        
#define SPI_cs0_sel_Msk                   (0x1UL << SPI_cs0_sel_Pos)                     /*!< 0x00000100 */
#define SPI_cs0_sel                       SPI_cs0_sel_Msk                                /*!< chip select for spi_master. 1: cs0 is selected.*/
#define SPI_cs1_sel_Pos                   (9U)        
#define SPI_cs1_sel_Msk                   (0x1UL << SPI_cs1_sel_Pos)                     /*!< 0x00000200 */
#define SPI_cs1_sel                       SPI_cs1_sel_Msk                                /*!< chip select for spi_master. 1: cs1 is selected.*/
#define SPI_cs2_sel_Pos                   (10U)        
#define SPI_cs2_sel_Msk                   (0x1UL << SPI_cs2_sel_Pos)                     /*!< 0x00000400 */
#define SPI_cs2_sel                       SPI_cs2_sel_Msk                                /*!< chip select for spi_master. 1: cs2 is selected.*/
#define SPI_cs3_sel_Pos                   (11U)        
#define SPI_cs3_sel_Msk                   (0x1UL << SPI_cs3_sel_Pos)                     /*!< 0x00000800 */
#define SPI_cs3_sel                       SPI_cs3_sel_Msk                                /*!< chip select for spi_master. 1: cs3 is selected.*/
#define SPI_cs4_sel_Pos                   (12U)        
#define SPI_cs4_sel_Msk                   (0x1UL << SPI_cs4_sel_Pos)                     /*!< 0x00001000 */
#define SPI_cs4_sel                       SPI_cs4_sel_Msk                                /*!< chip select for spi_master. 1: cs4 is selected.*/
/*****************  Bit definition for SPI_BUF_EN register       *******************/
#define SPI_buf_en_Pos                    (0U)        
#define SPI_buf_en_Msk                    (0x1UL << SPI_buf_en_Pos)                 /*!< 0x00000001 */
#define SPI_buf_en                        SPI_buf_en_Msk                            /*!< enable direct memory buffer access. 1: enable direct memory buffer access, 0: disable direct memory buffer access. All data are stored in FIFO*/
#define SPI_buf_min_bytes_Pos             (1U)        
#define SPI_buf_min_bytes_Msk             (0x3UL << SPI_buf_min_bytes_Pos)          /*!< 0x00000006 */
#define SPI_buf_min_bytes                 SPI_buf_min_bytes_Msk                     /*!< Define the number of byte shall be received before transfer to rx buffer. n: n+1 byte (Example:0:1 byte, 1:2bytes , 2:3bytes...)*/
/*****************  Bit definition for SPI_TXFIFO register       *******************/
#define SPI_write_data_Pos                (0U)
#define SPI_write_data_Msk                (0xFFFFFFFFUL << SPI_write_data_Pos)          /*!< 0xFFFFFFFF */
#define SPI_write_data                    SPI_write_data_Msk                            /*!< TX data to be transferred. Write one word (32 bits) to TXD FIFO*/
/*****************  Bit definition for SPI_RXFIFO register       *******************/
#define SPI_read_data_Pos                 (0U)        
#define SPI_read_data_Msk                 (0xFFFFFFFFUL << SPI_read_data_Pos)            /*!< 0xFFFFFFFF */
#define SPI_read_data                     SPI_read_data_Msk                              /*!< RX data received. Read one word (32 bits) from RXD FIFO*/
/*****************  Bit definition for SPI_TRXFIFO register       *******************/
#define SPI_nreadbytes_Pos                (0U)        
#define SPI_nreadbytes_Msk                (0x3UL << SPI_nreadbytes_Pos)                  /*!< 0x00000003 */
#define SPI_nreadbytes                    SPI_nreadbytes_Msk                            /*!< Number of bytes to be read from FIFO. 0: read 1 byte, 3: read 4 bytes*/
#define SPI_nwritebytes_Pos               (2U)          
#define SPI_nwritebytes_Msk               (0x3UL << SPI_nwritebytes_Pos)                /*!< 0x0000000C */
#define SPI_nwritebytes                   SPI_nwritebytes_Msk                          /*!< Number of bytes to be written into FIFO. 0: write 1 byte 3: write 4 bytes*/
/*****************  Bit definition for SPI_RXB_RDY register       *******************/
#define SPI_rxbuf_readyflag_Pos           (0U)        
#define SPI_rxbuf_readyflag_Msk           (0x1UL << SPI_rxbuf_readyflag_Pos)            /*!< 0x00000001 */
#define SPI_rxbuf_readyflag               SPI_rxbuf_readyflag_Msk                        /*!< RX buffer is ready or not. This flag is written by MCU. 0: rx buffer is not available, 1: rx buffer is ready to receive data*/
/*****************  Bit definition for SPI_TXBUF register       *******************/
#define SPI_txb_pointer_Pos               (0U)        
#define SPI_txb_pointer_Msk               (0xFFFFFFFFUL << SPI_txb_pointer_Pos)         /*!< 0xFFFFFFFF */
#define SPI_txb_pointer                   SPI_txb_pointer_Msk                            /*!< pointer to memory buffer for sending*/
/*****************  Bit definition for SPI_RXBUF register       *******************/
#define SPI_rxb_pointer_Pos               (0U)        
#define SPI_rxb_pointer_Msk               (0xFFFFFFFFUL << SPI_rxb_pointer_Pos)         /*!< 0xFFFFFFFF */
#define SPI_rxb_pointer                   SPI_rxb_pointer_Msk                            /*!< pointer to memory buffer for receiving*/
/*****************  Bit definition for SPI_BUF_SIZE register       *******************/
#define SPI_tx_max_bytes_Pos              (0U)
#define SPI_tx_max_bytes_Msk              (0xFFFUL << SPI_tx_max_bytes_Pos)             /*!< 0x00000FFF */
#define SPI_tx_max_bytes                  SPI_tx_max_bytes_Msk                          /*!< Maximum number of bytes to be sent, N+1.*/
#define SPI_rx_max_bytes_Pos              (16U)        
#define SPI_rx_max_bytes_Msk              (0xFFFUL << SPI_rx_max_bytes_Pos)             /*!< 0x0FFF0000 */
#define SPI_rx_max_bytes                  SPI_rx_max_bytes_Msk                          /*!<  Maximum number of bytes to be received, N+1.*/
/*****************  Bit definition for SPI_TRX_ST register         *******************/
#define SPI_txb_nbyte_Pos                 (0U)
#define SPI_txb_nbyte_Msk                 (0xFFFUL << SPI_txb_nbyte_Pos)                 /*!< 0x0FFF0000 */
#define SPI_txb_nbyte                     SPI_txb_nbyte_Msk                              /*!< Number of bytes has been sent out.*/
#define SPI_rxb_nbyte_Pos                 (16U)        
#define SPI_rxb_nbyte_Msk                 (0xFFFUL << SPI_rxb_nbyte_Pos)                 /*!< 0x00000FFF */
#define SPI_rxb_nbyte                     SPI_rxb_nbyte_Msk                              /*!< Number of bytes received.*/
#define SPI_rxfifo_nbyte_Pos              (28U)        
#define SPI_rxfifo_nbyte_Msk              (0xFUL << SPI_rxfifo_nbyte_Pos)               /*!< 0xF0000000 */
#define SPI_rxfifo_nbyte                  SPI_rxfifo_nbyte_Msk                          /*!< number of bytes in rxfifo.*/
/*****************  Bit definition for SPI_EVENT register       *******************/
#define SPI_EVENT_SPI_ON_Pos              (0U)
#define SPI_EVENT_SPI_ON_Msk              (0x1UL << SPI_EVENT_SPI_ON_Pos)           /*!< 0x00000001 */
#define SPI_EVENT_SPI_ON                  SPI_EVENT_SPI_ON_Msk                      /*!< spi is in transmission */
#define SPI_EVENT_TX_END_Pos              (1U)
#define SPI_EVENT_TX_END_Msk              (0x1UL << SPI_EVENT_TX_END_Pos)           /*!< 0x00000002 */
#define SPI_EVENT_TX_END                  SPI_EVENT_TX_END_Msk                      /*!< Enable Interrupt to TX_END Event */
#define SPI_EVENT_RX_END_Pos              (2U)  
#define SPI_EVENT_RX_END_Msk              (0x1UL << SPI_EVENT_RX_END_Pos)           /*!< 0x00000004 */
#define SPI_EVENT_RX_END                  SPI_EVENT_RX_END_Msk                      /*!< Enable Interrupt to RX_END Event */
#define SPI_EVENT_TXFIFO_EMPTY_Pos        (3U)  
#define SPI_EVENT_TXFIFO_EMPTY_Msk        (0x1UL << SPI_EVENT_TXFIFO_EMPTY_Pos)     /*!< 0x00000008 */
#define SPI_EVENT_TXFIFO_EMPTY            SPI_EVENT_TXFIFO_EMPTY_Msk                /*!< Enable Interrupt to TXFIFO_EMPTY Event */
#define SPI_EVENT_TXFIFO_FULL_Pos         (4U)  
#define SPI_EVENT_TXFIFO_FULL_Msk         (0x1UL << SPI_EVENT_TXFIFO_FULL_Pos)       /*!< 0x00000010 */
#define SPI_EVENT_TXFIFO_FULL             SPI_EVENT_TXFIFO_FULL_Msk                  /*!< Enable Interrupt to TXFIFO_FULL Event */
#define SPI_EVENT_TXB_EMPTY_Pos           (5U)  
#define SPI_EVENT_TXB_EMPTY_Msk           (0x1UL << SPI_EVENT_TXB_EMPTY_Pos)         /*!< 0x00000020 */
#define SPI_EVENT_TXB_EMPTY               SPI_EVENT_TXB_EMPTY_Msk                    /*!< Enable Interrupt to TXB_EMPTY Event (TX Buffer),tx buffer is empty, or all data have sent out */
#define SPI_EVENT_RXFIFO_EMPTY_Pos        (6U)  
#define SPI_EVENT_RXFIFO_EMPTY_Msk        (0x1UL << SPI_EVENT_RXFIFO_EMPTY_Pos)     /*!< 0x00000040 */
#define SPI_EVENT_RXFIFO_EMPTY            SPI_EVENT_RXFIFO_EMPTY_Msk                /*!< Enable Interrupt to RXFIFO_EMPTY Event */
#define SPI_EVENT_RXFIFO_FULL_Pos         (7U)  
#define SPI_EVENT_RXFIFO_FULL_Msk         (0x1UL << SPI_EVENT_RXFIFO_FULL_Pos)       /*!< 0x00000080 */
#define SPI_EVENT_RXFIFO_FULL             SPI_EVENT_RXFIFO_FULL_Msk                  /*!< Enable Interrupt to RXFIFO_FULL Event,*/
#define SPI_EVENT_RXB_FULL_Pos            (8U)  
#define SPI_EVENT_RXB_FULL_Msk            (0x1UL << SPI_EVENT_RXB_FULL_Pos)         /*!< 0x00000100 */
#define SPI_EVENT_RXB_FULL                SPI_EVENT_RXB_FULL_Msk                    /*!< Enable Interrupt to RXB_FULL Event (RX Buffer),rx buffer is full, or all expected data received */
#define SPI_EVENT_TXFIFO_OVR_ERR_Pos      (9U)
#define SPI_EVENT_TXFIFO_OVR_ERR_Msk      (0x1UL << SPI_EVENT_TXFIFO_OVR_ERR_Pos)    /*!< 0x00000200 */
#define SPI_EVENT_TXFIFO_OVR_ERR          SPI_EVENT_TXFIFO_OVR_ERR_Msk               /*!< Enable Interrupt to TXFIFO_OVR_ERR, txfifo overrun */
#define SPI_EVENT_RXFIFO_OVF_ERR_Pos      (10U)
#define SPI_EVENT_RXFIFO_OVF_ERR_Msk      (0x1UL << SPI_EVENT_RXFIFO_OVF_ERR_Pos)    /*!< 0x00000400 */
#define SPI_EVENT_RXFIFO_OVF_ERR          SPI_EVENT_RXFIFO_OVF_ERR_Msk               /*!< Enable Interrupt to RXFIFO_OVF_ERR,rxfifo overflow */
#define SPI_EVENT_TXB_RD_ERR_Pos          (11U)
#define SPI_EVENT_TXB_RD_ERR_Msk          (0x1UL << SPI_EVENT_TXB_RD_ERR_Pos)       /*!< 0x00000800 */
#define SPI_EVENT_TXB_RD_ERR              SPI_EVENT_TXB_RD_ERR_Msk                  /*!< Enable Interrupt to TXB_RD_ERR, tx buffer read error  */
#define SPI_EVENT_RXB_WR_ERR_Pos          (12U)  
#define SPI_EVENT_RXB_WR_ERR_Msk          (0x1UL << SPI_EVENT_RXB_WR_ERR_Pos)       /*!< 0x00001000 */
#define SPI_EVENT_RXB_WR_ERR              SPI_EVENT_RXB_WR_ERR_Msk                  /*!< Enable Interrupt to RXB_WR_ERR, rxbuffer write error  */

typedef struct
{
  __IO uint32_t SPI_EN;       /* Address offset: 0x00      */
  __IO uint32_t SPI_START;    /* Address offset: 0x04      */
  __IO uint32_t SPI_INT_EN;   /* Address offset: 0x08      */
  __IO uint32_t SPI_INT_CLR;  /* Address offset: 0x0C      */
  __IO uint32_t SPI_CFG;      /* Address offset: 0x10      */
  __IO uint32_t SPI_BUF_EN;   /* Address offset: 0x14      */
  __IO uint32_t SPI_TXFIFO;   /* Address offset: 0x18      */
  __I  uint32_t SPI_RXFIFO;   /* Address offset: 0x1C      */
  __IO uint32_t SPI_TRXFIFO;  /* Address offset: 0x20      */
  __IO uint32_t SPI_RXB_RDY;  /* Address offset: 0x24      */
  __IO uint32_t SPI_TXBUF;    /* Address offset: 0x28      */
  __IO uint32_t SPI_RXBUF;    /* Address offset: 0x2C      */
  __IO uint32_t SPI_BUF_SIZE; /* Address offset: 0x30      */
  __I  uint32_t SPI_TRX_ST;   /* Address offset: 0x34      */
  __I  uint32_t SPI_EVENT;    /* Address offset: 0x38      */
  
} stSPI_TypeDef;

#define CS_ALL_MASK           SPI_cs4_sel_Msk|SPI_cs3_sel_Msk|SPI_cs2_sel_Msk|SPI_cs1_sel_Msk|SPI_cs0_sel_Msk       /* PIN mask for ChipSelect Pin */

/** @defgroup SPI Module buf min group; number of byte shall be received before transfer to rx buffer. n: n+1 byte
  * @{
  */
#define spi_buf_min_1byte   ((uint32_t)(0x0UL << SPI_buf_min_bytes_Pos)) //1byte
#define spi_buf_min_2byte   ((uint32_t)(0x1UL << SPI_buf_min_bytes_Pos)) //2byte
#define spi_buf_min_3byte   ((uint32_t)(0x2UL << SPI_buf_min_bytes_Pos)) //3byte
#define spi_buf_min_4byte   ((uint32_t)(0x3UL << SPI_buf_min_bytes_Pos)) //4byte

/** @defgroup SPI Type Select
  * @{
  */
#define SPI_Type_Slave                      ((uint32_t)(0x0UL << SPI_type_Pos))        /*!< Configure to SPI Slave  */
#define SPI_Type_Master                     ((uint32_t)(0x1UL << SPI_type_Pos))       /*!< Configure to SPI Master */

/** @defgroup SPIInitTypeDef Mode Transmission      
  * @{      
  */      
typedef enum
{
  EN_SPI_mode0_phy  =((uint32_t)(0x0UL << SPI_mode_Pos)),  /*!< SPI Mode Used  */
  EN_SPI_mode2_phy  =((uint32_t)(0x2UL << SPI_mode_Pos))   /*!< SPI Mode Used */
}enSPIModephy;      

/** @defgroup SPIInitTypeDef bit order      
  * @{      
  */      
typedef enum
{
  EN_SPI_bitorder_lsb_first_phy  =((uint32_t)(0x0UL << SPI_bitorder_Pos)),   /*!< bit level, lsb first  */
  EN_SPI_bitorder_msb_first_phy  =((uint32_t)(0x1UL << SPI_bitorder_Pos))   /*!< bit level, msb first */
}enSPIBitOrderphy;      

/** @defgroup SPIInitTypeDef byte order Group
  * @{      
  */      
typedef enum
{
  EN_SPI_byteorder_byte0_first_phy = ((uint32_t)(0x0UL << SPI_byteorder_Pos)),  /*!< byte level, byte0 first  */
  EN_SPI_byteorder_byte3_first_phy = ((uint32_t)(0x1UL << SPI_byteorder_Pos))   /*!< byte level, byte3 first */
}enSPIByteOrderphy;

/** @defgroup SPIInitTypeDef SPI Clock Speed Group
  * @{      
  */      
typedef enum
{
  EN_SPI256KHz_phy  = (uint32_t)(0x0UL << SPI_clk_sel_Pos),
  EN_SPI32MHz_phy   = (uint32_t)(0x1UL << SPI_clk_sel_Pos),
  EN_SPI16MHz_phy   = (uint32_t)(0x2UL << SPI_clk_sel_Pos),
  EN_SPI8MHz_phy    = (uint32_t)(0x3UL << SPI_clk_sel_Pos),
  EN_SPI4MHz_phy    = (uint32_t)(0x4UL << SPI_clk_sel_Pos),
  EN_SPI2MHz_phy    = (uint32_t)(0x5UL << SPI_clk_sel_Pos),
  EN_SPI1MHz_phy    = (uint32_t)(0x6UL << SPI_clk_sel_Pos),
  EN_SPI512KHz_phy  = (uint32_t)(0x7UL << SPI_clk_sel_Pos)
}enSPIClockSpeedphy;

/** @defgroup SPI Chip Select Group
  * @{      
  */      
typedef enum
{
  EN_SPI_USE_CS0_phy = (0x1UL << SPI_cs0_sel_Pos),
  EN_SPI_USE_CS1_phy = (0x1UL << SPI_cs1_sel_Pos),
  EN_SPI_USE_CS2_phy = (0x1UL << SPI_cs2_sel_Pos),
  EN_SPI_USE_CS3_phy = (0x1UL << SPI_cs3_sel_Pos),
  EN_SPI_USE_CS4_phy = (0x1UL << SPI_cs4_sel_Pos),
}enSPIChipSelectphy;


// Section: QSPI ____________________________________________________________________
#define QSPI_BASE_ADDR  0x40030000UL

/**
  * @brief  QSPI Handle Structure definition
  */
typedef struct
{                                        
  __IO uint32_t QSPI_EN;                /*Address offset: 0x00 */
  __IO uint32_t QSPI_APB_START;         /*Address offset: 0x04 */
  __IO uint32_t QSPI_APB_CMD_CFG;       /*Address offset: 0x08 */
  __IO uint32_t QSPI_APB_CMD_1;         /*Address offset: 0x0C */
  __IO uint32_t QSPI_APB_CMD_2;         /*Address offset: 0x10 */
  __IO uint32_t QSPI_IBUF_LOAD;         /*Address offset: 0x14 */
  __O  uint32_t QSPI_APB_WDATA;         /*Address offset: 0x18 */
  __I  uint32_t QSPI_APB_RDATA;         /*Address offset: 0x1C */
  __IO uint32_t QSPI_INTERRUPT;         /*Address offset: 0x20 */
  __IO uint32_t QSPI_SETTINGS;          /*Address offset: 0x24 */
  __IO uint32_t QSPI_ACCESS_TYPE;       /*Address offset: 0x28 */
  __I  uint32_t QSPI_STATUS;            /*Address offset: 0x2C */
} stQSPI_HandleTypeDef;

/*****************  Bit definition for QSPI_EN register  *******************/
#define QSPI_clk_en_Pos              (0U)
#define QSPI_clk_en_Msk              (0x1UL << QSPI_clk_en_Pos)             /*!< 0x00000001 */
#define QSPI_clk_en                  QSPI_clk_en_Msk                        /*!< Clock Enable */

/*****************  Bit definition for QSPI_APB_START register  *******************/
#define QSPI_start_cmd_Pos            (0U)
#define QSPI_start_cmd_Msk            (0x1UL << QSPI_start_cmd_Pos)          /*!< 0x00000001 */
#define QSPI_start_cmd                QSPI_stop_cmd_Msk                      /*!< ... */
#define QSPI_stop_cmd_Pos             (0U)        
#define QSPI_stop_cmd_Msk             (0x1UL << QSPI_start_cmd_Pos)          /*!< 0x00000002 */
#define QSPI_stop_cmd                 QSPI_stop_cmd_Msk                      /*!< ... */
#define QSPI_cmd_type_Pos             (2U)        
#define QSPI_cmd_type_Msk             (0x7UL << QSPI_cmd_type_Pos)           /*!< 0x0000001C */
#define QSPI_cmd_type                 QSPI_cmd_type_Msk                      /*!< Indicate the current commandtype */

/*****************  Bit definition for QSPI_APB_CMD_CFG register  *******************/
#define QSPI_cmd_byte_Pos             (0U)
#define QSPI_cmd_byte_Msk             (0xFFUL << QSPI_cmd_byte_Pos)          /*!< 0x000000FF */
#define QSPI_cmd_byte                 QSPI_cmd_byte_Msk                      /*!< Command */
#define QSPI_cmd_mode_Pos             (8U)      
#define QSPI_cmd_mode_Msk             (0x3UL << QSPI_cmd_mode_Pos)           /*!< 0x00000300 */
#define QSPI_cmd_mode                 QSPI_cmd_mode_Msk                      /*!< ... */
#define QSPI_data_mode_Pos            (10U  )  
#define QSPI_data_mode_Msk            (0x3UL << QSPI_data_mode_Pos)          /*!< 0x00000C00 */
#define QSPI_data_mode                QSPI_data_mode_Msk                     /*!< ... */
#define QSPI_addr_mode_Pos            (12U)        
#define QSPI_addr_mode_Msk            (0x3UL << QSPI_addr_mode_Pos)          /*!< 0x00003000 */
#define QSPI_addr_mode                QSPI_addr_mode_Msk                     /*!< ... */
#define QSPI_nbytes_addr_Pos          (16U)        
#define QSPI_nbytes_addr_Msk          (0x7UL << QSPI_nbytes_addr_Pos)        /*!< 0x00070000 */
#define QSPI_nbytes_addr              QSPI_nbytes_addr_Msk                   /*!< ... */
#define QSPI_ncycles_dummy_Pos        (19U)        
#define QSPI_ncycles_dummy_Msk        (0xFUL << QSPI_ncycles_dummy_Pos)      /*!< 0x00780000 */
#define QSPI_ncycles_dummy            QSPI_ncycles_dummy_Msk                 /*!< ... */
#define QSPI_nbytes_data_Pos          (23U)        
#define QSPI_nbytes_data_Msk          (0x1FFUL << QSPI_nbytes_data_Pos)      /*!< 0xFF800000 */
#define QSPI_nbytes_data              QSPI_nbytes_data_Msk                   /*!< ... */

/*****************  Bit definition for QSPI_APB_CMD_1 register  *******************/
#define QSPI_byte_4_Pos               (0U)
#define QSPI_byte_4_Msk               (0xFFUL << QSPI_byte_4_Pos)            /*!< 0x000000FF */
#define QSPI_byte_4                   QSPI_byte_4_Msk                        /*!< ... */
#define QSPI_byte_3_Pos               (8U)  
#define QSPI_byte_3_Msk               (0xFFUL << QSPI_byte_3_Pos)            /*!< 0x0000FF00 */
#define QSPI_byte_3                   QSPI_byte_3_Msk                        /*!< ... */
#define QSPI_byte_2_Pos               (16U)  
#define QSPI_byte_2_Msk               (0xFFUL << QSPI_byte_2_Pos)            /*!< 0x00FF0000 */
#define QSPI_byte_2                   QSPI_byte_2_Msk                        /*!< ... */
#define QSPI_byte_1_Pos               (24U)  
#define QSPI_byte_1_Msk               (0xFFUL << QSPI_byte_1_Pos)            /*!< 0xFF000000 */
#define QSPI_byte_1                   QSPI_byte_1_Msk                        /*!< ... */

/*****************  Bit definition for QSPI_APB_CMD_2 register  *******************/
#define QSPI_byte_5_Pos               (8U)
#define QSPI_byte_5_Msk               (0xFFUL << QSPI_byte_5_Pos)            /*!< 0x0000FF00 */
#define QSPI_byte_5                   QSPI_byte_5_Msk                        /*!< ... */

/*****************  Bit definition for QSPI_IBUF_LOAD register  *******************/
#define QSPI_load_cmd_Pos             (0U)
#define QSPI_load_cmd_Msk             (0x1UL << QSPI_load_cmd_Pos)           /*!< 0x00000001 */
#define QSPI_load_cmd                 QSPI_load_cmd_Msk                      /*!< ... */
#define QSPI_ubuf_nblocks_Pos         (1U)
#define QSPI_ubuf_nblocks_Msk         (0x7FUL << QSPI_ubuf_nblocks_Pos)     /*!< 0x000000FE */
#define QSPI_ubuf_nblocks             QSPI_ubuf_nblocks_Msk                  /*!< ... */
#define QSPI_ubuf_start_addr_Pos      (8U)
#define QSPI_ubuf_start_addr_Msk      (0x7FUL << QSPI_ubuf_start_addr_Pos)   /*!< 0x00007F00 */
#define QSPI_ubuf_start_addr          QSPI_ubuf_start_addr_Msk               /*!< ... */
#define QSPI_flash_start_addr_Pos     (16U)
#define QSPI_flash_start_addr_Msk     (0x3FFFUL << QSPI_flash_start_addr_Pos)  /*!< 0x3FFF0000 */
#define QSPI_flash_start_addr         QSPI_flash_start_addr_Msk               /*!< ... */

/*****************  Bit definition for QSPI_APB_WDATA register  *******************/
#define QSPI_write_data_Pos           (0U)
#define QSPI_write_data_Msk           (0xFFFFFFFFUL << QSPI_write_data_Pos)  /*!< 0xFFFFFFFF */
#define QSPI_write_data               QSPI_write_data_Msk                   /*!< ... */

/*****************  Bit definition for QSPI_APB_RDATA register  *******************/
#define QSPI_read_data_Pos            (0U)
#define QSPI_read_data_Msk            (0xFFFFFFFFUL << QSPI_read_data_Pos)  /*!< 0xFFFFFFFF */
#define QSPI_read_data                QSPI_read_data_Msk                     /*!< ... */

/*****************  Bit definition for QSPI_INTERRUPT register  *******************/
#define QSPI_clear_Pos                (0U)
#define QSPI_clear_Msk                (0x1UL << QSPI_clear_Pos)             /*!< 0x00000001 */
#define QSPI_clear                    QSPI_clear_Msk                        /*!< ... */          
#define QSPI_ibuf_load_en_Pos         (1U)
#define QSPI_ibuf_load_en_Msk         (0x1UL << QSPI_ibuf_load_en_Pos)       /*!< 0x00000002 */
#define QSPI_ibuf_load_en             QSPI_ibuf_load_en_Msk                  /*!< ... */   
#define QSPI_apb_access_en_Pos        (2U)
#define QSPI_apb_access_en_Msk        (0x1UL << QSPI_apb_access_en_Pos)     /*!< 0x00000004 */
#define QSPI_apb_access_en            QSPI_apb_access_en_Msk                /*!< ... */   

/*****************  Bit definition for QSPI_SETTINGS register  *******************/
#define QSPI_ahb_endian_Pos           (0U)
#define QSPI_ahb_endian_Msk           (0x3UL << QSPI_ahb_endian_Pos)         /*!< 0x00000003 */
#define QSPI_ahb_endian               QSPI_ahb_endian_Msk                    /*!< ... */     
#define QSPI_apb_endian_Pos           (2U)    
#define QSPI_apb_endian_Msk           (0x1UL << QSPI_apb_endian_Pos)         /*!< 0x00000004 */
#define QSPI_apb_endian               QSPI_apb_endian_Msk                    /*!< ... */     
#define QSPI_boot_endian_Pos          (3U)    
#define QSPI_boot_endian_Msk          (0x1UL << QSPI_boot_endian_Pos)       /*!< 0x00000008 */
#define QSPI_boot_endian              QSPI_boot_endian_Msk                  /*!< ... */     
#define QSPI_aes_en_Pos               (4U)    
#define QSPI_aes_en_Msk               (0x1UL << QSPI_aes_en_Pos)             /*!< 0x00000010 */
#define QSPI_aes_en                   QSPI_aes_en_Msk                        /*!< ... */    
#define QSPI_aes_en_reg_sel_Pos       (5U)    
#define QSPI_aes_en_reg_sel_Msk       (0x1UL << QSPI_aes_en_reg_sel_Pos)     /*!< 0x00000020 */
#define QSPI_aes_en_reg_sel           QSPI_aes_en_reg_sel_Msk                /*!< ... */    

/*****************  Bit definition for QSPI_ACCESS_TYPE register  *******************/
#define QSPI_code_access_Pos          (0U)
#define QSPI_code_access_Msk          (0x3UL << QSPI_code_access_Pos)       /*!< 0x00000003 */
#define QSPI_code_access              QSPI_code_access_Msk                  /*!< ... */     

/*****************  Bit definition for QSPI_STATUS register  *******************/
#define QSPI_ibuf_load_done_Pos       (0U)
#define QSPI_ibuf_load_done_Msk       (0x1UL << QSPI_ibuf_load_done_Pos)     /*!< 0x00000001 */
#define QSPI_ibuf_load_done           QSPI_ibuf_load_done_Msk                /*!< ... */    
#define QSPI_apb_access_done_Pos      (1U)
#define QSPI_apb_access_done_Msk      (0x1UL << QSPI_apb_access_done_Pos)   /*!< 0x00000002 */
#define QSPI_apb_access_done          QSPI_apb_access_done_Msk              /*!< ... */    
#define QSPI_apb_data_ready_Pos       (2U)
#define QSPI_apb_data_ready_Msk       (0x1UL << QSPI_apb_data_ready_Pos)     /*!< 0x00000004 */
#define QSPI_apb_data_ready_done      QSPI_apb_data_ready_Msk                /*!< ... */   
#define QSPI_flash_code_ready_Pos     (8U)
#define QSPI_flash_code_ready_Msk     (0x1UL << QSPI_flash_code_ready_Pos)  /*!< 0x00000100 */
#define QSPI_flash_code_ready         QSPI_flash_code_ready_Msk             /*!< ... */         
#define QSPI_vendor_info_ready_Pos    (9U)
#define QSPI_vendor_info_ready_Msk    (0x1UL << QSPI_vendor_info_ready_Pos)  /*!< 0x00000200 */
#define QSPI_vendor_info_ready        QSPI_vendor_info_ready_Msk             /*!< ... */         

/** @defgroup QSPI_cmd_type
  * @{
  */
#define QSPI_ReloadUserConfig         ((uint32_t)(0x1UL << QSPI_cmd_type_Pos))          /*!<  */
#define QSPI_FlashDataReadCommand     ((uint32_t)(0x2UL << QSPI_cmd_type_Pos))          /*!<  */
#define QSPI_FlashDataWriteCommand    ((uint32_t)(0x3UL << QSPI_cmd_type_Pos))          /*!<  */
#define QSPI_BurstReadResetCommand    ((uint32_t)(0x4UL << QSPI_cmd_type_Pos))          /*!<  */
#define QSPI_BurstReadSetCommand      ((uint32_t)(0x5UL << QSPI_cmd_type_Pos))          /*!<  */
#define QSPI_FlashRegReadCommand      ((uint32_t)(0x6UL << QSPI_cmd_type_Pos))          /*!<  */
#define QSPI_FlashRegWriteCommand     ((uint32_t)(0x7UL << QSPI_cmd_type_Pos))          /*!<  */

/** @defgroup QSPI_cmd_mode
  * @{
  */
#define QSPI_Cmd_1_LINE               ((uint32_t)(0x0UL << QSPI_cmd_mode_Pos))          /*!< Command in standard-signle line */
#define QSPI_Cmd_2_LINES              ((uint32_t)(0x1UL << QSPI_cmd_mode_Pos))          /*!< Command in dual spi */
#define QSPI_Cmd_4_LINES              ((uint32_t)(0x2UL << QSPI_cmd_mode_Pos))          /*!< Command in quad spi*/

/** @defgroup QSPI_addr_mode
  * @{
  */
#define QSPI_Addr_1_LINE              ((uint32_t)(0x0UL << QSPI_addr_mode_Pos))         /*!< Address in standard-signle line */
#define QSPI_Addr_2_LINES             ((uint32_t)(0x1UL << QSPI_addr_mode_Pos))         /*!< Address in dual spi */
#define QSPI_Addr_4_LINES             ((uint32_t)(0x2UL << QSPI_addr_mode_Pos))         /*!< Address in quad spi*/

/** @defgroup QSPI_data_mode
  * @{
  */
#define QSPI_Data_1_LINE              ((uint32_t)(0x0UL << QSPI_data_mode_Pos))         /*!< Data in standard-signle line */
#define QSPI_Data_2_LINES             ((uint32_t)(0x1UL << QSPI_data_mode_Pos))         /*!< Data in dual spi */
#define QSPI_Data_4_LINES             ((uint32_t)(0x2UL << QSPI_data_mode_Pos))         /*!< Data in quad spi*/

/** @defgroup QSPI_ahb_endian
  * @{
  */
#define QSPI_AHB_Little_Endian        ((uint32_t)(0x0UL << QSPI_ahb_endian_Pos))        /*!< Little Endian on AHB*/
#define QSPI_AHB_Big_Endian_BE8       ((uint32_t)(0x2UL << QSPI_ahb_endian_Pos))        /*!< Big Endian on AHB (byte-invariant big endian(BE8))*/
#define QSPI_AHB_Big_Endian_BE32      ((uint32_t)(0x3UL << QSPI_ahb_endian_Pos))        /*!< Big Endian on AHB (word-invariant big endian(BE32))*/
 
/** @defgroup QSPI_apb_endian
  * @{
  */
#define QSPI_APB_Little_Endian        ((uint32_t)(0x0UL << QSPI_apb_endian_Pos))        /*!< Little Endian on APB*/
#define QSPI_APB_Big_Endian           ((uint32_t)(0x1UL << QSPI_apb_endian_Pos))         /*!< Big Endian on APB*/

/** @defgroup QSPI_boot_endian
  * @{
  */
#define QSPI_boot_Little_Endian       ((uint32_t)(0x0UL << QSPI_boot_endian_Pos))        /*!< Little Endian on boot*/
#define QSPI_boot_Big_Endian          ((uint32_t)(0x1UL << QSPI_boot_endian_Pos))         /*!< Big Endian on boot*/

/** @defgroup QSPI_aes_en
  * @{
  */
#define QSPI_AES_DISABLE              ((uint32_t)(0x0UL << QSPI_aes_en_Pos))            /*!< AES On*/
#define QSPI_AES_ENABLE               ((uint32_t)(0x1UL << QSPI_aes_en_Pos))             /*!< AES Off*/

/** @defgroup QSPI_aes_en_reg_sel
  * @{
  */
#define QSPI_AESSELECT_OTP            ((uint32_t)(0x0UL << QSPI_aes_en_reg_sel_Pos))  /*!< AES On/Off Refer to OTP pin */
#define QSPI_AESSELECT_AES_EN_REG     ((uint32_t)(0x1UL << QSPI_aes_en_reg_sel_Pos))  /*!< AES On/Off Refer to aes_en[4] register */

/** @defgroup QSPI_code_access
  * @{
  */
/** @defgroup QSPI_code_access
  * @{
  */
#define QSPI_Access_VendorConfigArea   ((uint32_t)(0x0UL << QSPI_code_access_Pos))  /*!< Access to Flash Vendor Config Area */
#define QSPI_Access_SystemConfigArea    ((uint32_t)(0x1UL << QSPI_code_access_Pos))  /*!< Access to Flash System Config Area */
#define QSPI_Access_FlashStorageSpace   ((uint32_t)(0x2UL << QSPI_code_access_Pos))  /*!< Access to Flash Storage SpaceArea */

// Section: SCR __________________________________________________________________
#define SCR_BASE_ADDR      (0x40020000)

typedef struct
{
  __IO uint32_t cpu_ctrl;         /* Address offset: 0x00      */
  __IO uint32_t pson_ctrl;        /* Address offset: 0x04      */
  __IO uint32_t rst_ctrl;         /* Address offset: 0x08      */
  __IO uint32_t sleep_mode;       /* Address offset: 0x0C      */
  __I  uint32_t RESERVED_0;       /* Address offset: 0x10      */
  __IO uint32_t RESERVED_1;       /* Address offset: 0x14      */
  __IO uint32_t RESERVED_2;       /* Address offset: 0x18      */
  __IO uint32_t RESERVED_3;       /* Address offset: 0x1C      */
  __IO uint32_t RESERVED_4;       /* Address offset: 0x20      */
  __IO uint32_t RESERVED_5;       /* Address offset: 0x24      */
  __I  uint32_t RESERVED_6;       /* Address offset: 0x28      */
  __IO uint32_t eadc;             /* Address offset: 0x2C      */
  __I  uint32_t eadc_read;        /* Address offset: 0x30      */
  __IO uint32_t xo;               /* Address offset: 0x34      */
  __IO uint32_t dft;              /* Address offset: 0x38      */
  __IO uint32_t ble;              /* Address offset: 0x3C      */
  __IO uint32_t rc32;             /* Address offset: 0x40      */
  __IO uint32_t RESERVED_7;       /* Address offset: 0x44      */
} stSCR_TypeDef;

#define SCR_CPU_CTRL_MASK_icache_refresh           0x08000000UL  // bit 27
#define SCR_CPU_CTRL_MASK_icache_bypass            0x04000000UL  // bit 26
#define SCR_CPU_CTRL_MASK_ibuf_reload              0x00010000UL  // bit 16
#define SCR_CPU_CTRL_MASK_skip_init_ibuf           0x00002000UL  // bit 13
#define SCR_CPU_CTRL_MASK_pson_wic_in_dsleep       0x00001000UL  // bit 12
#define SCR_CPU_CTRL_MASK_pson_ram_in_dsleep       0x00000800UL  // bit 11
#define SCR_CPU_CTRL_MASK_pson_ibuf_in_dsleep      0x00000400UL  // bit 10
#define SCR_CPU_CTRL_MASK_pson_icache_in_dsleep    0x00000200UL  // bit 9

#define SCR_CPU_CTRL_MASK_ALL_pson_in_dsleep       0x00001E00UL  // MASK ALL pson_in_dsleep

#define SCR_RST_CTRL_MASK_DMA            0x01000000UL  // bit 24
#define SCR_RST_CTRL_MASK_BIST           0x00800000UL  // bit 23
#define SCR_RST_CTRL_MASK_EFUSE          0x00400000UL  // bit 22
#define SCR_RST_CTRL_MASK_TIMER3         0x00100000UL  // bit 20
#define SCR_RST_CTRL_MASK_TIMER2         0x00080000UL  // bit 19
#define SCR_RST_CTRL_MASK_TIMER1         0x00040000UL  // bit 18
#define SCR_RST_CTRL_MASK_TIMER0         0x00020000UL  // bit 17
#define SCR_RST_CTRL_MASK_GPIO           0x00010000UL  // bit 16
#define SCR_RST_CTRL_MASK_I2C            0x00008000UL  // bit 15
#define SCR_RST_CTRL_MASK_SPI            0x00004000UL  // bit 14
#define SCR_RST_CTRL_MASK_UART1          0x00002000UL  // bit 13
#define SCR_RST_CTRL_MASK_UART0          0x00001000UL  // bit 12
#define SCR_RST_CTRL_MASK_CRC            0x00000800UL  // bit 11
#define SCR_RST_CTRL_MASK_TRNG           0x00000400UL  // bit 10
#define SCR_RST_CTRL_MASK_PKA            0x00000200UL  // bit  9
#define SCR_RST_CTRL_MASK_CRYPTO         0x00000100UL  // bit  8
#define SCR_RST_CTRL_MASK_BLE            0x00000010UL  // bit  4
#define SCR_RST_CTRL_MASK_UWB            0x00000001UL  // bit  0

#define SCR_PSON_ACK_MASK_BLE_SEC        0x02000000  // bit 25
#define SCR_PSON_ACK_MASK_BLE_CTRL       0x01000000  // bit 24
#define SCR_PSON_ACK_MASK_BLE_PHY        0x00800000  // bit 23
#define SCR_PSON_ACK_MASK_DMA            0x00400000  // bit 22
#define SCR_PSON_ACK_MASK_TRNG           0x00200000  // bit 21
#define SCR_PSON_ACK_MASK_PKA            0x00100000  // bit 20
#define SCR_PSON_ACK_MASK_CRYPTO         0x00080000  // bit 19
#define SCR_PSON_ACK_MASK_BLE            0x00040000  // bit 18
#define SCR_PSON_ACK_MASK_UWB            0x00020000  // bit 17
#define SCR_PSON_ACK_MASK_M33            0x00010000  // bit 16
#define SCR_PSON_CTRL_MASK_DMA           0x00000400  // bit 10
#define SCR_PSON_CTRL_MASK_TRNG          0x00000020  // bit  5
#define SCR_PSON_CTRL_MASK_PKA           0x00000010  // bit  4
#define SCR_PSON_CTRL_MASK_CRYPTO        0x00000008  // bit  3
#define SCR_PSON_CTRL_MASK_BLE           0x00000004  // bit  2
#define SCR_PSON_CTRL_MASK_UWB           0x00000002  // bit  1
#define SCR_PSON_CTRL_MASK_M33_EN        0x00000001  // bit  0

#define SCR_BLE_AHBFIXEDEN_MASK          0x00000020  // bit  5

/*****************  Bit definition for sleep_mode register     *******************/
#define SCR_forcesleep_Pos             (0U)
#define SCR_forcesleep_Msk             (0x1UL << SCR_forcesleep_Pos)           /*!< 0x00000001 */
#define SCR_forcesleep                 SCR_forcesleep_Msk                      /*!< Force to deepsleep or powerdown mode*/
#define SCR_sleeptime_Pos              (1U)
#define SCR_sleeptime_Msk              (0xFFFFFUL << SCR_sleeptime_Pos)       /*!< 0x001FFFFE */
#define SCR_sleeptime                  SCR_sleeptime_Msk                      /*!< Sleep Time in ms unit, maximum 1024seconds.*/


/**************      Bit definition for SCR_RC32 register    ************************/
#define SCR_RC32_cal_bypass_en_pos        (2U)
#define SCR_RC32_cal_bypass_en_msk        (0x1UL<<SCR_RC32_cal_bypass_en_pos) 
#define SCR_RC32_cal_bypass_en            SCR_RC32_cal_bypass_en_msk
#define SCR_RC32_cal_bypass_code_pos      (3U)
#define SCR_RC32_cal_bypass_code_msk      (0xFFUL<<SCR_RC32_cal_bypass_code_pos)
#define SCR_RC32_cal_bypass_code          SCR_RC32_cal_bypass_code_msk
#define SCR_RC32_idc_rc32k_pos            (24U)
#define SCR_RC32_idc_rc32k_msk            (0xFUL<<SCR_RC32_idc_rc32k_pos)
#define SCR_RC32_idc_rc32k          SCR_RC32_idc_rc32k_msk


/**************      Bit definition for SCR_XO register    ************************/
#define SCR_en_xo_ble_pos                  (9U)
#define SCR_en_xo_ble_msk                  (0x1UL<<SCR_en_xo_ble_pos) 
#define SCR_en_xo_ble                      SCR_en_xo_ble_msk


/************************ Bit definition for EADC ***************************************/
#define SCR_EADC_VIN_SEL_DFT_POS          (29U)
#define SCR_EADC_VIN_SEL_DFT_MSK          (0x1UL << SCR_EADC_VIN_SEL_DFT_POS)
#define SCR_EADC_VIN_SEL_DFT              SCR_EADC_VIN_SEL_DFT_MSK
#define SCR_EADC_MANUAL_CLK_POS           (28U)
#define SCR_EADC_MANUAL_CLK_MSK           (0x1UL << SCR_EADC_MANUAL_CLK_POS)
#define SCR_EADC_MANUAL_CLK               (SCR_EADC_MANUAL_CLK_MSK)
#define SCR_EADC_CLK_SEL_POS              (25U)
#define SCR_EADC_CLK_SEL_MSK              (0x7UL << SCR_EADC_CLK_SEL_POS)
#define SCR_EADC_VIN_MID_POS              (18U)
#define SCR_EADC_VIN_MID_MSK              (0x3FUL << SCR_EADC_VIN_MID_POS)
#define SCR_EADC_VIN_MID_CAL_POS          (13U)
#define SCR_EADC_VIN_MID_CAL_MSK          (0x1FUL << SCR_EADC_VIN_MID_CAL_POS)
#define SCR_EADC_OUTPUT_SIGN_POS          (12U)
#define SCR_EADC_OUTPUT_SIGN_MSK          (0x1UL << SCR_EADC_OUTPUT_SIGN_POS)
#define SCR_EADC_GAIN_POS                 (9U)
#define SCR_EADC_GAIN_MSK                 (0x7UL << SCR_EADC_GAIN_POS)  

/************************ Bit definition for DFT **************************************/
#define SCR_ANALOG_DFT_TOP_BUF_BIAS_POS   (12U)
#define SCR_ANALOG_DFT_TOP_BUF_BIAS_MSK   (0x7UL << SCR_ANALOG_DFT_TOP_BUF_BIAS_POS)
#define SCR_ANALOG_DFT_TOP_BUF_EN_POS     (11U)
#define SCR_ANALOG_DFT_TOP_BUF_EN_MSK     (0x1UL << SCR_ANALOG_DFT_TOP_BUF_EN_POS)
#define SCR_ANALOG_DFT_TOP_MUX_POS        (6U)
#define SCR_ANALOG_DFT_TOP_MUX_MSK        (0x1FUL << SCR_ANALOG_DFT_TOP_MUX_POS)
#define SCR_ANALOG_DFT_TOP_EN_POS         (5U)
#define SCR_ANALOG_DFT_TOP_EN_MSK         (0x1UL << SCR_ANALOG_DFT_TOP_EN_POS)

#endif // __CB_NONLIB_PERIPHERAL_PHY_ADDRESS_DATABASE_H
