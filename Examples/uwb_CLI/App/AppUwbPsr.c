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

#include "CB_system.h"
#include "CB_commtrx.h"

#include <string.h>

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

static uint8_t s_SuspendFlag = APP_FALSE;

//static volatile uint8_t s_UwbPayload[4] = {0x33, 0x66, 0x99, 0x55};
static volatile uint8_t s_UwbRxPayload[4];
//static cb_uwbsystem_txpayload_st s_UwbTxPayload;

//static cb_uwbsystem_rxall_signalinfo_st s_RssiResults;
//static uint32_t s_RxPacketStatus;
//static uint32_t s_RxPacketPhr;

static stAppUwbPsrIRQStatus s_IrqStatus;

static uint8_t s_PreambleCodeStatus[PREAMBLE_CODE_IDX_SIZE];

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

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
	s_SuspendFlag = APP_FALSE;
}

/**
  * @brief Main function that runs PSR functionalities based on initialized variables
  */
void app_uwb_psr_deal(void)
{
	cb_system_uwb_init();

	APP_UWB_PSR_RegisterIrqCallbacks();	

	cb_uwbsystem_preamblecodeidx_en preambleCodeIdx = app_uwb_psr_sequence();
	
	APP_UWB_PSR_DeregisterIrqCallbacks();		
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
	
	cb_uwbsystem_preamblecodeidx_en originalPreambleCodeIdx = CB_SYSTEM_UwbGetPreambleCodeIdx();

	if (s_PreambleScanningParams.trxMode == EN_PSR_RX)
	{
		app_uwb_psr_print("Scanning ");
		
		memset(&s_IrqStatus, 0, sizeof(s_IrqStatus));

		cb_uwbsystem_rx_irqenable_st stRxIrqEnable;
		memset(&stRxIrqEnable, 0, sizeof(cb_uwbsystem_rx_irqenable_st));
		memset(&s_UwbRxPayload, 0, sizeof(s_UwbRxPayload));
		stRxIrqEnable.rx0PdDone     = CB_FALSE;
		stRxIrqEnable.rx0SfdDetDone = CB_TRUE;
		stRxIrqEnable.rx0Done       = CB_TRUE;

		for (uint8_t i = PREAMBLE_CODE_IDX_MIN; i <= PREAMBLE_CODE_IDX_MAX; ++i)
		{
			app_uwb_psr_print(".");
			
			CB_SYSTEM_UwbSetPreambleCodeIdx(i);
			
      CB_SYSTEM_UwbPreambleCode_Config(CB_SYSTEM_UwbGetPreambleCodeIdx(), EN_UWB_CONFIG_RX); 
			
			uint32_t cpuCycleStartCount = DWT->CYCCNT;
			
			CB_COMMTRX_UwbReceive(EN_UWB_RX_0, stRxIrqEnable);
			
			while(1)
			{
				while (s_IrqStatus.Rx0Done == APP_FALSE)
				{
					if (s_IrqStatus.Rx0SfdDetected == APP_TRUE)
					{
						// Wait for 25us
						cb_system_delay_in_us(25);
						
						cb_uwbsystem_rx_phrstatus_st phrStatus = cb_system_uwb_get_rx_phr_status();
						
						if (((phrStatus.phrSec == APP_TRUE) || (phrStatus.phrDed == APP_TRUE)) || (cb_system_uwb_get_rx_packet_phr() == 0))
						{
							//app_uwb_psr_print("F1\n");
							CB_COMMTRX_UwbReceive(EN_UWB_RX_0, stRxIrqEnable);
							
							s_IrqStatus.Rx0PdDone = APP_FALSE;
							s_IrqStatus.Rx0SfdDetected = APP_FALSE;
							s_IrqStatus.Rx0Done = APP_FALSE;
						}
					}
					
					if ((app_uwb_psr_cpucycle_get_time_us(cpuCycleStartCount) >= (s_PreambleScanningParams.scanDuration * 1000)) && (s_SuspendFlag == APP_FALSE)) // 200ms
					{
						if (s_PreambleScanningParams.psrMode == EN_PSR_SINGLE_MODE)
						{
							s_IrqStatus.Rx0PdDone = APP_FALSE;
							s_IrqStatus.Rx0SfdDetected = APP_FALSE;
							s_IrqStatus.Rx0Done = APP_FALSE;

							cb_system_uwb_trx_stop();
							CB_SYSTEM_RxOff(EN_UWB_RX_0);
							cb_system_uwb_off();

							app_uwb_psr_print("\n");

							app_uwb_psr_display_table(CB_SYSTEM_UwbGetPreambleCodeIdx());
							
							return CB_SYSTEM_UwbGetPreambleCodeIdx();
						}
						break;
					}
				}
				
				cb_uwbsystem_rx_phrstatus_st phrStatus = cb_system_uwb_get_rx_phr_status();
				
				if (((phrStatus.phrSec == APP_TRUE) || (phrStatus.phrDed == APP_TRUE) || (phrStatus.rx0Ok == APP_FALSE)) || (cb_system_uwb_get_rx_packet_phr() == 0))
				{
					//app_uwb_psr_print("F2\n");
					if ((app_uwb_psr_cpucycle_get_time_us(cpuCycleStartCount) >= (s_PreambleScanningParams.scanDuration * 1000)) && (s_SuspendFlag == APP_FALSE)) // 200ms
						break;
					
					CB_COMMTRX_UwbReceive(EN_UWB_RX_0, stRxIrqEnable);
					
					s_IrqStatus.Rx0PdDone = APP_FALSE;
					s_IrqStatus.Rx0SfdDetected = APP_FALSE;
					s_IrqStatus.Rx0Done = APP_FALSE;
				}
				else
				{
					s_PreambleCodeStatus[CB_SYSTEM_UwbGetPreambleCodeIdx() - PREAMBLE_CODE_IDX_MIN] = APP_TRUE;
					
					break;
				}
			}
			
			s_IrqStatus.Rx0PdDone = APP_FALSE;
			s_IrqStatus.Rx0SfdDetected = APP_FALSE;
			s_IrqStatus.Rx0Done = APP_FALSE;
			
			cb_system_uwb_trx_stop();
			CB_SYSTEM_RxOff(EN_UWB_RX_0);
		}
		
		cb_system_uwb_off();
		
		app_uwb_psr_print("\n");
		
		app_uwb_psr_display_table(CB_SYSTEM_UwbGetPreambleCodeIdx());
	}
	
	cb_uwbsystem_preamblecodeidx_en returnPreambleCodeIdx = CB_SYSTEM_UwbGetPreambleCodeIdx();
	CB_SYSTEM_UwbSetPreambleCodeIdx(originalPreambleCodeIdx);
	
	return returnPreambleCodeIdx;
}

