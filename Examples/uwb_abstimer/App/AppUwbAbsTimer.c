/**
 * @file    AppUwbAbsTimer.c
 * @brief   [UWB] Absolute Timer Example
 * @details This example demonstrates the usage of UWB absolute timer functionality
 *          for scheduled UWB transactions with three use cases:
 *          - Case A: TX to TX transitions
 *          - Case B: RX to RX transitions 
 *          - Case C: TX->RX->TX cycle
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbAbsTimer.h"
#include "CB_uwbframework.h"
#include "AppSysIrqCallback.h"
#include "AppUwbTRXMemoryPool.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_ABSTIMER_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_ABSTIMER_UARTPRINT_ENABLE == APP_TRUE)
#include "app_uart.h"
#define app_uwb_abstimer_print(...) app_uart_printf(__VA_ARGS__)
#else
#define app_uwb_abstimer_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_ABSTIMER_TIMEOUT_MS    1000    // 1 second timeout

#define DEF_UWB_ABSTIMER_CASE_A  1  /* TX->TX transitions */
#define DEF_UWB_ABSTIMER_CASE_B  2  /* RX->RX transitions */
#define DEF_UWB_ABSTIMER_CASE_C  3  /* TX->RX->TX cycle */

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief States for Case A 
 */
typedef enum {
    EN_APP_A_STATE_INIT,           /**< Initial state: Prepare and start TX */
    EN_APP_A_STATE_WAIT,           /**< Wait for TX completion */
    EN_APP_A_STATE_TIMER_TRIGGERED /**< TX completed, handle timestamps and restart */
} app_uwb_abstimer_case_a_state_en;

/**
 * @brief States for Case B 
 */
typedef enum {
    EN_APP_B_STATE_INIT,           /**< Initial state: Prepare and start RX */
    EN_APP_B_STATE_WAIT,           /**< Wait for RX completion */
    EN_APP_B_STATE_TRIGGERED       /**< RX completed, handle timestamps and restart */
} app_uwb_abstimer_case_b_state_en;

/**
 * @brief States for Case C 
 */
typedef enum {
    EN_APP_C_STATE_INIT,           /**< Initial state: Prepare and start first TX */
    EN_APP_C_STATE_TX_WAIT,        /**< Wait for TX completion */
    EN_APP_C_STATE_TX_TRIGGERED,   /**< TX completed, prepare to switch to RX */
    EN_APP_C_STATE_RX_WAIT,        /**< Wait for RX completion */
    EN_APP_C_STATE_RX_TRIGGERED    /**< RX completed, prepare to switch back to TX */
} app_uwb_abstimer_case_c_state_en;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void app_uwb_abstimer_case_a(void);
static void app_uwb_abstimer_case_b(void);
static void app_uwb_abstimer_case_c(void);
static void app_uwb_print_tx_timestamp(void);
static void app_uwb_print_rx_timestamp(cb_uwbsystem_rxport_en enRxPort);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
// Case selection - Set DEF_UWB_ABSTIMER_CASE_SEL to one of the case values
static uint8_t s_selected_case = DEF_UWB_ABSTIMER_CASE_C;

// State variables for each case
static app_uwb_abstimer_case_a_state_en s_casea_state = EN_APP_A_STATE_INIT;
static app_uwb_abstimer_case_b_state_en s_caseb_state = EN_APP_B_STATE_INIT;
static app_uwb_abstimer_case_c_state_en s_casec_state = EN_APP_C_STATE_INIT;

// Interrupt flags
static uint8_t s_tx_done_flag = APP_FALSE;
static uint8_t s_rx0_sfd_flag = APP_FALSE;

/**
 * @brief UWB Packet Configuration 
 * 
 * Standard configuration for all UWB transmissions and receptions
 */
static cb_uwbsystem_packetconfig_st s_uwb_packet_config = 
{
  .prfMode            = EN_PRF_MODE_BPRF_62P4,            // PRF mode selection
  .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
  .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
  .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
  .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
  .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
  .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
  .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
  .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
  .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
  .stsKey             = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
  .stsVUpper          = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
  .stsVCounter        = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
  .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
};

/**
 * @brief Timer TX Payload configuration
 */
