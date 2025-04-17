/**
 * @file    CB_iomux.h
 * @brief   Header file for IOMUX (Input/Output Multiplexer) configuration.
 * @details This file contains function prototypes and definitions for configuring
 *          IOMUX settings, including GPIO modes, alternate functions, and event lists.
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef INC_IOMUX_H
#define INC_IOMUX_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define IOMUX_GPIOMODE          0x00000000UL  // mode_sel 00:GPIO src_sel:Don't Care

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief Enumeration defining IOMUX GPIO select options.
 */
typedef enum
{
  EN_IOMUX_GPIO_0 = 0,  /**< IOMUX GPIO 0 */
  EN_IOMUX_GPIO_1,      /**< IOMUX GPIO 1 */
  EN_IOMUX_GPIO_2,      /**< IOMUX GPIO 2 */
  EN_IOMUX_GPIO_3,      /**< IOMUX GPIO 3 */
  EN_IOMUX_GPIO_4,      /**< IOMUX GPIO 4 */
  EN_IOMUX_GPIO_5,      /**< IOMUX GPIO 5 */
  EN_IOMUX_GPIO_6,      /**< IOMUX GPIO 6 */
  EN_IOMUX_GPIO_7,      /**< IOMUX GPIO 7 */
  EN_IOMUX_GPIO_8,      /**< IOMUX GPIO 8 */
  EN_IOMUX_GPIO_9,      /**< IOMUX GPIO 9 */
  EN_IOMUX_GPIO_10,     /**< IOMUX GPIO 10 */
  EN_IOMUX_GPIO_11,     /**< IOMUX GPIO 11 */  
} enIomuxGpioSelect;

/**
 * @brief Enumeration defining IOMUX GPIO mode options.
 */
typedef enum
{
  EN_IOMUX_GPIO_MODE_GPIO = 0,            /**< GPIO mode */
  EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS,     /**< SOC peripherals mode */
  EN_IOMUX_GPIO_MODE_EVENT_LIST_0_OUTPUT, /**< Event list 0 output mode */
  EN_IOMUX_GPIO_MODE_EVENT_LIST_1_OUTPUT  /**< Event list 1 output mode */
} enIomuxGpioMode;

/**
 * @brief Enumeration defining alternate functions for IOMUX GPIO pins.
 */
typedef enum
{
  EN_IOMUX_GPIO_AF_SPIM_CS0 = 0,    /**< SPIM (SPI Master) CS0   */
  EN_IOMUX_GPIO_AF_SPIM_CLK,        /**< SPIM (SPI Master) CLK   */
  EN_IOMUX_GPIO_AF_SPIM_MOSI,       /**< SPIM (SPI Master) MOSI  */
  EN_IOMUX_GPIO_AF_SPIM_MISO,       /**< SPIM (SPI Master) MISO  */
  EN_IOMUX_GPIO_AF_SPIM_CS1,        /**< SPIM (SPI Master) CS1   */
  EN_IOMUX_GPIO_AF_SPIM_CS2,        /**< SPIM (SPI Master) CS2   */
  EN_IOMUX_GPIO_AF_SPIM_CS3,        /**< SPIM (SPI Master) CS3   */
  EN_IOMUX_GPIO_AF_SPIM_CS4,        /**< SPIM (SPI Master) CS4   */
  EN_IOMUX_GPIO_AF_SPIS_CS,         /**< SPIS (SPI Slave) CS     */
  EN_IOMUX_GPIO_AF_SPIS_CLK,        /**< SPIS (SPI Slave) CLK    */
  EN_IOMUX_GPIO_AF_SPIS_MOSI,       /**< SPIS (SPI Slave) MOSI   */
  EN_IOMUX_GPIO_AF_SPIS_MISO,       /**< SPIS (SPI Slave) MISO   */
  EN_IOMUX_GPIO_AF_UART0_TXD,       /**< UART0 TXD               */
  EN_IOMUX_GPIO_AF_UART0_RXD,       /**< UART0 RXD               */
  EN_IOMUX_GPIO_AF_UART0_CTSN,      /**< UART0 CTSN              */
  EN_IOMUX_GPIO_AF_UART0_RTSN,      /**< UART0 RTSN              */
  EN_IOMUX_GPIO_AF_UART1_TXD,       /**< UART1 TXD               */
  EN_IOMUX_GPIO_AF_UART1_RXD,       /**< UART1 RXD               */ 
  EN_IOMUX_GPIO_AF_UART1_CTSN,      /**< UART1 CTSN              */
  EN_IOMUX_GPIO_AF_UART1_RTSN,      /**< UART1 RTSN              */
  EN_IOMUX_GPIO_AF_I2C_SCK,         /**< I2C SCK                 */   
  EN_IOMUX_GPIO_AF_I2C_SDA,         /**< I2C SDA                 */
  EN_IOMUX_GPIO_AF_EXT_IRQ_I0,      /**< External IRQ I0         */
  EN_IOMUX_GPIO_AF_EXT_IRQ_I1,      /**< External IRQ I1         */
  EN_IOMUX_GPIO_AF_UWB_I0,          /**< UWB I0                  */    
  EN_IOMUX_GPIO_AF_UWB_I1,          /**< UWB I1                  */    
  EN_IOMUX_GPIO_AF_UWB_I2,          /**< UWB I2                  */    
  EN_IOMUX_GPIO_AF_UWB_I3           /**< UWB I3                  */
} enIomuxGpioAF;

