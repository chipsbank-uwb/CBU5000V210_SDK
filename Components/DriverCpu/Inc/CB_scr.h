/**
 * @file    CB_scr.h
 * @brief   Header file for System Control Registers (SCR) configuration.
 * @details This file contains the declarations of functions and structures
 *          used for configuring System Control Registers (SCR) settings on
 *          the microcontroller.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef INC_SCR_H
#define INC_SCR_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum 
{
  EN_EADC_GAIN_0 = 0,
  EN_EADC_GAIN_1 = 1,
  EN_EADC_GAIN_2 = 2,
  EN_EADC_GAIN_3 = 3,
  EN_EADC_GAIN_4 = 4, 
  EN_EADC_GAIN_5 = 5,   
} enEADCGain;

typedef enum 
{
  EN_EADC_DFT_MEASURE_GND = 0,
  EN_EADC_DFT_MEASURE_VBG = 1,    // measuring EADC bias voltage Vbg
} enEADCDFTMeasure;

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
 * @brief Configure RC code to be stablize for wake up from deepsleep.
 */
void cb_scr_stabilize_rc(void);

/**
 * @brief Turns on the GPIO module.
 */
void cb_scr_gpio_module_on(void);

/**
 * @brief Turns off the GPIO module.
 */
void cb_scr_gpio_module_off(void);

/**
 * @brief Turns on the CRC module.
 */
void cb_scr_crc_module_on(void);

/**
 * @brief Turns off the CRC module.
 */
void cb_scr_crc_module_off(void);

/**
 * @brief Turns on the SPI module.
 */
void cb_scr_spi_module_on(void);

/**
 * @brief Turns off the SPI module.
 */
void cb_scr_spi_module_off(void);

/**
 * @brief Turns on UART0 module.
 */
void cb_scr_uart0_module_on(void);

/**
 * @brief Turns off UART0 module.
 */
void cb_scr_uart0_module_off(void);

/**
 * @brief Turns on UART1 module.
 */
void cb_scr_uart1_module_on(void);

/**
 * @brief Turns off UART1 module.
 */
void cb_scr_uart1_module_off(void);

/**
 * @brief Turns on Timer0 module.
 */
void cb_scr_timer0_module_on(void);

/**
 * @brief Turns off Timer0 module.
 */
void cb_scr_timer0_module_off(void);

/**
 * @brief Turns on Timer1 module.
 */
void cb_scr_timer1_module_on(void);

/**
 * @brief Turns off Timer1 module.
 */
void cb_scr_timer1_module_off(void);

/**
 * @brief Turns on Timer2 module.
 */
void cb_scr_timer2_module_on(void);

/**
 * @brief Turns off Timer2 module.
 */
void cb_scr_timer2_module_off(void);

/**
 * @brief Turns on Timer3 module.
 */
void cb_scr_timer3_module_on(void);

/**
 * @brief Turns off Timer3 module.
 */
void cb_scr_timer3_module_off(void);

/**
 * @brief Turns on the PKAModule.
 */
void cb_scr_pka_module_on(void);

/**
 * @brief Turns off the PKAModule.
 */
void cb_scr_pka_module_off(void);

/**
 * @brief Turns on the TRNG module.
 */
void cb_scr_trng_module_on(void);

/**
 * @brief Turns off the TRNG module.
 */
void cb_scr_trng_module_off(void);

/**
 * @brief Turns on the BLE module.
 */
void cb_scr_ble_module_on(void);

/**
 * @brief Turns off the BLE module.
 */
void cb_scr_ble_module_off(void);

/**
 * @brief Turns on the UWB module.
 */
void cb_scr_uwb_module_on(void);

/**
 * @brief Turns off the UWB module.
 */
void cb_scr_uwb_module_off(void);

/**
 * @brief Turns on the I2C module.
 */
void cb_scr_i2c_module_on(void);

/**
 * @brief Turns off the I2C module.
 */
void cb_scr_i2c_module_off(void);

/**
 * @brief Turns on the Crypto module.
 */
void cb_scr_crypto_module_on(void);

/**
 * @brief Turns off the Crypto module.
 */
void cb_scr_crypto_module_off(void);

/**
 * @brief Turns on the DMA module.
 */
void cb_scr_dma_module_on(void);

/**
 * @brief Turns off the DMA module.
 */
void cb_scr_dma_module_off(void);

/**
 * @brief Turns on the EFUSE module.
 */
void cb_scr_efuse_module_on(void);

/**
 * @brief Turns off the EFUSE module.
 */
void cb_scr_efuse_module_off(void);

/**
 * @brief Sets up the DFT for the EADC (Enhanced Analog-to-Digital Converter).
 *
 * @param MeasureType Type of measurement for the DFT.
 */
void cb_scr_eadc_setup_dft(enEADCDFTMeasure MeasureType);

/**
 * @brief Measures the DFT using EADC.
 *
 * @param MeasureType Type of DFT measurement.
 * @param gain Gain configuration for the EADC.
 * @return DFT measurement value.
 */
uint16_t cb_scr_eadc_measure_dft(enEADCDFTMeasure MeasureType, enEADCGain gain);

/**
 * @brief Measures the analog input using EADC.
 *
 * @param gain Gain configuration for the EADC.
 * @return Measured analog input value.
 */
uint16_t cb_scr_eadc_measure_ain(enEADCGain gain);

/**
 * @brief		Wrapper function to disable/turn off CPU peripherals
 */
void cb_scr_disable_peripherals(void);

#endif /*INC_SCR_H*/

