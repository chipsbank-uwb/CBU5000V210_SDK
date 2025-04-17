/**
 * @file    CB_qspi.c
 * @brief   Flash memory operations for QSPI interface.
 * @details This file contains functions for reading from and writing to flash memory using the QSPI interface.
 *          It includes functions for configuring QSPI commands, checking and setting status bits.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_qspi.h"
#include "CB_Common.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include "system_ARMCM33.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_NON_REQUIRED 0
#define DEF_MAX_DATA_BYTE 0x1FF /*MAX number of byte can use in DataLen*/
#define DEF_BYTESIZE 8
#define DEF_WORDS_INBYTE 4 
#define DEF_QSPI_BASE_ADDR   0x40030000UL
#define pQSPI ((stQSPI_HandleTypeDef *) DEF_QSPI_BASE_ADDR)   /*APB Base Address of QSPI Controller*/

/* Defines used in timeout mechanism */
extern uint32_t SystemCoreClock;
#define DEF_QSPI_TIMEOUT_MS            500
#define DEF_QSPI_TIMEOUT_CPU_CYCLES    (((SystemCoreClock) / 1000U) * DEF_QSPI_TIMEOUT_MS)  // number of CPU cycles 

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum 
{
  EN_QSPI_PARAMETER_COMMAND = 0,
  EN_QSPI_PARAMETER_ADDRESS,
  EN_QSPI_PARAMETER_DATA,
  EN_QSPI_PARAMETER_FLASH_ACCESS,
} enQSPIParameterType;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/* timeout function / poll flag to use in QSPI only */
static CB_STATUS cb_qspi_wait_for_status(stQSPI_HandleTypeDef *hqspi, uint32_t flag)
{
    uint32_t startCPUCycle = DWT->CYCCNT;

    while ((hqspi->QSPI_STATUS & flag) != flag)
    {
      /* Capture the current CPU cycle counter and check for timeout */
      uint32_t ellapsedCPUCycles = (DWT->CYCCNT < startCPUCycle) ? 
            (0xFFFFFFFF - startCPUCycle + DWT->CYCCNT + 1) : DWT->CYCCNT - startCPUCycle;
      
      int64_t cycleDiff = (int64_t)((int64_t)(ellapsedCPUCycles) - (int64_t)(DEF_QSPI_TIMEOUT_CPU_CYCLES));
      
      if (cycleDiff > 0)
      {
        return CB_FAIL;
      }
    }
    return CB_PASS;
}

/* set qspi parameters helper function */
static uint32_t cb_qspi_param_mapping(enQSPIParameterType parameterType, uint8_t value)
{
    uint32_t retValue = 0U;
    switch (parameterType)
    {
      case EN_QSPI_PARAMETER_COMMAND:
        switch (value)
        {
          case EN_QSPI_NormalSPI_Command:
            retValue = QSPI_Cmd_1_LINE;
          break;

          case EN_QSPI_DualSPI_Command:
            retValue = QSPI_Cmd_2_LINES;
          break;

          case EN_QSPI_QuadSPI_Command:
            retValue = QSPI_Cmd_4_LINES;
          break;
        }
      break;

      case EN_QSPI_PARAMETER_ADDRESS:
        switch (value)
        {
          case EN_QSPI_NormalSPI_Addr:
            retValue = QSPI_Addr_1_LINE;
          break;

          case EN_QSPI_DualSPI_Addr:
            retValue = QSPI_Addr_2_LINES;
          break;

          case EN_QSPI_QuadSPI_Addr:
            retValue = QSPI_Addr_4_LINES;
          break;
        }
      break;

      case EN_QSPI_PARAMETER_DATA:
        switch (value)
        {
          case EN_QSPI_NormalSPI_Data:
            retValue = QSPI_Data_1_LINE;
          break;

          case EN_QSPI_DualSPI_Data:
            retValue = QSPI_Data_2_LINES;
          break;

          case EN_QSPI_QuadSPI_Data:
            retValue = QSPI_Data_4_LINES;
          break;
        }
      break;

      case EN_QSPI_PARAMETER_FLASH_ACCESS:
        switch (value)
        {
          case EN_VendorConfigArea:
            retValue = QSPI_Access_VendorConfigArea;
          break;
      
          case EN_SystemConfigArea:
            retValue = QSPI_Access_SystemConfigArea;
          break;

          case EN_FlashStorageSpace:
            retValue = QSPI_Access_FlashStorageSpace;
          break;
        }
      break;
    }

    return retValue;
}

/**
 * @brief   Read data from Quad SPI (QSPI) controller with address.
 * @param   hqspi   Pointer to QSPI handle
 * @param   cmd     Pointer to QSPI command structure
 * @param   pRbuf   Pointer to data buffer to store read data
 * @return  Return status (success/error)
 */
