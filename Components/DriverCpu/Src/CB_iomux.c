/**
 * @file    CB_iomux.c
 * @brief   Implementation of I/O Multiplexing (IOMUX) configuration functions.
 * @details This file contains the implementation of functions for configuring
 *          I/O Multiplexing (IOMUX) settings on the microcontroller.
 * @author  Chipsbank
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_iomux.h"
#include "CB_PeripheralPhyAddrDataBase.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

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
extern stIOMUX_TypeDef  *pIOMUX ;
stIOMUX_TypeDef  *pIOMUX  = (stIOMUX_TypeDef* ) IOMUX_BASE_ADDR;

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/**
 * @brief   Configures the I/O Multiplexing (IOMUX) settings for a GPIO pin.
 * 
 * This function configures the IOMUX settings for a specified GPIO pin based on
 * the provided configuration parameters.
 * 
 * @param   enGpio      The GPIO pin to configure.
 * @param   GpioModeSet Pointer to the structure containing GPIO mode and source select settings.
 */
void cb_iomux_config(enIomuxGpioSelect enGpio, stIomuxGpioMode* GpioModeSet)
{
    unGpioReg unGpioRegister;
    unGpioRegister.ui32Byte = 0; // initialize
    unGpioRegister.bit.modeSelect = (uint8_t)GpioModeSet->IomuxGpioMode;
    unGpioRegister.bit.srcSelect = (uint8_t)GpioModeSet->SourceSel;

    switch(enGpio)
    {
        case EN_IOMUX_GPIO_0:    pIOMUX->GPIO00 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_1:    pIOMUX->GPIO01 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_2:    pIOMUX->GPIO02 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_3:    pIOMUX->GPIO03 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_4:    pIOMUX->GPIO04 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_5:    pIOMUX->GPIO05 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_6:    pIOMUX->GPIO06 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_7:    pIOMUX->GPIO07 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_8:    pIOMUX->GPIO08 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_9:    pIOMUX->GPIO09 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_10:   pIOMUX->GPIO10 = (uint32_t)unGpioRegister.ui32Byte; break;
        case EN_IOMUX_GPIO_11:   pIOMUX->GPIO11 = (uint32_t)unGpioRegister.ui32Byte; break;
        default:  break;
    }
}
