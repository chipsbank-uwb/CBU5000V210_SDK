/**
 * @file CB_system.c
 * @brief Implementation of functions related to the CB_system module.
 * @details This file contains the implementation of functions for initializing and controlling
 *          the Ultra-Wideband (UWB) communication system.
 * @author Chipsbank
 * @date 2024
 */

 //-------------------------------
 // INCLUDE SECTION
 //-------------------------------
#include <string.h>

#include "CB_system.h"
#include "NonLIB_sharedUtils.h"
#include "CB_wdt.h"
#include "CB_timer.h"
#include "CB_UwbDrivers.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define DEF_RC_CALIBRATION_DEFAULTTIME_IN_MS 100

#define DEF_RC_CAL_USEIRQ_WDT 1
#define DEF_RC_CAL_USEIRQ_TIMER 2
#define DEF_RC_CAL_USEIRQ DEF_RC_CAL_USEIRQ_WDT

static uint32_t sRC_lastCPUCount = 0;
enum enRC_CAL_MODE
{
    EN_IDLE,
    EN_ONCE,
    EN_PERIODIC
};
static enum enRC_CAL_MODE enRCCalMode = EN_IDLE;
static uint32_t sRCCalibrationTimeInMs = 0;

extern float RC_CompensateRatio;

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define CB_SYSTEM_UARTPRINT_ENABLE CB_FALSE
#if (CB_SYSTEM_UARTPRINT_ENABLE == CB_TRUE)
#include "app_uart.h"
#define CB_SYSTEM_PRINT(...) app_uart_printf(__VA_ARGS__)
#else
#define CB_SYSTEM_PRINT(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_ABS_TIMER_MAX_TIMEOUT_US    34359738  // Maximum timeout: 34.36 seconds (34,359,738 us)
                                                  // 2^32 * 8ns (ABS count unit) = 34,359,738,368ns 

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

typedef struct
{
  cb_uwbsystem_systemconfig_st CB_SystemConfigContainer;
  cb_uwbsystem_packetconfig_st CB_TxConfigContainer;
  cb_uwbsystem_packetconfig_st CB_RxConfigContainer;
}  __attribute__((aligned(4))) stUwbAllConfigContainer;