CB_STATUS cb_qspi_read_data_with_addr(stQSPI_HandleTypeDef *hqspi,stQSPI_CmdTypeDef *cmd,uint8_t *pRbuf)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    uint32_t QSPIBUSYFLAG;
    uint16_t DataLen;
    uint16_t cnt = 0;

    if ((hqspi == NULL)|| (cmd == NULL) ||(pRbuf == NULL)) 
    {
      return ret;
    }

    /* Check the input parameters from cmd struct */
    if ((!IS_QSPI_ADDRESS_LEN(cmd->AddrLen)) && (!IS_QSPI_ADDRESS_MODE(cmd->enAddrModeUse)) && (!IS_QSPI_COMMAND_MODE(cmd->enCommandModeuse)) \
        && (!IS_QSPI_COMMAND(cmd->Command)) && (!IS_QSPI_DATA_MODE(cmd->enDataModeUse)) && (!IS_QSPI_ACCESS_AREA(cmd->enFlashAcessArea)))
    {
      return ret;
    }

    /* Return error if the QSPI module is busy */
    QSPIBUSYFLAG = hqspi->QSPI_STATUS;
    QSPIBUSYFLAG &= QSPI_apb_access_done_Msk;
    if (QSPIBUSYFLAG != QSPI_apb_access_done)
    {
      return ret; //Error: QSPI is BUSY
    }

    DataLen = cmd->DataLen;
    if ((DataLen == 0) || (DataLen > DEF_MAX_DATA_BYTE))
    {
      return ret; //Error: No Data to read or data exceed the valid range
    }

    tempbuf = hqspi->QSPI_APB_RDATA;  //<--------Special Read for the Purpose to Clr the DATARDYFLAG first before Start Read Command

    /**********SET QSPI_APB_CMD_CFG Register Start**********/
    tempbuf = (((uint32_t)(cmd->Command) << QSPI_cmd_byte_Pos) & QSPI_cmd_byte_Msk); /*Put the Command byte into the buffer*/

    /* Command mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_COMMAND, cmd->enCommandModeuse);

    /* Data mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_DATA, cmd->enDataModeUse);

    /* Address mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_ADDRESS, cmd->enAddrModeUse);

    tempbuf |= (((uint32_t)(cmd->AddrLen) << QSPI_nbytes_addr_Pos)&QSPI_nbytes_addr_Msk); /*Config AddressLen*/
    tempbuf |= (((uint32_t)(cmd->nDummyCycles) << QSPI_ncycles_dummy_Pos)&QSPI_ncycles_dummy_Msk);/*Config DummyCycles*/  
    tempbuf |= (((uint32_t)(cmd->DataLen) << QSPI_nbytes_data_Pos)&QSPI_nbytes_data_Msk);/*Config DataLen*/
    hqspi->QSPI_APB_CMD_CFG = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_CFG Register End**********/

    /**********SET QSPI_APB_CMD_1 Register Start**********/
    tempbuf |= (cmd->Addr << 8);
    if(cmd->AddrLen >= 4) /*Send Special Command after Addr**********/
    {
      tempbuf |= (((uint32_t)(cmd->SpecialCommandByte1) << QSPI_byte_4_Pos) & QSPI_byte_4_Msk); /*Config Byte 4*/   
    }

    hqspi->QSPI_APB_CMD_1 = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_1 Register End**********/

    /**********SET QSPI_APB_CMD_2 Register Start**********/
    if(cmd->AddrLen == 5) /*Send Special after Addr**********/
    {
      tempbuf |= (((uint32_t)(cmd->SpecialCommandByte2) << QSPI_byte_5_Pos) & QSPI_byte_5_Msk); /*Config Byte 5*/   
      hqspi->QSPI_APB_CMD_2 = tempbuf;
      tempbuf = 0x00000000UL;      
    }
    else
    {
      hqspi->QSPI_APB_CMD_2 = 0x00000000UL;
    }
    /**********SET QSPI_APB_CMD_2 Register End**********/

    /**********SET QSPI_ACCESS_TYPE Register Start**********/
    hqspi->QSPI_ACCESS_TYPE = cb_qspi_param_mapping(EN_QSPI_PARAMETER_FLASH_ACCESS, cmd->enFlashAcessArea);
    /**********SET QSPI_ACCESS_TYPE Register End**********/

    /**********SET QSPI_APB_START Register Start**********/
    tempbuf |= QSPI_FlashDataReadCommand;
    tempbuf |= QSPI_start_cmd;
    hqspi->QSPI_APB_START = tempbuf;
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_START Register End**********/

    /**********Read Data After Receive Data Rdy Start**********/
    /* Implement timeout to prevent deadlock while waiting for ready flag */
    while (1)
    {
      // Set timeout until ready flag SET
      CB_STATUS dataReadyFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_data_ready_done);
      if (dataReadyFlagSet == CB_FAIL)
      {
        return CB_FAIL;
      }
      
      tempbuf = hqspi->QSPI_APB_RDATA;
      if (DataLen <=DEF_WORDS_INBYTE)
      {
        for (uint16_t i=0;i<DataLen;i++)
        {
          pRbuf[cnt] = ((uint8_t) (tempbuf >> (i*DEF_BYTESIZE) & 0xFF));          
          cnt++;
        }        
        break; /*End of DataRead*/
      }
      pRbuf[cnt]=((uint8_t) (tempbuf       & 0xFF));
      cnt++;
      pRbuf[cnt]=((uint8_t)((tempbuf>>8)  & 0xFF));
      cnt++;
      pRbuf[cnt]=((uint8_t)((tempbuf>>16) & 0xFF));
      cnt++;
      pRbuf[cnt]=((uint8_t)((tempbuf>>24) & 0xFF));
      cnt++;
      DataLen -=DEF_WORDS_INBYTE;
    }
    /**********Read Data After Receive Data Rdy End**********/

    /**********Exit After QSPI is Free Start**********/
    // set timeout for polling for access done flag
    CB_STATUS accessDoneFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_access_done);
    if (accessDoneFlagSet == CB_PASS)
    {
      ret = CB_PASS;
    }
    /**********Exit After QSPI is Free End**********/

    return ret;
}

