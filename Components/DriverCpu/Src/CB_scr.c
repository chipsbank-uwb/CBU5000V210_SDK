/**
 * @file    CB_scr.c
 * @brief   Implementation of System Control Registers (SCR) configuration functions.
 * @details This file contains the implementation of functions for configuring
 *          System Control Registers (SCR) settings on the microcontroller.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_scr.h"
#include "CB_wdt.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include "NonLIB_sharedUtils.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define SCR_PSON_CTRL_ACK_ENABLE              CB_TRUE
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

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
static stSCR_TypeDef  *pSCR  = (stSCR_TypeDef* ) SCR_BASE_ADDR;

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Configure RC code to be stablize for wake up from deepsleep.
 */
void cb_scr_stabilize_rc(void)
{
    #define DEF_CAL_BYPASS_CODE_VALUE 0x7FUL //CalCode 0x7F for tested wake up from deep sleep. Default value is 0x68.
    #define DEF_IDC_VALUE 0xFUL //IDC value.

    /*Adjust Rc Clock to more stable & accurate*/
    pSCR->rc32 &= ~(SCR_RC32_idc_rc32k|SCR_RC32_cal_bypass_code);
    cb_hal_delay_in_us(40);// Wait for at least 20 microseconds to ensure that the register bit is cleared.
    pSCR->rc32 |=(((DEF_IDC_VALUE<<SCR_RC32_idc_rc32k_pos)&SCR_RC32_idc_rc32k_msk)|((DEF_CAL_BYPASS_CODE_VALUE<<SCR_RC32_cal_bypass_code_pos)&SCR_RC32_cal_bypass_code_msk));
    cb_hal_delay_in_us(40);
}

/**
 * @brief   Turns on the GPIO module by releasing its reset.
 */
