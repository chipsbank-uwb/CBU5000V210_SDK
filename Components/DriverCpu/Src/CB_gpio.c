/**
 * @file    CB_gpio.c
 * @brief   GPIO driver implementation
 * @details This file contains the implementation of GPIO driver functions for ARMCM33 microcontroller.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "ARMCM33_DSP_FP.h"
#include "CB_gpio.h"
#include "CB_iomux.h"
#include "CB_PeripheralPhyAddrDataBase.h"

//-------------------------------
// CONFIGURATION SECTION
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
static stGPIO_TypeDef  *pGPIO  = (stGPIO_TypeDef* ) GPIO_BASE_ADDR;
extern stIOMUX_TypeDef  *pIOMUX;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static uint32_t cb_gpio_trigger_status(void);
static void cb_gpio_clear_trigger(uint32_t GPIO_Pin);
static uint32_t cb_gpio_pin_select_mapping(enGpioPin Pin);

void cb_gpio_irq_callback (void);
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Initialize GPIO pins.
 * @details This function initializes GPIO pins based on the provided configuration.
 * @param   cb_gpio_init Pointer to a structure containing GPIO initialization parameters.
 */
void cb_gpio_init(stGPIO_InitTypeDef* GPIO_Init)
{
    uint32_t position = 0U;
    uint32_t selectedPin = 0U;
    uint32_t currentpin;
    uint32_t PullConfig;
    volatile uint32_t* pIOMuxGPIO;
    volatile uint32_t* pGPIOPinCfg;

    pIOMuxGPIO =&(pIOMUX->GPIO00);
    pGPIOPinCfg =&(pGPIO->GPIO_PIN0);

    /*GPIO Module Power On and Reset Release*/
    //cb_scr_gpio_module_on();
    pGPIO->TRIG_OUT =1;
      
    selectedPin = cb_gpio_pin_select_mapping(GPIO_Init->Pin);

    /*Actual Register value mapping for Pull-Up/Down Config*/
    switch (GPIO_Init->Pull)
    {
      case EN_GPIO_PULLUP:
        PullConfig = GPIO_PULLUP;
        break;
      case EN_GPIO_PULLDOWN:
        PullConfig = GPIO_PULLDOWN;
        break;
      case EN_GPIO_NOPULL:
        PullConfig = GPIO_NOPULL;
        break;
      default:
        break;
    }

    /*Check for Every Pin*/
    while (((selectedPin) >> position) != 0U)
    {    
      /* Search pin position to configure*/
      currentpin = (selectedPin) & (1UL << position);
      if (currentpin != CB_FALSE)
      {
        pIOMuxGPIO[position]=IOMUX_GPIOMODE;
        pGPIO->GPIO_MODE |= GPIO_MODULE_ENABLE;
        pGPIOPinCfg[position]=PullConfig; /*Configure Pull Up/Down Enable and Select*/

        switch (GPIO_Init->Mode)
        {
          case EN_GPIO_MODE_INPUT:
            pGPIO->GPIO_OUT &= ~(currentpin); /*Set CurrentPin to Input Mode*/
            pGPIO->TRIG &= ~(currentpin);     /*Clear Interrupt on this Pin*/
            pGPIO->TRIG_CFG &= ~(GPIO_TRIG_CLEAR_MSK <<(position*2)); /*Clear TriggeMode*/
            break;
          case EN_GPIO_MODE_OUTPUT:
            pGPIO->GPIO_OUT |= currentpin;     /*Set CurrentPin to Output Mode*/
            pGPIO->TRIG &= ~(currentpin);     /*Clear Interrupt on this Pin*/
            pGPIO->TRIG_CFG &= ~(GPIO_TRIG_CLEAR_MSK <<(position*2)); /*Clear TriggeMode*/
            break;
          case EN_GPIO_MODE_IT_RISING:
            pGPIO->GPIO_OUT &= ~(currentpin); /*Set CurrentPin to Input Mode*/
            pGPIO->TRIG |= currentpin;
            pGPIO->TRIG_CFG &= ~(GPIO_TRIG_CLEAR_MSK <<(position*2)); /*Clear TriggeMode*/
            pGPIO->TRIG_CFG |= (GPIO_TRIG_SET_RISING <<(position*2));  /*Set TriggeMode*/
            //NVIC_EnableIRQ(GPIO_IRQn);
            break;
          case EN_GPIO_MODE_IT_FALLING:
            pGPIO->GPIO_OUT &= ~(currentpin); /*Set CurrentPin to Input Mode*/
            pGPIO->TRIG |= currentpin;
            pGPIO->TRIG_CFG &= ~(GPIO_TRIG_CLEAR_MSK <<(position*2)); /*Clear TriggeMode*/
            pGPIO->TRIG_CFG |= (GPIO_TRIG_SET_FALLING <<(position*2));/*Set TriggeMode*/
            //NVIC_EnableIRQ(GPIO_IRQn);
            break;
          case EN_GPIO_MODE_IT_RISING_FALLING:
            pGPIO->GPIO_OUT &= ~(currentpin); /*Set CurrentPin to Input Mode*/
            pGPIO->TRIG |= currentpin;
            pGPIO->TRIG_CFG &= ~(GPIO_TRIG_CLEAR_MSK <<(position*2)); /*Clear TriggeMode*/
            pGPIO->TRIG_CFG |= (GPIO_TRIG_SET_RISINGFALLING <<(position*2));/*Set TriggeMode*/
            //NVIC_EnableIRQ(GPIO_IRQn);
            break;
        }
      }
      position++;
    }
}