/**
 * @brief   Write data to Quad SPI (QSPI) controller with address.
 * @param   hqspi   Pointer to QSPI handle
 * @param   cmd     Pointer to QSPI command structure
 * @param   pWbuf   Pointer to data buffer to write
 * @return  Return status (success/error)
 */
CB_STATUS cb_qspi_write_data_with_addr(stQSPI_HandleTypeDef *hqspi, stQSPI_CmdTypeDef *cmd, uint8_t *pWbuf)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    uint32_t QSPIBUSYFLAG;
    uint16_t DataLen;
    uint16_t cnt = 0;

    if ((hqspi == NULL) || (cmd == NULL) || (pWbuf == NULL)) 
    {
    return ret;
    }

    /* Check the input parameters from cmd struct */
    if ((!IS_QSPI_ACCESS_AREA(cmd->enFlashAcessArea)) && (!IS_QSPI_COMMAND_MODE(cmd->enCommandModeuse)) && (!IS_QSPI_ADDRESS_LEN(cmd->AddrLen)) \
      && (!IS_QSPI_ADDRESS_MODE(cmd->enAddrModeUse)) && (!IS_QSPI_COMMAND(cmd->Command)) && (!IS_QSPI_DATA_MODE(cmd->enDataModeUse)) && (!!IS_QSPI_ADDRESS_LEN(cmd->AddrLen)))
    {
    return ret;
    }

    /* Return error if the QSPI module is busy */
    QSPIBUSYFLAG = hqspi->QSPI_STATUS;
    QSPIBUSYFLAG &= QSPI_apb_access_done_Msk;
    if (QSPIBUSYFLAG != QSPI_apb_access_done)
    {
    return ret; //Error: QSPI is BUSY
    }

    DataLen = cmd->DataLen;
    if ((DataLen == 0) || (DataLen > DEF_MAX_DATA_BYTE))
    {
    return ret; //Error: No Data to write or data exceed the valid range
    }

    /**********SET QSPI_APB_CMD_CFG Register Start**********/
    tempbuf = (((uint32_t)(cmd->Command) << QSPI_cmd_byte_Pos) & QSPI_cmd_byte_Msk) ; /*Put the Command byte into the buffer*/

    /* Command mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_COMMAND, cmd->enCommandModeuse);

    /* Data output configuration */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_DATA, cmd->enDataModeUse);

    /* Address output configuration */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_ADDRESS, cmd->enAddrModeUse);

    tempbuf |= (((uint32_t)(cmd->AddrLen) << QSPI_nbytes_addr_Pos) & QSPI_nbytes_addr_Msk); /*Config AddressLen*/
    tempbuf |= (((uint32_t)(cmd->nDummyCycles) << QSPI_ncycles_dummy_Pos) & QSPI_ncycles_dummy_Msk);/*Config DummyCycles*/  
    tempbuf |= (((uint32_t)(cmd->DataLen) << QSPI_nbytes_data_Pos) & QSPI_nbytes_data_Msk);/*Config DataLen*/
    hqspi->QSPI_APB_CMD_CFG = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_CFG Register End**********/

    /**********SET QSPI_APB_CMD_1 Register Start**********/
    tempbuf |= (cmd->Addr << 8);
    if(cmd->AddrLen >= 4) /*Send Special Command after Addr**********/
    {
    tempbuf |= (((uint32_t)(cmd->SpecialCommandByte1) << QSPI_byte_4_Pos) & QSPI_byte_4_Msk); /*Config Byte 4*/   
    }

    hqspi->QSPI_APB_CMD_1 = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_1 Register End**********/

    /**********SET QSPI_APB_CMD_2 Register Start**********/
    if(cmd->AddrLen == 5) /*Send Special after Addr**********/
    {
    tempbuf |= (((uint32_t)(cmd->SpecialCommandByte2) << QSPI_byte_5_Pos) & QSPI_byte_5_Msk); /*Config Byte 5*/   
    hqspi->QSPI_APB_CMD_2 = tempbuf;
    tempbuf = 0x00000000UL;      
    }
    else
    {
    hqspi->QSPI_APB_CMD_2 = 0x00000000UL;
    }
    /**********SET QSPI_APB_CMD_2 Register End**********/

    /**********SET QSPI_APB_WDATA  Register Start**********/
    if (DataLen <= DEF_WORDS_INBYTE)
    {
    for (uint16_t i=0;i<DataLen;i++)
    {
      tempbuf |= (uint32_t) (pWbuf[cnt]<<(i*DEF_BYTESIZE));
      cnt++;
    }        
    }
    else
    {
    for (uint16_t i=0;i<DEF_WORDS_INBYTE;i++)
    {
      tempbuf |= (uint32_t) (pWbuf[cnt]<<(i*DEF_BYTESIZE));
      cnt++;
    }        
    }
    hqspi->QSPI_APB_WDATA = tempbuf;
    tempbuf = 0x00000000UL;
    /**********SET QSPI_APB_WDATA  Register End **********/

    /**********SET QSPI_ACCESS_TYPE Register Start**********/
    hqspi->QSPI_ACCESS_TYPE = cb_qspi_param_mapping(EN_QSPI_PARAMETER_FLASH_ACCESS, cmd->enFlashAcessArea);
    /**********SET QSPI_ACCESS_TYPE Register End**********/

    /**********SET QSPI_APB_START Register Start**********/
    tempbuf |= QSPI_FlashDataWriteCommand;
    tempbuf |= QSPI_start_cmd;
    hqspi->QSPI_APB_START = tempbuf;
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_START Register End**********/

    /**********Write Next Data After Receive Data Rdy Start**********/
    /* Implement simple timeout to prevent deadlock */
    while (1)
    {
    // Set timeout until ready flag SET
    CB_STATUS dataReadyFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_data_ready_done);
    if (dataReadyFlagSet != CB_PASS)
    {
      return CB_FAIL;
    }

    if (DataLen <=DEF_WORDS_INBYTE)
    {
      break;/*End of DataWrite*/
    }
      
    DataLen -=DEF_WORDS_INBYTE;
    if (DataLen <=DEF_WORDS_INBYTE)
    {
      for (uint16_t i=0;i<DataLen;i++)
      {
        tempbuf |= (uint32_t) (pWbuf[cnt]<<(i*DEF_BYTESIZE));
        cnt++;
      }        
    }
    else
    {
      for (uint8_t i=0;i<DEF_WORDS_INBYTE;i++)
      {
        tempbuf |= (uint32_t) (pWbuf[cnt]<<(i*DEF_BYTESIZE));
        cnt++;
      }        
    }

    hqspi->QSPI_APB_WDATA = tempbuf;
    tempbuf = 0x00000000UL;
    }

    /**********Write Next Data After Receive Data Rdy End**********/

    /**********Exit After QSPI is Free Start**********/
    // set timeout for QSPI access done flag
    CB_STATUS accessDoneFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_access_done);
    if (accessDoneFlagSet == CB_PASS)
    {
      ret = CB_PASS;
    }
    /**********Exit After QSPI is Free End**********/

    return ret;
}