void cb_scr_gpio_module_on(void)
{
    /*Reset Release for GPIO Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_GPIO;    // rst_gpio - bit 16
}

/**
 * @brief   Turns off the GPIO module by resetting it.
 */
void cb_scr_gpio_module_off(void)
{
    /*Reset for GPIO Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_GPIO);    // rst_gpio - bit 16
}

/**
 * @brief   Turns on the CRC module by releasing its reset.
 */
void cb_scr_crc_module_on(void)
{
    /*Reset Release for CRC Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_CRC;    // rst_crc - bit 11
}

/**
 * @brief   Turns off the CRC module by resetting it.
 */
void cb_scr_crc_module_off(void)
{
    /*Reset for CRC Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_CRC);    // rst_crc - bit 11
}

/**
 * @brief   Turns on the SPI module by releasing its reset.
 */
void cb_scr_spi_module_on(void)
{
    /*Reset Release for SPI Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_SPI;
}

/**
 * @brief   Turns off the SPI module by resetting it.
 */
void cb_scr_spi_module_off(void)
{
    /*Reset for SPI Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_SPI);
}

/**
 * @brief   Turns on the UART0 module by releasing its reset.
 */
void cb_scr_uart0_module_on(void)
{
    /*Reset Release for UART0 Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_UART0;
}

/**
 * @brief   Turns off the UART0 module by resetting it.
 */
void cb_scr_uart0_module_off(void)
{
    /*Reset for UART0 Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_UART0);
}

/**
 * @brief   Turns on the UART1 module by releasing its reset.
 */
void cb_scr_uart1_module_on(void)
{
    /*Reset Release for UART0 Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_UART1;
}

/**
 * @brief   Turns off the UART1 module by resetting it.
 */
void cb_scr_uart1_module_off(void)
{
    /*Reset for UART0 Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_UART1);
}

/**
 * @brief   Turns on the I2C module by releasing its reset.
 */
void cb_scr_i2c_module_on(void)
{
    /*Reset Release for I2C Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_I2C;
}

/**
 * @brief   Turns off the I2C module by resetting it.
 */
void cb_scr_i2c_module_off(void)
{
    /*Reset for I2C Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_I2C);
}

/**
 * @brief   Turns on the CRYPTO module by releasing its reset.
 */
void cb_scr_crypto_module_on(void)
{
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)
    /*To add CryptoModule Power On*/
    pSCR->pson_ctrl|=SCR_PSON_CTRL_MASK_CRYPTO;
    /*wait for the Crypto ACK to be acknowledged*/
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_CRYPTO)!=SCR_PSON_ACK_MASK_CRYPTO);
#endif  
    /*Reset Release for Crypto Module */
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_CRYPTO;
}

/**
 * @brief   Turns off the CRYPTO module by resetting it.
 */
void cb_scr_crypto_module_off(void)
{
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)
    /*To add CryptoModule Power Off*/
    pSCR->pson_ctrl&=(uint32_t)(~(SCR_PSON_CTRL_MASK_CRYPTO));
    /*wait for the Crypto ACK to be acknowledged*/
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_CRYPTO)==SCR_PSON_ACK_MASK_CRYPTO);
#endif
    /*Reset for Crypto Module */
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_CRYPTO);
}

/**
 * @brief   Turns on the TIMER0 module by releasing its reset.
 */
void cb_scr_timer0_module_on(void)
{
    /*Reset Release for Timer0 Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_TIMER0;
}

/**
 * @brief   Turns off the TIMER0 module by resetting it.
 */
void cb_scr_timer0_module_off(void)
{
    /*Reset Release for Timer0 Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_TIMER0);
}

/**
 * @brief   Turns on the TIMER1 module by releasing its reset.
 */
void cb_scr_timer1_module_on(void)
{  
    /*Reset Release for Timer1 Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_TIMER1;
}

/**
 * @brief   Turns off the TIMER1 module by resetting it.
 */
void cb_scr_timer1_module_off(void)
{  
    /*Reset Release for Timer1 Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_TIMER1);
}

/**
 * @brief   Turns on the TIMER2 module by releasing its reset.
 */
void cb_scr_timer2_module_on(void)
{
    /*Reset Release for Timer2 Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_TIMER2;
}

/**
 * @brief   Turns off the TIMER2 module by resetting it.
 */
void cb_scr_timer2_module_off(void)
{
    /*Reset Release for Timer2 Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_TIMER2);
}

/**
 * @brief   Turns on the TIMER3 module by releasing its reset.
 */
void cb_scr_timer3_module_on(void)
{  
    /*Reset Release for Timer3 Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_TIMER3;
}

/**
 * @brief   Turns off the TIMER3 module by resetting it.
 */
void cb_scr_timer3_module_off(void)
{
    /*Reset Release for Timer3 Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_TIMER3);
}

/**
 * @brief   Turns on the PLA module by releasing its reset.
 */
void cb_scr_pka_module_on(void) 
{
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)
    pSCR->pson_ctrl|= SCR_PSON_CTRL_MASK_PKA;
    /*wait for the pka ACK to be acknowledged*/
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_PKA)!=SCR_PSON_ACK_MASK_PKA);
#endif
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_PKA; 
}

/**
 * @brief   Turns off the PKA module by resetting it.
 */
void cb_scr_pka_module_off(void) 
{
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)
    pSCR->pson_ctrl&= (uint32_t)(~(SCR_PSON_CTRL_MASK_PKA));
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_PKA)==SCR_PSON_ACK_MASK_PKA);
#endif  
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_PKA);
}

/**
 * @brief   Turns on the TRNG module by releasing its reset.
 */
void cb_scr_trng_module_on(void) 
{
    pSCR->pson_ctrl |= SCR_PSON_CTRL_MASK_TRNG;
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)
    /*wait for the Trng ACK to be acknowledged*/
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_TRNG)!=SCR_PSON_ACK_MASK_TRNG);
#endif 
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_TRNG;    
}

/**
 * @brief   Turns off the TRNG module by resetting it.
 */
void cb_scr_trng_module_off(void)
{
    pSCR->pson_ctrl&= (uint32_t)(~(SCR_PSON_CTRL_MASK_TRNG));
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)  
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_TRNG)==SCR_PSON_ACK_MASK_TRNG);
#endif  
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_TRNG);
}