/**
 * @brief Enumeration defining events for Event List 0 output.
 */
typedef enum
{
  EN_TZ=0,                                /**< TZ event */
  EN_QSPI,                                /**< QSPI event */
  EN_DMA,                                 /**< DMA event */
  EN_SPA,                                 /**< SPA event */
  EN_PKA,                                 /**< PKA event */
  EN_TRNG,                                /**< TRNG event */
  EN_CRC,                                 /**< CRC event */
  EN_GPIO,                                /**< GPIO event */
  EN_SPIMS,                               /**< SPIMS event */
  EN_UART0,                               /**< UART0 event */
  EN_UART1,                               /**< UART1 event */
  EN_I2C,                                 /**< I2C event */
  EN_TIMER0,                              /**< Timer0 event */
  EN_TIMER1,                              /**< Timer1 event */
  EN_TIMER2,                              /**< Timer2 event */
  EN_TIMER3,                              /**< Timer3 event */
  EN_EXT_IRQ_IN_SYN0,                     /**< External IRQ in SYN0 event */
  EN_EXT_IRQ_IN_SYN1,                     /**< External IRQ in SYN1 event */
  EN_BLE,                                 /**< BLE event */
  EN_EVENT_IRQ_SYN,                       /**< Event IRQ SYN event */
  EN_UWB_DSR_RX_BUFFER_OVERFLOW_EVENT,    /**< UWB DSR RX buffer overflow event */
  EN_UWB_RX_RX0_AGC_EVENT,                /**< UWB RX0 AGC event */
  EN_UWB_RX_RX0_PD_DONE,                  /**< UWB RX0 PD done event */
  EN_UWB_RX_RX0_SFD_DET_DONE,             /**< UWB RX0 SFD detection done event */
  EN_UWB_RX_RX1_AGC_EVENT,                /**< UWB RX1 AGC event */
  EN_UWB_RX_RX1_PD_DONE,                  /**< UWB RX1 PD done event */
  EN_UWB_RX_RX1_SFD_DET_DONE,             /**< UWB RX1 SFD detection done event */
  EN_UWB_RX_RX2_AGC_EVENT,                /**< UWB RX2 AGC event */
  EN_UWB_RX_RX2_PD_DONE,                  /**< UWB RX2 PD done event */
  EN_UWB_RX_RX2_SFD_DET_DONE,             /**< UWB RX2 SFD detection done event */
  EN_UWB_RX_RX_STS_CIR_END,               /**< UWB RX status CIR end event */
  EN_UWB_RX_RX_PHY_PHR_DONE,              /**< UWB RX PHY PHR done event */
  EN_UWB_RX_RX_PHY_EVENT,                 /**< UWB RX PHY event */
  EN_UWB_TX_TX_DONE,                      /**< UWB TX done event */
  EN_UWB_TX_SFD_MARK,                     /**< UWB TX SFD mark event */
  EN_UWB_GP_GENERAL_PURPOSE_EVENT0,       /**< UWB general purpose event 0 */
  EN_UWB_GP_GENERAL_PURPOSE_EVENT1,       /**< UWB general purpose event 1 */
  EN_UWB_GP_GENERAL_PURPOSE_EVENT2,       /**< UWB general purpose event 2 */
  EN_UWB_GP_GENERAL_PURPOSE_EVENT3,       /**< UWB general purpose event 3 */
  EN_TIMER0_TOE0,//TIMER 39               /**< Timer0 TOE0 event */
  EN_TIMER0_TOE1,                         /**< Timer0 TOE1 event */
  EN_TIMER0_TOE2,                         /**< Timer0 TOE2 event */
  EN_TIMER0_TOE3,                         /**< Timer0 TOE3 event */
  EN_TIMER0_TOE_COM,                      /**< Timer0 TOE common event */
  EN_TIMER1_TOE0,                         /**< Timer1 TOE0 event */
  EN_TIMER1_TOE1,                         /**< Timer1 TOE1 event */
  EN_TIMER1_TOE2,                         /**< Timer1 TOE2 event */
  EN_TIMER1_TOE3,                         /**< Timer1 TOE3 event */
  EN_TIMER1_TOE_COM,                      /**< Timer1 TOE common event */
  EN_TIMER2_TOE0,                         /**< Timer2 TOE0 event */
  EN_TIMER2_TOE1,                         /**< Timer2 TOE1 event */
  EN_TIMER2_TOE2,                         /**< Timer2 TOE2 event */
  EN_TIMER2_TOE3,                         /**< Timer2 TOE3 event */
  EN_TIMER2_TOE_COM,                      /**< Timer2 TOE common event */
  EN_TIMER3_TOE0,                         /**< Timer3 TOE0 event */
  EN_TIMER3_TOE1,                         /**< Timer3 TOE1 event */
  EN_TIMER3_TOE2,                         /**< Timer3 TOE2 event */
  EN_TIMER3_TOE3,                         /**< Timer3 TOE3 event */
  EN_TIMER3_TOE_COM,                      /**< Timer3 TOE common event */
  EN_GPIO_TRIG_VAL0,                      /**< GPIO trigger value 0 */
  EN_GPIO_TRIG_VAL1,                      /**< GPIO trigger value 1 */
  EN_GPIO_TRIG_VAL2,                      /**< GPIO trigger value 2 */
  EN_GPIO_TRIG_VAL3,                      /**< GPIO trigger value 3 */
  EN_GPIO_TRIG_VAL4,                      /**< GPIO trigger value 4 */
}enEventList0Output;