/*Default uwb configuration on start up*/
static stUwbAllConfigContainer s_Local_UwbAllConfigContainer = {
  /*Local Container - UWB System Config*/
  .CB_SystemConfigContainer.channelNum          = EN_UWB_Channel_9,
  .CB_SystemConfigContainer.bbpllFreqOffest_rf  = 127,
  .CB_SystemConfigContainer.powerCode_tx        = 28,
  .CB_SystemConfigContainer.operationMode_rx    = EN_UWB_RX_OPERATION_MODE_COEXIST,
  //.CB_SystemConfigContainer.extendedArgs;     

   /*Local Container - UWB Tx Config*/
  .CB_TxConfigContainer.prfMode                 = EN_PRF_MODE_BPRF_62P4,                                       // PRF mode selection
  .CB_TxConfigContainer.psduDataRate            = EN_PSDU_DATA_RATE_6P81,                                 // PSDU data rate
  .CB_TxConfigContainer.bprfPhrDataRate         = EN_BPRF_PHR_DATA_RATE_0P85,                             // BPRF PHR data rate
  .CB_TxConfigContainer.preambleCodeIndex       = EN_UWB_PREAMBLE_CODE_IDX_9,                             // Preamble code index (9-32)
  .CB_TxConfigContainer.preambleDuration        = EN_PREAMBLE_DURATION_64_SYMBOLS,                        // Preamble duration (0-1)
  .CB_TxConfigContainer.sfdId                   = EN_UWB_SFD_ID_2,                                        // SFD identifier (0-4)
  .CB_TxConfigContainer.phrRangingBit           = 0x00,                                                   // PHR Ranging Bit (0-1)
  .CB_TxConfigContainer.rframeConfig            = EN_RFRAME_CONFIG_SP0,                                   // SP0, SP1, SP3
  .CB_TxConfigContainer.stsLength               = EN_STS_LENGTH_64_SYMBOLS,                               // STS Length
  .CB_TxConfigContainer.numStsSegments          = EN_NUM_STS_SEGMENTS_1,                                  // Number of STS segments
  .CB_TxConfigContainer.stsKey                  = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
  .CB_TxConfigContainer.stsVUpper               = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
  .CB_TxConfigContainer.stsVCounter             = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
  .CB_TxConfigContainer.macFcsType              = EN_MAC_FCS_TYPE_CRC16,                                  // CRC16
  
   /*Local Container - UWB Rx Config*/
  .CB_RxConfigContainer.prfMode                 = EN_PRF_MODE_BPRF_62P4,                                       // PRF mode selection
  .CB_RxConfigContainer.psduDataRate            = EN_PSDU_DATA_RATE_6P81,                                 // PSDU data rate
  .CB_RxConfigContainer.bprfPhrDataRate         = EN_BPRF_PHR_DATA_RATE_0P85,                             // BPRF PHR data rate
  .CB_RxConfigContainer.preambleCodeIndex       = EN_UWB_PREAMBLE_CODE_IDX_9,                             // Preamble code index (9-32)
  .CB_RxConfigContainer.preambleDuration        = EN_PREAMBLE_DURATION_64_SYMBOLS,                        // Preamble duration (0-1)
  .CB_RxConfigContainer.sfdId                   = EN_UWB_SFD_ID_2,                                        // SFD identifier (0-4)
  .CB_RxConfigContainer.phrRangingBit           = 0x00,                                                   // PHR Ranging Bit (0-1)
  .CB_RxConfigContainer.rframeConfig            = EN_RFRAME_CONFIG_SP0,                                   // SP0, SP1, SP3
  .CB_RxConfigContainer.stsLength               = EN_STS_LENGTH_64_SYMBOLS,                               // STS Length
  .CB_RxConfigContainer.numStsSegments          = EN_NUM_STS_SEGMENTS_1,                                  // Number of STS segments
  .CB_RxConfigContainer.stsKey                  = {0x14EB220FUL,0xF86050A8UL,0xD1D336AAUL,0x14148674UL},  // PhyHrpUwbStsKey
  .CB_RxConfigContainer.stsVUpper               = {0xD37EC3CAUL,0xC44FA8FBUL,0x362EEB34UL},               // PhyHrpUwbStsVUpper96
  .CB_RxConfigContainer.stsVCounter             = 0x1F9A3DE4UL,                                           // PhyHrpUwbStsVCounter
  .CB_RxConfigContainer.macFcsType              = EN_MAC_FCS_TYPE_CRC16,                                  // CRC16
};
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void cb_rc_calibration_callback_handler(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Initializes the UWB RAM for transmission and reception.
 *
 * This function initializes the Ultra-Wideband (UWB) RAM by setting up the transmission (Tx)
 * and reception (Rx) RAM addresses and sizes. It prepares the memory by calling the
 * `CB_SYSTEM_UwbSysRamInit` function with the specified arguments. Once the addresses and sizes
 * are initialized, it clears the targeted memory regions for both transmission and reception
 * using `cb_system_uwb_tx_memclr` and `cb_system_uwb_rx_memclr` respectively.
 *
 * @param pTxRamAddr Pointer to the base address of the Tx RAM.
 * @param pRxRamAddr Pointer to the base address of the Rx RAM.
 * @param TxRamSize Size of the Tx RAM.
 * @param RxRamSize Size of the Rx RAM.
 */
void cb_system_uwb_ram_init(void* pTxRamAddr, void* pRxRamAddr, uint32_t TxRamSize, uint32_t RxRamSize)
{
    uint32_t args[4] = { 0,0,0,0 };

    args[0] = (uint32_t)pTxRamAddr;
    args[1] = (uint32_t)pRxRamAddr;
    args[2] = TxRamSize;
    args[3] = RxRamSize;

    cb_uwbdriver_uwb_system_ram_init(args);  /*Init pointer*/

    /*Once address pointer & size initialized, we clear the targeted memory.*/
    cb_system_uwb_tx_memclr();
    cb_system_uwb_rx_memclr();
}

/**
 * @brief Initialize the UWB communication transmitter.
 *
 * This function initializes the UWB communication transmitter by performing various configuration steps.
 * It sets up the required parameters and resources for UWB communication.
 */
void cb_system_uwb_init(void)
{
  cb_uwbdriver_uwb_init(&s_Local_UwbAllConfigContainer.CB_SystemConfigContainer);
}

/**
 * @brief Turns off the UWB (Ultra-Wideband) system.
 * 
 * This function disables the UWB system by calling the corresponding
 * driver function to ensure proper shutdown of UWB functionality.
 */
void cb_system_uwb_off(void)
{
  cb_uwbdriver_uwb_off();
}

/**
 * @brief Initializes the UWB RX (Receiver) top module.
 * 
 * This function sets up the UWB RX top module by invoking the appropriate
 * driver initialization function. It prepares the system for UWB reception.
 */
void cb_system_uwb_rx_top_init(void)
{
  cb_uwbdriver_rx_top_init();
}

/**
 * @brief Initialize the UWB communication for TX and RX
 *
 */
void cb_system_uwb_trx_init(void)
{
  cb_uwbdriver_trx_init(); 
}

/**
 * @brief Configures the UWB transmitter with transmission payload settings.
 *
 * This function configures the Ultra-Wideband (UWB) transmitter by:
 * - Setting up packet type configuration including PRF mode, PSDU data rate
 * - Configuring preamble code index and SFD ID from packet config
 * - Setting preamble duration and STS parameters
 * - Configuring MAC FCS type
 * - Preparing and loading the transmission payload
 * - Configuring PHR and PSDU parameters
 *
 * @param config Pointer to packet configuration structure containing UWB parameters
 * @param txPayload Pointer to transmission payload structure containing:
 *              - ptrAddress: Pointer to payload data buffer
 *              - payloadSize: Size of payload in bytes
 * @param stTxIrqEnable Pointer to TX IRQ enable configuration structure
 *
 * @note This function must be called before starting a UWB transmission
 */
void cb_system_uwb_config_tx(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_txpayload_st* txPayload, cb_uwbsystem_tx_irqenable_st* stTxIrqEnable)
{
  /*Copy Configure Parameter to Local Data Container */
  memcpy(&s_Local_UwbAllConfigContainer.CB_TxConfigContainer,config,sizeof (cb_uwbsystem_packetconfig_st));

  /*System Configuration*/
  cb_system_uwb_tx_memclr();
  cb_system_uwb_configure_tx_irq(stTxIrqEnable); 
  cb_uwbdriver_configure_tx_timestamp_capture();
  cb_uwbdriver_configure_tx_power(s_Local_UwbAllConfigContainer.CB_SystemConfigContainer.powerCode_tx);

  /*Packet Type Configuration*/
  cb_uwbdriver_configure_prf_mode_psdu_data_rate(&s_Local_UwbAllConfigContainer.CB_TxConfigContainer, EN_UWB_CONFIG_TX); //Note: Load Up Setting Template here
  cb_uwbdriver_configure_preamble_code_index(&s_Local_UwbAllConfigContainer.CB_TxConfigContainer, EN_UWB_CONFIG_TX);
  cb_uwbdriver_configure_sfd_id(&s_Local_UwbAllConfigContainer.CB_TxConfigContainer, EN_UWB_CONFIG_TX);
  cb_uwbdriver_configure_preamble_duration(&s_Local_UwbAllConfigContainer.CB_TxConfigContainer, EN_UWB_CONFIG_TX);
  cb_uwbdriver_configure_sts(&s_Local_UwbAllConfigContainer.CB_TxConfigContainer, EN_UWB_CONFIG_TX);
  cb_uwbdriver_configure_mac_fcs_type(&s_Local_UwbAllConfigContainer.CB_TxConfigContainer, EN_UWB_CONFIG_TX);

  /*Payload Configuration*/
  cb_system_uwb_tx_prepare_payload(txPayload->ptrAddress, txPayload->payloadSize);
  cb_uwbdriver_configure_tx_phr_psdu(&s_Local_UwbAllConfigContainer.CB_TxConfigContainer, txPayload);   
}

/**
 * @brief Configures the UWB receiver with packet detection settings.
 *
 * This function configures the Ultra-Wideband (UWB) receiver by:
 * - Setting up packet type configuration including PRF mode, PSDU data rate
 * - Configuring preamble code index and SFD ID from packet config
 * - Setting preamble duration and STS parameters 
 * - Configuring MAC FCS type for reception
 * - Setting up packet detection parameters
 * - Configuring RX IRQ settings
 * - Setting up RX timestamp capture
 * - Configuring RX operation mode
 * - Setting up CFO bypass parameters
 *
 * @param config Pointer to packet configuration structure containing UWB parameters
 * @param stRxIrqEnable Pointer to RX IRQ enable configuration structure
 * @param stBypass_cfo Pointer to CFO bypass configuration structure
 *
 * @note This function must be called before starting UWB packet reception and  enabled exclusively during Production Test Mode
 */
void cb_system_uwb_config_ftm_rx(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable, cb_uwbsystem_rx_dbb_cfo_st* stBypass_cfo)
{
  /*Copy Configure Parameter to Local Data Container */
  memcpy(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer,config,sizeof (cb_uwbsystem_packetconfig_st));
  
  /*System Configuration*/
// cb_system_uwb_rx_memclr(); 
  cb_system_uwb_configure_rx_irq(stRxIrqEnable);
  cb_uwbdriver_configure_rx_timestamp_capture();
  cb_system_uwb_configure_rx_operation_mode(s_Local_UwbAllConfigContainer.CB_SystemConfigContainer.operationMode_rx);
  
  /*Packet Type Configuration*/
  cb_uwbdriver_configure_prf_mode_psdu_data_rate(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX); //Note: Load Up Setting Template here
  cb_uwbdriver_configure_preamble_code_index(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
  cb_uwbdriver_configure_sfd_id(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
  cb_uwbdriver_configure_preamble_duration(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
  cb_uwbdriver_configure_sts(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
  cb_uwbdriver_configure_mac_fcs_type(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
    
  cb_uwbdriver_configure_fixed_cfo_value(stBypass_cfo->enableBypass, stBypass_cfo->cfoValue);
}


/**
 * @brief Configures the UWB receiver with packet detection settings.
 *
 * This function configures the Ultra-Wideband (UWB) receiver by:
 * - Setting up packet type configuration including PRF mode, PSDU data rate
 * - Configuring preamble code index and SFD ID from packet config
 * - Setting preamble duration and STS parameters 
 * - Configuring MAC FCS type for reception
 * - Setting up packet detection parameters
 * - Configuring RX IRQ settings
 * - Setting up RX timestamp capture
 * - Configuring RX operation mode
 * - Setting up CFO bypass parameters
 *
 * @param config Pointer to packet configuration structure containing UWB parameters
 * @param stRxIrqEnable Pointer to RX IRQ enable configuration structure
 * @param stBypass_cfo Pointer to CFO bypass configuration structure
 *
 * @note This function must be called before starting UWB packet reception
 */
void cb_system_uwb_config_rx(cb_uwbsystem_packetconfig_st* config, cb_uwbsystem_rx_irqenable_st* stRxIrqEnable, cb_uwbsystem_rx_dbb_cfo_st* stBypass_cfo)
{
  /*Copy Configure Parameter to Local Data Container */
  memcpy(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer,config,sizeof (cb_uwbsystem_packetconfig_st));
  
  /*System Configuration*/
  cb_system_uwb_rx_memclr();
  cb_system_uwb_configure_rx_irq(stRxIrqEnable);
  cb_uwbdriver_configure_rx_timestamp_capture();
  cb_system_uwb_configure_rx_operation_mode(s_Local_UwbAllConfigContainer.CB_SystemConfigContainer.operationMode_rx);
  
  /*Packet Type Configuration*/
  cb_uwbdriver_configure_prf_mode_psdu_data_rate(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX); //Note: Load Up Setting Template here
  cb_uwbdriver_configure_preamble_code_index(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
  cb_uwbdriver_configure_sfd_id(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
  cb_uwbdriver_configure_preamble_duration(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
  cb_uwbdriver_configure_sts(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
  cb_uwbdriver_configure_mac_fcs_type(&s_Local_UwbAllConfigContainer.CB_RxConfigContainer, EN_UWB_CONFIG_RX);
    
  cb_uwbdriver_configure_fixed_cfo_value(stBypass_cfo->enableBypass, stBypass_cfo->cfoValue);
}

/**
 * @brief Configures the UWB RX operation mode.
 * 
 * @param mode The desired UWB RX operation mode.
 */
void cb_system_uwb_configure_rx_operation_mode(cb_uwbsystem_rxoperationmode_en mode)
{
  if (mode == EN_UWB_RX_OPERATION_MODE_COEXIST)
  {
    cb_system_uwb_set_rx_threshold(0x27101F40);
  }
  else if (mode == EN_UWB_RX_OPERATION_MODE_GENERAL)
  {
    cb_system_uwb_set_rx_threshold(0x17700BB8);
  }
}
/**
 * @brief Starts the UWB (Ultra-Wideband) receiver on the specified port.
 * 
 * This function starts the UWB receiver on the specified port with the given gain settings.
 * It configures the receiver gain parameters and enables reception on the selected port.
 *
 * @param enRxPort The UWB receiver port to start.Can be a single port
 *                 (EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2) or a combination of ports
 *                 (EN_UWB_RX_02, EN_UWB_RX_ALL)
 * @param stBypass_gain Pointer to structure containing gain bypass configuration parameters.
 */
void cb_system_uwb_rx_start(cb_uwbsystem_rxport_en enRxPort, cb_uwbsystem_rx_dbb_gain_st* stBypass_gain)
{
    cb_uwbdriver_rx_start(enRxPort, stBypass_gain); //combine function to test
}


/**
 * @brief Stops the UWB (Ultra-Wideband) receiver on the specified port.
 * 
 * @param enRxPort The UWB receiver port to stop. Can be a single port
 *                 (EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2) or a combination of ports
 *                 (EN_UWB_RX_02, EN_UWB_RX_ALL)
 */
void cb_system_uwb_rx_stop(cb_uwbsystem_rxport_en enRxPort)
{
  cb_uwbdriver_rx_stop(enRxPort);
}

/**
 * @brief Turns off the UWB (Ultra-Wideband) receiver on the specified port.
 * 
 * @param enRxPort The UWB receiver port to turn off.Can be a single port
 *                 (EN_UWB_RX_0, EN_UWB_RX_1, EN_UWB_RX_2) or a combination of ports
 *                 (EN_UWB_RX_02, EN_UWB_RX_ALL)
 */
void cb_system_uwb_rx_off(cb_uwbsystem_rxport_en enRxPort)
{
  cb_uwbdriver_rx_off(enRxPort);
}

/**
 * @brief Configures the UWB transmission interrupts.
 *
 * This function resets the necessary registers before configuring UWB transmission interrupts.
 * It enables or disables interrupts based on the provided parameters.
 *
 * @param irqEnable Pointer to structure containing the UWB interrupt enable flags.
 *                  - txDone: If true, enables the transmission done interrupt.
 *                  - sfdDone: If true, enables the SFD mark interrupt.
 */
void cb_system_uwb_configure_tx_irq(cb_uwbsystem_tx_irqenable_st* irqEnable)
{
  //----------------------------------
  // Reset register before configuring
  //----------------------------------
  cb_uwbdriver_irq_reset_registers();
  NVIC_DisableIRQ(UWB_TX_DONE_IRQn);
  NVIC_DisableIRQ(UWB_TX_SFD_MARK_IRQn);

  //----------------------------------
  // DIG UWB Interrupt Configuration
  //----------------------------------
  if (irqEnable->txDone == CB_TRUE)  { cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_TX_DONE); }   
  if (irqEnable->sfdDone == CB_TRUE) { cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_TX_SFD_MARK); }

  if (irqEnable->txDone == CB_TRUE)  { cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_TX_DONE);     }
  if (irqEnable->sfdDone == CB_TRUE) { cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_TX_SFD_MARK); }

  //----------------------------------
  // CPU Interrupt Configuration
  //----------------------------------
  if (irqEnable->txDone == CB_TRUE)    NVIC_EnableIRQ(UWB_TX_DONE_IRQn);
  if (irqEnable->sfdDone == CB_TRUE)   NVIC_EnableIRQ(UWB_TX_SFD_MARK_IRQn);
}

/**
 * @brief Configures the UWB reception interrupts.
 *
 * This function configures UWB reception interrupts based on the provided parameters.
 * It sets up the necessary masks and enables the corresponding interrupts.
 *
 * @param stRxIrqEnable Pointer to structure containing the UWB reception interrupt enable flags.
 *                    - rx0Done: If true, enables the RX0 done interrupt.
 *                    - rx0PdDone: If true, enables the RX0 preamble detected interrupt.
 *                    - rx0SfdDetDone: If true, enables the RX0 SFD detected interrupt.
 *                    - rx1Done: If true, enables the RX1 done interrupt.
 *                    - rx1PdDone: If true, enables the RX1 preamble detected interrupt.
 *                    - rx1SfdDetDone: If true, enables the RX1 SFD detected interrupt.
 *                    - rx2Done: If true, enables the RX2 done interrupt.
 *                    - rx2PdDone: If true, enables the RX2 preamble detected interrupt.
 *                    - rx2SfdDetDone: If true, enables the RX2 SFD detected interrupt.
 *                    - rxStsCirEnd: If true, enables the RX status CIR end interrupt.
 *                    - rxPhrDetected: If true, enables the RX PHR detected interrupt.
 *                    - rxDone: If true, enables the RX done interrupt.
 */
void cb_system_uwb_configure_rx_irq(cb_uwbsystem_rx_irqenable_st* stRxIrqEnable)
{
  //----------------------------------
  // UWB Interrupt Configuration
  //----------------------------------
  //----------------------------------------------------------------------------------------------------------------
  if (stRxIrqEnable->rx0Done == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX0_DONE);                
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX0_DONE);                   // Enable EVENT IRQ - RX0_Done
      NVIC_EnableIRQ(UWB_RX0_DONE_IRQn);                                        // Enable CPU IRQ   - RX0 Done
  }
  if (stRxIrqEnable->rx0PdDone == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX0_PD_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX0_PD_DONE);                // Enable EVENT IRQ - RX0 PD done
      NVIC_EnableIRQ(UWB_RX0_PD_DONE_IRQn);                                     // Enable CPU IRQ   - RX0 Preamble detected
  }
  if (stRxIrqEnable->rx0SfdDetDone == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX0_SFD_DET_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX0_SFD_DET_DONE);           // Enable EVENT IRQ - RX0 SFD Detected
      NVIC_EnableIRQ(UWB_RX0_SFD_DET_DONE_IRQn);                                // Enable CPU IRQ   - RX0 SFD detected
  }
  //----------------------------------------------------------------------------------------------------------------
  if (stRxIrqEnable->rx1Done == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX1_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX1_DONE);                   // Enable EVENT IRQ - RX1_Done
      NVIC_EnableIRQ(UWB_RX1_DONE_IRQn);                                        // Enable CPU IRQ   - RX1 Done
  }
  if (stRxIrqEnable->rx1PdDone == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX1_PD_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX1_PD_DONE);                // Enable EVENT IRQ - RX1 PD done
      NVIC_EnableIRQ(UWB_RX1_PD_DONE_IRQn);                                     // Enable CPU IRQ   - RX1 Preamble detected
  }
  if (stRxIrqEnable->rx1SfdDetDone == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX1_SFD_DET_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX1_SFD_DET_DONE);           // Enable EVENT IRQ - RX1 SFD Detected        
      NVIC_EnableIRQ(UWB_RX1_SFD_DET_DONE_IRQn);                                // Enable CPU IRQ   - RX1 SFD detected
  }
  //----------------------------------------------------------------------------------------------------------------
  if (stRxIrqEnable->rx2Done == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX2_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX2_DONE);                   // Enable EVENT IRQ - RX2_Done        
      NVIC_EnableIRQ(UWB_RX2_DONE_IRQn);                                        // Enable CPU IRQ   - RX2 Done
  }
  if (stRxIrqEnable->rx2PdDone == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX2_PD_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX2_PD_DONE);                // Enable EVENT IRQ - RX2 PD done        
      NVIC_EnableIRQ(UWB_RX2_PD_DONE_IRQn);                                     // Enable CPU IRQ   - RX2 Preamble detected
  }
  if (stRxIrqEnable->rx2SfdDetDone == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX2_SFD_DET_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX2_SFD_DET_DONE);           // Enable EVENT IRQ - RX2 SFD Detected   
      NVIC_EnableIRQ(UWB_RX2_SFD_DET_DONE_IRQn);                                // Enable CPU IRQ   - RX2 SFD detected
  }
  //----------------------------------------------------------------------------------------------------------------
  if (stRxIrqEnable->rxStsCirEnd == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX_STS_CIR_END);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX_STS_CIR_END);             // Enable EVENT IRQ - RX STS END
      NVIC_EnableIRQ(UWB_RX_STS_CIR_END_IRQn);                                  // Enable CPU IRQ   - RX STS END
  }
  if (stRxIrqEnable->rxPhrDetected == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX_PHY_PHR);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX_PHY_PHR);                 // Enable EVENT IRQ - RX PHR Detected        
      NVIC_EnableIRQ(UWB_RX_PHR_DETECTED_IRQn);                                 // Enable CPU IRQ   - RX PHR Detected

  }
  if (stRxIrqEnable->rxDone == CB_TRUE)
  {
      cb_uwbdriver_irq_mask_configuration(EN_UWB_IRQ_EVENT_RX_DONE);
      cb_uwbdriver_enable_event_irq(EN_UWB_IRQ_EVENT_RX_DONE);                    // Enable EVENT IRQ   - RX DONE
      NVIC_EnableIRQ(UWB_RX_DONE_IRQn);                                         // Enable CPU IRQ     - RX DONE
  }
  //----------------------------------------------------------------------------------------------------------------
}