/**
 * @brief   Deinitialize GPIO pins.
 * @details This function deinitializes GPIO pins based on the provided pin mask.
 * @param   GPIO_Pin The pin or pins to be deinitialized.
 */
void cb_gpio_deinit(enGpioPin GPIO_Pin)
{
    uint32_t position = 0U;
    uint32_t currentpin;
    uint32_t selectedpin;

    selectedpin = cb_gpio_pin_select_mapping(GPIO_Pin); 

    /*Check for Every Pin*/
    while (((selectedpin) >> position) != 0U)
    {    
      /* Search pin position to configure*/
      currentpin = (selectedpin) & (1UL << position);
      if (currentpin != CB_FALSE)
      {
        pGPIO->TRIG_CLEAR |= currentpin;   /*Clear Trigger if any*/
        pGPIO->GPIO_OUT &= ~(currentpin); /*Set back CurrentPin to Defualt Input Mode*/
        pGPIO->TRIG &= ~(currentpin);     /*Disable Interrupt on this Pin*/
        pGPIO->TRIG_CFG &= ~(GPIO_TRIG_CLEAR_MSK <<(position*2));
      }
      position++;
    }
}

/**
 * @brief   Initialize GPIO configuration structure with default values.
 * @details This function initializes the provided GPIO initialization structure 
 *          with default settings. The default configuration sets the mode to 
 *          output and the pull-up/pull-down resistor to no pull
 * @param   GPIO_Init Pointer to a GPIO initialization structure that will be initialized with default values.
 */
void cb_gpio_structinit(stGPIO_InitTypeDef* GPIO_Init)
{
    GPIO_Init->Mode = EN_GPIO_MODE_OUTPUT;   /* Set mode to output */
    GPIO_Init->Pull = EN_GPIO_NOPULL;        /* Set pull-up/pull-down to no pull */
}

/**
 * @brief   Read the input value of a GPIO pin.
 * @details This function reads the input value of a specified GPIO pin.
 * @param   GPIO_Pin The pin to be read.
 * @return  The input state of the GPIO pin (GPIO_PIN_SET or GPIO_PIN_RESET).
 */
enGPIO_PinState cb_gpio_read_input_pin(enGpioPin GPIO_Pin)
{
		uint32_t temp;
    uint32_t selectedpin;
    selectedpin = cb_gpio_pin_select_mapping(GPIO_Pin); 
    temp = ((pGPIO->GPIO_IN)& selectedpin);
    if (temp != DRIVER_CLR) /*GPIO HIGH*/
    {
      return EN_GPIO_PIN_SET;
    }
    else/*GPIO LOW*/
    {
      return EN_GPIO_PIN_RESET;
    }    
}

/**
 * @brief   Read the combined input value of all GPIO pins.
 * @details This function reads the combined input value of all GPIO pins and returns it as a single 32-bit value.
 * @return  A 32-bit value representing the input state of all GPIO pins.
 */
uint32_t cb_gpio_read_all_input_pins(void)
{
    return pGPIO->GPIO_IN;
}
	
/**
 * @brief   Read the output value of a GPIO pin.
 * @details This function reads the output value of a specified GPIO pin and returns 
 * @param   GPIO_Pin The pin to be read.
 * @return  The output state of the GPIO pin (EN_GPIO_PIN_SET for high, EN_GPIO_PIN_RESET for low).
 */
enGPIO_PinState cb_gpio_read_output_pin(enGpioPin GPIO_Pin)
{
    uint32_t temp;
    uint32_t selectedpin;
    selectedpin = cb_gpio_pin_select_mapping(GPIO_Pin); 
    temp = ((pGPIO->GPIO) & selectedpin);
    if (temp != DRIVER_CLR) /* GPIO HIGH */
    {
        return EN_GPIO_PIN_SET;
    }
    else /* GPIO LOW */
    {
        return EN_GPIO_PIN_RESET;
    } 
}

/**
 * @brief   Read the combined output value of all GPIO pins.
 * @details This function reads the combined output value of all GPIO pins and returns it as a single 32-bit value.
 * @return  A 32-bit value representing the output state of all GPIO pins.
 */
uint32_t cb_gpio_read_all_output_pins(void)
{
    return pGPIO->GPIO;
}

/**
 * @brief   Write a value to a GPIO pin.
 * @details This function writes a specified value to the output of a GPIO pin.
 * @param   GPIO_Pin The pin to be written to.
 * @param   PinState The state to be written to the pin (GPIO_PIN_SET or GPIO_PIN_RESET).
 */
