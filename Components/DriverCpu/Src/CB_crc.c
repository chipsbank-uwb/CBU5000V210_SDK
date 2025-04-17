/**
 * @file    CB_crc.c
 * @brief   CRC driver implementation.
 * @details This file contains the implementation of functions for CRC driver.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_crc.h"
#include "CB_scr.h"
#include "CB_PeripheralPhyAddrDataBase.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define BYTE_SIZE 8

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stCRC_TypeDef  *pCRC  = (stCRC_TypeDef* ) CRC_BASE_ADDR;
static uint8_t gCRCInProgessFlag= DRIVER_CLR;
static uint32_t gLastCRCResult=0;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void cb_crc_irq_callback(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/**
 * @brief   CRC interrupt handler function.
 * @details This function is called when a CRC interrupt occurs. It clears and disables the CRC IRQ,
 *          retrieves the CRC result, and clears the in-progress flag.
 */
void cb_crc_irqhandler(void)
{
  /*Clear and Disable CRC IRQ*/
    pCRC->crc_irq = CRC_IRQ_DISABLE | CRC_irq_clear;
    NVIC_DisableIRQ(CRC_IRQn);

    if (pCRC ->crc_end == CRC_end_Msk)
    {
      gLastCRCResult = pCRC->crc_out; 

      /*Clear InProgessFlag*/
      gCRCInProgessFlag= DRIVER_CLR;
    }

    cb_crc_irq_callback();
}

void cb_crc_irq_callback(void)
{
    cb_crc_app_irq_callback();
}

__WEAK void cb_crc_app_irq_callback(void)
{
}


/**
 * @brief   Initializes the CRC module.
 * @details This function powers on the CRC module, enables it, and initializes internal flags.
 */
void cb_crc_init(void)
{
    /*CRC Module Power On and Reset Release*/
    cb_scr_crc_module_on();

    /*Enable CRC Module*/
    pCRC->crc_en |= CRC_MODULE_ENABLE;

    gCRCInProgessFlag= DRIVER_CLR;
}

/**
 * @brief   Deinitializes the CRC module.
 * @details This function clears and disables the CRC IRQ, disables the CRC module, and powers off the CRC module.
 */
void cb_crc_deinit(void)
{
    /*Clear and Disable CRC IRQ*/
    pCRC->crc_irq = CRC_IRQ_DISABLE|CRC_irq_clear;
    pCRC ->crc_end = CRC_CompleteEventClr;
    NVIC_DisableIRQ(CRC_IRQn);

    /*Disable CRC Module*/
    pCRC->crc_en = CRC_MODULE_DISABLE;

    /*Disable CRC Module*/
    cb_scr_crc_module_off();

    gCRCInProgessFlag = DRIVER_CLR;
}

/**
 * @brief   Configures the CRC algorithm with specified parameters.
 *
 * This function configures the CRC (Cyclic Redundancy Check) calculation engine based on the input parameters, including
 * the CRC type (CRC8, CRC16, or CRC32), initial value, input/output reflection, and the polynomial to be used for the
 * CRC computation. The XOR value applied at the end of the calculation can also be specified.
 *
 * @param   CRCType   The CRC type (EN_CRC8, EN_CRC16, or EN_CRC32) which determines the polynomial size.
 * @param   InitVal   The initial value used in the CRC calculation.
 * @param   RefOut    The output reflection mode (enabled or disabled).
 * @param   RefIn     The input reflection mode (enabled or disabled).
 * @param   Poly      The polynomial to be used for the CRC calculation.
 * @param   Xor       The XOR value applied at the end of the CRC calculation.
 */