/**
 * @brief Initialize the UWB communication for TX
 *
 */
void cb_system_uwb_tx_init(void)
{
  cb_uwbdriver_tx_init();
}

/**
 * @brief Initialize the UWB communication for RX
 *
 */
void cb_system_uwb_rx_init(cb_uwbsystem_rxport_en enRxPort)
{
    switch (enRxPort)
    {
        case EN_UWB_RX_0:   cb_uwbdriver_rx0_init();     break;   // RX0 INIT
        case EN_UWB_RX_1:   cb_uwbdriver_rx1_init();     break;   // RX1 INIT
        case EN_UWB_RX_2:   cb_uwbdriver_rx2_init();     break;   // RX2 INIT
        case EN_UWB_RX_02:  cb_uwbdriver_rx02_init();     break;  // RX02 INIT
        case EN_UWB_RX_ALL: cb_uwbdriver_rx_all_init();   break;  // RXALL INIT
    }
}

/**
 * @brief Start the UWB communication for TX
 *
 */
void cb_system_uwb_tx_start(void)
{
  cb_uwbdriver_tx_start();
}

/**
 * @brief Start the UWB communication for TX (For deferred TX)
 *
 */
void cb_system_uwb_tx_start_prepare(void)
{
  cb_uwbdriver_tx_start_prepare();
}

