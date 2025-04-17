/**
  * @file    CB_spi.c
  * @brief   SPI communication interface implementation
  * @details This source file provides the implementation of the functions for initializing and using the SPI interface for communication between
  * a master device and one or more slave devices.
  * @author  Chipsbank
  * @date    2024
*/

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_spi.h"
#include "CB_scr.h"
#include "CB_PeripheralPhyAddrDataBase.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_MIN_TIMEOUT_CYCLE 2048 //Lowest Speed: 256KHz, 1byte=8 SPI Clock Pulse
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
volatile static uint8_t gSPIFifoTerminateFlag = DRIVER_CLR; // Main purpose is to abort the transmission in FIFO Mode
volatile static enTransmissonMode UW1000_SPIMode; //Use to differentiate the SPI operation 
static volatile uint32_t gReservedOnSPISDMAPnt[1];
static uint8_t (*SPI_intrpt_r_cbk)(uint8_t) = NULL; //Response Callback function
static void (*SPI_intrpt_c_cbk)(uint8_t) = NULL; //Complete Notification Callback Function
static uint8_t Slave_MOSIData;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/**
 * @brief Stops the SPI communication and sets the termination flag.
 *
 * This function halts the ongoing SPI operation by calling cb_spi_stop()
 * and then sets the global variable gSPIFifoTerminateFlag to DRIVER_SET.
 * This indicates that the SPI transmission should be terminated.
 */
inline static void cb_spi_stop_and_set_terminate_flag(void)
{
    cb_spi_stop();
    gSPIFifoTerminateFlag = DRIVER_SET;
}

/**
 * @brief Handles the SPI interrupt events.
 */
void cb_spi_irqhandler(void)
{
    pSPI->SPI_INT_CLR |= SPI_int_clr;
    switch (UW1000_SPIMode)
    {
      case EN_SPI_IDLE:
        break;
      case EN_SPI_MASTERFIFOTX:
        if((pSPI->SPI_EVENT &(SPI_EVENT_TXFIFO_OVR_ERR | SPI_EVENT_TX_END)) != DRIVER_CLR)
        {
          cb_spi_stop_and_set_terminate_flag();
        }  
        break;
      case EN_SPI_MASTERFIFORX:
        if((pSPI->SPI_EVENT &(SPI_EVENT_RXFIFO_OVF_ERR | SPI_EVENT_RX_END)) != DRIVER_CLR)
        {
          cb_spi_stop_and_set_terminate_flag();
        }  
        break;
      case EN_SPI_MASTERFIFOTRX_1BYTECOMMAND:
          if(  ((pSPI->SPI_EVENT &(SPI_EVENT_TX_END|SPI_EVENT_RX_END)) == (SPI_EVENT_TX_END|SPI_EVENT_RX_END)))
        {
          cb_spi_stop_and_set_terminate_flag();      
        }
        break;
      case EN_SPI_MASTERSDMATX:
        if(( cb_spi_get_irq_flags() &(SPI_EVENT_TXB_RD_ERR|SPI_EVENT_TX_END)) != DRIVER_CLR)
        {
          cb_spi_stop();
        }  
        break;
      case EN_SPI_MASTERSDMARX:
        if((pSPI->SPI_EVENT &(SPI_EVENT_RXB_WR_ERR|SPI_EVENT_RX_END)) != DRIVER_CLR)
        {
          cb_spi_stop();
        }        
        break;
      case EN_SPI_MASTERSDMATRX:
        if(((pSPI->SPI_EVENT &(SPI_EVENT_TXFIFO_OVR_ERR|SPI_EVENT_RXFIFO_OVF_ERR)) != DRIVER_CLR)||
            ((pSPI->SPI_EVENT &(SPI_EVENT_TX_END|SPI_EVENT_RX_END)) == (SPI_EVENT_TX_END|SPI_EVENT_RX_END)))
        {
          cb_spi_stop_and_set_terminate_flag();
        }
        break;
      case EN_SPI_SLAVEFIFO:
        if((pSPI->SPI_EVENT &(SPI_EVENT_RX_END)) == (SPI_EVENT_RX_END))
        {
          for (uint16_t timeout = 0;timeout < DEF_MIN_TIMEOUT_CYCLE;timeout++)
          {
            if ((pSPI->SPI_EVENT & SPI_EVENT_SPI_ON)== DRIVER_CLR)
              {
                break;
              }
          }
          cb_spi_stop_and_set_terminate_flag();
          if (SPI_intrpt_c_cbk!=NULL)
          {
            /*Passing Parameter: Byte counts matched to length set? If Yes: CB_PASS, No: CB_FAIL*/
            SPI_intrpt_c_cbk(
              ((pSPI->SPI_TRX_ST&(SPI_txb_nbyte_Msk|SPI_rxb_nbyte_Msk))==(pSPI->SPI_BUF_SIZE&(SPI_tx_max_bytes_Msk|SPI_rx_max_bytes_Msk)))?  (uint8_t)CB_PASS:(uint8_t)CB_FAIL
            );
          }    
        }
        else if((pSPI->SPI_TRX_ST &(SPI_rxfifo_nbyte)) != DRIVER_CLR)
        {
          /*Enter Critical Section*/  
          /*SysTick has the higher priority than SPI, it will pre-empt SPI and affect the FIFO MISO response*/
          //SysTick_Disable();        
          uint16_t RxLengthToReceive = ((pSPI->SPI_BUF_SIZE &SPI_rx_max_bytes)>>SPI_rx_max_bytes_Pos);
          /*To discard 1st byte data. Counter measure of RTL issue*/
          //https://www.notion.so/UW1000_TO2-Comprehensive-module-TC-Implementation-SPI-Slave-small-size-communication-bf7289ab4134434c9e9725e689646b10
          Slave_MOSIData = (uint8_t) pSPI->SPI_RXFIFO;
          RxLengthToReceive--;      

          while(RxLengthToReceive> 0)
          {
            if((pSPI->SPI_TRX_ST &(SPI_rxfifo_nbyte)) != DRIVER_CLR)
            {
              /*Receiving Part*/
              Slave_MOSIData = (uint8_t) pSPI->SPI_RXFIFO;
              RxLengthToReceive--;
              
              /*MISO Response Part*/
              if (SPI_intrpt_r_cbk!=NULL)
              {
                /*Callback to response MISO Data*/
                pSPI->SPI_TXFIFO = SPI_intrpt_r_cbk(Slave_MOSIData);
              }
              else
              {
                /*Dummy Response: Response Zero*/
                pSPI->SPI_TXFIFO = 0x00;
              }
            }
            /*Exit Critical Section*/
            /*SysTick has the higher priority than SPI, it will pre-empt SPI and affect the FIFO MISO response*/
            //SysTick_Init();
          }
        }
        break;
      case EN_SPI_SLAVESMDA:
          for (uint16_t timeout = 0;timeout < DEF_MIN_TIMEOUT_CYCLE;timeout++)
          {
            if ((pSPI->SPI_EVENT & SPI_EVENT_SPI_ON)== DRIVER_CLR)
              {
                break;
              }
          }
          cb_spi_stop_and_set_terminate_flag();
          if (SPI_intrpt_c_cbk!=NULL)
          {
            /*Passing Parameter: Byte counts matched to length set? If Yes: CB_PASS, No: CB_FAIL*/
            SPI_intrpt_c_cbk(
              ((pSPI->SPI_TRX_ST&(SPI_txb_nbyte_Msk|SPI_rxb_nbyte_Msk))==(pSPI->SPI_BUF_SIZE&(SPI_tx_max_bytes_Msk|SPI_rx_max_bytes_Msk)))?  (uint8_t)CB_PASS:(uint8_t)CB_FAIL
            );
          }
        break;
    }
}