static uint8_t s_uwb_tx_payload = 0x01;
static cb_uwbsystem_txpayload_st s_timer_payload_config = {
    .ptrAddress = &s_uwb_tx_payload,
    .payloadSize = sizeof(s_uwb_tx_payload),
};

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Main entry point for the absolute timer example
 * 
 * This function demonstrates different use cases of the UWB absolute timer:
 * 
 * @details The following cases are available:
 * - Case A (TX to TX): Uses absolute timer to schedule periodic TX operations
 *   After each TX completes, timer automatically triggers next TX
 * 
 * - Case B (RX to RX): Uses absolute timer for continuous RX operations
 *   Timer schedules next RX after current RX finishes
 * 
 * - Case C (TX->RX->TX): Demonstrates TX/RX cycle using two absolute timers
 *   TX completion triggers RX start, RX completion triggers next TX
 * 
 * Select a case by setting the DEF_UWB_ABSTIMER_CASE_SEL macro to DEF_UWB_ABSTIMER_CASE_A,
 * DEF_UWB_ABSTIMER_CASE_B, or DEF_UWB_ABSTIMER_CASE_C.
 */
void app_uwb_abstimer(void)
{
    if (s_selected_case == DEF_UWB_ABSTIMER_CASE_A) {
        app_uwb_abstimer_print("UWB Absolute Timer Case A: TX->TX transitions\r\n");
        app_uwb_abstimer_case_a();
    }
    else if (s_selected_case == DEF_UWB_ABSTIMER_CASE_B) {
        app_uwb_abstimer_print("UWB Absolute Timer Case B: RX->RX transitions\r\n"); 
        app_uwb_abstimer_case_b();
    }
    else if (s_selected_case == DEF_UWB_ABSTIMER_CASE_C) {
        app_uwb_abstimer_print("UWB Absolute Timer Case C: TX->RX->TX cycle\r\n");
        app_uwb_abstimer_case_c();
    }
    else {
        // Invalid selection - default to Case A
        app_uwb_abstimer_print("Invalid case selection - defaulting to Case A\r\n");
        app_uwb_abstimer_case_a();
    }
}

//-------------------------------
// CASE IMPLEMENTATIONS
//-------------------------------

/**
 * @brief Implementation for Case A: TX to TX transitions
 * 
 * This case demonstrates using the absolute timer to schedule repeated TX
 * operations. After each TX completes, the absolute timer automatically
 * triggers the next TX operation.
 */
static void app_uwb_abstimer_case_a(void)
{
    //--------------------------------
    // Init
    //--------------------------------
    cb_framework_uwb_init(); 

    //--------------------------------
    // Configuration
    //--------------------------------
    // TX absolute timer configuration
    cb_uwbframework_trx_scheduledconfig_st tx_abs_timer_config = {
        .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,   // Use timestamp mask 0
        .eventIndex         = EN_UWBEVENT_28_TX_DONE,         // Capture TX done event
        .absTimer           = EN_UWB_ABSOLUTE_TIMER_0,        // Use absolute timer 0
        .timeoutValue       = DEF_ABSTIMER_TIMEOUT_MS * 1000, // Convert to microseconds
        .eventCtrlMask      = EN_UWBCTRL_TX_START_MASK,       // Start TX on timeout
    };

    // TX IRQ configuration
    cb_uwbsystem_tx_irqenable_st tx_irq_config = { CB_FALSE };
    tx_irq_config.txDone = CB_TRUE;

    // Initialize state machine
    s_casea_state = EN_APP_A_STATE_INIT;
    
    // Clear flags to ensure clean state
    s_tx_done_flag = APP_FALSE;

    while(1)
    {
        switch (s_casea_state)
        {
        case EN_APP_A_STATE_INIT:
            // Enable the absolute timer and start first TX
            cb_framework_uwb_enable_scheduled_trx(tx_abs_timer_config);
            cb_framework_uwb_tx_start(&s_uwb_packet_config, &s_timer_payload_config, 
                                     &tx_irq_config, EN_TRX_START_NON_DEFERRED);
            s_casea_state = EN_APP_A_STATE_WAIT;
            break;

        case EN_APP_A_STATE_WAIT:
            // Wait for TX completion
            if (s_tx_done_flag)
            {
                s_tx_done_flag = APP_FALSE;
                s_casea_state = EN_APP_A_STATE_TIMER_TRIGGERED;
            }
            break;

        case EN_APP_A_STATE_TIMER_TRIGGERED:
            // TX completed - print timestamp and prepare for next TX
            app_uwb_print_tx_timestamp();
        
            cb_framework_uwb_tx_end();  
        
            // Reconfigure timer for next trigger
            cb_framework_uwb_configure_scheduled_trx(tx_abs_timer_config);   
            cb_framework_uwb_tx_start(&s_uwb_packet_config, &s_timer_payload_config, &tx_irq_config, EN_TRX_START_DEFERRED);
            s_casea_state = EN_APP_A_STATE_WAIT;
            break;

        default:
            break;
        }
    }
}

/**
 * @brief Implementation for Case B: RX to RX transitions
 * 
 * This case demonstrates using the absolute timer to schedule repeated RX
 * operations. After each RX completes, the absolute timer automatically
 * triggers the next RX operation.
 */
