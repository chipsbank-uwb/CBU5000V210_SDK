/**
 * @file    CB_i2c.c
 * @brief   I2C driver implementation.
 * @details This file contains the implementation for I2C driver.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_i2c.h"
#include "CB_dma.h"
#include "CB_scr.h"
#include "ARMCM33_DSP_FP.h"
#include "CB_PeripheralPhyAddrDataBase.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

#define BYTE_SIZE 8
#define DEF_I2C_TIMEOUT_MS              500 // Referencing 100kHz I2C speed with 4096 bytes to transfer
#define I2C_TIMEOUT_CPU_CYCLES          (((SystemCoreClock) / 1000U) * DEF_I2C_TIMEOUT_MS)

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stI2C_TypeDef *pI2C = (stI2C_TypeDef *)I2C_BASE_ADDR;
static stDMAConfig    I2CDMAConfig = {0};   // to be used in DMA mode
extern uint32_t SystemCoreClock;

volatile static uint8_t NACKflag = DRIVER_SET;
volatile static uint8_t I2CDMADone = DRIVER_CLR;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief I2C Interrupt Handler.
 * @detail Abort I2C Transmission if neccessary.
 */
void cb_i2c_irqhandler(void) 
{
    uint32_t irqstatus = cb_i2c_get_and_clear_int_status(EN_I2C_INT_ALL);
    
    if (irqstatus & EN_I2C_INT_NACK) {
        pI2C->irq_en &= ~(I2C_EN_NACK_IRQ | I2C_EN_READY_IRQ);
        pI2C->fr &= ~I2C_CR_I2C_ENABLE; // Abort the Transmission.
        NACKflag = DRIVER_SET;
    }

    if (irqstatus & EN_I2C_INT_RX_FULL) {
        // Handle RX_FULL interrupt
    }

    if (irqstatus & EN_I2C_INT_TX_EMPTY) {
        // Handle TX_EMPTY interrupt
    }

    if (irqstatus & EN_I2C_INT_READY) {
        // Handle READY interrupt
    }

    cb_i2c_app_irq_callback();
}

/**
 * @brief Weakly defined I2C Interrupt Handler at APP level.
 */
__WEAK void cb_i2c_app_irq_callback(void)
{
}

/**
 * @brief                 Init the I2C communication module with specified speed.
 * @detail                Set the correct speed for I2C and also registers I2C interrupt system-wise.
 * @param[in] I2CConfig   Pointer to the I2C configuration structure as shown in the header file
 */
void cb_i2c_init(stI2CConfig *I2CConfig) 
{
    uint32_t prescaler;
    uint32_t drivePeriod;

    /* Null check i2c config struct */
    if (I2CConfig == NULL)
    {
      return;
    }

    /*I2C Module Power On and Reset Release*/
    cb_scr_i2c_module_on();

    /* Setup NACK interrupt to always on */
    NVIC_EnableIRQ(I2C_IRQn);
    //pI2C->irq_en = I2C_EN_NACK_IRQ;

    prescaler = ((SystemCoreClock / I2CConfig->I2CSpeed) - 1);
    drivePeriod = (((SystemCoreClock / I2CConfig->I2CSpeed) / 4) - 1);    // set drive period to be 1/4 of SCL period
    if (prescaler % 2 == 0) /*To make sure it is odd number */
    {
      prescaler++;
    }

    if (drivePeriod % 2 == 0)
    {
      drivePeriod++;
    }

    pI2C->fr = ((drivePeriod << I2C_TWDRIVE_POS) & I2C_TWDRIVE_MSK)
              | ((prescaler << I2C_TWSCL_POS) & I2C_TWSCL_MSK);
}


/**
 * @brief   Initialize the I2C configuration structure with default values.
 * @details This function initializes the provided I2C configuration structure with default settings.
 *          The default configuration sets the I2C speed to 100 kHz and disables all interrupts.
 * @param   I2CConfig Pointer to the I2C configuration structure that will be initialized with default values.
 */
void cb_i2c_struct_init(stI2CConfig *I2CConfig)
{
    I2CConfig->I2CSpeed = 100000;              /* I2C Speed 100kHz */
    I2CConfig->I2CInt = EN_I2C_INT_DISABLE;    /* Disable all interrupts */
}

/**
 * @brief  Enable the I2C communication module.
 */
