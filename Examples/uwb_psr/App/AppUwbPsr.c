/**
 * @file    AppUwbPsr.c
 * @brief
 * @details
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "AppUwbPsr.h"
#include "AppSysIrqCallback.h"
#include "CB_uwbframework.h"
#include <string.h>
#include "NonLIB_sharedUtils.h"
//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_PSR_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_PSR_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_psr_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_psr_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define RX_PACKET_PHR_VERIFICATION_VALUE 4417
#define PREAMBLE_CODE_IDX_MIN 9
#define PREAMBLE_CODE_IDX_MAX 24
#define PREAMBLE_CODE_IDX_SIZE (PREAMBLE_CODE_IDX_MAX - PREAMBLE_CODE_IDX_MIN)

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
  volatile uint8_t TxDone;
  volatile uint8_t Rx0PdDone;
  volatile uint8_t Rx0SfdDetected;	
  volatile uint8_t Rx0Done;
}stAppUwbPsrIRQStatus;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stPreambleScanningParameters s_PreambleScanningParams;


static volatile uint8_t s_UwbRxPayload[4];

static stAppUwbPsrIRQStatus s_IrqStatus;

static cb_uwbsystem_rx_irqenable_st stRxIrqEnable;

static uint8_t s_PreambleCodeStatus[PREAMBLE_CODE_IDX_SIZE];

/* Default Rx packet configuration.*/
static cb_uwbsystem_packetconfig_st Rxpacketconfig = 
{
  .prfMode            = EN_PRF_MODE_BPRF_62P4,                 // PRF mode selection
  .psduDataRate       = EN_PSDU_DATA_RATE_6P81,           // PSDU data rate
  .bprfPhrDataRate    = EN_BPRF_PHR_DATA_RATE_0P85,       // BPRF PHR data rate
  .preambleCodeIndex  = EN_UWB_PREAMBLE_CODE_IDX_9,       // Preamble code index (9-32)
  .preambleDuration   = EN_PREAMBLE_DURATION_64_SYMBOLS,  // Preamble duration (0-1)
  .sfdId              = EN_UWB_SFD_ID_2,                  // SFD identifier (0-4)
  .phrRangingBit      = 0x00,                             // PHR Ranging Bit (0-1)
  .rframeConfig       = EN_RFRAME_CONFIG_SP0,             // SP0, SP1, SP3
  .stsLength          = EN_STS_LENGTH_64_SYMBOLS,         // STS Length
  .numStsSegments     = EN_NUM_STS_SEGMENTS_1,            // Number of STS segments
  .macFcsType         = EN_MAC_FCS_TYPE_CRC16,            // CRC16
};



cb_uwbsystem_preamblecodeidx_en cb_system_get_preamble_index(void)
{
  return Rxpacketconfig.preambleCodeIndex; 
}

void cb_system_set_preamble_index(cb_uwbsystem_preamblecodeidx_en index)
{
  Rxpacketconfig.preambleCodeIndex = index;
}
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void app_psr_start(void)
{
  stPreambleScanningParameters preambleScanningParams;
  memset(&preambleScanningParams, 0, sizeof(preambleScanningParams));
	
  preambleScanningParams.trxMode = EN_PSR_RX;
	
  switch (preambleScanningParams.trxMode)
  {
    case EN_PSR_RX:
		preambleScanningParams.psrMode = EN_PSR_FULL_MODE;
	    preambleScanningParams.scanDuration = 200;
		app_uwb_psr_init(preambleScanningParams); 
		app_uwb_psr_deal();
        break;
    
    case EN_PSR_TX:
	    break;
    
    default:
        break;
  }
}
/**
 * @brief Initializes the UWB PSR (Preamble Scanning Receiver) with the provided scanning parameters.
 * 
 * This function initializes the global UWB preamble scanning configuration, including setting up the scanning parameters, packet configuration, and the preamble scanning state. It also prepares the receiver for operation by clearing any previous settings and ensuring that the system is ready for the scanning process. 
 * The function must be called before the main scanning sequence starts, ensuring that the necessary configuration is in place before entering the main program flow.
 * 
 * @param[in] params The preamble scanning parameters to initialize the system with. 
 * These parameters define the scanning mode, TRX mode, and other key configurations for the UWB PSR. The members of the `stPreambleScanningParameters` struct are:
 * 
 * - `trxMode`: The TRX mode used for the scanning. Can be either `EN_PSR_RX` (Receiver mode) or `EN_PSR_TX` (Transmitter mode).
 * - `psrMode`: The scanning mode, either `EN_PSR_SINGLE_MODE` for a single scan or `EN_PSR_CONTINUOUS_MODE` for continuous scanning.
 * - `scanDuration`: The duration (in ms) for which the scanning should be performed in a single cycle.
 * - `uwbPacketConfig`: A structure containing UWB packet configuration details, including extended arguments for the receiver configuration.
 * 
 * @note This function should be called before the main application logic to ensure that the scanning configuration is properly set up before any scanning operations are performed.
 * 
 * @see app_uwb_psr_sequence
 */