static void app_uwb_abstimer_case_b(void)
{
    //--------------------------------
    // Init
    //--------------------------------
    cb_framework_uwb_init(); 

    //--------------------------------
    // Configuration
    //--------------------------------
    // RX absolute timer configuration
    cb_uwbframework_trx_scheduledconfig_st rx_abs_timer_config = {
        .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,   // Use timestamp mask 0
        .eventIndex         = EN_UWBEVENT_17_RX0_SFD_DET,     // Capture RX SFD event
        .absTimer           = EN_UWB_ABSOLUTE_TIMER_0,        // Use absolute timer 0
        .timeoutValue       = DEF_ABSTIMER_TIMEOUT_MS * 1000, // Convert to microseconds
        .eventCtrlMask      = EN_UWBCTRL_RX0_START_MASK,      // Start RX0 on timeout
    };

    // RX IRQ configuration
    cb_uwbsystem_rx_irqenable_st rx_irq_config = { CB_FALSE };
    rx_irq_config.rx0SfdDetDone = CB_TRUE;

    // Initialize state machine
    s_caseb_state = EN_APP_B_STATE_INIT;
    
    // Clear flags to ensure clean state
    s_rx0_sfd_flag = APP_FALSE;

    while(1)
    {
        switch (s_caseb_state)
        {
        case EN_APP_B_STATE_INIT:
            // Enable the absolute timer and start first RX
            cb_framework_uwb_enable_scheduled_trx(rx_abs_timer_config);
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_uwb_packet_config, &rx_irq_config, EN_TRX_START_NON_DEFERRED);
            s_caseb_state = EN_APP_B_STATE_WAIT;
            break;

        case EN_APP_B_STATE_WAIT:
            // Wait for RX completion
            if (s_rx0_sfd_flag)
            {
                s_rx0_sfd_flag = APP_FALSE;
                s_caseb_state = EN_APP_B_STATE_TRIGGERED;
            }
            break;

        case EN_APP_B_STATE_TRIGGERED:
            // RX completed - print timestamp and prepare for next RX
            app_uwb_print_rx_timestamp(EN_UWB_RX_0);
        
            cb_framework_uwb_rx_end(EN_UWB_RX_0);
        
            // Reconfigure timer for next trigger
            cb_framework_uwb_configure_scheduled_trx(rx_abs_timer_config);
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_uwb_packet_config, &rx_irq_config, EN_TRX_START_DEFERRED);
            s_caseb_state = EN_APP_B_STATE_WAIT;
            break;

        default:
            break;
        }
    }
}

/**
 * @brief Implementation for Case C: TX->RX->TX cycle
 * 
 * This case demonstrates using two absolute timers to create a continuous
 * cycle between TX and RX operations. TX completion triggers RX, and
 * RX completion triggers the next TX.
 */