void cb_i2c_enable(void)
{
    pI2C->fr |= I2C_CR_I2C_ENABLE;
}

/**
 * @brief  Disable the I2C communication module and suspend any ongoing transaction.
 */
void cb_i2c_disable(void)
{
    pI2C->fr &= ~(I2C_CR_I2C_ENABLE);
}

/**
 * @brief DeInit the I2C communication module.
 * @detail Disable I2C interrupts and turn off its enable bits.
 */
void cb_i2c_deinit(void) 
{
    uint32_t clearirq;
    /*Clear and Disable I2C IRQ*/
    clearirq = pI2C->irq_reg;
    pI2C->irq_en = 0x00000000UL;

    /*Clear Configuration*/
    pI2C->fr = 0x00000000UL;

    NVIC_DisableIRQ(I2C_IRQn);

    /*Disable I2C Module*/
    cb_scr_i2c_module_off();
}

/**
 * @brief  Polls for transaction done of the I2C (done/busy).
 * @return CB_PASS if I2C ready and no timeout, CB_FAIL otherwise 
 */
static CB_STATUS cb_i2c_wait_for_ready_flag(void)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t startCPUCycle = 0;
    uint32_t ellapsedCPUCycles = 0;

    /* The section below waits until the last byte/entire transmission has finihsed */
    /* Record start of transfer timestamp for timeout */
    startCPUCycle = DWT->CYCCNT;
    do 
    {
      if ((pI2C->status & I2C_READY_MSK) == I2C_READY)
      {
        ret = CB_PASS;
        break;
      }
      
      /* Record ellapsed time for timeout */
      ellapsedCPUCycles = (DWT->CYCCNT < startCPUCycle) ? 
            (0xFFFFFFFF - startCPUCycle + DWT->CYCCNT + 1) : DWT->CYCCNT - startCPUCycle;
    } while (ellapsedCPUCycles < I2C_TIMEOUT_CPU_CYCLES);

    return ret;
}

/**
 * @brief  Checks for transaction status of the I2C (done/busy).
 * @return CB_TRUE if I2C ready, CB_FALSE otherwise 
 */
uint8_t cb_i2c_is_ready(void)
{
    return ((pI2C->status & I2C_READY_MSK) == I2C_READY) ? CB_TRUE : CB_FALSE;
}

/**
 * @brief  Write as Master using I2C.
 * @param[in]  PeripheralAddr   7 bits address of Slave
 * @param[in]  txbuf            buffer containing data to be written
 * @param[in]  DataLen          Length of data to be written. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission successful
 *             CB_FAIL          NACK flag received from slave device or transmission timed out
 */
CB_STATUS cb_i2c_master_write(uint8_t PeripheralAddr, uint8_t *txbuf, uint16_t DataLen) 
{
    uint16_t Length, i, cnt = 0;
    uint32_t tempbuf;
    uint32_t startCPUCycle = 0;
    uint32_t ellapsedCPUCycles = 0;

    uint8_t ret = CB_FAIL;

    // if length of data exceeds maximum, return fail
    if (DataLen > I2C_MAX_DATA_LENGTH)
    {
      return CB_FAIL;
    }

    if (cb_i2c_is_ready() != CB_TRUE)
    {
      return CB_FAIL;
    }

    // clear NACK flag previously set by IRQ
    NACKflag = DRIVER_CLR;
    pI2C->irq_en = I2C_EN_NACK_IRQ;

    // configure control register
    pI2C->cr = ((((uint32_t)DataLen << I2C_BYTE_COUNT_POS) & I2C_BYTE_COUNT_MSK) |
                I2C_NACK_LAST_BYTE | I2C_NO_INCLUDE_REG_ADDR | I2C_WRITEMODE |
                ((uint32_t)PeripheralAddr & I2C_DEV_ADDR_MSK));

    Length = DataLen;
    tempbuf = 0x00000000UL;

    /* Record start of transfer timestamp for timeout */
    startCPUCycle = DWT->CYCCNT;

    // enable CR to start transmission
    pI2C->fr |= I2C_CR_I2C_ENABLE;

    // input data to txdata register
    do
    {
      if ((pI2C->status & I2C_TXDATA_EMPTY_MSK) == I2C_TXDATA_EMPTY) 
      {
        if (Length <= 4) /*Maximum 4 Bytes for each entry*/
        {
          for (i = 0; i < Length; i++) {
            tempbuf |= (uint32_t)(txbuf[cnt] << (i * BYTE_SIZE));
            cnt++;
          }
          pI2C->txdata = tempbuf;
          break; /*End of last Input byte*/
        } 
        
        else 
        {
          for (i = 0; i < 4; i++) 
          {
            tempbuf |= (uint32_t)(txbuf[cnt] << (i * BYTE_SIZE));
            cnt++;
          }
          Length -= 4;
          pI2C->txdata = tempbuf;
          tempbuf = 0x00000000UL;
        }
      }

      if (NACKflag == DRIVER_SET) 
      {
        return CB_FAIL; // NACK acknowledge
      }

      /* Record ellapsed time for timeout */
      ellapsedCPUCycles = (DWT->CYCCNT < startCPUCycle) ? 
            (0xFFFFFFFF - startCPUCycle + DWT->CYCCNT + 1) : DWT->CYCCNT - startCPUCycle;
    } while (ellapsedCPUCycles < I2C_TIMEOUT_CPU_CYCLES);

    /* The section below waits until the last byte/entire transmission has finihsed */
    ret = cb_i2c_wait_for_ready_flag();

    return ret;
}

