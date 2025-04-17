/******************************************************************************
 * @file     startup_ARMCM33.c
 * @brief    CMSIS-Core Device Startup File for Cortex-M33 Device
 * @version  V2.1.0
 * @date     16. December 2020
 ******************************************************************************/
/*
 * Copyright (c) 2009-2020 Arm Limited. All rights reserved.
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

#if defined (ARMCM33)
  #include "ARMCM33.h"
#elif defined (ARMCM33_TZ)
  #include "ARMCM33_TZ.h"
#elif defined (ARMCM33_DSP_FP)
  #include "ARMCM33_DSP_FP.h"
#elif defined (ARMCM33_DSP_FP_TZ)
  #include "ARMCM33_DSP_FP_TZ.h"
#else
  #error device not specified!
#endif

#include "APP_CompileOption.h"
#include "APP_common.h"

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
extern uint32_t __STACK_SEAL;
#endif

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler  (void);
            void Default_Handler(void);
            
/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler      (void) __attribute__ ((weak));
void MemManage_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SecureFault_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler        (void) __attribute__ ((weak));

void Interrupt0_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt1_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt2_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt3_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt4_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt5_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt6_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt7_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt8_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt9_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt10_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt11_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt12_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt13_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt14_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt15_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt16_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt17_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt18_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt19_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt20_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt21_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt22_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt23_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt24_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt25_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt26_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt27_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt28_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt29_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt30_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt31_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt32_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt33_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt34_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt35_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt36_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt37_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt38_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));

/*----------------------------------------------------------------------------
  Extern section: Project based interrupt handler
 *----------------------------------------------------------------------------*/