static void app_uwb_abstimer_case_c(void)
{
    //--------------------------------
    // Init
    //--------------------------------
    cb_framework_uwb_init(); 

    //--------------------------------
    // Configuration
    //--------------------------------
    // TX->RX absolute timer configuration (TX done triggers RX start)
    cb_uwbframework_trx_scheduledconfig_st tx_to_rx_config = {
        .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_0,   // Use timestamp mask 0
        .eventIndex         = EN_UWBEVENT_28_TX_DONE,         // Capture TX done event
        .absTimer           = EN_UWB_ABSOLUTE_TIMER_0,        // Use absolute timer 0
        .timeoutValue       = DEF_ABSTIMER_TIMEOUT_MS * 1000, // Convert to microseconds
    };

    // RX->TX absolute timer configuration (RX SFD triggers TX start)
    cb_uwbframework_trx_scheduledconfig_st rx_to_tx_config = {
        .eventTimestampMask = EN_UWBEVENT_TIMESTAMP_MASK_1,   // Use timestamp mask 1
        .eventIndex         = EN_UWBEVENT_17_RX0_SFD_DET,     // Capture RX SFD event
        .absTimer           = EN_UWB_ABSOLUTE_TIMER_1,        // Use absolute timer 1
        .timeoutValue       = DEF_ABSTIMER_TIMEOUT_MS * 1000, // Convert to microseconds
        .eventCtrlMask      = EN_UWBCTRL_TX_START_MASK,       // Start TX on timeout
    };

    // TX IRQ configuration
    cb_uwbsystem_tx_irqenable_st tx_irq_config = { CB_FALSE };
    tx_irq_config.txDone = CB_TRUE;

    // RX IRQ configuration
    cb_uwbsystem_rx_irqenable_st rx_irq_config = { CB_FALSE };
    rx_irq_config.rx0SfdDetDone = CB_TRUE;

    // Initialize state machine
    s_casec_state = EN_APP_C_STATE_INIT;
    
    // Clear flags to ensure clean state
    s_tx_done_flag = APP_FALSE;
    s_rx0_sfd_flag = APP_FALSE;

    while(1)
    {
        switch (s_casec_state)
        {
        case EN_APP_C_STATE_INIT:
            
            // Enable both absolute timers
            cb_framework_uwb_enable_scheduled_trx(tx_to_rx_config);
            cb_framework_uwb_enable_scheduled_trx(rx_to_tx_config);
        
            // Start first TX
            cb_framework_uwb_tx_start(&s_uwb_packet_config, &s_timer_payload_config, &tx_irq_config, EN_TRX_START_NON_DEFERRED);
            
            s_casec_state = EN_APP_C_STATE_TX_WAIT;
            break;

        case EN_APP_C_STATE_TX_WAIT:
            // Wait for TX completion
            if (s_tx_done_flag)
            {
                s_tx_done_flag = APP_FALSE;
                s_casec_state = EN_APP_C_STATE_TX_TRIGGERED;
            }
            break;
            
        case EN_APP_C_STATE_TX_TRIGGERED:
            // TX completed - print timestamp and prepare for RX
            app_uwb_print_tx_timestamp();
            app_uwb_abstimer_print("TX -> RX transition\r\n");
            
            // End TX operation
            cb_framework_uwb_tx_end();
            
            // Configure and start RX operation
            cb_framework_uwb_configure_scheduled_trx(tx_to_rx_config);
            cb_framework_uwb_rx_start(EN_UWB_RX_0, &s_uwb_packet_config, &rx_irq_config, EN_TRX_START_DEFERRED);
            
            s_casec_state = EN_APP_C_STATE_RX_WAIT;
            break;
            
        case EN_APP_C_STATE_RX_WAIT:
            // Wait for RX completion
            if (s_rx0_sfd_flag)
            {
                s_rx0_sfd_flag = APP_FALSE;
                s_casec_state = EN_APP_C_STATE_RX_TRIGGERED;
            }
            break;
            
        case EN_APP_C_STATE_RX_TRIGGERED:
            // RX completed - print timestamp and prepare for TX
            app_uwb_print_rx_timestamp(EN_UWB_RX_0);
            app_uwb_abstimer_print("RX -> TX transition\r\n");
            
            // End RX operation
            cb_framework_uwb_rx_end(EN_UWB_RX_0);
            
            // Configure and start next TX operation
            cb_framework_uwb_configure_scheduled_trx(rx_to_tx_config);
            cb_framework_uwb_tx_start(&s_uwb_packet_config, &s_timer_payload_config, &tx_irq_config, EN_TRX_START_DEFERRED);
            
            s_casec_state = EN_APP_C_STATE_TX_WAIT;
            break;

        default:
            break;
        }
    }
}

/**
 * @brief Prints the TX timestamp information.
 * 
 * This function retrieves the TX timestamp information and prints it via UART.
 */
static void app_uwb_print_tx_timestamp(void)
{
    cb_uwbsystem_tx_timestamp_st txTimestamp;  
    cb_framework_uwb_get_tx_raw_timestamp(&txTimestamp);

    app_uwb_abstimer_print(" > txStart  - %uns\r\n", txTimestamp.txStart);
    app_uwb_abstimer_print(" > txDone   - %uns\r\n", txTimestamp.txDone);
}

/**
 * @brief Prints the RX timestamp information.
 * 
 * @param enRxPort The RX port to get timestamps for.
 */
static void app_uwb_print_rx_timestamp(cb_uwbsystem_rxport_en enRxPort)
{
    cb_uwbsystem_rx_tsutimestamp_st rx_timestamp = { 0 };
    cb_framework_uwb_get_rx_tsu_timestamp(&rx_timestamp, EN_UWB_RX_0);
    
    app_uwb_abstimer_print(" > rxTsu - %fns\r\n", rx_timestamp.rxTsu);
}

//-------------------------------
// IRQ HANDLERS
//-------------------------------

/**
 * @brief TX done interrupt handler
 * 
 * Called when a TX operation completes.
 */
void cb_uwbapp_tx_done_irqhandler(void)
{ 
    s_tx_done_flag = APP_TRUE;
}

/**
 * @brief RX0 SFD detected interrupt handler
 * 
 * Called when SFD is detected on RX0 port.
 */
void cb_uwbapp_rx0_sfd_detected_irqcb(void)
{
    s_rx0_sfd_flag = APP_TRUE;
}

