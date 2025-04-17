/**
 * @file    CB_crc.h
 * @brief   CRC Header File
 * @details This file contains definitions and prototypes for CRC module configuration and operations.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef INC_CRC_H
#define INC_CRC_H

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
  EN_InitValZero = 0,
  EN_InitValOne = 1
}enCRCInitVal;

typedef enum
{
  EN_CRC8  = 0,
  EN_CRC16 = 1,
  EN_CRC32 = 2
}enCRCType;

typedef enum
{
  EN_CRCRefOut_Enable  = 0,
  EN_CRCRefOut_Disable = 1,
}enCRCRefOut;

typedef enum
{
  EN_CRCRefIn_Enable  = 0,
  EN_CRCRefIn_Disable = 1,
}enCRCRefIn;

typedef enum
{
  EN_CRC_ReInit_Disable = 0,
  EN_CRC_ReInit_Enable  = 1
} enCRCReInit;

typedef enum
{
  EN_CRC_IRQ_Disable = 0,
  EN_CRC_IRQ_Enable  = 1
} enCRCIrq;

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
 * @brief   CRC interrupt handler function.
 * @details This function is called when a CRC interrupt occurs. It clears and disables the CRC IRQ,
 *          retrieves the CRC result, and clears the in-progress flag.
 */
void cb_crc_irqhandler(void);

/**
 * @brief   Weakly defined CRC interrupt APP layer callback.
 * @details This function is called when a CRC interrupt occurs. User can implement
 *          custom routines for CRC interrupt inside this function at APP layer.
 */
void cb_crc_app_irq_callback(void);

/**
 * @brief   Initializes the CRC module.
 * @details This function powers on the CRC module, enables it, and initializes internal flags.
 */
void cb_crc_init(void);

/**
 * @brief   Deinitializes the CRC module.
 * @details This function clears and disables the CRC IRQ, disables the CRC module, and powers off the CRC module.
 */
void cb_crc_deinit(void);

/**
 * @brief   Configures the CRC algorithm with specified parameters.
 *
 * @details This function configures the CRC (Cyclic Redundancy Check) calculation engine based on the input parameters, including
 *          the CRC type (CRC8, CRC16, or CRC32), initial value, input/output reflection, and the polynomial to be used for the
 *          CRC computation. The XOR value applied at the end of the calculation can also be specified.
 *
 * @param[in]   CRCType   The CRC type (EN_CRC8, EN_CRC16, or EN_CRC32) which determines the polynomial size.
 * @param[in]   InitVal   The initial value used in the CRC calculation.
 * @param[in]   RefOut    The output reflection mode (enabled or disabled).
 * @param[in]   RefIn     The input reflection mode (enabled or disabled).
 * @param[in]   Poly      The polynomial to be used for the CRC calculation.
 * @param[in]   Xor       The XOR value applied at the end of the CRC calculation.
 */
void cb_crc_algo_config(enCRCType CRCType, enCRCInitVal InitVal, enCRCRefOut RefOut, enCRCRefIn RefIn, uint32_t Poly, uint32_t Xor);

/**
 * @brief   Starts CRC calculation using provided input data.
 *
 * @details This function processes the input data provided in the form of a byte array, performs CRC (Cyclic Redundancy Check)
 *          calculation using the APB (Advanced Peripheral Bus) interface, and returns the result. The function supports the 
 *          reinitialization of the CRC engine if requested, and it uses a polling mechanism to check for completion of the 
 *          calculation, with a built-in timeout.
 *
 * @param[in]   Data        Pointer to the input data array for CRC calculation.
 * @param[in]   DataLen     Length of the input data array in bytes.
 * @param[in]   ReInit_Sel  Specifies whether to reinitialize the CRC engine (EN_CRC_ReInit_Enable) or continue with the 
 *                          current CRC configuration.
 * @return      CB_PASS if the CRC calculation is successful, or CB_FAIL if an error occurs (such as a timeout or if the 
 *              CRC engine is already busy).
 */
CB_STATUS cb_crc_process_from_input_data(uint8_t *Data, uint16_t DataLen, enCRCReInit ReInit_Sel);

/**
 * @brief   Starts CRC calculation on a block of memory data.
 *
 * @details This function initiates a CRC (Cyclic Redundancy Check) calculation on a specified block of memory. It can operate in
 * either interrupt mode (IRQ) or polling mode. In IRQ mode, the function will return immediately after starting the
 * calculation, while in polling mode it will wait for the CRC process to complete. Reinitialization of the CRC engine is
 * also supported, allowing for a fresh calculation or continuation of a previous CRC configuration.
 *
 * @param[in]   StartAddr   The starting address of the memory block to be processed for CRC calculation.
 * @param[in]   DataLen     The length of the memory block in bytes.
 * @param[in]   ReInit_Sel  A flag to indicate whether to reinitialize the CRC engine (EN_CRC_ReInit_Enable) or not.
 * @param[in]   IRQEnable   A flag to indicate whether to enable interrupt mode (CB_TRUE for IRQ mode, CB_FALSE for polling mode).
 *
 * @return      In IRQ mode, the function returns `CB_PASS` if the operation starts successfully. In polling mode, the function
 *              returns `CB_PASS` if the CRC calculation completes successfully, or `CB_FAIL` if the process times out or the
 *              CRC engine is busy.
 */
CB_STATUS cb_crc_process_from_memory(uint32_t StartAddr, uint16_t DataLen, enCRCReInit ReInit_Sel, enCRCIrq IRQEnable);

/**
 * @brief   Checks if the CRC module is idle.
 * @details This function checks if the CRC module is idle and returns the status.
 * @return  Status of the CRC module (CB_PASS if idle, CB_FAIL otherwise).
 */
CB_STATUS  cb_crc_check_idle(void);

/**
 * @brief   Gets the last CRC result.
 * @details This function retrieves the last CRC result calculated by the CRC module.
 * @return  The last CRC result.
 */
uint32_t cb_crc_get_crc_result(void);

#endif /*INC_CRC_H*/