/**
 * @brief Initializes the SPI module with the specified parameters.
*/
void cb_spi_init(stSPI_InitTypeDef *InitParameters)
{
    uint32_t tempbuf;
    /*SPI Module Power On and Reset Release*/
    cb_scr_spi_module_on();

    /*Enable SPI Module*/
    pSPI->SPI_EN |= spi_en;

    /*Clear Interrupt*/
    pSPI->SPI_INT_CLR |= SPI_int_clr;

    /*Enable SPI IRQ*/
    NVIC_EnableIRQ(SPI_IRQn);
    enSPIBitOrderphy bitorder;
    switch (InitParameters->BitOrder)
    {
      case EN_SPI_bitorder_lsb_first:
        bitorder=EN_SPI_bitorder_lsb_first_phy;
        break;
      case EN_SPI_bitorder_msb_first:
        bitorder=EN_SPI_bitorder_msb_first_phy;
        break;
      default:bitorder=EN_SPI_bitorder_msb_first_phy; break; 
    }
    enSPIByteOrderphy byteorder;
    switch (InitParameters->ByteOrder)
    {
      case EN_SPI_byteorder_byte0_first:
        byteorder=EN_SPI_byteorder_byte0_first_phy;
        break;
      case EN_SPI_byteorder_byte3_first:
        byteorder=EN_SPI_byteorder_byte3_first_phy;
        break;
      default:byteorder=EN_SPI_byteorder_byte0_first_phy;break;
    }
    enSPIClockSpeedphy speed;
    switch (InitParameters->Speed)
    {
      case EN_SPI32MHz:
        speed=EN_SPI32MHz_phy;
        break;
      case EN_SPI16MHz:
        speed=EN_SPI16MHz_phy;
        break;
      case EN_SPI8MHz:
        speed=EN_SPI8MHz_phy;
        break;
      case EN_SPI4MHz:
        speed=EN_SPI4MHz_phy;
        break;
      case EN_SPI2MHz:
        speed=EN_SPI2MHz_phy;
        break;
      case EN_SPI1MHz:
        speed=EN_SPI1MHz_phy;
        break;
      case EN_SPI512KHz:
        speed=EN_SPI512KHz_phy;
        break;
      case EN_SPI256KHz:
        speed=EN_SPI256KHz_phy;
        break;
      default:speed=EN_SPI2MHz_phy;break;
    }
    enSPIModephy spimode;
    switch (InitParameters->SpiMode)
    {
      case EN_SPI_mode0:
        spimode=EN_SPI_mode0_phy;
        break;
      case EN_SPI_mode2:
        spimode=EN_SPI_mode2_phy;
        break;
      default:spimode=EN_SPI_mode0_phy;break;
    }
    tempbuf = pSPI->SPI_CFG;
    tempbuf &= ~(SPI_clk_sel_Msk | SPI_byteorder_Msk| SPI_bitorder_Msk | SPI_mode_Msk );
    tempbuf |= (uint32_t)(speed) | 
                (uint32_t)(byteorder)| 
                (uint32_t)(bitorder)| 
                (uint32_t)(spimode);

    pSPI->SPI_CFG = tempbuf;

    UW1000_SPIMode = EN_SPI_IDLE;

}

/**
 * @brief Deinitializes the SPI module.
 */
void cb_spi_deinit(void)
{
    /*Abort any on-going tranmission*/
    cb_spi_stop();

    /*Clear fptr*/
    SPI_intrpt_r_cbk = NULL;
    SPI_intrpt_c_cbk = NULL;

    /*Clear Interrupt and Config setting*/
    pSPI->SPI_INT_EN = 0x00000000UL;
    pSPI->SPI_INT_CLR |= SPI_int_clr;
    gSPIFifoTerminateFlag = DRIVER_CLR;
    pSPI->SPI_CFG = 0x00000000UL;

    /*Disable SPI Module*/
    pSPI->SPI_EN &= ~(spi_en);

    /*Disable SPI IRQ*/
    NVIC_DisableIRQ(SPI_IRQn);

    /*Disable CRC Module*/
    cb_scr_spi_module_off();
}