/**
 * @brief Start the UWB communication for RX0 (For deferred RX)
 *
 */
void cb_system_uwb_rx_start_prepare(void)
{
  cb_uwbdriver_rx_start_prepare();
}

/**
 * @brief Restart the UWB communication for TX
 *
 */
void cb_system_uwb_stage_tx_start(void)
{
  cb_uwbdriver_stage_tx_start();
}

/**
 * @brief Stop the UWB communication for TX
 *
 */
void cb_system_uwb_tx_stop(void)
{
  cb_uwbdriver_tx_stop();
}

/**
 * @brief Turns off the UWB transceiver.
 */
void cb_system_uwb_trx_off(void)
{
  cb_system_uwb_rx_off(EN_UWB_RX_0);
  cb_system_uwb_tx_off();
}

/**
 * @brief Turns off the UWB receiver top.
 */
void cb_system_uwb_rx_top_off (void)
{
  cb_uwbdriver_rx_top_off();
}

/**
 * @brief Turns off the UWB transmitter.
 */
void cb_system_uwb_tx_off(void)
{
  cb_uwbdriver_tx_off();
}

/**
 * @brief Freezes the UWB transmitter PLL.
 * 
 * This function freezes the Phase-Locked Loop (PLL) of the UWB transmitter,
 * which can be useful for maintaining stable frequency reference during
 * specific operations or power management scenarios.
 */
void cb_system_uwb_tx_freeze_pll(void)
{
  cb_uwbdriver_tx_freezepll();
}

/**
 * @brief Unfreezes the UWB transmitter PLL.
 * 
 * This function unfreezes the Phase-Locked Loop (PLL) of the UWB transmitter,
 * allowing it to resume normal operation and frequency tracking after being
 * previously frozen.
 */
void cb_system_uwb_tx_unfreeze_pll(void)
{
  cb_uwbdriver_tx_unfreezepll();
}

/**
 * @brief Delay in milliseconds(ms)
 */
void cb_system_delay_in_ms(uint32_t milliseconds)
{
  cb_hal_delay_in_ms(milliseconds);
}

/**
 * @brief Delay in microseconds(us)
 */
void cb_system_delay_in_us(uint32_t microseconds)
{
  cb_hal_delay_in_us(microseconds);
}

/**
 * @brief Prepare Payload/PSDU for UWB Tx Phy
 */
void cb_system_uwb_tx_prepare_payload(uint8_t* pTxpayloadAddress, uint16_t SizeInByte)
{
  memcpy(cb_uwbdriver_get_uwb_tx_memory_start_addr(), pTxpayloadAddress, SizeInByte);
}

/**
 * @brief Clear UWB Tx PSDU Memory
 */
void cb_system_uwb_tx_memclr(void)
{
  memset(cb_uwbdriver_get_uwb_tx_memory_start_addr(), 0x00, cb_uwbdriver_get_uwb_tx_memory_size());
}

/**
 * @brief Clear UWB Rx PSDU Memory
 */
void cb_system_uwb_rx_memclr(void)
{ 
  memset(cb_uwbdriver_get_uwb_rx_memory_start_addr(), 0x00, cb_uwbdriver_get_uwb_rx_memory_size());
}

/**
 * @brief Read UWB Rx Payload/PSDU Content
 */
void cb_system_uwb_rx_get_payload(uint8_t* pRxpayloadAddress, uint16_t SizeInByte)
{   
  memcpy(pRxpayloadAddress, cb_uwbdriver_get_uwb_rx_memory_start_addr(), SizeInByte);
}

/**
 * @brief Retrieves the Packet Header (PHR) status.
 *
 * This function reads the packet status from the RX0 registers and extracts
 * the PHR status information including the PHR security and dedicated
 * indicators as well as the RX0 packet status.
 *
 * @return cb_uwbsystem_rx_phrstatus_st A structure containing the PHR status flags:
 *         - phrSec: Indicates if the PHR is secure (1) or not (0).
 *         - phrDed: Indicates if the PHR is dedicated (1) or not (0).
 *         - rx0Ok:  Indicates if the RX0 packet is okay (1) or not (0).
 */
cb_uwbsystem_rx_phrstatus_st cb_system_uwb_get_rx_phr_status(void)
{
  cb_uwbsystem_rxstatus_un rx0PacketStatus = cb_system_uwb_get_rx_status();

  cb_uwbsystem_rx_phrstatus_st phrStatus;
  phrStatus.phrSec = rx0PacketStatus.phr_sec; 
  phrStatus.phrDed = rx0PacketStatus.phr_ded;
  phrStatus.rx0Ok  = rx0PacketStatus.rx0_ok;
  return phrStatus;
}


/**
 * @brief Sets the UWB threshold value.
 *
 * This function sets the global UWB threshold value to the specified
 * parameter. This value can be used for various threshold checks
 * within the UWB system.
 *
 * @param threshold The threshold value to be set for the UWB system.
 */
void cb_system_uwb_set_rx_threshold(uint32_t threshold)
{
  cb_uwbdriver_set_rx_threshold(threshold);
}

/**
 * @brief Initializes the receiver gain setting.
 *
 * This function sets the initial gain value for the receiver. The gain
 * value is limited to a maximum of 7 to ensure it stays within the
 * allowable range. If the provided gain exceeds this maximum, it is
 * capped to 7.
 *
 * @param gainRxInit The initial gain value for the receiver (0 to 7).
 */
void cb_system_uwb_set_gain_rx_init(uint32_t gainRxInit)
{
  cb_uwbdriver_set_gain_rx_init(gainRxInit);
}

/**
 * @brief Gets the TX RF PLL lock status
 *
 * @return The lock status of the TX RF PLL
 */
uint32_t cb_system_uwb_get_tx_rf_pll_lock(void)
{
  return cb_uwbdriver_get_tx_rfpll_lock();
}

/**
 * @brief Retrieves the current temperature of the chip.
 *
 * This function reads the chip's temperature
 *
 * @return The current temperature of the chip in degrees Celsius.
 */
float cb_system_get_chip_temperature(void)
{
  return cb_uwbdriver_get_chip_temp();
}

/**
 * @brief Reads ADC voltage with specified gain stage.
 * 
 * This function provides an application layer interface to read the auxiliary 
 * ADC voltage value using the specified gain stage. It wraps the lower-level
 * driver function and provides the same input validation and functionality.
 * 
 * Gain Stage Voltage Ranges:
 * | Gain Stage | Voltage Range (V) |
 * |------------|-------------------|
 * |     0      |   0.0 to 3.3      |
 * |     1      |   0.0 to 2.5      |
 * |     2      |   0.0 to 1.8      |
 * |     3      |   0.0 to 1.5      |
 * |     4      |   0.0 to 1.2      |
 * |     5      |   0.0 to 0.9      |
 * 
 * @param gain_stage ADC gain stage setting (valid range: 0-5).
 *                   Invalid values (>5) will cause the function to return 0.
 * 
 * @return float The ADC voltage reading value. Returns 0.0 for invalid gain_stage.
 * 
 * @note If an invalid gain_stage (>5) is provided, the function will return 0.0
 *       without performing any ADC operation.
 */
float cb_system_adc_read_AIN_voltage(uint8_t gain_stage)
{
  return cb_adc_read_AIN_voltage(gain_stage);
}

/**
 * @brief Perform a one-time calibration of the RC clock.
 *
 * This function calibrates the drift time of the RC clock
 * to improve timing accuracy. The calibration process adjusts
 * the RC clock to compensate for environmental or operational
 * factors that might affect its frequency stability.
 * @return CB_PASS or CB_FAIL. Fail if calibration is in-progress.
 */
