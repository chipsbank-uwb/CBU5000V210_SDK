/**
 * @file    AppUwbRxPer.c
 * @brief   [UWB] Implementation of UWB TRX RX PER measurement.
 * @details This file contains the implementation of UWB TRX RX PER (Packet Error Rate) measurement.
 *          It includes functions for initializing the measurement, handling IRQ callbacks,
 *          logging packet counts, and checking for timer timeout events.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include "AppUwbRxPer.h"
#include "CB_timer.h"
#include "CB_scr.h"
#include "AppSysIrqCallback.h"
#include "CB_system_types.h"
#include "CB_uwbframework.h"
#include "NonLIB_sharedUtils.h"
//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_UWB_RXPER_UARTPRINT_ENABLE APP_TRUE
#if (APP_UWB_RXPER_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_uwb_rxper_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_uwb_rxper_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
/* Default Rx packet configuration.*/
static cb_uwbsystem_packetconfig_st Rxpacketconfig = {
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


static  cb_uwbsystem_rx_irqenable_st stRxIrqEnable;

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_uwb_rxper_packet_count_logging(cb_uwbsystem_rxport_en enRxPort);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
//static uint8_t rxPayload[100];
static uint32_t rxPacketCount;
static uint32_t rxPacketCountBuf;
static uint32_t rxPacketCountWithAllStatusOK;
static uint32_t rxPacketNotReceived;
static uint8_t is_timer_timeout_flag;
static cb_uwbsystem_rxstatus_un statusRegisterNotOK;
static uint32_t countPositionWhenStatusRegisterNotOK;

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void app_rx_per(void)
{
  cb_uwbsystem_rxport_en enRxPort = EN_UWB_RX_0;
  uint32_t rxTimeIntervalInMs  = 10000;
  app_uwbtrx_rx_per(enRxPort, rxTimeIntervalInMs);
}

//timer init
void app_per_timer_init(uint32_t timer)
{
  // Configure TIMER0-EVENT0 in One-Shot mode that timeout at rxTimeIntervalInMs
  cb_scr_timer0_module_on();        // Turn on Timer 0
  cb_timer_disable_interrupt();    // Disable Timer's module interrupt
  NVIC_EnableIRQ(TIMER_0_IRQn);    // Enable CPU Timer 0 interrupt
  
  stTimerSetUp stTimerSetup;
  stTimerSetup.Timer                         = EN_TIMER_0;
  stTimerSetup.TimerMode                     = EN_TIMER_MODE_ONE_SHOT;
  stTimerSetup.TimeUnit                      = EN_TIMER_MS;
  stTimerSetup.stTimeOut.timeoutVal[0]       = timer;
  stTimerSetup.stTimeOut.timeoutVal[1]       = 0;
  stTimerSetup.stTimeOut.timeoutVal[2]       = 0;
  stTimerSetup.stTimeOut.timeoutVal[3]       = 0;
  stTimerSetup.stTimeOut.TimerTimeoutEvent   = DEF_TIMER_TIMEOUT_EVENT_0;
  stTimerSetup.TimerEvtComMode               = EN_TIMER_EVTCOM_MODE_00;
  stTimerSetup.AutoStartTimer                = EN_START_TIMER_ENABLE;
  stTimerSetup.TimerEvtComEnable             = EN_TIMER_EVTCOM_DISABLE;
  stTimerSetup.TimerInterrupt                = EN_TIMER_INTERUPT_ENABLE;
  stTimerSetup.stPulseWidth.NumberOfCycles   = 0;
  stTimerSetup.stPulseWidth.TimerPulseWidth  = EN_TIMER_PULSEWIDTH_DISABLE;
  cb_timer_configure_timer(&stTimerSetup);    

}

void timer_deinit(void)
{
  cb_timer_disable_timer(EN_TIMER_0);  // Disable timer
}


void rx_print_log_message(uint32_t interval_time)
{

  app_uwb_rxper_print("UWB RX PER - num. of packets received in %d ms\n", interval_time);  
  app_uwb_rxper_print("> rxPacketCount:                %d\n", rxPacketCount);
  app_uwb_rxper_print("> rxPacketCountWithAllStatusOK: %d\n", rxPacketCountWithAllStatusOK);
  app_uwb_rxper_print("> rxPacketNotReceived:          %d\n\n",rxPacketNotReceived);
  
  app_uwb_rxper_print("> countPositionWhenStatusRegisterNotOK:   %d\n", countPositionWhenStatusRegisterNotOK);
  app_uwb_rxper_print("> statusRegisterNotOK \n");
  app_uwb_rxper_print("  >> rx0_ok      %d\n", statusRegisterNotOK.rx0_ok       );
  app_uwb_rxper_print("  >> rx1_ok      %d\n", statusRegisterNotOK.rx1_ok       );  
  app_uwb_rxper_print("  >> rx2_ok      %d\n", statusRegisterNotOK.rx2_ok       );
  app_uwb_rxper_print("  >> pd0_det     %d\n", statusRegisterNotOK.pd0_det      );
  app_uwb_rxper_print("  >> pd1_det     %d\n", statusRegisterNotOK.pd1_det      );
  app_uwb_rxper_print("  >> pd2_det     %d\n", statusRegisterNotOK.pd2_det      );
  app_uwb_rxper_print("  >> sfd0_det    %d\n", statusRegisterNotOK.sfd0_det     );
  app_uwb_rxper_print("  >> sfd1_det    %d\n", statusRegisterNotOK.sfd1_det     );
  app_uwb_rxper_print("  >> sfd2_det    %d\n", statusRegisterNotOK.sfd2_det     );
  app_uwb_rxper_print("  >> no0_signal  %d\n", statusRegisterNotOK.no0_signal   );
  app_uwb_rxper_print("  >> no1_signal  %d\n", statusRegisterNotOK.no1_signal   );
  app_uwb_rxper_print("  >> no2_signal  %d\n", statusRegisterNotOK.no2_signal   );
  app_uwb_rxper_print("  >> phr_ded     %d\n", statusRegisterNotOK.phr_ded      );
  app_uwb_rxper_print("  >> phr_sec     %d\n", statusRegisterNotOK.phr_sec      );
  app_uwb_rxper_print("  >> crc_fail    %d\n", statusRegisterNotOK.crc_fail     );
  app_uwb_rxper_print("  >> dsr_ovf     %d\n", statusRegisterNotOK.dsr_ovf      );




}

void per_param_init(cb_uwbsystem_rxport_en enRxPort)
{
  is_timer_timeout_flag = CB_FALSE;
  rxPacketCount = 0;
  rxPacketCountBuf = 0;
  rxPacketCountWithAllStatusOK = 0;
  rxPacketNotReceived = 0;
  statusRegisterNotOK.value = 0;
  countPositionWhenStatusRegisterNotOK = 0;
  memset(&stRxIrqEnable, 0, sizeof(cb_uwbsystem_rx_irqenable_st));
  switch (enRxPort)
  {
    case EN_UWB_RX_0:
      stRxIrqEnable.rx0Done       = CB_TRUE;
      stRxIrqEnable.rx0PdDone     = CB_FALSE;
      stRxIrqEnable.rx0SfdDetDone = CB_FALSE;
      break;
    
    case EN_UWB_RX_1:
      stRxIrqEnable.rx1Done       = CB_TRUE;
      stRxIrqEnable.rx1PdDone     = CB_FALSE;
      stRxIrqEnable.rx1SfdDetDone = CB_FALSE;
      break;    
    
    case EN_UWB_RX_2:
      stRxIrqEnable.rx2Done       = CB_TRUE;
      stRxIrqEnable.rx2PdDone     = CB_FALSE;
      stRxIrqEnable.rx2SfdDetDone = CB_FALSE;
      break;
    
    case EN_UWB_RX_02: //unused case
    case EN_UWB_RX_ALL:
      break;
    
    default: 
      break;
  }
}

/**
 * @brief Performs UWB TRX RX PER measurement.
 * 
 * This function initializes the necessary variables and configurations for performing
 * UWB TRX RX PER measurement. It sets up the timer to timeout at the specified interval,
 * configures the UWB receiver port, and registers IRQ callbacks. It continuously receives
 * UWB packets and logs packet counts. Once the timer expires, it logs the received packet
 * count and status register information.
 * 
 * @param packetConfig The configuration of the UWB packet.
 * @param enRxPort The UWB receiver port to use.
 * @param rxTimeIntervalInMs The time interval in milliseconds for the measurement.
 */
void app_uwbtrx_rx_per(cb_uwbsystem_rxport_en enRxPort, uint32_t rxTimeIntervalInMs)
{
  // Initialization
  cb_framework_uwb_init();
  per_param_init(enRxPort); 
  app_per_timer_init(rxTimeIntervalInMs);                                          
  uint32_t startTime = 0;
  while(1)
  {
    cb_framework_uwb_rx_start(enRxPort, &Rxpacketconfig, &stRxIrqEnable, EN_TRX_START_NON_DEFERRED); // RX START
    while ((rxPacketCount == rxPacketCountBuf)&&(is_timer_timeout_flag == CB_FALSE))
    {
       if(cb_hal_is_time_elapsed(startTime, 3) == CB_PASS)   // Delay 3ms
       {
          rxPacketNotReceived++;
          break;
       }
    }
    rxPacketCountBuf = rxPacketCount;
    startTime = cb_hal_get_tick();
    if (is_timer_timeout_flag == CB_TRUE) 
    {
      break;  // break the while(1) loop once timeout
    }
  }
  timer_deinit();
  rx_print_log_message(rxTimeIntervalInMs);
      
 
}


/**
 * @brief Logs the packet count and status for a specific UWB RX port.
 * 
 * This function logs the packet count and status for a specific UWB RX port, such as RX0, RX1, or RX2.
 * 
 * @param enRxPort The UWB RX port for which to log the packet count and status.
 */  
void app_uwb_rxper_packet_count_logging(cb_uwbsystem_rxport_en enRxPort)
{
  cb_uwbsystem_rxstatus_un statusRegister = cb_framework_uwb_get_rx_status();

  uint16_t rx_ok     = 0;
  uint16_t sfd_det   = 0;
  uint16_t pd_det    = 0;
  
  switch(enRxPort)
  {
    case EN_UWB_RX_0:
      rx_ok    = statusRegister.rx0_ok;
      sfd_det  = statusRegister.sfd0_det;
      pd_det   = statusRegister.pd0_det;
      break;
    
    case EN_UWB_RX_1:
      rx_ok    = statusRegister.rx1_ok;
      sfd_det  = statusRegister.sfd1_det;
      pd_det   = statusRegister.pd1_det;
      break; 
    
    case EN_UWB_RX_2:
      rx_ok    = statusRegister.rx2_ok;
      sfd_det  = statusRegister.sfd2_det;
      pd_det   = statusRegister.pd2_det;
      break;
    
    case EN_UWB_RX_02: //unused case
    case EN_UWB_RX_ALL:
      break;
    
    default:  break;
  }
  
  if ((rx_ok == CB_TRUE) && (sfd_det == CB_TRUE) && (pd_det == CB_TRUE))
  {
    rxPacketCountWithAllStatusOK++;
  }
  else
  {
    statusRegisterNotOK = statusRegister;
    countPositionWhenStatusRegisterNotOK = rxPacketCount;
  }
  rxPacketCount++;  
}



/**
 * @brief Handler for rx0 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void cb_uwbapp_rx0_done_irqcb(void) 
{ 
   app_uwb_rxper_packet_count_logging(EN_UWB_RX_0);
}

/**
 * @brief Handler for rx1 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void cb_uwbapp_rx1_done_irqhandler(void) 
{ 
   app_uwb_rxper_packet_count_logging(EN_UWB_RX_1);
}

/**
 * @brief Handler for rx2 done interrupt.
 * @detail Call logger and flip global flag to let device continue receiving.
 */
void cb_uwbapp_rx2_done_irqcb(void) 
{ 
  app_uwb_rxper_packet_count_logging(EN_UWB_RX_2);
}


void cb_timer_0_app_irq_callback(void)  
{
   is_timer_timeout_flag = CB_TRUE;
}