/**
 * @brief Initializes the SPI configuration structure with default values.
 *
 * This function sets the default configuration parameters for the SPI interface.
 *
 * @param InitParameters Pointer to the SPI initialization structure to be initialized.
 */
void cb_spi_struct_init(stSPI_InitTypeDef *InitParameters)
{
    if (InitParameters == NULL) 
    {
        return; // Return if the pointer is NULL to avoid dereferencing a null pointer
    }

    // Set default bit order to MSB first
    InitParameters->BitOrder = EN_SPI_bitorder_msb_first;

    // Set default byte order to byte 0 first
    InitParameters->ByteOrder = EN_SPI_byteorder_byte0_first;

    // Set default SPI speed to 2 MHz
    InitParameters->Speed = EN_SPI2MHz;

    // Set default SPI mode to Mode 0
    InitParameters->SpiMode = EN_SPI_mode0;
}

/**
 * @brief Enables or disables the SPI module.
 *
 * This function enables or disables the SPI module based on the provided command.
 *
 * @param Cmd Command to enable or disable the SPI module.
 *            - SPI_DISABLE: Disable the SPI module.
 *            - SPI_ENABLE: Enable the SPI module.
 */
void cb_spi_cmd(uint8_t Cmd)
{
    if (Cmd == CB_TRUE)
    {
      /* Enable SPI Module */
      pSPI->SPI_EN |= spi_en;
      
      /* Enable SPI IRQ */
      NVIC_EnableIRQ(SPI_IRQn);
    }
    else if (Cmd == CB_FAIL)
    {
      /* Disable SPI Module */
      pSPI->SPI_EN &= ~(spi_en);
      
      /* Disable SPI IRQ */
      NVIC_DisableIRQ(SPI_IRQn);
    }
}

/**
 * @brief Stops the ongoing SPI operation.
 */
void cb_spi_stop(void)
{
    if ((pSPI->SPI_EVENT &SPI_EVENT_SPI_ON) == SPI_EVENT_SPI_ON)
    {
      /*Stop SPI*/
      pSPI->SPI_START = spi_stop;
    }
    UW1000_SPIMode = EN_SPI_IDLE;
}

/**
 * @brief Transmits data over SPI in FIFO mode.
 * @param ChipSelct Chip select for SPI device.
 * @param txdata Pointer to the data buffer to be transmitted.
 * @param Length Number of bytes to transmit.
 * @return CB_STATUS Status of the operation, CB_PASS if successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_fifo_write(enSPIChipSelect ChipSelct,uint8_t* txdata,uint16_t Length)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    uint16_t XferCount;
    enSPIChipSelectphy CS;
    switch (ChipSelct)
    {
      case EN_SPI_USE_CS0:
        CS=EN_SPI_USE_CS0_phy;
        break;
      case EN_SPI_USE_CS1:
        CS=EN_SPI_USE_CS1_phy;
        break;
      case EN_SPI_USE_CS2:
        CS=EN_SPI_USE_CS2_phy;
        break;  
      case EN_SPI_USE_CS3:
        CS=EN_SPI_USE_CS3_phy;
        break;
      case EN_SPI_USE_CS4:
        CS=EN_SPI_USE_CS4_phy;
        break;
      default:CS=EN_SPI_USE_CS0_phy;break;
    }
    /*Set ChipSlect and SPI Type*/
    tempbuf = pSPI->SPI_CFG; 
    tempbuf &= ~(CS_ALL_MASK|SPI_type_Msk);
    tempbuf |= CS|SPI_Type_Master;
    pSPI->SPI_CFG = tempbuf;

    if ((UW1000_SPIMode!= EN_SPI_IDLE) || ((pSPI->SPI_EVENT &SPI_EVENT_SPI_ON) ==SPI_EVENT_SPI_ON))
    {
      return ret; //Return Error: SPI is Busy
    }
    else
    {
      UW1000_SPIMode = EN_SPI_MASTERFIFOTX;
    }

    /*Enable Interrupt for TXOverRunError & TXEndTransmission*/
    pSPI->SPI_INT_EN = SPI_TXFIFO_OVR_ERR|SPI_TX_END;

    /*Disable DMA, Use FIFO Instead*/
    pSPI->SPI_BUF_EN &= ~(SPI_buf_en|SPI_buf_min_bytes_Msk);

    pSPI->SPI_BUF_SIZE = (((((uint32_t)Length) <<SPI_rx_max_bytes_Pos)&SPI_rx_max_bytes)|((((uint32_t)Length) <<SPI_tx_max_bytes_Pos)&SPI_tx_max_bytes)); //Setting as N

    /*Single Byte write into FIFO*/
    pSPI->SPI_TRXFIFO = (((uint32_t)0x0UL)<<SPI_nwritebytes_Pos)&SPI_nwritebytes_Msk;    

    XferCount = Length;
    /*Start SPI*/
    pSPI->SPI_START = spi_start;
    gSPIFifoTerminateFlag =DRIVER_CLR;

    do
    {
      if ((pSPI->SPI_EVENT&SPI_EVENT_TXFIFO_EMPTY) == SPI_EVENT_TXFIFO_EMPTY)
      {
        pSPI->SPI_TXFIFO = *txdata;
        txdata++;
        XferCount--;
      }
      if (gSPIFifoTerminateFlag == DRIVER_SET)break;
    }while (XferCount > 0U);

    /*Check Results*/
    tempbuf = (pSPI->SPI_TRX_ST & SPI_txb_nbyte_Msk)>>SPI_txb_nbyte_Pos;
    if (tempbuf == Length)
    {
      ret = CB_PASS;
    }

    return ret;
}