/**
 * @brief Enumeration defining events for Event List 1.
 */
typedef enum
{
  EN_GPIO_TRIG_VAL5,      /**< GPIO trigger value 5 */
  EN_GPIO_TRIG_VAL6,      /**< GPIO trigger value 6 */
  EN_GPIO_TRIG_VAL7,      /**< GPIO trigger value 7 */
  EN_GPIO_TRIG_VAL8,      /**< GPIO trigger value 8 */
  EN_GPIO_TRIG_VAL9,      /**< GPIO trigger value 9 */
  EN_GPIO_TRIG_VAL10,     /**< GPIO trigger value 10 */
  EN_GPIO_TRIG_VAL11,     /**< GPIO trigger value 11 */
  EN_DMA_CH0_READY,       /**< DMA channel 0 ready */
  EN_DMA_CH1_READY,       /**< DMA channel 1 ready */
  EN_DMA_CH2_READY,       /**< DMA channel 2 ready */
  EN_DMA_CH3_READY,       /**< DMA channel 3 ready */
  //need to fill up [115:75] uwb event 
}enEventList1;

/**
 * @brief Union for selecting different sources.
 */
typedef union
{
  enIomuxGpioAF      IomuxGpioAF;   /**< IOMUX GPIO alternate function */
  enEventList0Output EventList0;    /**< Event List 0 output */
  enEventList1       EventList1;    /**< Event List 1 */
  uint8_t            SrcSel;        /**< Source selector */
}unSourceSel;

/**
 * @brief Structure defining IOMUX GPIO mode and source selection.
 */
typedef struct
{
  enIomuxGpioMode IomuxGpioMode;    /**< IOMUX GPIO mode */
  uint8_t         SourceSel;        /**< Source selection */
}stIomuxGpioMode;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief Union representing GPIO register bits.
 */
typedef union
{
  struct
  {
    uint8_t srcSelect:6;    /**< Source select bits */
    uint8_t reserved1:2;    /**< Reserved bits       */
    uint8_t modeSelect:2;   /**< Mode select bits   */
    uint8_t reserved2:6;    /**< Reserved bits       */
    uint8_t reserved3;      /**< Reserved byte       */
    uint8_t reserved4;      /**< Reserved byte       */
  }bit;                     /**< Bit-field representation */
  uint32_t ui32Byte;        /**< Byte representation       */
}unGpioReg;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Configures the I/O Multiplexing (IOMUX) settings for a GPIO pin.
 * 
 * This function configures the IOMUX settings for a specified GPIO pin based on
 * the provided configuration parameters.
 * 
 * @param[in]   enGpio      The GPIO pin to configure.
 * @param[in]   GpioModeSet Pointer to the structure containing GPIO mode and source select settings.
 */
void cb_iomux_config(enIomuxGpioSelect enGpio,stIomuxGpioMode* GpioModeSet);

#endif /*INC_IOMUX_H*/