void app_uwb_psr_init(stPreambleScanningParameters params)
{
  s_PreambleScanningParams = params;
}

/**
  * @brief Main function that runs PSR functionalities based on initialized variables
  */
void app_uwb_psr_deal(void)
{
  app_uwb_psr_sequence();	
}



void rx_scan_param_init(void)
{   
  memset(&stRxIrqEnable, 0, sizeof(cb_uwbsystem_rx_irqenable_st));
  memset(&s_UwbRxPayload, 0, sizeof(s_UwbRxPayload));
  memset(&s_IrqStatus, 0, sizeof(s_IrqStatus));
  stRxIrqEnable.rx0PdDone     = CB_FALSE;
  stRxIrqEnable.rx0SfdDetDone = CB_TRUE;
  stRxIrqEnable.rx0Done       = CB_TRUE;

}


/**
 * @brief Executes the preamble scanning sequence for the PSR (Preamble Scanning Receiver).
 * 
 * This function performs the scanning sequence by configuring the UWB receiver, processing the received signals, and evaluating the results based on preamble code indices. It handles the scanning of multiple preamble codes, waits for SFD (Start of Frame Delimiters), and checks for valid PHR (Preamble Header) conditions. Depending on the scanning mode (single or continuous), it either returns a result for the detected preamble code index or continues scanning.
 * 
 * The function works in the following sequence:
 * - Initializes various system configurations and IRQ settings.
 * - Starts scanning by cycling through preamble code indices.
 * - Receives data and waits for frame detection.
 * - Verifies the validity of the frame header and retries if necessary.
 * - Stops scanning if the scan duration is reached or if the system is in suspend mode.
 * 
 * @return The detected preamble code index based on the scanning results.
 * 
 * @note The function handles two scanning modes:
 *       - **Single Mode**: Stops after scanning once and returns the detected preamble code index.
 *       - **Continuous Mode**: Continues scanning without returning until a valid result is obtained or the scan duration ends.
 * 
 */
cb_uwbsystem_preamblecodeidx_en app_uwb_psr_sequence(void)
{
  memset(s_PreambleCodeStatus, 0, sizeof(s_PreambleCodeStatus));	
    
  cb_uwbsystem_preamblecodeidx_en originalPreambleCodeIdx = cb_system_get_preamble_index();
    
  cb_framework_uwb_init();   
  if(s_PreambleScanningParams.trxMode == EN_PSR_RX)
  {
	 app_uwb_psr_print("Scanning\r\n");	
     rx_scan_param_init();
	 for(uint8_t i = PREAMBLE_CODE_IDX_MIN; i <= PREAMBLE_CODE_IDX_MAX; ++i)
	 {
		cb_system_set_preamble_index(i);
		uint32_t  startTime = cb_hal_get_tick();		    
		cb_framework_uwb_rx_start(EN_UWB_RX_0, &Rxpacketconfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED); // RX START		
		while(1)
		{
		   while(s_IrqStatus.Rx0Done == APP_FALSE)
		   {
                if(s_IrqStatus.Rx0SfdDetected == APP_TRUE)
                {
                   cb_system_delay_in_us(25);
                   cb_uwbsystem_rx_phrstatus_st phrStatus = cb_framework_uwb_get_rx_phr_status();
                    
                   if(((phrStatus.phrSec == APP_TRUE) || (phrStatus.phrDed == APP_TRUE)) || (cb_framework_uwb_is_rx_phr_empty()))
                   {                  
                        cb_framework_uwb_rx_start(EN_UWB_RX_0, &Rxpacketconfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED); 
                        s_IrqStatus.Rx0PdDone = APP_FALSE;
                        s_IrqStatus.Rx0SfdDetected = APP_FALSE;
                        s_IrqStatus.Rx0Done = APP_FALSE;
                   }
                }	
				if((cb_hal_is_time_elapsed(startTime,s_PreambleScanningParams.scanDuration) == CB_PASS)) // 200ms
				{
				   if(s_PreambleScanningParams.psrMode == EN_PSR_SINGLE_MODE)
				   {
                        s_IrqStatus.Rx0PdDone = APP_FALSE;
                        s_IrqStatus.Rx0SfdDetected = APP_FALSE;
                        s_IrqStatus.Rx0Done = APP_FALSE;

                        cb_framework_uwb_rx_end(EN_UWB_RX_0);     
                        cb_framework_uwb_off();
                        app_uwb_psr_print("\n");
                        app_uwb_psr_display_table(cb_system_get_preamble_index());
                        
                        return cb_system_get_preamble_index();
				   }
				   break;
			   }
			}
			cb_uwbsystem_rx_phrstatus_st phrStatus = cb_framework_uwb_get_rx_phr_status();				
			if(((phrStatus.phrSec == APP_TRUE) || (phrStatus.phrDed == APP_TRUE) || (phrStatus.rx0Ok == APP_FALSE)) ||\
               (cb_framework_uwb_is_rx_phr_empty()))
			{
               if((cb_hal_is_time_elapsed(startTime,s_PreambleScanningParams.scanDuration) == CB_PASS)) // 200ms
               {
                  break;
               }	              
               cb_framework_uwb_rx_start(EN_UWB_RX_0, &Rxpacketconfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED); 
               s_IrqStatus.Rx0PdDone = APP_FALSE;
               s_IrqStatus.Rx0SfdDetected = APP_FALSE;
               s_IrqStatus.Rx0Done = APP_FALSE;
			}
			else
			{
			   s_PreambleCodeStatus[cb_system_get_preamble_index() - PREAMBLE_CODE_IDX_MIN] = APP_TRUE;	
			   break;
			}
		}
			
        s_IrqStatus.Rx0PdDone = APP_FALSE;
        s_IrqStatus.Rx0SfdDetected = APP_FALSE;
        s_IrqStatus.Rx0Done = APP_FALSE;
			
		cb_framework_uwb_rx_end(EN_UWB_RX_0);
	}
		
	cb_framework_uwb_off();	
	app_uwb_psr_display_table(cb_system_get_preamble_index());
    app_uwb_psr_print("\n");
  }	
  cb_uwbsystem_preamblecodeidx_en returnPreambleCodeIdx = cb_system_get_preamble_index();
  cb_system_set_preamble_index(originalPreambleCodeIdx);
	
  return returnPreambleCodeIdx;
}