void cb_gpio_write_pin(enGpioPin GPIO_Pin,enGPIO_PinState PinState)
{
    uint32_t selectedpin;
    selectedpin = cb_gpio_pin_select_mapping(GPIO_Pin); 

    if (PinState == EN_GPIO_PIN_SET)  /*GPIO Output HIGH*/
    {
      pGPIO->GPIO |= selectedpin;
    }
    else /*GPIO Output LOW*/
    {
      pGPIO->GPIO &= ~(selectedpin);
    }
}

/**
 * @brief   Write a value to all GPIO pins.
 * @details This function writes a specified 32-bit value to the output of all GPIO pins.
 * @param   PinState The 32-bit value to be written to all GPIO pins.
 */
void cb_gpio_write_all_pins(uint32_t PinState)
{
    pGPIO->GPIO = PinState;
}

/**
 * @brief   Toggle the output value of a GPIO pin.
 * @details This function toggles the output value of a specified GPIO pin.
 * @param   GPIO_Pin The pin to be toggled.
 */
void cb_gpio_toggle_pin(enGpioPin GPIO_Pin)
{
    uint32_t temp;
    uint32_t selectedpin;
    selectedpin = cb_gpio_pin_select_mapping(GPIO_Pin); 

    temp = ((pGPIO->GPIO)& selectedpin);
    if (temp != DRIVER_CLR) /*Prev GPIO state is HIGH*/
    {
      pGPIO->GPIO &= ~(selectedpin); //Toggling (From High to Low)
    }
    else/*Prev GPIO state is LOW*/
    {
      pGPIO->GPIO |= selectedpin; //Toggling (From High to Low)
    }    
}  

/**
 * @brief   Get the trigger status of GPIO pins.
 * @details This function returns the trigger status of GPIO pins.
 * @return  The trigger status of GPIO pins.
 */
static uint32_t cb_gpio_trigger_status(void)
{
    return pGPIO->TRIG_VAL;
}

/**
 * @brief   Clear the trigger status of GPIO pins.
 * @details This function clears the trigger status of specified GPIO pins.
 * @param   GPIO_Pin The pin or pins whose trigger status is to be cleared.
 */
static void cb_gpio_clear_trigger(uint32_t GPIO_Pin)
{
    pGPIO->TRIG_CLEAR = GPIO_Pin;
}
/**
 * @brief   Register Mapping for Selected GPIO pin.
 * @details This function return the register bit based on selected parameter.
 * @param   GPIO_Pin The pin to be mapped.
 */
static uint32_t cb_gpio_pin_select_mapping(enGpioPin Pin)
{
    uint32_t SelectedPin;
    /*Actual Register value mapping for Pin Selected*/
    /*Disallowed user to configure PIN9,10,11 due to UW1000V200 rtl design issue in SWD mode.*/
    switch (Pin)
    {
      case EN_GPIO_PIN_0:
        SelectedPin = GPIO_PIN_0;
        break;
      case EN_GPIO_PIN_1:
        SelectedPin = GPIO_PIN_1;    
        break;
      case EN_GPIO_PIN_2:
        SelectedPin = GPIO_PIN_2;    
        break;
      case EN_GPIO_PIN_3:
        SelectedPin = GPIO_PIN_3;    
        break;
      case EN_GPIO_PIN_4:
        SelectedPin = GPIO_PIN_4;   
        break;    
      case EN_GPIO_PIN_5:
        SelectedPin = GPIO_PIN_5;   
        break;    
      case EN_GPIO_PIN_6:
        SelectedPin = GPIO_PIN_6; 
        break;    
      case EN_GPIO_PIN_7:
        SelectedPin = GPIO_PIN_7;    
        break;
      case EN_GPIO_PIN_8:
        SelectedPin = GPIO_PIN_8;    
        break;
      case EN_GPIO_PIN_9:
        SelectedPin = GPIO_PIN_9;    
        break;
      case EN_GPIO_PIN_10:
        SelectedPin = GPIO_PIN_10;    
        break;
      case EN_GPIO_PIN_11:
        SelectedPin = GPIO_PIN_11;    
        break;
      default:
        SelectedPin = 0;
        break;
    }
    return SelectedPin;
}



#if 1
/**
 * @brief   GPIO interrupt handler function.
 * @details This function handles GPIO interrupts by disabling the GPIO IRQ,
 *          retrieving the trigger status, invoking the callback function with
 *          the trigger status as parameter, and then enabling the GPIO IRQ.
 */
void cb_gpio_irqhandler(void)
{
    uint32_t TrigStatus;
    TrigStatus = cb_gpio_trigger_status();
    cb_gpio_clear_trigger(TrigStatus);

    cb_gpio_irq_callback();
}

/**
 * @brief   Weakly defined GPIO interrupt callback function.
 * @details This function is intended to be overridden by the user to handle
 *          GPIO interrupt events. By default, it clears the trigger of the
 *          specified GPIO pin.
 * @param   GPIO_Pin The pin that triggered the interrupt.
 */
void cb_gpio_irq_callback (void)
{
    cb_gpio_app_irq_callback();
}

__WEAK void cb_gpio_app_irq_callback(void)
{
}

#endif