/**
 * @brief Reads data from SPI in FIFO mode.
 * @param ChipSelct Chip select for SPI device.
 * @param rxdata Pointer to the buffer where received data will be stored.
 * @param Length Number of bytes to read.
 * @return CB_STATUS Status of the operation, CB_PASS if successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_fifo_read(enSPIChipSelect ChipSelct,uint8_t* rxdata,uint16_t Length)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    uint16_t XferCount;
    enSPIChipSelectphy CS;
    switch (ChipSelct)
    {
      case EN_SPI_USE_CS0:
        CS=EN_SPI_USE_CS0_phy;
        break;
      case EN_SPI_USE_CS1:
        CS=EN_SPI_USE_CS1_phy;
        break;
      case EN_SPI_USE_CS2:
        CS=EN_SPI_USE_CS2_phy;
        break;  
      case EN_SPI_USE_CS3:
        CS=EN_SPI_USE_CS3_phy;
        break;
      case EN_SPI_USE_CS4:
        CS=EN_SPI_USE_CS4_phy;
        break;
      default:CS=EN_SPI_USE_CS0_phy;break; 
    }
    /*Set ChipSlect and SPI Type*/
    tempbuf = pSPI->SPI_CFG; 
    tempbuf &= ~(CS_ALL_MASK|SPI_type_Msk);
    tempbuf |= CS|SPI_Type_Master;
    pSPI->SPI_CFG = tempbuf;

    if ((UW1000_SPIMode!= EN_SPI_IDLE) || ((pSPI->SPI_EVENT &SPI_EVENT_SPI_ON) ==SPI_EVENT_SPI_ON))
    {
      return ret; //Return Error: SPI is Busy
    }
    else
    {
      UW1000_SPIMode = EN_SPI_MASTERFIFORX;
    }

    /*Enable Interrupt for RXOverFlowError & RXEndTransmission*/
    pSPI->SPI_INT_EN = SPI_RXFIFO_OVF_ERR|SPI_RX_END;

    /*Disable DMA, Use FIFO Instead*/
    pSPI->SPI_BUF_EN &= ~(SPI_buf_en|SPI_buf_min_bytes_Msk);

    pSPI->SPI_BUF_SIZE = (((((uint32_t)Length) <<SPI_rx_max_bytes_Pos)&SPI_rx_max_bytes)|((((uint32_t)Length) <<SPI_tx_max_bytes_Pos)&SPI_tx_max_bytes)); //Setting as N

    /*Single Byte read into FIFO*/
    pSPI->SPI_TRXFIFO = (((uint32_t)0x0UL)<<SPI_nreadbytes_Pos)&SPI_nreadbytes_Msk;    

    XferCount = Length;
    /*Start SPI*/
    pSPI->SPI_START = spi_start;
    gSPIFifoTerminateFlag =DRIVER_CLR;

    /*To initiate/start SPI waveform output */
    pSPI->SPI_TXFIFO = 0x00;

    do
    {
      if ((pSPI->SPI_EVENT&SPI_EVENT_RXFIFO_EMPTY) != SPI_EVENT_RXFIFO_EMPTY)
      {
        *rxdata =(uint8_t) pSPI->SPI_RXFIFO;
        rxdata++;
        XferCount--;
      }
      if (gSPIFifoTerminateFlag == DRIVER_SET)
      {
        if (((pSPI->SPI_EVENT&SPI_EVENT_RXFIFO_EMPTY) != SPI_EVENT_RXFIFO_EMPTY)&& (XferCount != DRIVER_CLR))
        {
          *rxdata =(uint8_t) pSPI->SPI_RXFIFO;
        }
        break;
      }
    }while (XferCount > 0U);

    /*Check Results*/
    tempbuf = (pSPI->SPI_TRX_ST & SPI_rxb_nbyte_Msk)>>SPI_rxb_nbyte_Pos;
    if (tempbuf == Length)
    {
      ret = CB_PASS;
    }

    return ret;
}