/**
 * @brief   Turns on the BLE module by releasing its reset.
 */
void cb_scr_ble_module_on(void)
{
    /*To add BLE Module Power On*/
    pSCR->pson_ctrl |= SCR_PSON_CTRL_MASK_BLE;
    /*wait for the BLE ACK to be acknowledged*/
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)  
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_BLE)!=SCR_PSON_ACK_MASK_BLE);
#endif  
    /*Reset Release for BLE Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_BLE;
    /*Clear the "ahb_fixed_en" to recover back the BLE RAM accesbility*/
    pSCR->ble &= (uint32_t)(~( SCR_BLE_AHBFIXEDEN_MASK));

    /*Enable BLE XO*/
    pSCR->xo |= SCR_en_xo_ble;
}

/**
 * @brief   Turns off the BLE module by resetting it.
 */
void cb_scr_ble_module_off(void)
{  
    /*To add BLE Module Power Off*/
    pSCR->pson_ctrl &= (uint32_t)(~(SCR_PSON_CTRL_MASK_BLE));
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)    
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_BLE)==SCR_PSON_ACK_MASK_BLE);
#endif
    /*Reset for BLE Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_BLE);

    /*Disable BLE XO*/
    pSCR->xo &= ~(SCR_en_xo_ble);
}

/**
 * @brief   Turns on the DMA module by releasing its reset.
 */
void cb_scr_dma_module_on(void)
{
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)  
    /*To add DMA Module Power On*/
    pSCR->pson_ctrl |= SCR_PSON_CTRL_MASK_DMA;
    /*wait for the DMA ACK to be acknowledged*/
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_DMA)!=SCR_PSON_ACK_MASK_DMA);
#endif  
    /*Reset Release for DMA Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_DMA;
}

/**
 * @brief   Turns off the DMA module by resetting it.
 */
void cb_scr_dma_module_off(void)
{  
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)  
    /*To add DMA Module Power Off*/
    pSCR->pson_ctrl &= (uint32_t)(~(SCR_PSON_CTRL_MASK_DMA));
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_DMA)==SCR_PSON_ACK_MASK_DMA);
#endif  
    /*Reset for DMA Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_DMA);

}

/**
 * @brief   Turns on the UWB module by releasing its reset.
 */
void cb_scr_uwb_module_on(void)
{
    /* UWB Module Power On*/
    pSCR->pson_ctrl |= SCR_PSON_CTRL_MASK_UWB;
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)  
    /*wait for the UWB ACK to be acknowledged*/
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_UWB)!=SCR_PSON_ACK_MASK_UWB);
#endif  
    /*Reset Release for UWB Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_UWB;
}

/**
 * @brief   Turns off the UWB module by resetting it.
 */
