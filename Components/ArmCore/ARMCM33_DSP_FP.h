/**************************************************************************//**
 * @file     ARMCM33_DSP_FP.h
 * @brief    CMSIS Core Peripheral Access Layer Header File for
 *           ARMCM33 Device (configured for ARMCM33 with FPU, with DSP extension)
 * @version  V5.3.1
 * @date     09. July 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ARMCM33_DSP_FP_H
#define ARMCM33_DSP_FP_H

#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------  Interrupt Number Definition  ------------------------ */

typedef enum IRQn
{
/* -------------------  Processor Exceptions Numbers  ----------------------------- */
  NonMaskableInt_IRQn           = -14,     /*  2 Non Maskable Interrupt */
  HardFault_IRQn                = -13,     /*  3 HardFault Interrupt */
  MemoryManagement_IRQn         = -12,     /*  4 Memory Management Interrupt */
  BusFault_IRQn                 = -11,     /*  5 Bus Fault Interrupt */
  UsageFault_IRQn               = -10,     /*  6 Usage Fault Interrupt */
  SecureFault_IRQn              =  -9,     /*  7 Secure Fault Interrupt */
  SVCall_IRQn                   =  -5,     /* 11 SV Call Interrupt */
  DebugMonitor_IRQn             =  -4,     /* 12 Debug Monitor Interrupt */
  PendSV_IRQn                   =  -2,     /* 14 Pend SV Interrupt */
  SysTick_IRQn                  =  -1,     /* 15 System Tick Interrupt */

/* -------------------  Processor Interrupt Numbers  ------------------------------ */
  Interrupt0_IRQn               =   0,
  Interrupt1_IRQn               =   1,
  DMA_IRQn                      =   2,
  CRYPTO_IRQn                   =   3,
  PKA_IRQn                      =   4,
  TRNG_IRQn                     =   5,
  CRC_IRQn                      =   6,
  GPIO_IRQn                     =   7,
  SPI_IRQn                      =   8,
  UART0_IRQn                    =   9,
  UART1_IRQn                    =   10,
  I2C_IRQn                      =   11,
  TIMER_0_IRQn                  =   12,
  TIMER_1_IRQn                  =   13,
  TIMER_2_IRQn                  =   14,
  TIMER_3_IRQn                  =   15,
  Interrupt16_IRQn              =   16,
  Interrupt17_IRQn              =   17,
  BLE_IRQn                      =   18,
  Interrupt19_IRQn              =   19,  
  Interrupt20_IRQn              =   20,
  UWB_RX0_DONE_IRQn             =   21, //- RXMASK[0]
  UWB_RX0_PD_DONE_IRQn          =   22, //- RXMASK[1]
  UWB_RX0_SFD_DET_DONE_IRQn     =   23, //- RXMASK[2]
  UWB_RX1_DONE_IRQn             =   24, //- RXMASK[3]
  UWB_RX1_PD_DONE_IRQn          =   25, //- RXMASK[4]
  UWB_RX1_SFD_DET_DONE_IRQn     =   26, //- RXMASK[5]
  UWB_RX2_DONE_IRQn             =   27, //- RXMASK[6]
  UWB_RX2_PD_DONE_IRQn          =   28, //- RXMASK[7]
  UWB_RX2_SFD_DET_DONE_IRQn     =   29, //- RXMASK[8]
  UWB_RX_STS_CIR_END_IRQn       =   30, //- RXMASK[9]
  UWB_RX_PHR_DETECTED_IRQn      =   31, //- RXMASK[10]
  UWB_RX_DONE_IRQn              =   32, //- RXMASK[11]
  UWB_TX_DONE_IRQn              =   33, //- TXMASK[0]
  UWB_TX_SFD_MARK_IRQn          =   34, //- TXMASK[1] 
  Interrupt35_IRQn              =   35, 
  Interrupt36_IRQn              =   36, 
  Interrupt37_IRQn              =   37, 
  Interrupt38_IRQn              =   38   
  /* Interrupts 39 .. 480 are left out */
} IRQn_Type;


/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __CM33_REV                0x0000U   /* Core revision r0p1 */
#define __SAUREGION_PRESENT       0U        /* SAU regions present */
#define __MPU_PRESENT             1U        /* MPU present */
#define __VTOR_PRESENT            1U        /* VTOR present */
#define __NVIC_PRIO_BITS          3U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             1U        /* FPU present */
#define __DSP_PRESENT             1U        /* DSP extension present */

#include "core_cm33.h"                      /* Processor and core peripherals */
#include "system_ARMCM33.h"                 /* System Header */


/* --------  End of section using anonymous unions and disabling warnings  -------- */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


#ifdef __cplusplus
}
#endif

#endif  /* ARMCM33_DSP_FP_H */