/**
 * @brief Performs single-byte read or write transaction on SPI using FIFO.
 * @param ChipSelct The SPI chip select to use for the transaction.
 * @param ReadOrWrite Specifies whether the transaction is read or write.
 * @param command Command byte to send in the transaction.
 * @param trxbuffer Pointer to the buffer for read/write data.
 * @param Length Length of data to read or write.
 * @return CB_STATUS CB_PASS if the transaction is successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_fifo_write_read(enSPIChipSelect ChipSelct,enTRXCommandReadWriteBit ReadOrWrite,uint8_t command,uint8_t* trxbuffer,uint16_t Length)
{
    CB_STATUS ret = CB_FAIL;
    uint8_t txfirstcommandbyte;
    uint32_t tempbuf;
    uint16_t XferCount;
    enSPIChipSelectphy CS;

    if (command > 0x7F)
    {
      return ret; //Return Error: Invalid Command
    }
    else
    {
      txfirstcommandbyte = command;
      if (ReadOrWrite == EN_SPI_TRX_READ)
      {
        txfirstcommandbyte =  command|0x80; //Insert ReadBit
      }
    }

    switch (ChipSelct)
    {
      case EN_SPI_USE_CS0:
        CS=EN_SPI_USE_CS0_phy;
        break;
      case EN_SPI_USE_CS1:
        CS=EN_SPI_USE_CS1_phy;
        break;
      case EN_SPI_USE_CS2:
        CS=EN_SPI_USE_CS2_phy;
        break;  
      case EN_SPI_USE_CS3:
        CS=EN_SPI_USE_CS3_phy;
        break;
      case EN_SPI_USE_CS4:
        CS=EN_SPI_USE_CS4_phy;
        break;
      default:CS=EN_SPI_USE_CS0_phy;break;  
    }
    /*Set ChipSlect and SPI Type*/
    tempbuf = pSPI->SPI_CFG; 
    tempbuf &= ~(CS_ALL_MASK|SPI_type_Msk);
    tempbuf |= CS|SPI_Type_Master;
    pSPI->SPI_CFG = tempbuf;

    if ((UW1000_SPIMode!= EN_SPI_IDLE) || ((pSPI->SPI_EVENT &SPI_EVENT_SPI_ON) ==SPI_EVENT_SPI_ON))
    {
      return ret; //Return Error: SPI is Busy
    }
    else
    {
      UW1000_SPIMode = EN_SPI_MASTERFIFOTRX_1BYTECOMMAND;
    }

    /*Enable Interrupt*/
    pSPI->SPI_INT_EN = SPI_RXFIFO_OVF_ERR|SPI_TXFIFO_OVR_ERR|SPI_RX_END|SPI_TX_END;

    /*Disable DMA, Use FIFO Instead*/
    pSPI->SPI_BUF_EN &= ~(SPI_buf_en|SPI_buf_min_bytes_Msk);

    pSPI->SPI_BUF_SIZE = (((((uint32_t)Length+1) <<SPI_rx_max_bytes_Pos)&SPI_rx_max_bytes)|((((uint32_t)Length+1) <<SPI_tx_max_bytes_Pos)&SPI_tx_max_bytes)); //Setting as N

    /*Single Byte Access into FIFO*/
    pSPI->SPI_TRXFIFO = ((((uint32_t)0x0UL)<<SPI_nwritebytes_Pos)&SPI_nwritebytes_Msk) |((((uint32_t)0x0UL)<<SPI_nreadbytes_Pos)&SPI_nreadbytes_Msk);    

    XferCount = Length;
    /*Start SPI*/
    pSPI->SPI_START = spi_start;
    gSPIFifoTerminateFlag =DRIVER_CLR;

    if ((pSPI->SPI_EVENT&SPI_EVENT_TXFIFO_FULL) != SPI_EVENT_TXFIFO_FULL)
    {
      pSPI->SPI_TXFIFO = txfirstcommandbyte;
      if (ReadOrWrite == EN_SPI_TRX_READ)
      {
        while ((pSPI->SPI_EVENT&(SPI_EVENT_RXFIFO_EMPTY)) == (SPI_EVENT_RXFIFO_EMPTY));//Busy Wait for Tx Send out 1st Command Byte
        /*Ignore first byte (Meaningless RxData(MISO) on Comamnd Byte)*/
        tempbuf = pSPI->SPI_RXFIFO;
      }
    }
    else
    {
      cb_spi_stop();
      return ret; //Error TX FIFO FULL.
    }

    do
    {
      if (ReadOrWrite == EN_SPI_TRX_READ)
      {
        if ((pSPI->SPI_EVENT&SPI_EVENT_RXFIFO_EMPTY) != SPI_EVENT_RXFIFO_EMPTY)
        {
          *trxbuffer =(uint8_t) pSPI->SPI_RXFIFO;
          trxbuffer++;
          XferCount--;
        }
        if (gSPIFifoTerminateFlag == DRIVER_SET)
        {
          if (((pSPI->SPI_EVENT&SPI_EVENT_RXFIFO_EMPTY) != SPI_EVENT_RXFIFO_EMPTY)&& (XferCount != DRIVER_CLR))
          {
            *trxbuffer =(uint8_t) pSPI->SPI_RXFIFO;
          }
          break;
        }
      }
      else
      {
        if ((pSPI->SPI_EVENT&SPI_EVENT_TXFIFO_EMPTY) == SPI_EVENT_TXFIFO_EMPTY)
        {
          pSPI->SPI_TXFIFO = *trxbuffer;
          trxbuffer++;
          XferCount--;
        }
      }
      if (gSPIFifoTerminateFlag == DRIVER_SET)break;
    }while (XferCount > 0U);

    /*Check Results*/
    if (ReadOrWrite == EN_SPI_TRX_READ)
    {
      tempbuf = (pSPI->SPI_TRX_ST & SPI_rxb_nbyte_Msk)>>SPI_rxb_nbyte_Pos;
    }
    else
    {
      tempbuf = (pSPI->SPI_TRX_ST & SPI_txb_nbyte_Msk)>>SPI_txb_nbyte_Pos;
    }
    if (tempbuf == (Length+1))//Data Lengeth+1 Command byte;
    {
      ret = CB_PASS;
    }

    return ret;
}