void cb_crc_algo_config(enCRCType CRCType, enCRCInitVal InitVal, enCRCRefOut RefOut, enCRCRefIn RefIn, uint32_t Poly, uint32_t Xor)
{
    uint32_t temp;

    /*Config ByteOrder, InitVal, CrcType,Refout,RefIn*/
    temp  = (CRC_ByteOrder_LittleEndian  |
            ((uint32_t)(InitVal<< CRC_init_value_Pos)&CRC_init_value_Msk) |
            ((uint32_t)(CRCType<< CRC_order_sel_Pos)&CRC_order_sel_Msk) |
            ((uint32_t)(RefOut<< CRC_refout_Pos)&CRC_refout_Msk)  |
            ((uint32_t)(RefIn<< CRC_refin_Pos)&CRC_refin_Msk)
            );
    pCRC->crc_cfg  = temp;

    if (CRCType == EN_CRC8)
    {
      pCRC->crc_poly = (Poly & 0x000000FF);
      pCRC->crc_xor = (Xor & 0x000000FF);
    }
    else if(CRCType == EN_CRC16)
    {
      pCRC->crc_poly = (Poly & 0x0000FFFF);
      pCRC->crc_xor = (Xor & 0x0000FFFF);
    }
    else
    {
      pCRC->crc_poly = Poly;
      pCRC->crc_xor = Xor;
    }
}

/**
 * @brief   Starts CRC calculation using provided input data.
 *
 * This function processes the input data provided in the form of a byte array, performs CRC (Cyclic Redundancy Check)
 * calculation using the APB (Advanced Peripheral Bus) interface, and returns the result. The function supports the 
 * reinitialization of the CRC engine if requested, and it uses a polling mechanism to check for completion of the 
 * calculation, with a built-in timeout.
 *
 * @param   Data        Pointer to the input data array for CRC calculation.
 * @param   DataLen     Length of the input data array in bytes.
 * @param   ReInit_Sel  Specifies whether to reinitialize the CRC engine (EN_CRC_ReInit_Enable) or continue with the 
 *                      current CRC configuration.
 * @return  CB_PASS if the CRC calculation is successful, or CB_FAIL if an error occurs (such as a timeout or if the 
 *          CRC engine is already busy).
 */
CB_STATUS cb_crc_process_from_input_data(uint8_t *Data, uint16_t DataLen ,enCRCReInit ReInit_Sel)
{
    uint16_t Length, i, cnt = 0;
    uint32_t tempbuf = 0x00000000UL;

    // timeout mechanism for polling operation
    uint32_t timeout;
    extern uint32_t SystemCoreClock;


    if (gCRCInProgessFlag == DRIVER_SET)
    {
      return CB_FAIL; //CRC not Idle.
    }

    pCRC->crc_end = CRC_CompleteEventClr;

    // set busy flag
    gCRCInProgessFlag = DRIVER_SET;
    Length = DataLen;

    pCRC->crc_data_sel = CRC_datasel_APB;
    pCRC->crc_ahb_addr = (uint32_t) 0;
    pCRC->crc_data_len = (uint32_t) DataLen;
    if (ReInit_Sel == EN_CRC_ReInit_Enable)
    {
      pCRC->crc_start = CRC_init_en | CRC_start;
    }
    else
    {
      pCRC->crc_start = CRC_start;  
    }

    /*Clear all the data*/
    while (1)
    {
      if (Length <= 4) /*Maximum 4 Bytes for each entry*/
      {
        for (i = 0; i < Length; i++)
        {
          tempbuf |= (uint32_t) (Data[cnt]<<(i*BYTE_SIZE));
          cnt++;
        }
        pCRC->crc_in = tempbuf;
        break; /*End of last Input byte*/      
      }
      else
      {
        for (i = 0; i < 4; i++) 
        {
          tempbuf |= (uint32_t) (Data[cnt]<<(i*BYTE_SIZE));
          cnt++;
        }        
        Length -= 4;
        pCRC->crc_in = tempbuf;
        tempbuf = 0x00000000UL;
      }
    }

    // poll for data without interrupt
    timeout = ((SystemCoreClock / 1000) * 10);  // 10ms timeout
    do {  
      tempbuf = pCRC->crc_end;
      timeout--;

      // if takes to long to complete -> return error
      if (timeout == 0)
      {
        return CB_FAIL;
      }
    } while (tempbuf != CRC_end); //Wait For Data to be Ready 

    // save the result 
    gLastCRCResult = pCRC->crc_out; 

    /*Clear InProgessFlag*/
    gCRCInProgessFlag = DRIVER_CLR;

    return CB_PASS;  
}

