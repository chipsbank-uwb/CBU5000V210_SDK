/*
 * @file    main.c
 * @brief   TDOA Tag main entry point
 * @details Based on uwb_simple_rx template. Initialises SDK subsystems and
 *          enters the Tag application main loop.
 * @author  Chipsbank
 * @date    2026
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "main.h"
#include "app_uart.h"
#include "CB_system.h"
#include "tdoa_config.h"
#include "AppUwbTdoaTag.h"

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
//  app_uart_change_baudrate(EN_UART_BAUDRATE_115200);

  // Init interrupt Priority of modules
  Initialize_IRQ_Priority();

  cb_uwbsystem_systemconfig_st uwbSystemConfig = {
    .channelNum           = EN_UWB_Channel_9,
    .bbpllFreqOffest_rf   = 127,
    .powerCode_tx         = 62,
    .operationMode_rx     = EN_UWB_RX_OPERATION_MODE_COEXIST,
  };

  cb_system_uwb_set_system_config(&uwbSystemConfig);

  cb_system_uwb_ram_init(&g_UWB_TXBANKMEMORY, &g_UWB_RXBANKMEMORY,
                         sizeof(g_UWB_TXBANKMEMORY), sizeof(g_UWB_RXBANKMEMORY));

  app_uart_printf("[TAG] TDOA Tag v1.0\r\n");

  // Load TDOA configuration from NVM (or defaults)
  tdoa_config_init();

  // Enter Tag main loop (never returns)
  while (1)
  {
    app_tdoa_tag();
  }
}

void DWT_Init(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

void Initialize_IRQ_Priority(void)
{
    NVIC_SetPriority(UART0_IRQn,                 2);
    NVIC_SetPriority(UWB_RX0_DONE_IRQn,          1);
    NVIC_SetPriority(UWB_RX0_SFD_DET_DONE_IRQn,  1);
    NVIC_SetPriority(UWB_RX_DONE_IRQn,           1);
}