/**
 * @brief   Write a single command to the Quad SPI (QSPI) controller.
 * @param   hqspi   Pointer to QSPI handle
 * @param   cmd     Pointer to QSPI command structure
 * @return  Return status (success/error)
 */
CB_STATUS cb_qspi_write_single_command(stQSPI_HandleTypeDef *hqspi, stQSPI_CmdTypeDef *cmd)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    uint32_t QSPIBUSYFLAG;

    if ((hqspi == NULL)|| (cmd == NULL)) 
    {
      return ret;
    }

    /* Check cmd struct parameters */
    if (!IS_QSPI_COMMAND_MODE(cmd->enCommandModeuse))
    {
      return ret;
    }

    QSPIBUSYFLAG = hqspi->QSPI_STATUS;
    QSPIBUSYFLAG &= QSPI_apb_access_done_Msk;
    if (QSPIBUSYFLAG != QSPI_apb_access_done)
    {
      return ret; //Error: QSPI is BUSY
    }

    /**********SET QSPI_APB_CMD_CFG Register Start**********/
    tempbuf = (((uint32_t)(cmd->Command) << QSPI_cmd_byte_Pos) & QSPI_cmd_byte_Msk) ; /*Put the Command byte into the buffer*/

    /* Command mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_COMMAND, cmd->enCommandModeuse);

    hqspi->QSPI_APB_CMD_CFG = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_CFG Register End**********/

    /**********SET QSPI_APB_CMD_1 Register Start*/
    hqspi->QSPI_APB_CMD_1 = 0x00000000UL; 
    /**********SET QSPI_APB_CMD_1 Register End**********/

    /**********SET QSPI_APB_CMD_2 Register Start**********/
    hqspi->QSPI_APB_CMD_2 = 0x00000000UL;
    /**********SET QSPI_APB_CMD_2 Register End**********/
      
    /**********SET QSPI_ACCESS_TYPE Register Start**********/
    hqspi->QSPI_ACCESS_TYPE = cb_qspi_param_mapping(EN_QSPI_PARAMETER_FLASH_ACCESS, cmd->enFlashAcessArea);
    /**********SET QSPI_ACCESS_TYPE Register End**********/

    /**********SET QSPI_APB_START Register Start**********/
    tempbuf |= QSPI_FlashDataWriteCommand;
    tempbuf |= QSPI_start_cmd;
    hqspi->QSPI_APB_START = tempbuf;
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_START Register End**********/

    /**********Exit After QSPI is Free Start**********/
    /* Implement timeout to poll for done flag */
    // set timeout for QSPI access done flag
    CB_STATUS accessDoneFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_access_done);
    if (accessDoneFlagSet == CB_PASS)
    {
      ret = CB_PASS;
    }
    /**********Exit After QSPI is Free End**********/
    return ret;
}

