/**
 * @file    CB_qspi.h
 * @brief   Header file for Quad SPI (QSPI) Controller.
 * @details This file defines structures, constants, and function prototypes for qspi.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __CB_QSPI_H_
#define __CB_QSPI_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include "CB_Common.h"
#include "CB_PeripheralPhyAddrDataBase.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
/* QSPI command handle parameter check macros */
#define IS_QSPI_COMMAND(COMMAND)              (((COMMAND) >= 0x00U))

#define IS_QSPI_ADDRESS_LEN(ADDR_LEN)         (((ADDR_LEN) > 0x00U) && ((ADDR_LEN) <= 0x07U))

#define IS_QSPI_ACCESS_AREA(ACCESS_AREA)      (((ACCESS_AREA) == EN_VendorConfigArea) || \
                                              ((ACCESS_AREA) == EN_SystemConfigArea) || \
                                              ((ACCESS_AREA) == EN_FlashStorageSpace))

#define IS_QSPI_DATA_MODE(DATA_MODE)          (((DATA_MODE) == EN_QSPI_NormalSPI_Data) || \
                                              ((DATA_MODE) == EN_QSPI_DualSPI_Data) || \
                                              ((DATA_MODE) == EN_QSPI_QuadSPI_Data))

#define IS_QSPI_ADDRESS_MODE(ADDRESS_MODE)    (((ADDRESS_MODE) == EN_QSPI_NormalSPI_Addr) || \
                                              ((ADDRESS_MODE) == EN_QSPI_DualSPI_Addr) || \
                                              ((ADDRESS_MODE) == EN_QSPI_QuadSPI_Addr))

#define IS_QSPI_COMMAND_MODE(COMMAND_MODE)    (((COMMAND_MODE) == EN_QSPI_NormalSPI_Command) || \
                                              ((COMMAND_MODE) == EN_QSPI_DualSPI_Command) || \
                                              ((COMMAND_MODE) == EN_QSPI_QuadSPI_Command))

#define IS_QSPI_DATA_LEN(DATA_LEN)            (((DATA_LEN) > 0) && \
                                              ((DATA_LEN) <= 4096))

#define IS_QSPI_DUMMY_CYCLES(DUMMY_CYCLES)    (((DUMMY_CYCLES) > 0x00U) && ((DUMMY_CYCLES) <= 0x0FU))

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief   Return status values.
 */
typedef enum
{
  success = 0,
  error
} ret_status;

/**
 * @brief Enumeration to represent bit check results.
 *
 * This enum defines the possible results of a bit check operation.
 */
typedef enum
{
  BIT_CLR = 0,
  BIT_SET
} bitcheck_ret;
/**
 * @brief Enumeration defining actions for burst read operations.
 */
typedef enum
{
  EN_BURSTREAD_SET = 0,    /**< Set burst read mode */
  EN_BURSTREAD_RESET      /**< Reset burst read mode */
}enBurstReadAction;

typedef enum
{
  EN_FLASHTYPE_PUYA =0,
  EN_FLASHTYPE_MACRONIX 
}enFlashType;

/* Adapted from FLM project */
typedef enum
{
	EN_VendorConfigArea = 0,
	EN_SystemConfigArea,
	EN_FlashStorageSpace
} enCodeAccess;

typedef enum
{
  EN_QSPI_Unused_Addr = 0,
  EN_QSPI_NormalSPI_Addr,
  EN_QSPI_DualSPI_Addr,
  EN_QSPI_QuadSPI_Addr
} enAddrMode;

typedef enum
{
  EN_QSPI_Unused_Data = 0,
  EN_QSPI_NormalSPI_Data,
  EN_QSPI_DualSPI_Data,
  EN_QSPI_QuadSPI_Data
} enDataMode;

typedef enum
{
  EN_QSPI_Unused_Command = 0,
  EN_QSPI_NormalSPI_Command,
  EN_QSPI_DualSPI_Command,
  EN_QSPI_QuadSPI_Command
} enCommandMode;

//-------------------------------
// STRUCT/UNION SECTION
/**
 * @brief   Quad SPI (QSPI) command structure definition.
 */