/**
 * @brief Writes data over SPI in SDMA mode.
 * @param ChipSelct Chip select for SPI device.
 * @param Length Number of bytes to transmit.
 * @param StartAddr Starting address of the transmit buffer.
 * @return CB_STATUS Status of the operation, CB_PASS if successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_sdma_write(enSPIChipSelect ChipSelct,uint16_t Length, uint32_t StartAddr)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    enSPIChipSelectphy CS;
    switch (ChipSelct)
    {
      case EN_SPI_USE_CS0:
        CS=EN_SPI_USE_CS0_phy;
        break;
      case EN_SPI_USE_CS1:
        CS=EN_SPI_USE_CS1_phy;
        break;
      case EN_SPI_USE_CS2:
        CS=EN_SPI_USE_CS2_phy;
        break;  
      case EN_SPI_USE_CS3:
        CS=EN_SPI_USE_CS3_phy;
        break;
      case EN_SPI_USE_CS4:
        CS=EN_SPI_USE_CS4_phy;
        break;
      default:CS=EN_SPI_USE_CS0_phy;break; 
    }
    /*Set ChipSlect and SPI Type*/
    tempbuf = pSPI->SPI_CFG; 
    tempbuf &= ~(CS_ALL_MASK|SPI_type_Msk);
    tempbuf |= CS|SPI_Type_Master;
    pSPI->SPI_CFG = tempbuf;

    if ((UW1000_SPIMode!= EN_SPI_IDLE) || ((pSPI->SPI_EVENT &SPI_EVENT_SPI_ON) ==SPI_EVENT_SPI_ON))
    {
      return ret; //Return Error: SPI is Busy
    }
    else
    {
      UW1000_SPIMode = EN_SPI_MASTERSDMATX;
    }

    /*Enable Interrupt for TXBufferReadError & TXEndTransmission*/
    pSPI->SPI_INT_EN = SPI_TXB_RD_ERR|SPI_TX_END;

    /*Reset DMA Buffer*/
    pSPI->SPI_BUF_EN = 0x00000000;

    /*Enable DMA*/
    pSPI->SPI_BUF_EN = spi_buf_min_2byte|SPI_buf_en;

    /*Only receive 1byte at reserved buffer to prevent data overwrite the RAM Address data*/
    pSPI->SPI_BUF_SIZE = (((((uint32_t)1) <<SPI_rx_max_bytes_Pos)&SPI_rx_max_bytes)|((((uint32_t)Length) <<SPI_tx_max_bytes_Pos)&SPI_tx_max_bytes)); //Setting as N

    /*Four Bytes (a word) write into FIFO*/
    pSPI->SPI_TRXFIFO = (((uint32_t)0x3UL)<<SPI_nwritebytes_Pos)&SPI_nwritebytes_Msk;    

    /*Set DMA TXB & RXB pointer*/
    pSPI->SPI_TXBUF = (uint32_t) StartAddr;
    pSPI->SPI_RXBUF = (uint32_t) &gReservedOnSPISDMAPnt[0];

    /*Start SPI*/
    pSPI->SPI_START = spi_start;

    return CB_PASS;
}

/**
 * @brief Reads data from SPI in SDMA mode.
 * @param ChipSelct Chip select for SPI device.
 * @param Length Number of bytes to read.
 * @param StartAddr Starting address of the receive buffer.
 * @return CB_STATUS Status of the operation, CB_PASS if successful, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_sdma_read(enSPIChipSelect ChipSelct,uint16_t Length, uint32_t StartAddr)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    enSPIChipSelectphy CS;
    switch (ChipSelct)
    {
      case EN_SPI_USE_CS0:
        CS=EN_SPI_USE_CS0_phy;
        break;
      case EN_SPI_USE_CS1:
        CS=EN_SPI_USE_CS1_phy;
        break;
      case EN_SPI_USE_CS2:
        CS=EN_SPI_USE_CS2_phy;
        break;  
      case EN_SPI_USE_CS3:
        CS=EN_SPI_USE_CS3_phy;
        break;
      case EN_SPI_USE_CS4:
        CS=EN_SPI_USE_CS4_phy;
        break;
      default:CS=EN_SPI_USE_CS0_phy;break;
    }
    /*Set ChipSlect and SPI Type*/
    tempbuf = pSPI->SPI_CFG; 
    tempbuf &= ~(CS_ALL_MASK|SPI_type_Msk);
    tempbuf |= CS|SPI_Type_Master;
    pSPI->SPI_CFG = tempbuf;

    if ((UW1000_SPIMode!= EN_SPI_IDLE) || ((pSPI->SPI_EVENT &SPI_EVENT_SPI_ON) ==SPI_EVENT_SPI_ON))
    {
      return ret; //Return Error: SPI is Busy
    }
    else
    {
      UW1000_SPIMode = EN_SPI_MASTERSDMARX;
    }

    /*Enable Interrupt for RXBufferWriteError & RXEndTransmission*/
    pSPI->SPI_INT_EN = SPI_EVENT_RXB_WR_ERR|SPI_RX_END;

    /*Reset DMA Buffer*/
    pSPI->SPI_BUF_EN = 0x00000000;

    /*Enable DMA*/
    pSPI->SPI_BUF_EN = spi_buf_min_1byte|SPI_buf_en;

    pSPI->SPI_BUF_SIZE = (((((uint32_t)Length) <<SPI_rx_max_bytes_Pos)&SPI_rx_max_bytes)|((((uint32_t)Length) <<SPI_tx_max_bytes_Pos)&SPI_tx_max_bytes)); //Setting as N

    /*Four Bytes (a word) write into FIFO*/
    pSPI->SPI_TRXFIFO = (((uint32_t)0x3UL)<<SPI_nreadbytes_Pos)&SPI_nreadbytes_Msk;    

    /*Set DMA TXB & RXB pointer*/
    pSPI->SPI_TXBUF = (uint32_t) &gReservedOnSPISDMAPnt[0];
    pSPI->SPI_RXBUF = (uint32_t) StartAddr;

    /*Start SPI*/
    pSPI->SPI_START = spi_start;

    return CB_PASS;
}

/**
 * @brief Performs DMA-based write and read transaction on SPI.
 * @param ChipSelct The SPI chip select to use for the transaction.
 * @param Length Length of data to transfer.
 * @param TxBufAddr DMA buffer address for transmit data.
 * @param RxBufAddr DMA buffer address for receive data.
 * @return CB_STATUS CB_PASS if the transaction is successfully initiated, otherwise CB_FAIL.
 */