CB_STATUS cb_system_rc_calibration(void)
{  
  if (enRCCalMode != EN_IDLE)
  {
    return CB_FAIL;
  }
  enRCCalMode = EN_ONCE;
  sRCCalibrationTimeInMs = DEF_RC_CALIBRATION_DEFAULTTIME_IN_MS;
  
  // cb_scr_stabilize_rc();//make sure RC clock is stabilize.
  
  /*Run the RC Calibration value once. Value to be update after DEF_RC_CALIBRATION_DEFAULTTIME_IN_MS in NMI Handler*/
#if (DEF_RC_CAL_USEIRQ == DEF_RC_CAL_USEIRQ_WDT)
  stWdtConfig config = {
    .WdtMode = EN_WDT_INTERVAL,
    .WdtRunInHalt = 0,
    .WdtRunInSleep = 0,
    .Interval = DEF_RC_CALIBRATION_DEFAULTTIME_IN_MS,
    .GraceAfterInt = 100,
  };
  // Configure and start the watchdog
  cb_wdt_init(&config);
  sRC_lastCPUCount = DWT->CYCCNT;
  cb_wdt_enable();
  cb_wdt_nmi_rc_irq_callback(cb_rc_calibration_callback_handler);

#elif(DEF_RC_CAL_USEIRQ == DEF_RC_CAL_USEIRQ_TIMER)

  stWdtConfig config = {
    .WdtMode = EN_WDT_INTERVAL,
    .WdtRunInHalt = 0,
    .WdtRunInSleep = 0,
    .Interval = DEF_RC_CALIBRATION_DEFAULTTIME_IN_MS*2,
    .GraceAfterInt = 100,
  };
  // Configure and start the watchdog
  cb_wdt_init(&config);
  cb_wdt_enable();
  cb_wdt_irq_config(0);

  cb_scr_timer3_module_on();

  // Disable Timer's module interrupt
  cb_timer_disable_interrupt();

  // Enable CPU Timer 0,1,2,3 Interrupt
  NVIC_EnableIRQ(TIMER_3_IRQn);	
  
  // Configure TIMER0-EVENT0 in FreeRun mode that timeout every 1000ms.
  stTimerSetUp stTimerSetup;
  stTimerSetup.Timer 		 						 		   = EN_TIMER_3;
  stTimerSetup.TimerMode 						 		   = EN_TIMER_MODE_FREERUN;
  stTimerSetup.TimeUnit                    = EN_TIMER_MS;
  stTimerSetup.stTimeOut.timeoutVal[0]     = DEF_RC_CALIBRATION_DEFAULTTIME_IN_MS;
  stTimerSetup.stTimeOut.timeoutVal[1]     = 0;
  stTimerSetup.stTimeOut.timeoutVal[2]     = 0;
  stTimerSetup.stTimeOut.timeoutVal[3]     = 0;  
  stTimerSetup.stTimeOut.TimerTimeoutEvent = DEF_TIMER_TIMEOUT_EVENT_0;
  stTimerSetup.TimerEvtComMode 						 = EN_TIMER_EVTCOM_MODE_00;
  stTimerSetup.AutoStartTimer							 = EN_START_TIMER_ENABLE;
  stTimerSetup.TimerEvtComEnable					 = EN_TIMER_EVTCOM_DISABLE;
  stTimerSetup.TimerInterrupt 					   = EN_TIMER_INTERUPT_ENABLE;
  stTimerSetup.stPulseWidth.NumberOfCycles = 0;
  stTimerSetup.stPulseWidth.TimerPulseWidth= EN_TIMER_PULSEWIDTH_DISABLE;

  cb_timer_3_rc_irq_callback(cb_rc_calibration_callback_handler);

  cb_wdt_refresh();
  cb_timer_configure_timer(&stTimerSetup);
#else
#error "BLE Unknown RC IRQ Selection.".  
#endif
  return CB_PASS;
}
/**
 * @brief Start periodic RC clock calibration with a specified interval.
 *
 * This function enables periodic calibration of the RC clock
 * The periodic calibration helps to continuously compensate for
 * RC clock drift, ensuring accurate timing over extended periods.
 * This is especially useful in systems where timing precision is
 * critical, and the RC clock drift may vary due to environmental
 * or operational changes.
 *
 * @param PeriodicIntervalInMs The interval time, in milliseconds
 * @return CB_PASS or CB_FAIL. Fail if calibration is in-progress.
 */
CB_STATUS cb_system_start_periodic_rc_calibration(uint32_t PeriodicIntervalInMs)
{
  if (enRCCalMode != EN_IDLE)
  {
    return CB_FAIL;
  }
  enRCCalMode = EN_PERIODIC;
  sRCCalibrationTimeInMs = PeriodicIntervalInMs;
  
  // cb_scr_stabilize_rc();//make sure RC clock is stabilize.

#if (DEF_RC_CAL_USEIRQ == DEF_RC_CAL_USEIRQ_WDT)
  stWdtConfig config = {
    .WdtMode = EN_WDT_INTERVAL,
    .WdtRunInHalt = 0,
    .WdtRunInSleep = 0,
    .Interval = PeriodicIntervalInMs,
    .GraceAfterInt = 100,
  };
  // Configure and start the watchdog
  cb_wdt_init(&config);
  sRC_lastCPUCount = DWT->CYCCNT;
  cb_wdt_enable();
  cb_wdt_nmi_rc_irq_callback(cb_rc_calibration_callback_handler);

#elif(DEF_RC_CAL_USEIRQ == DEF_RC_CAL_USEIRQ_TIMER)

  stWdtConfig config = {
    .WdtMode = EN_WDT_INTERVAL,
    .WdtRunInHalt = 0,
    .WdtRunInSleep = 0,
    .Interval = PeriodicIntervalInMs*2,
    .GraceAfterInt = 100,
  };
  // Configure and start the watchdog
  cb_wdt_init(&config);
  cb_wdt_enable();
  cb_wdt_irq_config(0);

  cb_scr_timer3_module_on();

  // Disable Timer's module interrupt
  cb_timer_disable_interrupt();

  // Enable CPU Timer 0,1,2,3 Interrupt
  NVIC_EnableIRQ(TIMER_3_IRQn);	
  
  // Configure TIMER0-EVENT0 in FreeRun mode that timeout every 1000ms.
  stTimerSetUp stTimerSetup;
  stTimerSetup.Timer 		 						 		   = EN_TIMER_3;
  stTimerSetup.TimerMode 						 		   = EN_TIMER_MODE_FREERUN;
  stTimerSetup.TimeUnit                    = EN_TIMER_MS;
  stTimerSetup.stTimeOut.timeoutVal[0]     = PeriodicIntervalInMs;
  stTimerSetup.stTimeOut.timeoutVal[1]     = 0;
  stTimerSetup.stTimeOut.timeoutVal[2]     = 0;
  stTimerSetup.stTimeOut.timeoutVal[3]     = 0;      
  stTimerSetup.stTimeOut.TimerTimeoutEvent = DEF_TIMER_TIMEOUT_EVENT_0;
  stTimerSetup.TimerEvtComMode 						 = EN_TIMER_EVTCOM_MODE_00;
  stTimerSetup.AutoStartTimer							 = EN_START_TIMER_ENABLE;
  stTimerSetup.TimerEvtComEnable					 = EN_TIMER_EVTCOM_DISABLE;
  stTimerSetup.TimerInterrupt 					   = EN_TIMER_INTERUPT_ENABLE;
  stTimerSetup.stPulseWidth.NumberOfCycles = 0;
  stTimerSetup.stPulseWidth.TimerPulseWidth= EN_TIMER_PULSEWIDTH_DISABLE;

  cb_timer_3_rc_irq_callback(cb_rc_calibration_callback_handler);

  cb_wdt_refresh();
  cb_timer_configure_timer(&stTimerSetup);
#else
#error "BLE Unknown RC IRQ Selection.".  
#endif
  return CB_PASS;
}

/**
 * @brief Stop RC clock calibration.
 *
 * This function disables the calibration process of the RC
 * clock. It can be used when periodic calibration is no longer
 * required if the timing precision is not critical.
 */
void cb_system_stop_rc_calibration(void)
{
  cb_wdt_disable();
  if (enRCCalMode != EN_IDLE)
  {
    #if (DEF_RC_CAL_USEIRQ == DEF_RC_CAL_USEIRQ_WDT)
    cb_wdt_nmi_clear_irq_handler();
    #elif(DEF_RC_CAL_USEIRQ == DEF_RC_CAL_USEIRQ_TIMER)
    cb_timer_disable_timer((enTimer)EN_TIMER_3);
    cb_timer_3_clear_irq_handler(); 
    #endif
  }
  enRCCalMode = EN_IDLE;
}

