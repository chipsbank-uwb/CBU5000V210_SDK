/**
 * @file AppSysIrqCallback.h
 * @brief [SYSTEM] Header file for IRQ callback management.
 * @details This file contains declarations related to managing IRQ callback functions.
 *          It defines enums for IRQ entry numbers and structures for callback lists.
 * @author Chipsbank
 * @date 2024
 */

#ifndef __APP_SYS_IRQ_CALLBACK_H
#define __APP_SYS_IRQ_CALLBACK_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"
#include "CB_uwb.h"
#include "CB_gpio.h"
#include "CB_timer.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum
{
  EN_IRQENTRY_UWB_TX_DONE_APP_IRQ   = 0    ,																			    
  EN_IRQENTRY_UWB_TX_SFD_MARK_APP_IRQ      ,
  EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ         ,
  EN_IRQENTRY_UWB_RX0_PD_DONE_APP_IRQ      ,
  EN_IRQENTRY_UWB_RX0_SFD_DET_DONE_APP_IRQ ,
  EN_IRQENTRY_UWB_RX1_DONE_APP_IRQ         ,
  EN_IRQENTRY_UWB_RX1_PD_DONE_APP_IRQ      ,
  EN_IRQENTRY_UWB_RX1_SFD_DET_DONE_APP_IRQ ,
  EN_IRQENTRY_UWB_RX2_DONE_APP_IRQ         ,
  EN_IRQENTRY_UWB_RX2_PD_DONE_APP_IRQ      ,
  EN_IRQENTRY_UWB_RX2_SFD_DET_DONE_APP_IRQ ,
  EN_IRQENTRY_UWB_RX_STS_CIR_END_APP_IRQ   ,
  EN_IRQENTRY_UWB_RX_PHR_DETECTED_APP_IRQ  ,
  EN_IRQENTRY_UWB_RX_DONE_APP_IRQ          ,
	
  EN_IRQENTRY_GPIO_APP_IRQ                 ,
	
  EN_IRQENTRY_TIMER_0_APP_IRQ              ,
  EN_IRQENTRY_TIMER_1_APP_IRQ              ,
  EN_IRQENTRY_TIMER_2_APP_IRQ              ,
  EN_IRQENTRY_TIMER_3_APP_IRQ              ,

  EN_IRQENTRY_UART_0_RXB_FULL_APP_IRQ      ,
  EN_IRQENTRY_UART_0_TXB_EMPTY_APP_IRQ     ,

  EN_IRQENTRY_UART_1_RXB_FULL_APP_IRQ      ,
  EN_IRQENTRY_UART_1_TXB_EMPTY_APP_IRQ     ,

  EN_IRQENTRY_WDT_APP_IRQ                  ,	

  EN_IRQENTRY_CRYPTO_APP_IRQ               ,

  EN_IRQENTRY_CRC_APP_IRQ                  ,	

  EN_IRQENTRY_DMA_APP_IRQ                  ,	

  EN_IRQENTRY_DMA_CHANNEL_0_APP_IRQ        ,
  EN_IRQENTRY_DMA_CHANNEL_1_APP_IRQ        ,
  EN_IRQENTRY_DMA_CHANNEL_2_APP_IRQ        ,	
  
  EN_IRQENTRY_I2C_APP_IRQ                  ,

  EN_IRQENTRY_MAX_NUMBER                   ,
} enIrqEntry;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef void (*irq_callback_t)(void);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_irq_register_irqcallback(enIrqEntry entryNumber, irq_callback_t callback);
void app_irq_deregister_irqcallback (enIrqEntry entryNumber, irq_callback_t callback);
  
#endif // __APP_SYS_IRQ_CALLBACK_H