CB_STATUS cb_spi_master_sdma_write_read(enSPIChipSelect ChipSelct,uint16_t Length, uint32_t TxBufAddr, uint32_t RxBufAddr)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    enSPIChipSelectphy CS;
    switch (ChipSelct)
    {
      case EN_SPI_USE_CS0:
        CS=EN_SPI_USE_CS0_phy;
        break;
      case EN_SPI_USE_CS1:
        CS=EN_SPI_USE_CS1_phy;
        break;
      case EN_SPI_USE_CS2:
        CS=EN_SPI_USE_CS2_phy;
        break;  
      case EN_SPI_USE_CS3:
        CS=EN_SPI_USE_CS3_phy;
        break;
      case EN_SPI_USE_CS4:
        CS=EN_SPI_USE_CS4_phy;
        break;
      default:CS=EN_SPI_USE_CS0_phy;break;
    }
    /*Set ChipSlect and SPI Type*/
    tempbuf = pSPI->SPI_CFG; 
    tempbuf &= ~(CS_ALL_MASK|SPI_type_Msk);
    tempbuf |= CS|SPI_Type_Master;
    pSPI->SPI_CFG = tempbuf;

    if ((UW1000_SPIMode!= EN_SPI_IDLE) || ((pSPI->SPI_EVENT &SPI_EVENT_SPI_ON) ==SPI_EVENT_SPI_ON))
    {
      return ret; //Return Error: SPI is Busy
    }
    else
    {
      UW1000_SPIMode = EN_SPI_MASTERSDMATRX;
    }

    /*Enable Interrupt*/
    pSPI->SPI_INT_EN = SPI_RXB_WR_ERR|SPI_TXB_RD_ERR|SPI_RX_END|SPI_TX_END;

    /*Reset DMA Buffer*/
    pSPI->SPI_BUF_EN = 0x00000000;

    /*Enable DMA*/
    pSPI->SPI_BUF_EN = spi_buf_min_1byte|SPI_buf_en;

    pSPI->SPI_BUF_SIZE = ((((uint32_t)Length) <<SPI_rx_max_bytes_Pos)&SPI_rx_max_bytes)|((((uint32_t)Length) <<SPI_tx_max_bytes_Pos)&SPI_tx_max_bytes); //Setting as N

    /*Four Bytes (a word) write into FIFO*/
    pSPI->SPI_TRXFIFO = (((uint32_t)0x3UL)<<SPI_nwritebytes_Pos)&SPI_nwritebytes_Msk;    

    /*Set DMA TXB & RXB pointer*/
    pSPI->SPI_TXBUF = (uint32_t) TxBufAddr;
    pSPI->SPI_RXBUF = (uint32_t) RxBufAddr;

    /*Start SPI*/
    pSPI->SPI_START = spi_start;

    return CB_PASS;
}

/**
 * @brief Starts SPI operation in slave mode using FIFO.
 * @param Length Length of data to be handled by SPI FIFO.
 * @param p_ResponseCB Pointer to the callback function.
 * @param p_CompletionCB Pointer to the callback function for completion notification.
 * @return CB_STATUS CB_PASS if SPI operation is successfully started, otherwise CB_FAIL.
 */
/*Register a Callback Func on SPI IRQ Handler to response of MISO Data to SPI Master*/
CB_STATUS cb_spi_slave_fifo_start(uint16_t Length,spi_responsecallbk p_ResponseCB, spi_completecallbk p_CompletionCB)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;

    //Registering UserCallbackFuncPtr for FIFO Response
    if (p_ResponseCB != NULL) 
    {
      SPI_intrpt_r_cbk = p_ResponseCB;
    }
    //Registering UserCallbackFuncPtr for Completion Notification
    if (p_CompletionCB != NULL) 
    {
      SPI_intrpt_c_cbk = p_CompletionCB;
    }

    if ((UW1000_SPIMode!= EN_SPI_IDLE))
    {
      return ret; //Return Error: SPI is Busy
    }
    else
    {
      UW1000_SPIMode = EN_SPI_SLAVEFIFO;
    }

    /*Enable Interrupt for TXFIFO_EMPTY &RX_END*/
    pSPI->SPI_INT_EN = SPI_TXFIFO_EMPTY|SPI_RX_END;

    /*Disable DMA, Use FIFO Instead*/
    pSPI->SPI_BUF_EN &= ~(SPI_buf_en|SPI_buf_min_bytes_Msk);

    pSPI->SPI_BUF_SIZE = (((((uint32_t)Length) <<SPI_rx_max_bytes_Pos)&SPI_rx_max_bytes)|((((uint32_t)Length) <<SPI_tx_max_bytes_Pos)&SPI_tx_max_bytes)); //Setting as N

    pSPI->SPI_RXB_RDY |= (uint32_t)SPI_rxbuf_readyflag;

    /*Single Byte Access into FIFO*/
    pSPI->SPI_TRXFIFO = ((((uint32_t)0x0UL)<<SPI_nwritebytes_Pos)&SPI_nwritebytes_Msk) |((((uint32_t)0x0UL)<<SPI_nreadbytes_Pos)&SPI_nreadbytes_Msk);    

    /*Set SPI Type*/
    tempbuf = pSPI->SPI_CFG; 
    tempbuf &= ~(SPI_type_Msk);
    tempbuf |= SPI_Type_Slave;
    pSPI->SPI_CFG = tempbuf;

    /*Start SPI*/
    pSPI->SPI_START = spi_start;

    //https://www.notion.so/UW1000_TO2-Comprehensive-module-TC-Implementation-SPI-Slave-small-size-communication-bf7289ab4134434c9e9725e689646b10
    /*Write 2 Dummy Bytes to MISO, to hack TxFIFOEmpty IRQ trigger*/
    pSPI->SPI_TXFIFO = 0xFF;
    pSPI->SPI_TXFIFO = 0xFF;

    gSPIFifoTerminateFlag =DRIVER_SET;

    ret = CB_PASS;
    return ret;
}