/**
 * @brief   Starts CRC calculation on a block of memory data.
 *
 * This function initiates a CRC (Cyclic Redundancy Check) calculation on a specified block of memory. It can operate in
 * either interrupt mode (IRQ) or polling mode. In IRQ mode, the function will return immediately after starting the
 * calculation, while in polling mode it will wait for the CRC process to complete. Reinitialization of the CRC engine is
 * also supported, allowing for a fresh calculation or continuation of a previous CRC configuration.
 *
 * @param   StartAddr   The starting address of the memory block to be processed for CRC calculation.
 * @param   DataLen     The length of the memory block in bytes.
 * @param   ReInit_Sel  A flag to indicate whether to reinitialize the CRC engine (EN_CRC_ReInit_Enable) or not.
 * @param   IRQEnable   A flag to indicate whether to enable interrupt mode (CB_TRUE for IRQ mode, CB_FALSE for polling mode).
 *
 * @return  In IRQ mode, the function returns `CB_PASS` if the operation starts successfully. In polling mode, the function
 *          returns `CB_PASS` if the CRC calculation completes successfully, or `CB_FAIL` if the process times out or the
 *          CRC engine is busy.
 */
CB_STATUS cb_crc_process_from_memory(uint32_t StartAddr, uint16_t DataLen, enCRCReInit ReInit_Sel, enCRCIrq IRQEnable)
{
    uint32_t tempbuf = 0x00000000UL;

    // timeout mechanism for polling operation
    uint32_t timeout;
    extern uint32_t SystemCoreClock;

    if (gCRCInProgessFlag == DRIVER_SET)
    {
      return CB_FAIL; //CRC not Idle.
    }

    pCRC->crc_end = CRC_CompleteEventClr;

    /*Set InProgessFlag*/
    gCRCInProgessFlag= DRIVER_SET;

    // enable interrupt
    if (IRQEnable == CB_TRUE)
    {
      pCRC->crc_irq = CRC_IRQ_ENABLE|CRC_irq_clear;
      NVIC_EnableIRQ(CRC_IRQn); 
    }

    pCRC->crc_data_sel = CRC_datasel_AHB;
    pCRC->crc_ahb_addr = (uint32_t) StartAddr;
    pCRC->crc_data_len = (uint32_t) DataLen;
    if (ReInit_Sel == EN_CRC_ReInit_Enable)
    {
      pCRC->crc_start = CRC_init_en | CRC_start;
    }
    else
    {
      pCRC->crc_start = CRC_start;  
    }

    // we can return from here if interrupt enabled
    if (IRQEnable == EN_CRC_IRQ_Enable)
    {
      return CB_PASS;
    }

    // poll for data without interrupt
    timeout = ((SystemCoreClock / 1000) * 10);  // 10ms timeout
    do {  
      tempbuf = pCRC->crc_end;
      timeout--;

      // if takes to long to complete -> return error
      if (timeout == 0)
      {
        return CB_FAIL;
      }
    } while (tempbuf != CRC_end); //Wait For Data to be Ready 

    // save the result
    gLastCRCResult = pCRC->crc_out; 

    /*Clear InProgessFlag*/
    gCRCInProgessFlag = DRIVER_CLR;

    return CB_PASS;  
}

/**
 * @brief   Checks if the CRC module is idle.
 * @return  Status of the CRC module (CB_PASS if idle, CB_FAIL otherwise).
 * @details This function checks if the CRC module is idle and returns the status.
 */
CB_STATUS cb_crc_check_idle(void)
{
    if (gCRCInProgessFlag == DRIVER_SET)
    {
      return CB_FAIL;
    }
    else
    {
      return CB_PASS;
    }
}

/**
 * @brief   Gets the last CRC result.
 * @return  The last CRC result.
 * @details This function retrieves the last CRC result calculated by the CRC module.
 */
uint32_t cb_crc_get_crc_result(void)
{
    pCRC->crc_end = CRC_CompleteEventClr;     // clear CRC event before returning result
    return gLastCRCResult;
}