extern __NO_RETURN void cb_gpio_irqhandler(void);
extern __NO_RETURN void cb_dma_irqhandler(void);
extern __NO_RETURN void cb_crypto_irqhandler(void);
extern __NO_RETURN void cb_pka_irqhandler(void);
extern __NO_RETURN void cb_trng_irqhandler(void);
extern __NO_RETURN void cb_crc_irqhandler(void);
extern __NO_RETURN void cb_i2c_irqhandler(void);
extern __NO_RETURN void cb_timer_0_irqhandler(void);
extern __NO_RETURN void cb_timer_1_irqhandler(void);
extern __NO_RETURN void cb_timer_2_irqhandler(void);
extern __NO_RETURN void cb_timer_3_irqhandler(void);
extern __NO_RETURN void cb_uart_0_irqhandler(void);
extern __NO_RETURN void cb_uart_1_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx0_done_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx0_preamble_detected_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx0_sfd_detected_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx1_done_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx1_preamble_detected_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx1_sfd_detected_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx2_done_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx2_preamble_detected_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx2_sfd_detected_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx_sts_cir_end_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx_phr_detected_irqhandler(void);
extern __NO_RETURN void cb_uwb_rx_done_irqhandler(void);
extern __NO_RETURN void cb_uwb_tx_done_irqhandler(void);
extern __NO_RETURN void cb_uwb_tx_sfd_mark_irqhandler(void);
extern __NO_RETURN void cb_spi_irqhandler(void);
extern __NO_RETURN void CB_BLE_IRQ_Handler (void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const VECTOR_TABLE_Type __VECTOR_TABLE[496];
       const VECTOR_TABLE_Type __VECTOR_TABLE[496] __VECTOR_TABLE_ATTRIBUTE = {
  (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*     Initial Stack Pointer */
  Reset_Handler,                            /*     Reset Handler */
  NMI_Handler,                              /* -14 NMI Handler */
  HardFault_Handler,                        /* -13 Hard Fault Handler */
  MemManage_Handler,                        /* -12 MPU Fault Handler */
  BusFault_Handler,                         /* -11 Bus Fault Handler */
  UsageFault_Handler,                       /* -10 Usage Fault Handler */
  SecureFault_Handler,                      /*  -9 Secure Fault Handler */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  SVC_Handler,                              /*  -5 SVCall Handler */
  DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
  0,                                        /*     Reserved */
  PendSV_Handler,                           /*  -2 PendSV Handler */
  SysTick_Handler,                          /*  -1 SysTick Handler */

  /* Interrupts */
  Interrupt0_Handler,                       /*   0 Interrupt 0 */
  Interrupt1_Handler,                       /*   1 Interrupt 1 */
  cb_dma_irqhandler,                        /*   2 Interrupt 2 */
  cb_crypto_irqhandler,                     /*   3 Interrupt 3 */
  cb_pka_irqhandler,                        /*   4 Interrupt 4 */
  cb_trng_irqhandler,                       /*   5 Interrupt 5 */
  cb_crc_irqhandler,                        /*   6 Interrupt 6 */
  cb_gpio_irqhandler,                       /*   7 Interrupt 7 */
  cb_spi_irqhandler,                        /*   8 Interrupt 8 */
  cb_uart_0_irqhandler,                     /*   9  UART0_IRQn */
  cb_uart_1_irqhandler,                     /*   10 UART1_IRQn */
  cb_i2c_irqhandler,                        /*   11 Interrupt 11 */                              
  cb_timer_0_irqhandler,                    /*   12 TIMER_0_IRQn */
  cb_timer_1_irqhandler,                    /*   13 TIMER_1_IRQn */
  cb_timer_2_irqhandler,                    /*   14 TIMER_2_IRQn */
  cb_timer_3_irqhandler,                    /*   15 TIMER_3_IRQn */
  Interrupt16_Handler,                      /*   16 Interrupt 16 */
  Interrupt17_Handler,                      /*   17 Interrupt 17 */
#if __has_include(<CB_ble.h>)
  CB_BLE_IRQ_Handler,                       /*   18 Interrupt 18 */
#else
  Interrupt18_Handler,
#endif
  Interrupt19_Handler,                      /*   19 Interrupt 19 */
  Interrupt20_Handler,                      /*   20 Interrupt 20 */
  cb_uwb_rx0_done_irqhandler,               /*   21 UWB_RX0_DONE_IRQn */
  cb_uwb_rx0_preamble_detected_irqhandler,  /*   22 UWB_RX0_PD_DONE_IRQn */
  cb_uwb_rx0_sfd_detected_irqhandler,       /*   23 UWB_RX0_SFD_DET_DONE */
  cb_uwb_rx1_done_irqhandler,               /*   24 UWB_RX1_DONE_IRQn */
  cb_uwb_rx1_preamble_detected_irqhandler,  /*   25 UWB_RX1_PD_DONE_IRQn */
  cb_uwb_rx1_sfd_detected_irqhandler,       /*   26 UWB_RX1_SFD_DET_DONE */
  cb_uwb_rx2_done_irqhandler,               /*   27 UWB_RX2_DONE_IRQn */
  cb_uwb_rx2_preamble_detected_irqhandler,  /*   28 UWB_RX2_PD_DONE_IRQn */
  cb_uwb_rx2_sfd_detected_irqhandler,       /*   29 UWB_RX2_SFD_DET_DONE */
  cb_uwb_rx_sts_cir_end_irqhandler,         /*   30 UWB_RX_STS_CIR_END_IRQn */
  cb_uwb_rx_phr_detected_irqhandler,        /*   31 UWB_RX_PHR_DETECTED_IRQn */
  cb_uwb_rx_done_irqhandler,                /*   32 UWB_RX_DONE_IRQn */
  cb_uwb_tx_done_irqhandler,                /*   33 UWB_TX_DONE_IRQn */
  cb_uwb_tx_sfd_mark_irqhandler,            /*   34 UWB_TX_SFD_MARK_IRQn */
  Interrupt35_Handler,                      /*   35 Interrupt 35 */
  Interrupt36_Handler,                      /*   36 Interrupt 36 */
  Interrupt37_Handler,                      /*   37 Interrupt 37 */
  Interrupt38_Handler,                      /*   38 Interrupt 38 */
                                            /* Interrupts 39 .. 480 are left out */
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Initialize SysTick timer function
 *----------------------------------------------------------------------------*/
#define  SYSTICK_FREQ    1000U       // 1ms tick resolution
void SysTick_Init(void) 
{
  // Configure SysTick: SystemCoreClock / SYSTICK_FREQ generates 1ms tick
  SysTick_Config(SystemCoreClock / SYSTICK_FREQ);

  // Enable SysTick interrupt in the NVIC
  NVIC_EnableIRQ(SysTick_IRQn);
}

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{
  __set_PSP((uint32_t)(&__INITIAL_SP));

  __set_MSPLIM((uint32_t)(&__STACK_LIMIT));
  __set_PSPLIM((uint32_t)(&__STACK_LIMIT));

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
  __TZ_set_STACKSEAL_S((uint32_t *)(&__STACK_SEAL));
#endif

  SystemInit();                             /* CMSIS System Initialization */
  #if (APP_FREERTOS_ENABLE != APP_TRUE) 
  SysTick_Init();
  #endif  
  __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}


#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

/*----------------------------------------------------------------------------
  Hard Fault Handler
 *----------------------------------------------------------------------------*/
void HardFault_Handler(void)
{
  while(1);
}

/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
volatile uint32_t sysTickCounter = 0;// System tick counter
void SysTick_Handler(void) 
{
  sysTickCounter++;
}
/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
  while(1);
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic pop
#endif

/*----------------------------------------------------------------------------
  WEAK function body for IRQ handler
 *----------------------------------------------------------------------------*/
__WEAK void cb_dma_irqhandler										    (void) {while (1) {}}
__WEAK void cb_crypto_irqhandler                    (void) {while (1) {}}
__WEAK void cb_pka_irqhandler                       (void) {while (1) {}}
__WEAK void cb_trng_irqhandler                      (void) {while (1) {}}
__WEAK void cb_crc_irqhandler                       (void) {while (1) {}}
__WEAK void cb_gpio_irqhandler                      (void) {while (1) {}}
__WEAK void cb_spi_irqhandler                       (void) {while (1) {}}
__WEAK void cb_uart_0_irqhandler                    (void) {while (1) {}}
__WEAK void cb_uart_1_irqhandler                    (void) {while (1) {}}
__WEAK void cb_i2c_irqhandler                       (void) {while (1) {}}
__WEAK void cb_timer_0_irqhandler                   (void) {while (1) {}}
__WEAK void cb_timer_1_irqhandler                   (void) {while (1) {}}
__WEAK void cb_timer_2_irqhandler                   (void) {while (1) {}}
__WEAK void cb_timer_3_irqhandler                   (void) {while (1) {}}
__WEAK void cb_uwb_rx0_done_irqhandler              (void) {while (1) {}}
__WEAK void cb_uwb_rx0_preamble_detected_irqhandler (void) {while (1) {}}
__WEAK void cb_uwb_rx0_sfd_detected_irqhandler      (void) {while (1) {}}
__WEAK void cb_uwb_rx1_done_irqhandler              (void) {while (1) {}}
__WEAK void cb_uwb_rx1_preamble_detected_irqhandler (void) {while (1) {}}
__WEAK void cb_uwb_rx1_sfd_detected_irqhandler      (void) {while (1) {}}
__WEAK void cb_uwb_rx2_done_irqhandler              (void) {while (1) {}}
__WEAK void cb_uwb_rx2_preamble_detected_irqhandler (void) {while (1) {}}
__WEAK void cb_uwb_rx2_sfd_detected_irqhandler      (void) {while (1) {}}
__WEAK void cb_uwb_rx_sts_cir_end_irqhandler        (void) {while (1) {}}
__WEAK void cb_uwb_rx_phr_detected_irqhandler       (void) {while (1) {}}
__WEAK void cb_uwb_rx_done_irqhandler               (void) {while (1) {}}
__WEAK void cb_uwb_tx_done_irqhandler               (void) {while (1) {}}
__WEAK void cb_uwb_tx_sfd_mark_irqhandler           (void) {while (1) {}}