typedef struct
{
  enCodeAccess enFlashAcessArea;
  enCommandMode enCommandModeuse;
  uint8_t Command;
  enAddrMode enAddrModeUse;
  uint32_t Addr;
  uint8_t AddrLen;
  uint8_t SpecialCommandByte1;
  uint8_t SpecialCommandByte2;
  uint8_t nDummyCycles;
  enDataMode enDataModeUse;
  uint16_t DataLen;
  //uint8_t Reserved[2];  /*No used,added to remove structing padding warning*/
} stQSPI_CmdTypeDef;


//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/* timeout function / poll flag to use in QSPI only */
void cb_qspi_configure_read_mode(enBurstReadAction BurstReadAction);

/**
 * @brief   Read data from Quad SPI (QSPI) controller with address.
 * @param   hqspi   Pointer to QSPI handle
 * @param   cmd     Pointer to QSPI command structure
 * @param   pRbuf   Pointer to data buffer to store read data
 * @return  Return status (success/error)
 */
CB_STATUS cb_qspi_read_data_with_addr(stQSPI_HandleTypeDef *hqspi,stQSPI_CmdTypeDef *cmd,uint8_t *pRbuf);

/**
 * @brief   Write data to Quad SPI (QSPI) controller with address.
 * @param   hqspi   Pointer to QSPI handle
 * @param   cmd     Pointer to QSPI command structure
 * @param   pWbuf   Pointer to data buffer to write
 * @return  Return status (success/error)
 */
CB_STATUS cb_qspi_write_data_with_addr(stQSPI_HandleTypeDef *hqspi,stQSPI_CmdTypeDef *cmd,uint8_t *pWbuf);

/**
 * @brief   Write a single command to the Quad SPI (QSPI) controller.
 * @param   hqspi   Pointer to QSPI handle
 * @param   cmd     Pointer to QSPI command structure
 * @return  Return status (success/error)
 */
CB_STATUS cb_qspi_write_single_command(stQSPI_HandleTypeDef *hqspi,stQSPI_CmdTypeDef *cmd);

/**
 * @brief   Write a command to the Quad SPI (QSPI) controller with param.
 * @param   hqspi   Pointer to QSPI handle
 * @param   cmd     Pointer to QSPI command structure
 * @return  Return status (success/error)
 */
CB_STATUS cb_qspi_write_command_with_param(stQSPI_HandleTypeDef *hqspi, stQSPI_CmdTypeDef *cmd);

/**
 * @brief Sends a QSPI command with the specified address and additional parameters.
 * @param[in] hqspi Pointer to a `stQSPI_HandleTypeDef` structure that contains
 *                  the configuration information for the QSPI module.
 * @param[in] cmd   Pointer to a `stQSPI_CmdTypeDef` structure that specifies
 *                  the command, address, and additional parameters to be used.
 * @return ret_status Returns `success` if the command is sent and completed successfully,
 *         or `error` if there was an issue with sending the command or if the QSPI is busy.
 */
CB_STATUS cb_qspi_write_command_and_addr(stQSPI_HandleTypeDef *hqspi,stQSPI_CmdTypeDef *cmd);

/**
 * @brief Sends a QSPI command with the specified address and additional parameters.
 * @param[in] hqspi Pointer to a `stQSPI_HandleTypeDef` structure that contains
 *                  the configuration information for the QSPI module.
 * @param[in] cmd   Pointer to a `stQSPI_CmdTypeDef` structure that specifies
 *                  the command, address, and additional parameters to be used.
 * @return ret_status Returns `success` if the command is sent and completed successfully,
 *         or `error` if there was an issue with sending the command or if the QSPI is busy.
 */
CB_STATUS cb_qspi_read_data_without_addr(stQSPI_HandleTypeDef *hqspi,stQSPI_CmdTypeDef *cmd,uint8_t *pRbuf);

/**
 * @brief Sets or resets the burst read action by APB.
 * @param BurstReadAction The action to perform (enBurstReadAction).
 */
CB_STATUS cb_qspi_send_read_mode_command(enBurstReadAction BurstReadAction, stQSPI_CmdTypeDef *cmd, uint8_t burstReadData);

#endif /* __CB_QSPI_H_ */