void cb_scr_uwb_module_off(void)
{
    /* UWB Module Power Off*/
    pSCR->pson_ctrl &= (uint32_t)(~(SCR_PSON_CTRL_MASK_UWB));
#if (SCR_PSON_CTRL_ACK_ENABLE == CB_TRUE)  
    while((pSCR->pson_ctrl&SCR_PSON_ACK_MASK_UWB)==SCR_PSON_ACK_MASK_UWB);
#endif
    /*Reset for UWB Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_UWB);
}

/**
 * @brief   Turns on the EFUSE module.
 */
void cb_scr_efuse_module_on(void)
{  
    /*Reset Release for EFUSE Module*/
    pSCR->rst_ctrl |= SCR_RST_CTRL_MASK_EFUSE;
}

/**
 * @brief   Turns off the EFUSE module.
 */
void cb_scr_efuse_module_off(void)
{  
    /*Reset Reset for EFUSE Module*/
    pSCR->rst_ctrl &= ~(SCR_RST_CTRL_MASK_EFUSE);
}

// need a SCR pointer to access EADC registers
static stSCR_TypeDef *SCR  = (stSCR_TypeDef *) SCR_BASE_ADDR;

void cb_scr_eadc_setup_dft(enEADCDFTMeasure MeasureType)
{
    // not sure what the purpose is, especially the bias value
    // but: 
    // analog_dft_top_buf_bias[14:12] = 3'b011
    // analog_dft_top_buf_en[11]      = 1'b1
    // could be setup the DFT unit to output ground voltage or Vbg voltage

    if (MeasureType == EN_EADC_DFT_MEASURE_GND)
    {
        SCR->dft  = ((0x3UL << SCR_ANALOG_DFT_TOP_BUF_BIAS_POS) & SCR_ANALOG_DFT_TOP_BUF_BIAS_MSK)
                                | ((0x1UL << SCR_ANALOG_DFT_TOP_BUF_EN_POS) & SCR_ANALOG_DFT_TOP_BUF_EN_MSK);
    }

    else 
    {
        SCR->dft  = ((0x3UL << SCR_ANALOG_DFT_TOP_BUF_BIAS_POS) & SCR_ANALOG_DFT_TOP_BUF_BIAS_MSK)
                                | ((0x1UL << SCR_ANALOG_DFT_TOP_BUF_EN_POS) & SCR_ANALOG_DFT_TOP_BUF_EN_MSK)
                                | ((0x1UL << SCR_ANALOG_DFT_TOP_EN_POS) & SCR_ANALOG_DFT_TOP_EN_MSK);
    }
}

uint16_t cb_scr_eadc_measure_dft(enEADCDFTMeasure MeasureType, enEADCGain gain)
{
    uint16_t EADCValue = 0;

    // setup DFT to input GND or VBG voltage to EADC
    cb_scr_eadc_setup_dft(MeasureType);

    // setting up SCR_EADC registers, but still for measuring DFT (unknown acronym)
    // at this, we measure voltage Vbg. This could be a global bias voltage
    // as this is included in the final calculation
    // the procedure and register fields are unexplained, especially eadc_vin_mid_cal 
    // But general description are in the steps
    if (gain == EN_EADC_GAIN_0) 
    {
        SCR->eadc = 0x20EA0002; //set gain=0, vmid=1.45V, reset
        SCR->eadc = 0x20EA0003; //release reset

        // start to measure for GND voltage (should be 0V as per calibration)
        cb_hal_delay_in_us(1);
        SCR->eadc = 0x30EA0003; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x20EA0003; 
        cb_hal_delay_in_us(1); //manual clk=0			
    } 
    
    else if (gain == EN_EADC_GAIN_1) 
    {		
        SCR->eadc = 0x20C20202; //set gain=1, vmid=1.2V, reset	
        SCR->eadc = 0x20C20203; //release reset	

        // start to measure for GND voltage (should be 0V as per calibration)
        cb_hal_delay_in_us(1);
        SCR->eadc = 0x30C20203; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x20C20203; 
        cb_hal_delay_in_us(1); //manual clk=0
    } 
    
    else if (gain == EN_EADC_GAIN_2) 
    {		
        SCR->eadc = 0x208A0402; //set gain=2, vmid=0.85V, reset	
        SCR->eadc = 0x208A0403; //release reset	

        // start to measure GND voltage
        cb_hal_delay_in_us(1);
        SCR->eadc = 0x308A0403; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x208A0403;
        cb_hal_delay_in_us(1) ; //manual clk=0	
    } 
    
    else if (gain == EN_EADC_GAIN_3) 
    {		
        SCR->eadc = 0x206E0602; //set gain=3, vmid=0.675V, reset	
        SCR->eadc = 0x206E0603; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x306E0603; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x206E0603; 
        cb_hal_delay_in_us(1); //manual clk=0
    } 
    
    else if (gain == EN_EADC_GAIN_4) 
    {		
        SCR->eadc = 0x20560802; //set gain=4, vmid=0.525V, reset	
        SCR->eadc = 0x20560803; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x30560803; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x20560803; 
        cb_hal_delay_in_us(1); //manual clk=0
    } 
    
    else
    {		
        SCR->eadc = 0x20460A02; //set gain=5, vmid=0.425V, reset	
        SCR->eadc = 0x20460A03; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x30460A03; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x20460A03; 
        cb_hal_delay_in_us(1); //manual clk=0	
    }

    // delay 1ms here because the sample code also wait 1ms
    cb_hal_delay_in_ms(1);

    // get the eadc reading
    EADCValue = SCR->eadc_read & 0x000003FFUL;
    return EADCValue;
}

uint16_t cb_scr_eadc_measure_ain(enEADCGain gain)
{
    uint16_t EADCValue;

    if (gain == EN_EADC_GAIN_0) 
    {		
        SCR->eadc = 0xEA0002; //set gain=0, vmid=1.45V, reset	
        SCR->eadc = 0xEA0003; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x10EA0003; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0xEA0003; 
        cb_hal_delay_in_us(1); //manual clk=0
    }		

    else if (gain == EN_EADC_GAIN_1) 
    {		
        SCR->eadc = 0xC20202; //set gain=1, vmid=1.2V, reset	
        SCR->eadc = 0xC20203; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x10C20203; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0xC20203; 
        cb_hal_delay_in_us(1); //manual clk=0
    }

    else if (gain == EN_EADC_GAIN_2) 
    {		
        SCR->eadc = 0x8A0402; //set gain=2, vmid=0.85V, reset	
        SCR->eadc = 0x8A0403; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x108A0403; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x8A0403; 
        cb_hal_delay_in_us(1); //manual clk=0
    }		

    else if (gain == EN_EADC_GAIN_3) {		
        SCR->eadc = 0x6E0602; //set gain=3, vmid=0.675V, reset	
        SCR->eadc = 0x6E0603; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x106E0603;
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x6E0603; 
        cb_hal_delay_in_us(1); //manual clk=0
    }

    else if (gain == EN_EADC_GAIN_4) 
    {		
        SCR->eadc = 0x560802; //set gain=4, vmid=0.525V, reset	
        SCR->eadc = 0x560803; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x10560803; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x560803; 
        cb_hal_delay_in_us(1); //manual clk=0
    }		

    else 
    {		
        SCR->eadc = 0x460A02; //set gain=5, vmid=0.425V, reset	
        SCR->eadc = 0x460A03; //release reset	

        cb_hal_delay_in_us(1);
        SCR->eadc = 0x10460A03; 
        cb_hal_delay_in_us(1); //manual clk=1
        SCR->eadc = 0x460A03; 
        cb_hal_delay_in_us(1); //manual clk=0
    }		

    // delay 1ms here because the sample code also wait 1ms
    cb_hal_delay_in_ms(1);

    // get the eadc reading
    EADCValue = SCR->eadc_read & 0x000003FFUL;
    return EADCValue;
}

/**
 * @brief		Wrapper function to disable/turn off CPU peripherals
 */
void cb_scr_disable_peripherals(void)
{
    cb_wdt_disable();
    cb_scr_gpio_module_off();
    cb_scr_crc_module_off();
    cb_scr_spi_module_off();
    cb_scr_uart0_module_off();
    cb_scr_uart1_module_off();
    cb_scr_timer0_module_off();
    cb_scr_timer1_module_off();
    cb_scr_timer2_module_off();
    cb_scr_timer3_module_off();
    cb_scr_pka_module_off();
    cb_scr_trng_module_off();
    cb_scr_ble_module_off();
    cb_scr_uwb_module_off();
    cb_scr_i2c_module_off();
    cb_scr_crypto_module_off();
    cb_scr_dma_module_off();
    cb_scr_efuse_module_off();
}