/**
 * @brief Callback handler for RC clock calibration calculations.
 *
 * This function is triggered as part of the RC calibration process.
 * It handles the calculations required to adjust the RC clock's
 * timing parameters, compensating for drift and ensuring accuracy.
 */
void cb_rc_calibration_callback_handler(void) 
{
#if (DEF_RC_CAL_USEIRQ == DEF_RC_CAL_USEIRQ_WDT)
  uint32_t delta_CPUCount = (DWT->CYCCNT < sRC_lastCPUCount) ? 
                             (0xFFFFFFFF - sRC_lastCPUCount + DWT->CYCCNT + 1) : DWT->CYCCNT - sRC_lastCPUCount;

  if (delta_CPUCount > (SystemCoreClock/100))//10ms at least.
  {
    sRC_lastCPUCount = DWT->CYCCNT;
    float NewCalVal = delta_CPUCount/(SystemCoreClock*((double)sRCCalibrationTimeInMs/1000.0));
    if (NewCalVal >= 0.5f && NewCalVal <= 1.5f)//ratio boundary check
    {
      RC_CompensateRatio = NewCalVal; //only update the value within this boundary check
    }
  }
#elif(DEF_RC_CAL_USEIRQ == DEF_RC_CAL_USEIRQ_TIMER)
  uint32_t delta_WDTTick = (((32*sRCCalibrationTimeInMs)*2)-cb_wdt_get_current_count());
  cb_wdt_refresh();
  float NewCalVal = 1/(delta_WDTTick/(double)(sRCCalibrationTimeInMs*32));
  if (NewCalVal >= 0.5f && NewCalVal <= 1.5f)//ratio boundary check
  {
    RC_CompensateRatio = NewCalVal; //only update the value within this boundary check
  }
#endif
  if (enRCCalMode == EN_ONCE)
  {
    cb_system_stop_rc_calibration();
  }
  
}

/**
 * @brief Get the transmission power code from the uwb system configuration.
 * 
 * This function returns the current transmission power code as stored in
 * the system configuration.
 * 
 * @return uint8_t The uwb transmission power code.
 */
uint8_t cb_system_uwb_get_tx_power_code(void)
{
  return s_Local_UwbAllConfigContainer.CB_SystemConfigContainer.powerCode_tx;
}

/**
 * @brief Sets the UWB transmission power code.
 * 
 * @param TxPowerCode The transmission power code to be set.
 */
void cb_system_uwb_set_tx_power_code(uint8_t TxPowerCode)
{
  s_Local_UwbAllConfigContainer.CB_SystemConfigContainer.powerCode_tx = TxPowerCode;
}

/**
 * @brief Retrieves the current UWB receiver operation mode.
 * 
 * @return The current UWB receiver operation mode as an cb_uwbsystem_rxoperationmode_en enum.
 */
cb_uwbsystem_rxoperationmode_en cb_system_uwb_get_rx_opmode(void) 
{
  return s_Local_UwbAllConfigContainer.CB_SystemConfigContainer.operationMode_rx;
}

/**
 * @brief Sets the UWB receiver operation mode.
 * 
 * @param RxOperationMode The receiver operation mode to be set, as an cb_uwbsystem_rxoperationmode_en enum.
 */
void cb_system_uwb_set_rx_opmode(cb_uwbsystem_rxoperationmode_en RxOperationMode)
{
  s_Local_UwbAllConfigContainer.CB_SystemConfigContainer.operationMode_rx = RxOperationMode;
}

/**
 * @brief Retrieves the current UWB system configuration.
 * 
 * @return The current UWB system configuration as an cb_uwbsystem_systemconfig_st structure.
 */
cb_uwbsystem_systemconfig_st cb_system_uwb_get_system_config(void) 
{
  /*Return a copy of the structure*/
  return s_Local_UwbAllConfigContainer.CB_SystemConfigContainer;  
}

/**
 * @brief Sets the UWB system configuration.
 * 
 * @param newConfig Pointer to the new UWB system configuration to be set.
 */
void cb_system_uwb_set_system_config(cb_uwbsystem_systemconfig_st* newConfig) 
{
  /*Assign the entire structure*/
  /*Copy Configure Parameter to Local Data Container */
  memcpy(&s_Local_UwbAllConfigContainer.CB_SystemConfigContainer,newConfig,sizeof (cb_uwbsystem_systemconfig_st));
}

//-----------------------------------------------------------
// SYSTEM_CORE Related functions
//-----------------------------------------------------------
/**
 * @brief Initializes the chip system, including reading the chip ID, 
 *        initializing PMU, setting RF parameters, and configuring voltage 
 *        and temperature sensing.
 * 
 * This function performs several key chip initialization tasks:
 *  - Reads the chip ID and checks the TO (Test Operations) version.
 *  - Initializes the PMU (Power Management Unit) based on the TO version.
 *  - Sets the RF (Radio Frequency) parameters.
 *  - Configures voltage and temperature sensing, including temperature compensation.
 */
void cb_system_chip_init(void)
{
  cb_uwbdriver_chip_init();
}

/**
 * @brief Gets the received packet's Packet Header (PHR).
 *
 * This function retrieves the PHR from the received packet by reading
 * the relevant bits from the RX packet status register.
 *
 * @return uint32_t The value of the received packet's PHR.
 */
uint32_t cb_system_uwb_get_rx_packet_phr(void)
{
  return cb_uwbdriver_get_rx_packet_phr();
}

/**
 * @brief Get the received packet's PHR (Physical Layer Header).
 *
 * This function retrieves the Physical Layer Header (PHR) from the received packet, which
 * contains information about the packet's size and type.
 *
 * @return The received packet's PHR value.
 */
uint16_t cb_system_uwb_get_rx_packet_size(cb_uwbsystem_packetconfig_st* config)
{
  return cb_uwbdriver_get_rx_packet_size(config);
}

/**
 * @brief Get the Rx PHR Ranging bit infomation.
 *
 * This function retrive PHR Ranging bit infomation of the received packet
 *
 * @param  config Pointer to UWB configuration parameter structure 
 * @return Rx PHR Ranging bit.
 */
uint8_t cb_system_uwb_get_rx_phr_ranging_bit(cb_uwbsystem_packetconfig_st* config)
{
  return cb_uwbdriver_get_rx_phr_ranging_bit(config);
}

/**
 * @brief Retrieve the TX timestamps.
 *
 * This function retrieves the timestamps related to the transmission start, 
 * SFD mark, STS1 mark, STS2 mark, and transmission completion. These timestamps 
 * are read from specific memory-mapped registers and stored in the provided 
 * `cb_uwbsystem_tx_timestamp_st` structure.
 *
 * The `cb_uwbsystem_tx_timestamp_st` structure contains the following members:
 * - `txStart`: TX start timest

amp.
 * - `sfdMark`: SFD mark timestamp.
 * - `sts1Mark`: STS 1 mark timestamp.
 * - `sts2Mark`: STS 2 mark timestamp.
 * - `txDone`: TX done timestamp.
 *
 * @param txTimestamp Pointer to a `cb_uwbsystem_tx_timestamp_st` structure where the 
 *                    retrieved timestamps will be stored.
 */
void cb_system_uwb_get_tx_raw_timestamp(cb_uwbsystem_tx_timestamp_st* txTimestamp)
{
  cb_uwbdriver_get_tx_raw_timestamp(txTimestamp);
}

/**
 * @brief Retrieves the transmitter TSU (Timestamp Unit) timestamp.
 *
 * This function reads the transmitter timestamp from the hardware register, processes it to remove the lower
 * 2 bits, and then converts it into integer and fractional parts. The calculated timestamp is stored in the 
 * provided `cb_uwbsystem_tx_tsutimestamp_st` structure.
 *
 * @param[out] p_txTsuTimestamp  Pointer to a `cb_uwbsystem_tx_tsutimestamp_st` structure where the retrieved timestamp will 
 *                                be stored. The structure will be updated with the integer and fractional 
 *                                components of the timestamp.
 */
void cb_system_uwb_get_tx_tsu_timestamp(cb_uwbsystem_tx_tsutimestamp_st * outTxTsu)
{
  cb_uwbdriver_get_tx_tsu_timestamp(outTxTsu);
}

/**
 * @brief Fetches RxTSU data from the hardware registers.
 *
 * This function reads data from specific memory addresses corresponding to various
 * RxTSU events and stores them into the provided data structure.
 *
 * @param rxTsu Pointer to the structure where RxTSU data will be stored.
 */
void cb_system_uwb_get_rx_raw_timestamp(cb_uwbsystem_rx_tsu_st* rxTsu)
{
  cb_uwbdriver_get_rx_raw_timestamp(rxTsu);
}