/**
 * @brief Write as Master using I2C, to a specific register.
 * @param[in]  PeripheralAddr   7 bits address of Slave
 * @param[in]  RegAddr          8 bits address of Register
 * @param[in]  txbuf            buffer containing data to be written
 * @param[in]  DataLen          Length of data to be written. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission successful
 *             CB_FAIL          NACK flag received from slave device or transmission timed out
 */
CB_STATUS cb_i2c_master_write_to_register(uint8_t PeripheralAddr, uint8_t RegAddr, uint8_t *txbuf, uint16_t DataLen) 
{
    uint8_t ret = CB_FAIL;
    uint32_t tempbuf;
    uint16_t Length, i, cnt = 0;
    uint32_t startCPUCycle = 0;
    uint32_t ellapsedCPUCycles = 0;

    // if data length exceeds maximum length
    if (DataLen > I2C_MAX_DATA_LENGTH)
    {
      return CB_FAIL;
    }

    // check if I2C module is ready
    if (cb_i2c_is_ready() != CB_TRUE)
    {
      return CB_FAIL;
    }

    // clear NACK flag previously set by IRQ
    NACKflag = DRIVER_CLR;
    pI2C->irq_en = I2C_EN_NACK_IRQ;

    // enable CR to start transmission
    pI2C->fr |= I2C_CR_I2C_ENABLE;

    // configure control register
    pI2C->cr = ((((uint32_t)DataLen << I2C_BYTE_COUNT_POS) & I2C_BYTE_COUNT_MSK) |
                I2C_NACK_LAST_BYTE | I2C_INCLUDE_REG_ADDR |
                (((uint32_t)RegAddr << I2C_REG_ADDR_POS) & I2C_REG_ADDR_MSK) |
                I2C_WRITEMODE | ((uint32_t)PeripheralAddr & I2C_DEV_ADDR_MSK));

    Length = DataLen;
    tempbuf = 0x00000000UL;

    /* Record start of transfer timestamp for timeout */
    startCPUCycle = DWT->CYCCNT;

    // input data to txdata register
    do
    {
      if ((pI2C->status & I2C_TXDATA_EMPTY_MSK) == I2C_TXDATA_EMPTY) 
      {
        if (Length <= 4) /*Maximum 4 Bytes for each entry*/
        {
          for (i = 0; i < Length; i++) {
            tempbuf |= (uint32_t)(txbuf[cnt] << (i * BYTE_SIZE));
            cnt++;
          }
          pI2C->txdata = tempbuf;
          break; /*End of last Input byte*/
        } 
        
        else 
        {
          for (i = 0; i < 4; i++) {
            tempbuf |= (uint32_t)(txbuf[cnt] << (i * BYTE_SIZE));
            cnt++;
          }
          Length -= 4;
          pI2C->txdata = tempbuf;
          tempbuf = 0x00000000UL;
        }
      }

      if (NACKflag == DRIVER_SET) 
      {
        return CB_FAIL; // NACK acknowledge
      }
      
      /* Record ellapsed time for timeout */
      ellapsedCPUCycles = (DWT->CYCCNT < startCPUCycle) ? 
            (0xFFFFFFFF - startCPUCycle + DWT->CYCCNT + 1) : DWT->CYCCNT - startCPUCycle;
    } while (ellapsedCPUCycles < I2C_TIMEOUT_CPU_CYCLES);

    /* The section below waits until the last byte/entire transmission has finihsed */
    ret = cb_i2c_wait_for_ready_flag();

    return ret;
}