/**
 * @brief Suspends the UWB Preamble Scanning Receiver (PSR) operation.
 * 
 * This function sets the suspend flag to true, effectively suspending the current UWB PSR operation. It is typically used to halt the scanning process. 
 * The function logs a message indicating that the PSR operation has been stopped.
 */
void APP_UWB_PSR_Suspend(void)
{
  s_SuspendFlag = APP_TRUE;

  app_uwb_psr_print("[APP_UWB_PSR_Stop]\n");
}

/**
 * @brief Calculates the time passed in microseconds based on the CPU cycle count.
 * 
 * This function computes the elapsed time in microseconds by comparing the current CPU cycle count to a given start count. The calculation takes into account the possibility of the cycle counter overflow and uses the system clock frequency (SystemCoreClock) to convert the cycle difference into microseconds.
 * 
 * @param[in] cpuCycleStartCount The starting CPU cycle count from which to calculate the elapsed time.
 * 
 * @return The elapsed time in microseconds (us)
 * 
 * @note This function assumes that the CPU cycle counter (`DWT->CYCCNT`) is running and provides accurate cycle counts.
 */
uint32_t app_uwb_psr_cpucycle_get_time_us(uint32_t cpuCycleStartCount)
{
	uint32_t cpuCyclesConsumed = (DWT->CYCCNT < cpuCycleStartCount) ?
																(0xFFFFFFFF - cpuCycleStartCount + DWT->CYCCNT + 1) : DWT->CYCCNT - cpuCycleStartCount;

	return (uint32_t)((((double)cpuCyclesConsumed*1000)/SystemCoreClock)*1000);	
}