/**
 * @brief Retrieves the RX TSU timestamp.
 *
 * This function retrieves the timestamp of the RX SFD Mark, which consists of an integer part 
 * and a fractional part. The integer part represents the time in counts of 1/124.8MHz (~8ns), 
 * while the fractional part represents the time in counts of 1/124.8MHz/512 (~15.6ps). 
 * The complete RX TSU timestamp is stored in the provided `cb_uwbsystem_rx_tsutimestamp_st` structure, 
 * which includes:
 * - `rxTsuInt`: The integer part of the RX TSU timestamp, indicating the time in 
 *   counts of 1/124.8MHz.
 * - `rxTsuFrac`: The fractional part of the RX TSU timestamp, indicating the time 
 *   in counts of 1/124.8MHz/512.
 * - `rxTsu`: The complete RX TSU timestamp as a double, combining both the integer 
 *   and fractional parts for higher precision.
 *
 * @param rxTsuTimestamp Pointer to the `cb_uwbsystem_rx_tsutimestamp_st` structure where the retrieved 
 *                      RX TSU timestamp will be stored.
 * @param enRxPort        The RX port identifier used for fetching the RX TSU data.
 */
void cb_system_uwb_get_rx_tsu_timestamp(cb_uwbsystem_rx_tsutimestamp_st* rxTsuTimestamp, cb_uwbsystem_rxport_en enRxPort)
{
  cb_uwbdriver_get_rx_tsu_timestamp(rxTsuTimestamp, enRxPort);
}

void cb_system_uwb_store_rx_tsu_status(cb_uwbsystem_rx_tsustatus_st* p_rxTsuStatus, cb_uwbsystem_rx_tsu_st* p_rxTimeStampData, cb_uwbsystem_rxport_en enRxPort)
{
  cb_uwbdriver_store_rx_tsu_status(p_rxTsuStatus, p_rxTimeStampData, enRxPort);
}

/**
 * @brief Copies CIR register data into the provided array of `cb_uwbsystem_rx_cir_iqdata_st`.
 *
 * @param destArray Pointer to the destination array of `cb_uwbsystem_rx_cir_iqdata_st`.
 * @param enRxPort RX port number (used for address calculation).
 * @param startingPosition Offset within the CIR register memory.
 * @param numSamples Number of `cb_uwbsystem_rx_cir_iqdata_st` samples to copy.
 */
void cb_system_uwb_store_rx_cir_register(cb_uwbsystem_rx_cir_iqdata_st* destArray, cb_uwbsystem_rxport_en enRxPort, uint32_t startingPosition, uint32_t numSamples)
{
  cb_uwbdriver_store_rx_cir_register(destArray, enRxPort, startingPosition, numSamples);
}

cb_uwbalg_poa_outputperpacket_st cb_system_uwb_pdoa_cir_processing(enUwbPdoaCalType calType, uint8_t packageNum, const uint8_t numRxUsed, const cb_uwbsystem_rx_cir_iqdata_st *cirRegisterData, uint16_t cirDataSize)
{
  return cb_uwbalg_pdoa_cir_post_processing(calType, packageNum, numRxUsed, cirRegisterData, cirDataSize);
}


/**
 * @brief Retrieve the CIR quality flag.
 *
 * This function retrieves the quality flag for the Channel Impulse Response (CIR) by
 * first storing the CIR register data and then performing a quality check on the stored
 * data. It calls the `cb_system_uwb_store_rx_cir_register` function to obtain the necessary 
 * register values and subsequently calls `CB_SYSTEM_cir_quality_check` to evaluate 
 * the quality of the CIR.
 *
 * @return The CIR quality flag as a uint8_t, indicating the quality status of the CIR.
 */
uint8_t cb_system_uwb_get_rx_cir_quality_flag(void)
{
  return cb_uwbdriver_get_rx_cir_quality_flag();
}

/**
 * @brief Get the Digital Compensation Offset Correction (DCOC) for a specified receiver port.
 *
 * This function retrieves the Digital Compensation Offset Correction (DCOC) values for 
 * a specified UWB receiver port. It reads the corresponding hardware registers to obtain 
 * the I and Q components of the DCOC, which are then converted from unsigned to signed 
 * integers using two's complement representation.
 *
 * @param enRxPort The UWB receiver port from which to get the DCOC (e.g., EN_UWB_RX_0, 
 *               EN_UWB_RX_1, EN_UWB_RX_2, or EN_UWB_RX_ALL).
 *
 * @return A struct of type `cb_uwbsystem_rx_dcoc_st` containing the DCOC Q and I values.
 */
cb_uwbsystem_rx_dcoc_st cb_system_uwb_get_rx_dcoc(cb_uwbsystem_rxport_en enRxPort)
{
  return cb_uwbdriver_get_rx_dcoc(enRxPort);
}

/**
 * @brief Retrieve the RSSI (Received Signal Strength Indicator) values for specified RX ports.
 *
 * This function reads the RSSI values from the specified UWB RX ports
 * and compensates for gain based on predefined compensation values. 
 * The function returns a structure containing the RSSI results for 
 * each RX port specified in the `rssiRxPorts` parameter.
 *
 * @param rssiRxPorts A bitmask indicating which RX ports to retrieve 
 *                    RSSI values from. Valid values are 
 *                    EN_UWB_RX_0, EN_UWB_RX_1, and EN_UWB_RX_2.
 *
 * @return A structure of type `cb_uwbsystem_rx_signalinfo_st` containing the RSSI 
 *         values, gain indices, CFO estimates, and DCOC results for 
 *         the specified RX ports.
 *
 */
cb_uwbsystem_rx_signalinfo_st cb_system_uwb_get_rx_rssi(uint8_t rssiRxPorts)
{
  return cb_uwbdriver_get_rx_rssi(rssiRxPorts);
}

/**
 * @brief Get the etc status logging
 *
 * This function retrieves the status register value of the etc register status.\
 *
 * @param etcStatus Pointer to output cb_uwbsystem_rx_etc_statusregister_st.
 */
void cb_system_uwb_get_rx_etc_status_register(cb_uwbsystem_rx_etc_statusregister_st* const etcStatus) 
{
  cb_uwbdriver_get_uwb_rx_etc_status_register(etcStatus);
}

/**
 * @brief Get the status register value of the UWB communication module.
 *
 * This function retrieves the status register value of the UWB communication module.
 * It reads the status register and returns the value.
 *
 * @return Status register value.
 */
cb_uwbsystem_rxstatus_un cb_system_uwb_get_rx_status(void)
{
  return cb_uwbdriver_get_uwb_rx_status_register();
}


/**
 * @brief Reads the Cir Ctl Idx value
 *
 * @return CirCtlidx value
*/
uint16_t cb_system_uwb_get_rx_cir_ctl_idx(void)
{
  return cb_uwbdriver_get_rx_cir_ctl_idx();
}

/**
 * @brief Enables the UWB absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to enable.
 */
void cb_system_uwb_abs_timer_on(enUwbAbsoluteTimer enAbsoluteTimer)
{
  cb_uwbdriver_abs_timer_on(enAbsoluteTimer);
}

/**
 * @brief Disables the UWB absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to disable.
 */
void cb_system_uwb_abs_timer_off(enUwbAbsoluteTimer enAbsoluteTimer)
{
  cb_uwbdriver_abs_timer_off(enAbsoluteTimer);
}

/**
 * @brief Clears the internal occurrence of the UWB absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to clear.
 */
void cb_system_uwb_abs_timer_clear_internal_occurence(enUwbAbsoluteTimer enAbsoluteTimer)
{
  cb_uwbdriver_abs_timer_clear_internal_occurence (enAbsoluteTimer);
}

/**
 * @brief Configures the timeout value for the UWB absolute timer.
 * 
 * @param enAbsoluteTimer The absolute timer to configure.
 * @param baseTime The base time for the timer.
 * @param targetTimeoutTime The target timeout time.
 */
void cb_system_uwb_abs_timer_configure_timeout_value(enUwbAbsoluteTimer enAbsoluteTimer, uint32_t baseTime, uint32_t targetTimeoutTime)
{
  uint32_t timeoutValue = 0;
  
  // Validate targetTimeoutTime limit: 34,359,738 us
  if (targetTimeoutTime > DEF_ABS_TIMER_MAX_TIMEOUT_US) {
    targetTimeoutTime = DEF_ABS_TIMER_MAX_TIMEOUT_US;  // Clamp to maximum
  }
  
  timeoutValue = (uint32_t)((uint64_t)(targetTimeoutTime) * DEF_US_TO_NS / DEF_ABS_TIMER_UNIT);
  
  cb_uwbdriver_abs_timer_configure_timeout_value(enAbsoluteTimer, baseTime, timeoutValue);
}

/**
 * @brief Configures the event commander for the UWB absolute timer.
 * 
 * @param control The enable/disable control for the event.
 * @param enAbsoluteTimer The absolute timer to configure.
 * @param uwbEventControl The event control settings.
 */