/**
 * @brief  Read as Master using I2C from a slave.
 * @param[in]   PeripheralAddr  7 bits address of Slave.
 * @param[out]  rxbuf           buffer to contain read data.
 * @param[in]   DataLen         Length of data to be read. Maximum is 4095 bytes at a time.
 * @return      CB_PASS         I2C Transmission successful
 *              CB_FAIL         NACK flag received from slave device or transmission timed out
 */
CB_STATUS cb_i2c_master_read(uint8_t PeripheralAddr, uint8_t *rxbuf, uint16_t DataLen) 
{
    uint8_t ret = CB_FAIL;
    uint32_t tempbuf;
    uint16_t Length, i, cnt = 0;
    uint32_t startCPUCycle = 0;
    uint32_t ellapsedCPUCycles = 0;

    // data length exceeds I2C max buffer size
    if (DataLen > I2C_MAX_DATA_LENGTH)
    {
      return CB_FAIL;
    }

    // I2C module not ready (busy)
    if (cb_i2c_is_ready() != CB_TRUE)
    {
      return CB_FAIL;
    }

    // clear previously set NACK flag
    NACKflag = DRIVER_CLR;
    pI2C->irq_en = I2C_EN_NACK_IRQ;

    // configure control register
    pI2C->cr = ((((uint32_t)DataLen << I2C_BYTE_COUNT_POS) & I2C_BYTE_COUNT_MSK) |
                I2C_NACK_LAST_BYTE |I2C_NO_INCLUDE_REG_ADDR | I2C_READMODE |
                ((uint32_t)PeripheralAddr & I2C_DEV_ADDR_MSK));

    Length = DataLen;
    tempbuf = 0x00000000UL;

    /* Record start of transfer timestamp for timeout */
    startCPUCycle = DWT->CYCCNT;

    // enable CR to start transmission
    pI2C->fr |= I2C_CR_I2C_ENABLE;

    // read data from rxdata register
    do
    {
      if ((pI2C->status & I2C_RXDATA_FULL_MSK) == I2C_RXDATA_FULL) 
      {
        tempbuf = pI2C->rxdata;
        
        if (Length <= 4) 
        {
          for (i = 0; i < Length; i++) 
          {
            rxbuf[cnt] = ((uint8_t)(tempbuf >> (i * BYTE_SIZE) & 0xFF));
            cnt++;
          }
          break; /*End of DataRead*/
        }

        rxbuf[cnt] = ((uint8_t)(tempbuf & 0xFF));
        cnt++;
        rxbuf[cnt] = ((uint8_t)((tempbuf >> 8) & 0xFF));
        cnt++;
        rxbuf[cnt] = ((uint8_t)((tempbuf >> 16) & 0xFF));
        cnt++;
        rxbuf[cnt] = ((uint8_t)((tempbuf >> 24) & 0xFF));
        cnt++;
        Length -= 4;
      }
      
      if ((pI2C->status & I2C_READY_MSK) == I2C_READY) 
      {
        break;
      }
      
      if (NACKflag == DRIVER_SET) 
      {
        return CB_FAIL; // NACK acknowledge
      }

      /* Record ellapsed time for timeout */
      ellapsedCPUCycles = (DWT->CYCCNT < startCPUCycle) ? 
            (0xFFFFFFFF - startCPUCycle + DWT->CYCCNT + 1) : DWT->CYCCNT - startCPUCycle;
    } while (ellapsedCPUCycles < I2C_TIMEOUT_CPU_CYCLES);

    /* Poll for transaction done flag */
    ret = cb_i2c_wait_for_ready_flag();

    return ret;
}