/**
 * @brief Registers interrupt callbacks for UWB PSR IRQ events.
 * 
 * This function registers interrupt service routine (ISR) callbacks for various UWB PSR (Preamble Scanning Receiver) IRQ events. The registered callbacks handle different stages of the UWB reception and transmission process, ensuring that appropriate actions are taken when these events occur.
 * 
 * @note This function should be called to enable handling of UWB PSR interrupts. Failure to register these callbacks may result in missing or incorrect processing of UWB events.
 */
void APP_UWB_PSR_RegisterIrqCallbacks(void)
{
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ, APP_UWB_PSR_TX_DONE_IRQ_Callback);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_PD_DONE_APP_IRQ, APP_UWB_PSR_RX0_PD_DONE_IRQ_CallBack);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_SFD_DET_DONE_APP_IRQ, APP_UWB_PSR_RX0_SFD_DET_DONE_IRQ_Callback);
  app_irq_register_irqcallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, APP_UWB_PSR_RX0_DONE_IRQ_Callback);
}

/**
 * @brief Deregisters interrupt callbacks for UWB PSR IRQ events.
 * 
 * This function deregisters interrupt service routine (ISR) callbacks for UWB PSR IRQ events. It is used to remove the previously registered callbacks, effectively disabling further handling of these interrupts.
 * 
 * @note This function should be called to disable UWB PSR interrupt handling. After calling this function, the system will no longer respond to the UWB PSR IRQ events.
 */
void APP_UWB_PSR_DeregisterIrqCallbacks(void)
{
  app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ, APP_UWB_PSR_TX_DONE_IRQ_Callback);
  app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX0_PD_DONE_APP_IRQ, APP_UWB_PSR_RX0_PD_DONE_IRQ_CallBack);
  app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX0_SFD_DET_DONE_APP_IRQ, APP_UWB_PSR_RX0_SFD_DET_DONE_IRQ_Callback);
  app_irq_deregister_irqcallback(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ, APP_UWB_PSR_RX0_DONE_IRQ_Callback);
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
  app_uwb_psr_print("+---------------------+--------------+\n");
  app_uwb_psr_print("| Preamble Code Index |    Status    |\n");
  app_uwb_psr_print("+---------------------+--------------+\n");
	
	if (s_PreambleScanningParams.psrMode == EN_PSR_SINGLE_MODE)
		app_uwb_psr_print("|         %2u          | %-12s |\n", CB_SYSTEM_UwbGetPreambleCodeIdx(), s_PreambleCodeStatus[CB_SYSTEM_UwbGetPreambleCodeIdx() - PREAMBLE_CODE_IDX_MIN] ? "Used" : "Not Used");
	else
	{
  for (uint8_t i = PREAMBLE_CODE_IDX_MIN; i <= preambleCodeIdx; ++i)
    app_uwb_psr_print("|         %2u          | %-12s |\n", i, s_PreambleCodeStatus[i - PREAMBLE_CODE_IDX_MIN] ? "Used" : "Not Used");
	}
    
  app_uwb_psr_print("+---------------------+--------------+\n");	
}

/**
 * @brief Callback function for the UWB TX Done IRQ.
 *
 * This function is called when the UWB TX Done IRQ is triggered. It fetches
 * the timestamp related to the TX Done event and prints it via UART.
 */
void APP_UWB_PSR_TX_DONE_IRQ_Callback(void)
{
	CB_SYSTEM_TxOff();
	s_IrqStatus.TxDone = APP_TRUE;
}

/**
 * @brief Callback function for the UWB RX0 PD Done IRQ.
 *
 * This function is called when the UWB RX0 PD Done IRQ is triggered.
 */
void APP_UWB_PSR_RX0_PD_DONE_IRQ_CallBack(void)
{
	s_IrqStatus.Rx0PdDone = APP_TRUE;
}

/**
 * @brief Callback function for the UWB RX0 SFD Detection Done IRQ.
 *
 * This function is called when the UWB RX0 SFD Detection Done IRQ is triggered.
 */
void APP_UWB_PSR_RX0_SFD_DET_DONE_IRQ_Callback(void)
{
	s_IrqStatus.Rx0SfdDetected = APP_TRUE;
}

/**
 * @brief Callback function for the UWB RX0 Done IRQ.
 *
 * This function is called when the UWB RX0 Done IRQ is triggered.
 */
void APP_UWB_PSR_RX0_DONE_IRQ_Callback(void)
{
	s_IrqStatus.Rx0Done = APP_TRUE;
}