void cb_system_uwb_abs_timer_configure_event_commander(enUwbEnable control, enUwbAbsoluteTimer enAbsoluteTimer, enUwbEventControl uwbEventControl)
{
  cb_uwbdriver_abs_timer_configure_event_commander(control, enAbsoluteTimer,uwbEventControl);  
}

/**
 * @brief Enables or disables the event timestamp functionality.
 * 
 * @param enable Enable or disable control.
 */
void cb_system_uwb_enable_event_timestamp(enUwbEnable enable)
{
  cb_uwbdriver_enable_event_timestamp(enable);
}

/**
 * @brief Configures the event timestamp mask.
 * 
 * @param eventTimestampMask The mask for the event timestamp.
 * @param uwbEventIndex The index of the UWB event.
 */
void cb_system_uwb_configure_event_timestamp_mask(enUwbEventTimestampMask eventTimestampMask,enUwbEventIndex uwbEventIndex)
{
  cb_uwbdriver_configure_event_timestamp_mask(eventTimestampMask, uwbEventIndex);
}

/**
 * @brief Retrieves the value of the event timestamp.
 * 
 * @param eventTimestampMask The mask for the event timestamp.
 * @return The value of the event timestamp.
 */
uint32_t cb_system_uwb_get_event_timestamp_in_ns(enUwbEventTimestampMask eventTimestampMask)
{
  return cb_uwbdriver_get_event_timestamp_in_ns(eventTimestampMask);
}

/**
 * @brief Clears the TSU (Timestamp Unit).
 */
void cb_system_uwb_tsu_clear(void)
{
  cb_uwbdriver_tsu_clear();
}

/**
 * @brief Estimates the PDOA (Phase Difference of Arrival) using the given angles.
 * 
 * @param poa_deg1 The first angle in degrees.
 * @param poa_deg2 The second angle in degrees.
 * @return The estimated PDOA value.
 */
double cb_system_uwb_alg_pdoa_estimation(double poa_deg1, double poa_deg2)
{
  return cb_uwbalg_pdoa_estimation(poa_deg1, poa_deg2);
}

/**
 * @brief Calculates the propagation time using ranging results.
 * 
 * @param result1 The first ranging result.
 * @param result2 The second ranging result.
 * @return The calculated propagation time.
 */
double cb_system_uwb_alg_prop_calculation(cb_uwbsystem_rangingtroundtreply_st* result1, cb_uwbsystem_rangingtroundtreply_st* result2)
{
  return cb_uwbalg_prop_calculation(result1, result2);
}

/**
 * @brief Compensates for the 3D antenna bias in AOA (Angle of Arrival) calculations.
 * 
 * @param pdoa_raw The raw PDOA 3D data containing phase differences between antenna pairs
 * @param pd01_bias Phase difference bias between antenna 0 and 1 (in degrees)
 * @param pd02_bias Phase difference bias between antenna 0 and 2 (in degrees)
 * @param pd12_bias Phase difference bias between antenna 1 and 2 (in degrees)
 * @return stAOA_CompensatedData Structure containing the bias-compensated phase differences
 */
stAOA_CompensatedData cb_system_uwb_aoa_biascomp(cb_uwbsystem_pdoa_3ddata_st pdoa_raw, float pd01_bias, float pd02_bias, float pd12_bias)
{
  return cb_uwbaoa_pdoa_biascomp(pdoa_raw, pd01_bias, pd02_bias, pd12_bias);

}

/**
 * @brief   Calculate 3D Angle of Arrival (AOA) using lookup table method
 * @details This function calculates the 3D AOA by using phase differences between antenna pairs
 *          and lookup tables (LUT) to estimate azimuth and elevation angles. The algorithm 
 *          processes compensated phase difference data to determine angles in 3D space.
 *
 * @param   AOA_PD      Pointer to structure containing compensated phase differences (pd01, pd02, pd12)
 * @param   ant_attr    Pointer to 3D antenna attributes structure containing
 * @param   lut_attr    Pointer to LUT attributes structure containing reference data and parameters
 * @param   azi_result  Pointer to store the calculated azimuth angle in degrees (-90° to +90°)
 * @param   ele_result  Pointer to store the calculated elevation angle in degrees (-90° to +90°)
 * @return  void
 */
void cb_system_uwb_aoa_lut_full3d(stAOA_CompensatedData* AOA_PD, st_antenna_attribute_3d* ant_attr, cb_uwbaoa_lut_attribute_st* lut_attr, float* azi_result, float* ele_result)
{
  cb_uwbaoa_lut_full3d(AOA_PD, ant_attr, lut_attr, azi_result, ele_result);
}
/**
 * @brief Detects angle inversion in AOA (Angle of Arrival) calculations
 * @details Only works for antenna type 0 (A at top, B and C at bottom) and type 2 (A and C at top, B at bottom).
 *          All other antenna types are treated as out-of-FOV.
 *
 *          Type 0:            Type 2:
 *             A               A     C
 *          B     C               B
 *
 * @param fov_list Pointer to array containing field of view angles
 * @param ant_attr Pointer to 3D antenna attributes structure containing antenna positions and type
 * @param FOV_attr Pointer to FOV attributes structure containing reference data and parameters
 * @param AOA_PD Pointer to structure containing compensated phase differences
 * @return uint8_t Returns 1 if angle inversion is detected, 0 otherwise
 */
uint8_t cb_system_uwb_detect_angle_inversion(float* fov_list, st_antenna_attribute_3d* ant_attr, cb_uwbaoa_fov_attribute_st* FOV_attr, stAOA_CompensatedData* AOA_PD)
{
  return cb_uwbaoa_detect_angle_inversion(fov_list, ant_attr, FOV_attr, AOA_PD);
}
/**
 * @brief Calculates the 2D AOA (Angle of Arrival) using phase differences and lookup tables.
 * 
 * @param pd_azi Pointer to phase difference for azimuth calculation
 * @param ele_ref Pointer to reference elevation angle in degrees
 * @param ant_attr Pointer to 2D antenna attributes structure containing antenna positions and type
 * @param lut_attr Pointer to LUT attributes structure containing reference data and parameters
 * @param azi_result Pointer to store the calculated azimuth angle in degrees
 * @return void
 */
void cb_system_uwb_aoa_lut_full2d(float* pd_azi, float* ele_ref, st_antenna_attribute_2d* ant_attr, cb_uwbaoa_lut_attribute_st* lut_attr, float* azi_result)
{
  cb_uwbaoa_lut_full2d(pd_azi, ele_ref, ant_attr, lut_attr, azi_result);
}

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXX                         Radar Wrapper Functions                   XXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

/**
 * @brief Configures the radar system with specified parameters.
 *
 * This function initializes the radar subsystem components including TX, RX modules,
 * and sets the power amplifier and scaling parameters.
 * @param pa        The power amplifier setting (5-bit value, 0-31 range)
 * @param scale_bit The scaling factor for radar signal (3-bit value, 0-7 range)
 */
void cb_system_radar_config(uint32_t pa, uint32_t scale_bit)
{
  cb_uwbdriver_radar_config(pa, scale_bit);
}

/**
 * @brief Starts the radar system with specified gain settings.
 *
 * This function initiates the radar operation by starting TX and RX modules,
 * configuring timing registers, and setting the receive gain index based on
 * the current radar library configuration.
 *
 * @param gain_idx The gain index for the receiver (3-bit value, 0-7 range)
 */
void cb_system_radar_start(uint32_t gain_idx)
{
  cb_uwbdriver_radar_start(gain_idx);
}

void cb_system_radar_getcir(cb_uwbsystem_rx_cir_iqdata_st* destArray,cb_uwbsystem_rxport_en enRxPort,uint32_t NumCirSample)
{
  cb_uwbdriver_radar_getcir(destArray,enRxPort,NumCirSample);
}


/**
 * @brief Stop radar TX and RX operations
 */
void cb_system_radar_stop(void)
{
  cb_uwbdriver_radar_stop();
}

/**
 * @brief Deinitializes and powers down the radar system.
 *
 * This function turns off all radar-related modules.
 */
void cb_system_radar_off(void)
{
  cb_uwbdriver_radar_off();
}

/**
 * @brief Perform FFT processing on radar data.
 *
 * This function performs Fast Fourier Transform (FFT) processing on the provided data.
 * It supports different FFT lengths and can perform both forward and inverse FFT operations.
 *
 * @param fft_len The FFT length
 * @param pSrc Pointer to the source data array
 * @param ifftFlag Flag to indicate inverse FFT (1) or forward FFT (0)
 * @param doBitReverse Flag to indicate if bit reversal should be performed
 */
void cb_system_fft(cb_uwbradar_en fft_len, float* pSrc, uint8_t ifftFlag, uint8_t doBitReverse)
{
  cb_uwbdriver_fft(fft_len, pSrc, ifftFlag, doBitReverse);
}
