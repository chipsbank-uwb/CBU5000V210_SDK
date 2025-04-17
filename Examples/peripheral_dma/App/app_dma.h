/**
 * @file    app_dma.h
 * @brief   [CPU Subsystem] DMA Application Module Header
 * @details This header file contains function prototypes for DMA examples.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __APP_CPU_DMA_H
#define __APP_CPU_DMA_H


//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"
#include "CB_dma.h"

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
extern volatile uint32_t timer_value;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_dma_init(void);
void app_amd_irq_callback(void);
void app_dma_ram_to_ram_example(void);
void APP_DMA_PertoRAM(void);
void app_dma_ram_to_ram_lli(void);
void APP_DMA_TIMER_0_IRQ_Callback(void);
void app_peripheral_dma_demo_init(void);

 #endif