/**
 * @brief   Write a command to the Quad SPI (QSPI) controller with param.
 * @param   hqspi   Pointer to QSPI handle
 * @param   cmd     Pointer to QSPI command structure
 * @return  Return status (success/error)
 */
CB_STATUS cb_qspi_write_command_with_param(stQSPI_HandleTypeDef *hqspi, stQSPI_CmdTypeDef *cmd)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    uint32_t QSPIBUSYFLAG;

    if ((hqspi == NULL)|| (cmd == NULL)) 
    {
      return ret;
    }

    /* Check cmd struct parameters */
    if (!IS_QSPI_COMMAND_MODE(cmd->enCommandModeuse))
    {
      return ret;
    }

    QSPIBUSYFLAG = hqspi->QSPI_STATUS;
    QSPIBUSYFLAG &= QSPI_apb_access_done_Msk;
    if (QSPIBUSYFLAG != QSPI_apb_access_done)
    {
      return ret; //Error: QSPI is BUSY
    }

    /**********SET QSPI_APB_CMD_CFG Register Start**********/
    tempbuf = (((uint32_t)(cmd->Command) << QSPI_cmd_byte_Pos) & QSPI_cmd_byte_Msk) ; /*Put the Command byte into the buffer*/

    /* Command mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_COMMAND, cmd->enCommandModeuse);
    tempbuf |= (((uint32_t)(cmd->DataLen) << QSPI_nbytes_data_Pos) & QSPI_nbytes_data_Msk);

    hqspi->QSPI_APB_CMD_CFG = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_CFG Register End**********/

    /**********SET QSPI_APB_CMD_1 Register Start*/
    hqspi->QSPI_APB_CMD_1 = 0x00000000UL; 
    /**********SET QSPI_APB_CMD_1 Register End**********/

    /**********SET QSPI_APB_CMD_2 Register Start**********/
    hqspi->QSPI_APB_CMD_2 = 0x00000000UL;
    /**********SET QSPI_APB_CMD_2 Register End**********/
      
    /**********SET QSPI_ACCESS_TYPE Register Start**********/

    /**********SET QSPI_ACCESS_TYPE Register End**********/

    /**********SET QSPI_APB_WDATA Register Start**********/
    tempbuf = cmd->SpecialCommandByte1;
    hqspi->QSPI_APB_WDATA = tempbuf;
    tempbuf = 0x00000000UL;
    /**********SET QSPI_APB_WDATA Register End**********/

    /**********SET QSPI_APB_START Register Start**********/
    tempbuf |= QSPI_FlashRegWriteCommand;
    tempbuf |= QSPI_start_cmd;
    hqspi->QSPI_APB_START = tempbuf;
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_START Register End**********/

    /**********Exit After QSPI is Free Start**********/
    /* Implement timeout to poll for done flag */
    // set timeout for QSPI access done flag
    CB_STATUS accessDoneFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_access_done);
    if (accessDoneFlagSet == CB_PASS)
    {
      ret = CB_PASS;
    }
    /**********Exit After QSPI is Free End**********/
    return ret;
}

/**
 * @brief Sends a QSPI command with the specified address and additional parameters.
 *
 * This function configures and sends a QSPI command to the memory. It sets up the
 * command, address, and other parameters, then issues the command to the QSPI peripheral.
 * The function waits for the command to complete and checks if the QSPI is busy before
 * issuing the command. It handles special command bytes and ensures that the QSPI is
 * free before returning.
 *
 * @param[in] hqspi Pointer to a `stQSPI_HandleTypeDef` structure that contains
 *                  the configuration information for the QSPI module.
 * @param[in] cmd   Pointer to a `stQSPI_CmdTypeDef` structure that specifies
 *                  the command, address, and additional parameters to be used.
 *
 * @return ret_status Returns `success` if the command is sent and completed successfully,
 *         or `error` if there was an issue with sending the command or if the QSPI is busy.
 */
