/*
 * @file    main.c
 * @brief   TDOA Anchor Initiator main entry point
 * @details System initialization and entry to TDOA Initiator application loop.
 *          Based on uwb_simple_tx main.c framework with TDOA-specific modifications.
 * @author  Chipsbank
 * @date    2026
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "main.h"
#include "CB_system.h"
#include "CB_uwbframework.h"
#include "tdoa_config.h"
#include "CB_flash.h"
#include "AppUwbTdoaInitiator.h"
#include "app_uart.h"

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
uint8_t g_UWB_TXBANKMEMORY[4096] __attribute__((section("SPECIFIC_UWB_TXBANK_RAMMEMORY")));
uint8_t g_UWB_RXBANKMEMORY[4096] __attribute__((section("SPECIFIC_UWB_RXBANK_RAMMEMORY")));

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief The main function of the program.
 *
 * Initialization sequence per Interface Contract section 4.1,
 * then enters the TDOA Initiator application loop (never returns).
 */
int main(void)
{
    // 1. Initializes the Data Watchpoint and Trace (DWT)
    DWT_Init();

    // 2. Run RC Timing Calibration once (100ms background via NMI handler)
    cb_system_rc_calibration();

    // 3. UWB chip initialization
    cb_system_chip_init();

    // 4. UART initialization + set baudrate to 115200
    app_uart_init();
    app_uart_change_baudrate(EN_UART_BAUDRATE_115200);

    // 5. Init interrupt priority of modules
    Initialize_IRQ_Priority();

    // 6. UWB system configuration
    cb_uwbsystem_systemconfig_st uwbSystemConfig = {
        .channelNum           = EN_UWB_Channel_9,
        .bbpllFreqOffest_rf   = 127,
        .powerCode_tx         = 62,
        .operationMode_rx     = EN_UWB_RX_OPERATION_MODE_COEXIST,
    };

    app_uart_printf("TDOA Initiator v1.0\n");

    cb_system_uwb_set_system_config(&uwbSystemConfig);

    // 7. UWB TX/RX RAM initialization (once at start-up)
    cb_system_uwb_ram_init(&g_UWB_TXBANKMEMORY, &g_UWB_RXBANKMEMORY,
                           sizeof(g_UWB_TXBANKMEMORY), sizeof(g_UWB_RXBANKMEMORY));

    // 8. TDOA configuration (load from NVM or defaults)
    tdoa_config_init();

    // 9. Enter TDOA Initiator application loop (never returns)
    while (1) {
        app_tdoa_initiator();
    }
}

/**
 * @brief Initializes the Data Watchpoint and Trace (DWT) unit for performance measurement.
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
 * Per Interface Contract section 6.1 (Initiator IRQ priorities).
 */
void Initialize_IRQ_Priority(void)
{
    NVIC_SetPriority(UART0_IRQn,                2);
    NVIC_SetPriority(TIMER_0_IRQn,              1);
    NVIC_SetPriority(UWB_TX_DONE_IRQn,          1);
    NVIC_SetPriority(UWB_TX_SFD_MARK_IRQn,      1);
    NVIC_SetPriority(UWB_RX0_DONE_IRQn,         1);
    NVIC_SetPriority(UWB_RX0_SFD_DET_DONE_IRQn, 1);
    NVIC_SetPriority(UWB_RX_DONE_IRQn,          1);
}