/**
 * @brief  Read as Master using I2C from a specific register
 * @param[in]  PeripheralAddr   7 bits address of Slave.
 * @param[in]  RegAddr          8 bits address of Register.
 * @param[out] rxbuf            buffer to contain read data.
 * @param[in]  DataLen          Length of data to be read. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission successful
 *             CB_FAIL          NACK flag received from slave device or transmission timed out
 */
CB_STATUS cb_i2c_master_read_from_register(uint8_t PeripheralAddr, uint8_t RegAddr, uint8_t *rxbuf, uint16_t DataLen) 
{
    uint8_t ret = CB_FAIL;
    uint32_t startCPUCycle = 0;
    uint32_t ellapsedCPUCycles = 0;
    uint32_t tempbuf;

    // return error if length to read is more than I2C max length
    if (DataLen > I2C_MAX_DATA_LENGTH) 
    {
      return CB_FAIL; 
    }

    // return failed if I2C module not ready
    if (cb_i2c_is_ready() != CB_TRUE) 
    {
      return CB_FAIL;
    }

    // clear previously set NACK flag
    NACKflag = DRIVER_CLR;
    pI2C->irq_en = I2C_EN_NACK_IRQ;

    // enable CR to send out the address to be read
    pI2C->fr |= I2C_CR_I2C_ENABLE;

    // configure control register
    pI2C->cr = ((((uint32_t)0x01UL << I2C_BYTE_COUNT_POS) & I2C_BYTE_COUNT_MSK) |
                I2C_NO_STOP | I2C_NACK_LAST_BYTE | I2C_NO_INCLUDE_REG_ADDR |
                I2C_WRITEMODE | ((uint32_t)PeripheralAddr & I2C_DEV_ADDR_MSK));

    // send the register address to be read from
    pI2C->txdata = RegAddr;

    /* Record start of transfer timestamp for timeout */
    if (cb_i2c_wait_for_ready_flag() != CB_PASS)
    {
      return CB_FAIL;
    }

    /*Continue to I2C Read Part.*/
    if (NACKflag == DRIVER_CLR) 
    {
      ret = cb_i2c_master_read(PeripheralAddr, rxbuf, DataLen);
    }

    else 
    {
      return CB_FAIL;
    }

    return ret;
}

/**
 * @brief  Write as Master using I2C+DMA to a specific register.
 * @param[in]  DMAChannel       DMA channel to use for I2C writes
 * @param[in]  PeripheralAddr   8 bits address of Slave.
 * @param[in]  RegAddr          7 bits address of Register.
 * @param[in]  BufAddr          address of buffer containing data to be written.
 * @param[in]  DataLen          Length of data to be written. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission setup successful
 *             CB_FAIL          I2C module not ready or data length exceeded maximum or NACK received
 */
CB_STATUS cb_i2c_master_write_to_register_dma(enDMAChannel DMAChannel, uint8_t PeripheralAddr, uint8_t RegAddr, uint32_t BufAddr, uint32_t DataLen) 
{
    uint32_t tempbuf = 0;

    // if max length is exceeded
    if (DataLen > I2C_MAX_DATA_LENGTH)
    {
      return CB_FAIL;
    }

    // if I2C module is not ready
    if (cb_i2c_is_ready() != CB_TRUE)
    {
      return CB_FAIL;
    }

    // clear NACK flag previously set
    tempbuf = pI2C->irq_reg;
    NACKflag = DRIVER_CLR;

    /* Enable I2C Ready Interrupt */
    pI2C->irq_en = I2C_EN_READY_IRQ | I2C_EN_NACK_IRQ;

    /* Clear DMA request before starting transfer */
    cb_dma_disable_channel(&I2CDMAConfig);

    // reset DMA_SEL field
    pI2C->fr &= ~(I2C_DMA_SEL_MSK | I2C_CR_I2C_ENABLE_MSK);

    // flow controlled by txdata_empty
    pI2C->fr |= I2C_DMA_SEL_TX;

    // enable CR to send out the address to be read
    pI2C->fr |= I2C_CR_I2C_ENABLE;

    // configure control register
    pI2C->cr = ((((uint32_t)DataLen << I2C_BYTE_COUNT_POS) & I2C_BYTE_COUNT_MSK) |
                I2C_NACK_LAST_BYTE | I2C_INCLUDE_REG_ADDR |
                (((uint32_t)RegAddr << I2C_REG_ADDR_POS) & I2C_REG_ADDR_MSK) |
                I2C_WRITEMODE | ((uint32_t)PeripheralAddr & I2C_DEV_ADDR_MSK));

    /* DMA setup */
    I2CDMAConfig.DMAChannel       = DMAChannel;
    I2CDMAConfig.IRQEnable        = EN_DMA_IRQ_ENABLE;    // force enable interrupt for done flag check
    I2CDMAConfig.DataWidth        = EN_DMA_DATA_WIDTH_WORD;
    I2CDMAConfig.ContinuousMode   = EN_DMA_CONTINUOUS_MODE_DISABLE;
    I2CDMAConfig.SrcAddr          = BufAddr;
    I2CDMAConfig.DestAddr         = (uint32_t)&(pI2C->txdata);
    I2CDMAConfig.DataLen          = DataLen;
    I2CDMAConfig.SrcInc           = EN_DMA_SRC_ADDR_INC_ENABLE;
    I2CDMAConfig.DestInc          = EN_DMA_DEST_ADDR_INC_DISABLE;
    I2CDMAConfig.FlowControlSrc   = EN_DMA_FLOW_CONTROL_SRC_DISABLE;
    I2CDMAConfig.FlowControlDest  = EN_DMA_FLOW_CONTROL_DEST_SEL_0;

    cb_dma_setup(&I2CDMAConfig);
    cb_dma_enable_channel(&I2CDMAConfig);
    /* Done DMA setup */

    return CB_PASS;
}