CB_STATUS cb_qspi_write_command_and_addr(stQSPI_HandleTypeDef *hqspi,stQSPI_CmdTypeDef *cmd)
{
    /* Note: The QSPI module doesn't support writing with command and address only
        (for example: Page erase for QSPI flash). Even if we set nbytes_data = 0, then the 
        QSPI module will not transmit anything at all
        Therefore a method here is to direct the address output to the data output register,
        and utilise flash reg write command mode (111). That way, command is being sent as 
        usual, whereas address bits being sent out as a data output */
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    uint32_t QSPIBUSYFLAG;
    uint8_t AddrLen;

    if ((hqspi == NULL)|| (cmd == NULL)) 
    {
      return ret;
    }

    /* Check input parameters from cmd struct*/
    if ((!IS_QSPI_COMMAND_MODE(cmd->enCommandModeuse)) && (!IS_QSPI_ADDRESS_LEN(cmd->AddrLen)) \
        && (!IS_QSPI_ADDRESS_MODE(cmd->enAddrModeUse)) && (!IS_QSPI_ACCESS_AREA(cmd->enFlashAcessArea)))
    {
      return ret;
    }

    QSPIBUSYFLAG = hqspi->QSPI_STATUS;
    QSPIBUSYFLAG &= QSPI_apb_access_done_Msk;
    if (QSPIBUSYFLAG != QSPI_apb_access_done)
    {
      return ret; //Error: QSPI is BUSY
    }

    if ((cmd->AddrLen == 0) || (cmd->AddrLen > 5))
    {
      return ret; //Error: No Addr to read. or Invalid Addr range
    }
    AddrLen = cmd->AddrLen;

    /**********SET QSPI_APB_CMD_CFG Register Start**********/
    tempbuf = (((uint32_t)(cmd->Command) << QSPI_cmd_byte_Pos) & QSPI_cmd_byte_Msk) ; /*Put the Command byte into the buffer*/

    /* Command mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_COMMAND, cmd->enCommandModeuse);

    /* Address (directed to data output) mode config*/
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_DATA, cmd->enAddrModeUse);

    tempbuf |= (((uint32_t)(cmd->AddrLen) << QSPI_nbytes_data_Pos) & QSPI_nbytes_data_Msk);           /*Specially Allocate "Addr" to be in Data Field*/
    tempbuf |= (((uint32_t)(cmd->nDummyCycles) << QSPI_ncycles_dummy_Pos) & QSPI_ncycles_dummy_Msk);  /*Config DummyCycles*/  
    hqspi->QSPI_APB_CMD_CFG = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_CFG Register End**********/

    /**********SET QSPI_APB_CMD_1 Register Start**********/
    hqspi->QSPI_APB_CMD_1 = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_1 Register End**********/

    /**********SET QSPI_APB_CMD_2 Register Start**********/
    hqspi->QSPI_APB_CMD_2 = tempbuf;
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_2 Register End**********/


    /**********SET QSPI_APB_WDATA  Register Start**********/

    tempbuf |= ((0xffUL & cmd->Addr) << 16);
    tempbuf |= ((0xffUL << 8) & cmd->Addr);
    tempbuf |= (((0xffUL << 16) & cmd->Addr) >> 16);
    if (AddrLen>=4) //Special Character included.
    {
      tempbuf |= ((uint32_t)(cmd->SpecialCommandByte1)<< 24);    
    }
    hqspi->QSPI_APB_WDATA = tempbuf;
    tempbuf = 0x00000000UL;
    /**********SET QSPI_APB_WDATA  Register End **********/

    /**********SET QSPI_ACCESS_TYPE Register Start**********/
    hqspi->QSPI_ACCESS_TYPE = cb_qspi_param_mapping(EN_QSPI_PARAMETER_FLASH_ACCESS, cmd->enFlashAcessArea);
    /**********SET QSPI_ACCESS_TYPE Register End**********/


    /**********SET QSPI_APB_START Register Start**********/
    tempbuf |= QSPI_FlashRegWriteCommand;
    tempbuf |= QSPI_start_cmd;
    hqspi->QSPI_APB_START = tempbuf;
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_START Register End**********/

    /**********Write Special Command 2 Start**********/
    /* Implement timeout while polling for data ready flag */
    while (1)
    {
      CB_STATUS dataReadyFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_data_ready_done);
      if (dataReadyFlagSet != CB_PASS)
      {
        return CB_FAIL;
      }

      if (AddrLen <=DEF_WORDS_INBYTE)
      {
        break;/*End of Address & Special character sent*/
      }
      tempbuf |= ((uint32_t)cmd->SpecialCommandByte2);    
      AddrLen -=DEF_WORDS_INBYTE;
      hqspi->QSPI_APB_WDATA = tempbuf;
      tempbuf = 0x00000000UL;
    }
    /**********Write Special Command 2 End**********/
    /**********Exit After QSPI is Free Start**********/
    /* Implement timeout to poll for access done flag */
    CB_STATUS accessDoneFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_access_done);
    if (accessDoneFlagSet == CB_PASS)
    {
      ret = CB_PASS;
    }
    /*******Exit After QSPI is Free End**********/

    return ret;
}

