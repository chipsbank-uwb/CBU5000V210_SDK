/**
 * @file    AppUwbDSTWR.h
 * @brief   
 * @details 
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_UWB_DSTWR_H
#define __APP_UWB_DSTWR_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
  uint8_t logOpt;
  uint32_t numberOfCycles;
  int32_t rangingBias;
} app_dstwrconfig_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_dstwr_initiator(void);
/**
 * @brief   Initialize the application timer module.
 * @details This function turns on Timer 0 disables their interrupts, 
 *          and enables CPU Timer 0 interrupts.
 */
void app_dstwr_timer_init(uint16_t timeoutMs);

/**
 * @brief   Off the application timer module.
 * @details This function turns off Timer 0 disables their interrupts, 
 *          and disable CPU Timer 0 interrupts.
 */
void app_dstwr_timer_off(void);


#endif // __APP_UWB_DSTWR_H