/**
 * @brief Displays the status table of the UWB preamble code index.
 * 
 * This function displays a table showing the status of each UWB preamble code index. It prints whether a particular preamble code index is "Used" or "Not Used", based on the scanning mode and status of each preamble code. The display is customized depending on the scanning mode: either single mode or a mode that handles multiple preamble codes.
 * 
 * @param[in] preambleCodeIdx The current preamble code index to be displayed.
 * 
 * @note This function is typically used for debugging and monitoring the status of the UWB scanning process. The table can help identify which preamble code indices have been used during the scanning.
 */
void app_uwb_psr_display_table(cb_uwbsystem_preamblecodeidx_en preambleCodeIdx)
{

  app_uwb_psr_print("| Preamble Code Index |    Status    |\n");
  app_uwb_psr_print("+---------------------+--------------+\n");
	
  if(s_PreambleScanningParams.psrMode == EN_PSR_SINGLE_MODE)
  {  
    app_uwb_psr_print("|         %2u          | %-12s |\n", cb_system_get_preamble_index(),\
                     s_PreambleCodeStatus[cb_system_get_preamble_index() - PREAMBLE_CODE_IDX_MIN] ? "Used" : "Not Used");     
  }	
  else
  {
    for (uint8_t i = PREAMBLE_CODE_IDX_MIN; i <= preambleCodeIdx; ++i)
    {
      app_uwb_psr_print("|         %2u          | %-12s |\n", i, s_PreambleCodeStatus[i - PREAMBLE_CODE_IDX_MIN] ? "Used" : "Not Used");   
    } 
  }  
  app_uwb_psr_print("+---------------------+--------------+\n");	
}



/**
 * @brief Callback function for the UWB RX0 PD Done IRQ.
 *
 * This function is called when the UWB RX0 PD Done IRQ is triggered.
 */
void cb_uwbapp_rx0_preamble_detected_irqcb(void)
{
  s_IrqStatus.Rx0PdDone = APP_TRUE;
}

/**
 * @brief Callback function for the UWB RX0 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX0 SFD Detection Done IRQ is triggered.
 */
void cb_uwbapp_rx0_sfd_detected_irqcb(void)
{
  s_IrqStatus.Rx0SfdDetected = APP_TRUE;
}

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 *
 * This function is called when the UWB RX0 Done IRQ is triggered.
 */
void cb_uwbapp_rx0_done_irqcb(void)
{
  s_IrqStatus.Rx0Done = APP_TRUE;
}