/**
 * @brief Reads data from the QSPI flash memory without address.
 *
 * This function performs a read operation from the QSPI flash memory. It sets up
 * the necessary QSPI registers and configurations, initiates the read command, and
 * retrieves the data into a buffer. The function continuously checks for the data
 * ready flag and ensures the QSPI is not busy before proceeding with the read operation.
 *
 * @param[in]  hqspi Pointer to the QSPI handle structure.
 * @param[in]  cmd   Pointer to the QSPI command configuration structure.
 * @param[out] pRbuf Pointer to the buffer where the read data will be stored.
 *
 * @return ret_status Returns `success` if the read operation is completed successfully,
 *         otherwise returns `error` if the operation fails.
 */
CB_STATUS cb_qspi_read_data_without_addr(stQSPI_HandleTypeDef *hqspi,stQSPI_CmdTypeDef *cmd,uint8_t *pRbuf)
{
    CB_STATUS ret = CB_FAIL;
    uint32_t tempbuf;
    uint32_t QSPIBUSYFLAG;
    uint16_t DataLen;
    uint16_t cnt = 0;

    if ((hqspi == NULL)|| (cmd == NULL) ||(pRbuf == NULL)) 
    {
      return ret;
    }

    /* cmd struct parameters check */
    if ((!IS_QSPI_COMMAND(cmd->Command)) && (!IS_QSPI_COMMAND_MODE(cmd->enCommandModeuse))\
        && (!IS_QSPI_DATA_MODE(cmd->enDataModeUse)) && (!IS_QSPI_ACCESS_AREA(cmd->enFlashAcessArea)))
    {
      return ret;
    }

    QSPIBUSYFLAG = hqspi->QSPI_STATUS;
    QSPIBUSYFLAG &= QSPI_apb_access_done_Msk;
    if (QSPIBUSYFLAG != QSPI_apb_access_done)
    {
      return ret; //Error: QSPI is BUSY
    }

    DataLen = cmd->DataLen;
    if ((DataLen == 0) || (DataLen > DEF_MAX_DATA_BYTE))
    {
      return ret; //Error: No Data to read or data exceed the valid range
    }

    tempbuf = hqspi->QSPI_APB_RDATA;  //<--------Special Read for the Purpose to Clr the DATARDYFLAG first before Start Read Command

    /**********SET QSPI_APB_CMD_CFG Register Start**********/
    tempbuf = (((uint32_t)(cmd->Command) << QSPI_cmd_byte_Pos) & QSPI_cmd_byte_Msk) ; /*Put the Command byte into the buffer*/

    /* Command mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_COMMAND, cmd->enCommandModeuse);

    /* Data mode config */
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_DATA, cmd->enDataModeUse);

    tempbuf |= ((0x00000000UL << QSPI_nbytes_addr_Pos) & QSPI_nbytes_addr_Msk);             /*AddressLen = 0*/
    tempbuf |= ((0x00000000UL << QSPI_ncycles_dummy_Pos) & QSPI_ncycles_dummy_Msk);         /*DummyCycles = 0*/  
    tempbuf |= (((uint32_t)(cmd->DataLen)<< QSPI_nbytes_data_Pos) & QSPI_nbytes_data_Msk);  /*Config DataLen*/
    hqspi->QSPI_APB_CMD_CFG = tempbuf; 
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_CMD_CFG Register End**********/

    /**********SET QSPI_APB_CMD_1 Register Start*/
    hqspi->QSPI_APB_CMD_1 = 0x00000000UL; 
    /**********SET QSPI_APB_CMD_1 Register End**********/

    /**********SET QSPI_APB_CMD_2 Register Start**********/
    hqspi->QSPI_APB_CMD_2 = 0x00000000UL;
    /**********SET QSPI_APB_CMD_2 Register End**********/

    /**********SET QSPI_ACCESS_TYPE Register Start**********/
    hqspi->QSPI_ACCESS_TYPE = cb_qspi_param_mapping(EN_QSPI_PARAMETER_FLASH_ACCESS, cmd->enFlashAcessArea);
    /**********SET QSPI_ACCESS_TYPE Register End**********/

    /**********SET QSPI_APB_START Register Start**********/
    tempbuf |= QSPI_FlashRegReadCommand;
    tempbuf |= QSPI_start_cmd;
    hqspi->QSPI_APB_START = tempbuf;
    tempbuf = 0x00000000UL;      
    /**********SET QSPI_APB_START Register End**********/

    /**********Read Data After Receive Data Rdy Start**********/
    /* Implement timeout polling for the data ready flag */
    while (1)
    {
      CB_STATUS dataReadyFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_data_ready_done);
      if (dataReadyFlagSet != CB_PASS)
      {
        return CB_FAIL;
      }
      
      tempbuf = hqspi->QSPI_APB_RDATA;
        
      if (DataLen <=DEF_WORDS_INBYTE)
      {
        for (uint16_t i=0;i<DataLen;i++)
        {
          pRbuf[cnt]=((uint8_t) (tempbuf>>(i*DEF_BYTESIZE)       & 0xFF));          
          cnt++;
        }        
        break; /*End of DataRead*/
      }
      pRbuf[cnt]=((uint8_t) (tempbuf      & 0xFF));
      cnt++;
      pRbuf[cnt]=((uint8_t)((tempbuf>>8)  & 0xFF));
      cnt++;
      pRbuf[cnt]=((uint8_t)((tempbuf>>16) & 0xFF));
      cnt++;
      pRbuf[cnt]=((uint8_t)((tempbuf>>24) & 0xFF));
      cnt++;
      DataLen -=DEF_WORDS_INBYTE;
    }
    /**********Read Data After Receive Data Rdy End**********/

    /**********Exit After QSPI is Free Start**********/
    /* Implement timeout for polling for access done flag */
    CB_STATUS accessDoneFlagSet = cb_qspi_wait_for_status(hqspi, QSPI_apb_access_done);
    if (accessDoneFlagSet == CB_PASS)
    {
      ret = CB_PASS;
    }
    /**********Exit After QSPI is Free End**********/

    return ret;
}