/**
 * @brief  Read as Master using I2C+DMA from a specific register.
 * @param[in]  DMAChannel       DMA channel to use for I2C writes
 * @param[in]  PeripheralAddr   8 bits address of Slave.
 * @param[in]  RegAddr          7 bits address of Register.
 * @param[in]  BufAddr          address of buffer containing data to be read.
 * @param[in]  DataLen          Length of data to be read. Maximum is 4095 bytes at a time.
 * @return     CB_PASS          I2C Transmission setup successful
 *             CB_FAIL          I2C module not ready or data length exceeded maximum or NACK received
 */
CB_STATUS cb_i2c_master_read_from_register_dma(enDMAChannel DMAChannel, uint8_t PeripheralAddr, uint8_t RegAddr,uint32_t BufAddr, uint32_t DataLen) 
{
    uint32_t tempbuf = 0;
    // if maximum data length exceeded
    if (DataLen > I2C_MAX_DATA_LENGTH)
    {
      return CB_FAIL;
    }

    // if I2C Module not yet ready
    if (cb_i2c_is_ready() != CB_TRUE)
    {
      return CB_FAIL;
    }

    // clear NACK flag previously set
    tempbuf = pI2C->irq_reg;
    NACKflag = DRIVER_CLR;

    /* Enable I2C Ready Interrupt */
    pI2C->irq_en = I2C_EN_READY_IRQ | I2C_EN_NACK_IRQ;

    /* Clear DMA request before starting transfer */
    cb_dma_disable_channel(&I2CDMAConfig);

    // enable CR to send out the address to be read
    pI2C->fr |= I2C_CR_I2C_ENABLE;

    // configure control register
    pI2C->cr = ((((uint32_t)0x01UL << I2C_BYTE_COUNT_POS) & I2C_BYTE_COUNT_MSK) |
                I2C_NO_STOP | I2C_NACK_LAST_BYTE | I2C_NO_INCLUDE_REG_ADDR |
                I2C_WRITEMODE | ((uint32_t)PeripheralAddr & I2C_DEV_ADDR_MSK));

    // send the register address to be read from
    pI2C->txdata = RegAddr;

    if (cb_i2c_wait_for_ready_flag() != CB_PASS)
    {
      return CB_FAIL;
    }

    if (NACKflag != DRIVER_CLR) 
    {
      return CB_FAIL;
    }

    /* DMA Setup */
    I2CDMAConfig.DMAChannel       = DMAChannel;
    I2CDMAConfig.IRQEnable        = EN_DMA_IRQ_ENABLE;    // force enable interrupt for done flag check
    I2CDMAConfig.DataWidth        = EN_DMA_DATA_WIDTH_WORD;
    I2CDMAConfig.ContinuousMode   = EN_DMA_CONTINUOUS_MODE_DISABLE;
    I2CDMAConfig.DestAddr         = BufAddr;
    I2CDMAConfig.SrcAddr          = (uint32_t)(&(pI2C->rxdata));
    I2CDMAConfig.DataLen          = DataLen;
    I2CDMAConfig.SrcInc           = EN_DMA_SRC_ADDR_INC_DISABLE;
    I2CDMAConfig.DestInc          = EN_DMA_DEST_ADDR_INC_ENABLE;
    I2CDMAConfig.FlowControlSrc   = EN_DMA_FLOW_CONTROL_SRC_SEL_0;
    I2CDMAConfig.FlowControlDest  = EN_DMA_FLOW_CONTROL_DEST_DISABLE;
    /* Done DMA setup */

    // reset DMA_SEL field
    pI2C->fr &= ~(I2C_DMA_SEL_MSK | I2C_CR_I2C_ENABLE_MSK);

    // flow controlled by rxdata_full
    pI2C->fr |= I2C_DMA_SEL_RX;

    // clear NACK flag previously set
    tempbuf = pI2C->irq_reg;
    NACKflag = DRIVER_CLR;

    // enable CR to start transmission
    pI2C->fr |= I2C_CR_I2C_ENABLE;

    // configure CR
    pI2C->cr = ((((uint32_t)DataLen << I2C_BYTE_COUNT_POS) & I2C_BYTE_COUNT_MSK) |
                I2C_NACK_LAST_BYTE | I2C_NO_INCLUDE_REG_ADDR |
                I2C_READMODE |
                ((uint32_t)PeripheralAddr & I2C_DEV_ADDR_MSK));

    cb_dma_setup(&I2CDMAConfig);
    cb_dma_enable_channel(&I2CDMAConfig);

    return CB_PASS;
}