/**
 * @brief Starts SPI operation in slave mode using SDMA.
 *
 *                         !!!!!!!!!!!!!!!!!  Attention  !!!!!!!!!!!!!!!!!!
 * In SDMA mode, the first byte received by the slave is a clock alignment byte and should be ignored.
 * The actual valid data starts from the second byte. Therefore, to receive n bytes of valid data, the master
 * should send n+1 bytes. After the slave receives n+1 bytes, the valid data will be stored in the buffer
 * starting from the second byte (i.e., indices 1 to n+1), while the first byte (index 0) should be ignored.
 * 
 * The host also needs to remove the first byte received when connecting to the slave,
 * that is, the host sends n+1 and can only receive the slave nbyte
 *
 * @param Length Length of data to be handled by SPI DMA. The actual valid data starts with the second byte,
 *               so to receive n bytes of valid data, the Length should be set to n+1.
 * @param TxBufAddr Memory address of the TX buffer for DMA.
 * @param RxBufAddr Memory address of the RX buffer for DMA.
 * @param p_CompletionCB Pointer to the callback function for completion notification.
 * @return CB_STATUS CB_PASS if SPI operation is successfully started, otherwise CB_FAIL.
 */
/* Register a callback function for completion notification. In SDMA mode, the first byte in the RxBuffer
 * is a clock alignment byte and should be ignored. The valid data starts from the second byte. */
CB_STATUS cb_spi_slave_sdma_start(uint16_t Length,uint32_t TxBufAddr, uint32_t RxBufAddr, spi_completecallbk p_CompletionCB)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;

    //Registering UserCallbackFuncPtr for Completion Notification
    if (p_CompletionCB != NULL) 
    {
      SPI_intrpt_c_cbk = p_CompletionCB;
    }

    if ((UW1000_SPIMode!= EN_SPI_IDLE))
    {
      return ret; //Return Error: SPI is Busy
    }
    else
    {
      UW1000_SPIMode = EN_SPI_SLAVESMDA;
    }

    /*Enable Interrupt*/
    pSPI->SPI_INT_EN = SPI_RX_END;

    /*Reset DMA Buffer*/
    pSPI->SPI_BUF_EN = 0x00000000;

    /*Enable DMA*/
    pSPI->SPI_BUF_EN = spi_buf_min_1byte|SPI_buf_en;

    pSPI->SPI_BUF_SIZE = (((((uint32_t)Length) <<SPI_rx_max_bytes_Pos)&SPI_rx_max_bytes)|((((uint32_t)Length) <<SPI_tx_max_bytes_Pos)&SPI_tx_max_bytes)); //Setting as N

    pSPI->SPI_RXB_RDY |= (uint32_t)SPI_rxbuf_readyflag;

    /*Four Bytes (a word) write into FIFO*/
    pSPI->SPI_TRXFIFO = (((uint32_t)0x0UL)<<SPI_nwritebytes_Pos)&SPI_nwritebytes_Msk;    

    /*Set SPI Type*/
    tempbuf = pSPI->SPI_CFG; 
    tempbuf &= ~(SPI_type_Msk);
    tempbuf |= SPI_Type_Slave;
    pSPI->SPI_CFG = tempbuf;

    /*Set DMA TXB & RXB pointer*/
    pSPI->SPI_TXBUF = (uint32_t) TxBufAddr;
    pSPI->SPI_RXBUF = (uint32_t) RxBufAddr;

    /*Start SPI*/
    pSPI->SPI_START = spi_start;
    gSPIFifoTerminateFlag =DRIVER_CLR;

    ret = CB_PASS;
    return ret;
}

/**
 * @brief Retrieves the current transmission mode of the SPI interface.
 */
enTransmissonMode cb_spi_get_current_transmission_mode(void)
{
    return UW1000_SPIMode;
}

/**
 * @brief Configures the SPI interrupts.
 *
 * This function configures the specified SPI interrupt based on the provided command.
 *
 * @param SPI_IT Specifies the SPI interrupt to be configured.
 *               - SPI_FRAME_END: Frame end interrupt.
 *               - SPI_TX_END: Transmission end interrupt.
 *               - SPI_RX_END: Reception end interrupt.
 *               - SPI_TXFIFO_EMPTY: TX FIFO empty interrupt.
 *               - SPI_TXFIFO_FULL: TX FIFO full interrupt.
 *               - SPI_TXB_EMPTY: TX buffer empty interrupt.
 *               - SPI_RXFIFO_EMPTY: RX FIFO empty interrupt.
 *               - SPI_RXFIFO_FULL: RX FIFO full interrupt.
 *               - SPI_RXB_FULL: RX buffer full interrupt����
 * @param NewState Command to enable or disable the specified interrupt.
 *            - SPI_IT_DISABLE: Disable the specified interrupt.
 *            - SPI_IT_ENABLE: Enable the specified interrupt.
 */
void cb_spi_irq_config(enSPIINT SPI_IT, uint8_t NewState)
{
    if (NewState == CB_TRUE)
    {
        /* Enable the specified SPI interrupt */
        pSPI->SPI_INT_EN |= (1UL << SPI_IT);
    }
    else if (NewState == CB_FAIL)
    {
        /* Disable the specified SPI interrupt */
        pSPI->SPI_INT_EN &= ~(1UL << SPI_IT);
    }
}

/**
 * @brief Retrieves the current SPI interrupt flags.
 *
 * This function returns the current state of the SPI interrupt flags.
 *
 * @return uint32_t The current SPI interrupt flags.
 */
uint32_t cb_spi_get_irq_flags(void)
{
    return pSPI->SPI_EVENT;
}