/**
 * @brief Sets or resets the burst read action by APB.
 *
 * This function sets or resets the burst read action by APB.
 *
 * @param BurstReadAction The action to perform (enBurstReadAction).
 */
void cb_qspi_configure_read_mode(enBurstReadAction BurstReadAction)
{
    volatile uint32_t* p_QSPI_APB_CMD_CFG = ((volatile uint32_t *)(0x40030000 + 0x08));
    volatile uint32_t* p_QSPI_APB_WDATA   = ((volatile uint32_t *)(0x40030000 + 0x18));
    volatile uint32_t* p_QSPI_APB_START   = ((volatile uint32_t *)(0x40030000 + 0x04));
    *p_QSPI_APB_CMD_CFG = 0x02000877;   //cmd_byte=0x77,data_mode=0x10;nbytes_data=0x04;
    if (BurstReadAction == EN_BURSTREAD_SET)
    {
      *p_QSPI_APB_WDATA   = 0x40000000;   //Set burst wrap length as 32;
      *p_QSPI_APB_START   = 0x00000015;   //Start qspi,with burst read set; 
    }
    if (BurstReadAction == EN_BURSTREAD_RESET)
    {
      *p_QSPI_APB_WDATA   = 0x10000000;   //Set burst wrap as default;
      *p_QSPI_APB_START   = 0x00000011;   //Start qspi,with burst read reset;
    }  
}

/**
 * @brief Sets or resets the burst read action by APB.
 *
 * This function sets or resets the burst read action by APB.
 *
 * @param BurstReadAction The action to perform (enBurstReadAction).
 */
CB_STATUS cb_qspi_send_read_mode_command(enBurstReadAction BurstReadAction, stQSPI_CmdTypeDef *cmd, uint8_t burstReadData)
{
    /* Check the input parameters from cmd struct */
    if ((!IS_QSPI_COMMAND_MODE(cmd->enCommandModeuse)) \
        && (!IS_QSPI_COMMAND(cmd->Command)) && (!IS_QSPI_DATA_MODE(cmd->enDataModeUse)) && (!IS_QSPI_ACCESS_AREA(cmd->enFlashAcessArea)))
    {
      return CB_FAIL;
    }

    /* Length size limit for burst read data currently is 4 bytes */
    if ((cmd->DataLen > DEF_WORDS_INBYTE) || (cmd->DataLen == 0))
    {
      return CB_FAIL;
    }

    volatile uint32_t* p_QSPI_APB_CMD_CFG = ((volatile uint32_t *)(0x40030000 + 0x08));
    volatile uint32_t* p_QSPI_APB_WDATA   = ((volatile uint32_t *)(0x40030000 + 0x18));
    volatile uint32_t* p_QSPI_APB_START   = ((volatile uint32_t *)(0x40030000 + 0x04));
    uint32_t tempbuf = 0x00000000UL;

    /* Accomondate command config */
    tempbuf |= (((uint32_t) cmd->DataLen) << 23) | (cmd->Command);
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_COMMAND, cmd->enCommandModeuse);
    tempbuf |= cb_qspi_param_mapping(EN_QSPI_PARAMETER_DATA, cmd->enDataModeUse);

    /* Assign command bytes */
    *p_QSPI_APB_CMD_CFG = tempbuf;

    /* reset tempbuf for APB_WDATA */
    tempbuf = 0x00000000UL;
    tempbuf |= (uint32_t) (((uint32_t) burstReadData) << ((cmd->DataLen - 1) * DEF_BYTESIZE));

    *p_QSPI_APB_WDATA   = tempbuf;


    if (BurstReadAction == EN_BURSTREAD_SET)
    {
      *p_QSPI_APB_START   = 0x00000015;   //Start qspi,with burst read set; 
    }

    else 
    {
      *p_QSPI_APB_START   = 0x00000011;   //Start qspi,with burst read reset;
    }

    return CB_PASS;
}
