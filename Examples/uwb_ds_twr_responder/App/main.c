/*
 * @file    main.c
 * @brief   Implementation file for the main module.
 * @details This file contains the implementation of the main module, including
 *          the main function, initialization functions, and other utility functions.
 *          It also includes necessary headers and sections for function prototypes,
 *          definitions, enums, structs/unions, and global variables.
 * @author  Chipsbank
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "main.h"
#if (APP_FREERTOS_ENABLE == APP_TRUE)
#include "AppSysRtos.h"
#else
#include "TaskHandler.h"
#endif
#include "app_uart.h"
#include "CB_system.h"
#include "CB_SleepDeepSleep.h"
#include "AppUwbDstwr.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void DWT_Init(void);
void Initialize_IRQ_Priority(void);

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
uint8_t g_UWB_TXBANKMEMORY[4096] __attribute__((section("SPECIFIC_UWB_TXBANK_RAMMEMORY"))) ;
uint8_t g_UWB_RXBANKMEMORY[4096] __attribute__((section("SPECIFIC_UWB_RXBANK_RAMMEMORY"))) ;

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief The main function of the program.
 * 
 * This function serves as the entry point of the program. It is called automatically
 * when the program starts executing. The function contains an infinite loop, which
 * ensures that the program continues running indefinitely.
 * 
 * @return An integer value representing the exit status of the program. 
 *         This function always returns 0, indicating successful execution.
 */
int main(void)
{  
  // Initializes the Data Watchpoint and Trace (DWT) 
  DWT_Init();
  
  // Run RC Timing Calibration once. Calibration done in background after 100ms (NMI handler).
  cb_system_rc_calibration();
  
  // UWB initialization
  cb_system_chip_init();

  // Application Initialization
  app_uart_init();

  // Init interrupt Priority of modules
  Initialize_IRQ_Priority();

  cb_uwbsystem_systemconfig_st uwbSystemConfig = {   
    .channelNum           = EN_UWB_Channel_9,
    .bbpllFreqOffest_rf   = 127,
    .powerCode_tx         = 28,
    .operationMode_rx     = EN_UWB_RX_OPERATION_MODE_COEXIST,
    .antennaID            = EN_ANTENNA_0_0_3D, 
  };  
  
  app_uart_printf("DSTWR RESPONDER\n");
  
  cb_system_uwb_set_system_config(&uwbSystemConfig);
  
  cb_system_uwb_ram_init(&g_UWB_TXBANKMEMORY,&g_UWB_RXBANKMEMORY,sizeof (g_UWB_TXBANKMEMORY), sizeof (g_UWB_RXBANKMEMORY)); //Initializes only once upon start-up. 

#if (APP_FREERTOS_ENABLE == APP_TRUE)
  // Init uwb task
  APP_RTOS_Init();
	//start scheduler
	APP_RTOS_StartScheduler();
#else
  while(1)
  {
    app_dstwr_responder();
  }
#endif
}

/**
 * @brief Initializes the Data Watchpoint and Trace (DWT) unit for performance measurement.
 * 
 * This function checks if the DWT unit is enabled. If not, it enables the DWT unit
 * and configures it to count processor cycles for performance measurement.
 * 
 */
void DWT_Init(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
 * @brief Initializes the priority of Interrupt Service Routines (ISRs).
 * 
 * This function sets the priority of various Interrupt Service Routines (ISRs) 
 * to ensure proper handling of interrupts. The priority levels are determined 
 * by the number of bits available for interrupt priority levels (__NVIC_PRIO_BITS).
 * 
 */
void Initialize_IRQ_Priority(void)
{    
//  NVIC_SetPriority(Interrupt0_IRQn             , 1);
//  NVIC_SetPriority(Interrupt1_IRQn             , 1);
//  NVIC_SetPriority(Interrupt2_IRQn             , 1);
//  NVIC_SetPriority(CRYPTO_IRQn                 , 1);
//  NVIC_SetPriority(PKA_IRQn                    , 1);
//  NVIC_SetPriority(TRNG_IRQn                   , 1);
//  NVIC_SetPriority(Interrupt6_IRQn             , 1);
//  NVIC_SetPriority(Interrupt7_IRQn             , 1);
//  NVIC_SetPriority(SPI_IRQn                    , 1);  
    NVIC_SetPriority(UART0_IRQn                  , 2);
//  NVIC_SetPriority(UART1_IRQn                  , 2);
//  NVIC_SetPriority(Interrupt11_IRQn            , 1);
    NVIC_SetPriority(TIMER_0_IRQn                , 1);
    NVIC_SetPriority(TIMER_1_IRQn                , 1);
//  NVIC_SetPriority(TIMER_2_IRQn                , 1);
//  NVIC_SetPriority(TIMER_3_IRQn                , 1);
//  NVIC_SetPriority(Interrupt16_IRQn            , 1);
//  NVIC_SetPriority(Interrupt17_IRQn            , 1);
//  NVIC_SetPriority(BLE_IRQn                    , 1);
//  NVIC_SetPriority(Interrupt19_IRQn            , 1);
//  NVIC_SetPriority(Interrupt20_IRQn            , 1);
    NVIC_SetPriority(UWB_RX0_DONE_IRQn           , 1);
    NVIC_SetPriority(UWB_RX0_PD_DONE_IRQn        , 1);
    NVIC_SetPriority(UWB_RX0_SFD_DET_DONE_IRQn   , 1);
    NVIC_SetPriority(UWB_RX1_DONE_IRQn           , 1);
    NVIC_SetPriority(UWB_RX1_PD_DONE_IRQn        , 1);
    NVIC_SetPriority(UWB_RX1_SFD_DET_DONE_IRQn   , 1);
    NVIC_SetPriority(UWB_RX2_DONE_IRQn           , 1);
    NVIC_SetPriority(UWB_RX2_PD_DONE_IRQn        , 1);
    NVIC_SetPriority(UWB_RX2_SFD_DET_DONE_IRQn   , 1);
    NVIC_SetPriority(UWB_RX_STS_CIR_END_IRQn     , 1);
    NVIC_SetPriority(UWB_RX_PHR_DETECTED_IRQn    , 1);
    NVIC_SetPriority(UWB_RX_DONE_IRQn            , 1);
    NVIC_SetPriority(UWB_TX_DONE_IRQn            , 1);
    NVIC_SetPriority(UWB_TX_SFD_MARK_IRQn        , 1);
//  NVIC_SetPriority(Interrupt35_IRQn            , 1);
//  NVIC_SetPriority(Interrupt36_IRQn            , 1);
//  NVIC_SetPriority(Interrupt37_IRQn            , 1);
//  NVIC_SetPriority(Interrupt38_IRQn            , 1);
}