/**
 * @brief   Returns the staus of NACK flag
 *          This function is useful to check if the previous transmission has been successful
 *          or not.
 *
 * @return  CB_TRUE          NACK flag is SET
 *          CB_FAIL          NACK flag is CLEARED
 */
uint8_t cb_i2c_is_nack(void)
{
    if (NACKflag == DRIVER_SET)
    {
      return CB_TRUE;
    }

    return CB_FALSE;
}


/**
 * @brief   Configure I2C interrupts.
 * @param[in] I2C_INT     Specifies the I2C interrupts sources to be enabled or disabled.
 *                      This parameter can be any combination of the following values:
 *                      - @arg EN_I2C_INT_RX_FULL: Enable interrupt when the receive buffer is full.
 *                      - @arg EN_I2C_INT_TX_EMPTY: Enable interrupt when the transmit buffer is empty.
 *                      - @arg EN_I2C_INT_READY: Enable interrupt when the I2C is ready.
 *                      - @arg EN_I2C_INT_NACK: Enable interrupt when a NACK is received.
 * @param[in] NewState   The new state for the interrupt. Use ENABLE to enable the interrupt and DISABLE to disable it.
 */
void cb_i2c_int_config(uint16_t I2C_INT, uint8_t NewState)
{
    if (NewState == CB_TRUE)
    {
        pI2C->irq_en |= I2C_INT;  // Enable the specified I2C interrupt
    }
    else
    {
        pI2C->irq_en &= ~I2C_INT;  // Disable the specified I2C interrupt
    }
}

/**
 * @brief   Get and clear the status of I2C interrupts.
 * @param[in] I2C_INT     Specifies the I2C interrupts sources to check and clear.
 *                      This parameter can be any combination of the following values:
 *                      - @arg EN_I2C_INT_RX_FULL: Check and clear the receive buffer full interrupt.
 *                      - @arg EN_I2C_INT_TX_EMPTY: Check and clear the transmit buffer empty interrupt.
 *                      - @arg EN_I2C_INT_READY: Check and clear the I2C ready interrupt.
 *                      - @arg EN_I2C_INT_NACK: Check and clear the NACK received interrupt.
 *                      - @arg EN_I2C_INT_ALL: Check and clear all I2C interrupts.
 * @return              The status of the specified I2C interrupts. Each bit corresponds to the status of one interrupt source.
 */
uint8_t cb_i2c_get_and_clear_int_status(uint16_t I2C_INT)
{
    uint16_t irqstatus = pI2C->irq_reg;  // Reading clears the specified interrupts
    return irqstatus & I2C_INT;           // Return only the specified interrupt statuses
}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
