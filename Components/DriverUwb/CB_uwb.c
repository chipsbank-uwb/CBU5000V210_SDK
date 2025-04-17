/**
 * @file CB_uwb.c
 * @brief Ultra-Wideband (UWB) interrupt handlers implementation
 * 
 * This file implements the handlers for UWB interrupt events, including:
 * - RX port events (done, preamble detection, SFD detection)
 * - TX events (done, SFD mark)
 * - Special event handling (STS CIR end, PHR detection)
 * 
 * The implementation disables the corresponding event and CPU IRQs, then calls 
 * the application callback functions which can be overridden by applications.
 * 
 * @author ChipsBank
 * @date 2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "string.h"
#include "CB_uwb.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include "CB_CompileOption.h"
#include "CB_UwbDrivers.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

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

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Internal callback for UWB RX0 done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx0_done_irqcb(void);

/**
 * @brief Internal callback for UWB RX0 preamble detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx0_preamble_detected_irqcb(void);

/**
 * @brief Internal callback for UWB RX0 SFD detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx0_sfd_detected_irqcb(void);

/**
 * @brief Internal callback for UWB RX1 done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx1_done_irqcb(void);

/**
 * @brief Internal callback for UWB RX1 preamble detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx1_preamble_detected_irqcb(void);

/**
 * @brief Internal callback for UWB RX1 SFD detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx1_sfd_detected_irqcb(void);

/**
 * @brief Internal callback for UWB RX2 done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx2_done_irqcb(void);

/**
 * @brief Internal callback for UWB RX2 preamble detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx2_preamble_detected_irqcb(void);

/**
 * @brief Internal callback for UWB RX2 SFD detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx2_sfd_detected_irqcb(void);

/**
 * @brief Internal callback for UWB RX STS CIR end event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx_sts_cir_end_irqcb(void);

/**
 * @brief Internal callback for UWB RX PHR detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx_phr_detected_irqcb(void);

/**
 * @brief Internal callback for general UWB RX done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx_done_irqcb(void);

/**
 * @brief Internal callback for UWB TX done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_tx_done_irqcb(void);

/**
 * @brief Internal callback for UWB TX SFD mark event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_tx_sfd_mark_irqcb(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/**
 * @brief IRQ handler for UWB RX0 done event
 *
 * This function is called when the UWB receiver port 0 has completed
 * a packet reception. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx0_done_irqhandler(void)          
{  
  //-----------------------
  // Disable RX EVENT IRQ - RX0 done
  // Disable RX CPU IRQ   - RX0 done
  //-----------------------  
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX0_DONE);
  NVIC_DisableIRQ(UWB_RX0_DONE_IRQn);    
  
  cb_uwb_rx0_done_irqcb();
}

/**
 * @brief IRQ handler for UWB RX0 preamble detection event
 *
 * This function is called when the UWB receiver port 0 has detected
 * a packet preamble. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx0_preamble_detected_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX0 PD done
  // Disable RX CPU IRQ   - RX0 PD done
  //-----------------------  
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX0_PD_DONE);
  NVIC_DisableIRQ(UWB_RX0_PD_DONE_IRQn);  

  cb_uwb_rx0_preamble_detected_irqcb();
}

/**
 * @brief IRQ handler for UWB RX0 SFD detection event
 *
 * This function is called when the UWB receiver port 0 has detected
 * a start frame delimiter (SFD). It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx0_sfd_detected_irqhandler(void)
{ 
  //-----------------------
  // Disable RX EVENT IRQ - RX0 SFD Detection Done
  // Disable RX CPU IRQ   - RX0 SFD Detection Done
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX0_SFD_DET_DONE);
  NVIC_DisableIRQ(UWB_RX0_SFD_DET_DONE_IRQn);    
  
  cb_uwb_rx0_sfd_detected_irqcb();
}

/**
 * @brief IRQ handler for UWB RX1 done event
 *
 * This function is called when the UWB receiver port 1 has completed
 * a packet reception. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx1_done_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX1 done
  // Disable RX CPU IRQ   - RX1 done
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX1_DONE);
  NVIC_DisableIRQ(UWB_RX1_DONE_IRQn);

  cb_uwb_rx1_done_irqcb();
}

/**
 * @brief IRQ handler for UWB RX1 preamble detection event
 *
 * This function is called when the UWB receiver port 1 has detected
 * a packet preamble. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx1_preamble_detected_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX1 PD done
  // Disable RX CPU IRQ   - RX1 PD done
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX1_PD_DONE);
  NVIC_DisableIRQ(UWB_RX1_PD_DONE_IRQn);  

  cb_uwb_rx1_preamble_detected_irqcb();
}

/**
 * @brief IRQ handler for UWB RX1 SFD detection event
 *
 * This function is called when the UWB receiver port 1 has detected
 * a start frame delimiter (SFD). It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx1_sfd_detected_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX1 SFD Detection Done
  // Disable RX CPU IRQ   - RX1 SFD Detection Done
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX1_SFD_DET_DONE);
  NVIC_DisableIRQ(UWB_RX1_SFD_DET_DONE_IRQn);  

  cb_uwb_rx1_sfd_detected_irqcb();
}

/**
 * @brief IRQ handler for UWB RX2 done event
 *
 * This function is called when the UWB receiver port 2 has completed
 * a packet reception. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx2_done_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX2 done
  // Disable RX CPU IRQ   - RX2 done
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX2_DONE);
  NVIC_DisableIRQ(UWB_RX2_DONE_IRQn); 
  
  cb_uwb_rx2_done_irqcb();
}

/**
 * @brief IRQ handler for UWB RX2 preamble detection event
 *
 * This function is called when the UWB receiver port 2 has detected
 * a packet preamble. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx2_preamble_detected_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX2 PD done
  // Disable RX CPU IRQ   - RX2 PD done
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX2_PD_DONE);
  NVIC_DisableIRQ(UWB_RX2_PD_DONE_IRQn);  

  cb_uwb_rx2_preamble_detected_irqcb();
}

/**
 * @brief IRQ handler for UWB RX2 SFD detection event
 *
 * This function is called when the UWB receiver port 2 has detected
 * a start frame delimiter (SFD). It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx2_sfd_detected_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX2 SFD Detection Done
  // Disable RX CPU IRQ   - RX2 SFD Detection Done
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX2_SFD_DET_DONE);
  NVIC_DisableIRQ(UWB_RX2_SFD_DET_DONE_IRQn);    

  cb_uwb_rx2_sfd_detected_irqcb();
}

/**
 * @brief IRQ handler for UWB RX STS CIR end event
 *
 * This function is called when the UWB receiver has completed processing
 * the STS (Scrambled Timestamp Sequence) CIR (Channel Impulse Response).
 * It disables the corresponding event and CPU IRQs, then calls the application callback.
 */
void cb_uwb_rx_sts_cir_end_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX STS CIR End
  // Disable RX CPU IRQ   - RX STS CIR End
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX_STS_CIR_END);
  NVIC_DisableIRQ(UWB_RX_STS_CIR_END_IRQn);  

  cb_uwb_rx_sts_cir_end_irqcb();
}

/**
 * @brief IRQ handler for UWB RX PHR detection event
 *
 * This function is called when the UWB receiver has detected a PHR
 * (PHY Header). It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx_phr_detected_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX PHR Detected
  // Disable RX CPU IRQ   - RX PHR Detected
  //----------------------- 
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX_PHY_PHR);
  NVIC_DisableIRQ(UWB_RX_PHR_DETECTED_IRQn);    
 
  cb_uwb_rx_phr_detected_irqcb();
}

/**
 * @brief IRQ handler for general UWB RX done event
 *
 * This function is called when the UWB receiver has completed
 * a packet reception across all ports. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx_done_irqhandler(void)
{
  //-----------------------
  // Disable RX EVENT IRQ - RX Done
  // Disable RX CPU IRQ   - RX Done
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_RX_DONE);
  NVIC_DisableIRQ(UWB_RX_DONE_IRQn);    
 
  cb_uwb_rx_done_irqcb();
}

/**
 * @brief IRQ handler for UWB TX done event
 *
 * This function is called when the UWB transmitter has completed
 * sending a packet. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_tx_done_irqhandler(void)
{  
  //-----------------------
  // Disable TX EVENT IRQ - TX Done
  // Disable TX CPU IRQ   - TX Done 
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_TX_DONE);
  NVIC_DisableIRQ(UWB_TX_DONE_IRQn);    
  
  cb_uwb_tx_done_irqcb();
}

/**
 * @brief IRQ handler for UWB TX SFD mark event
 *
 * This function is called when the UWB transmitter has sent the SFD
 * (Start Frame Delimiter) portion of a packet. It disables the corresponding event
 * and CPU IRQs, then calls the application callback.
 */
void cb_uwb_tx_sfd_mark_irqhandler(void)
{
  //-----------------------
  // Disable TX EVENT IRQ - TX SRD Mark
  // Disable TX CPU IRQ   - TX SRD Mark 
  //-----------------------
  cb_uwbdriver_disable_event_irq(EN_UWB_IRQ_EVENT_TX_SFD_MARK);
  NVIC_DisableIRQ(UWB_TX_SFD_MARK_IRQn);  
 
  cb_uwb_tx_sfd_mark_irqcb();
}

/**
 * @brief Internal callback for UWB RX0 done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx0_done_irqcb(void)
{
  cb_uwbapp_rx0_done_irqcb();
}

/**
 * @brief Application callback for UWB RX0 done event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 0 done event.
 */
__WEAK void cb_uwbapp_rx0_done_irqcb(void)
{
}

/**
 * @brief Internal callback for UWB RX0 preamble detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx0_preamble_detected_irqcb(void)
{
  cb_uwbapp_rx0_preamble_detected_irqcb();
}

/**
 * @brief Application callback for UWB RX0 preamble detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 0 preamble detection event.
 */
__WEAK void cb_uwbapp_rx0_preamble_detected_irqcb(void)
{
}

/**
 * @brief Internal callback for UWB RX0 SFD detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx0_sfd_detected_irqcb(void)
{
  cb_uwbapp_rx0_sfd_detected_irqcb();
}

/**
 * @brief Application callback for UWB RX0 SFD detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 0 SFD detection event.
 */
__WEAK void cb_uwbapp_rx0_sfd_detected_irqcb(void)
{
}

/**
 * @brief Internal callback for UWB RX1 done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx1_done_irqcb(void)
{
  cb_uwbapp_rx1_done_irqhandler();
}

/**
 * @brief Application callback for UWB RX1 done event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 1 done event.
 */
__WEAK void cb_uwbapp_rx1_done_irqhandler(void)
{
}

/**
 * @brief Internal callback for UWB RX1 preamble detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx1_preamble_detected_irqcb(void)
{
  cb_uwbapp_rx1_preamble_detected_irqhandler();
}

/**
 * @brief Application callback for UWB RX1 preamble detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 1 preamble detection event.
 */
__WEAK void cb_uwbapp_rx1_preamble_detected_irqhandler(void)
{
}

/**
 * @brief Internal callback for UWB RX1 SFD detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx1_sfd_detected_irqcb(void)
{
  cb_uwbapp_rx1_sfd_detected_irqcb();
}

/**
 * @brief Application callback for UWB RX1 SFD detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 1 SFD detection event.
 */
__WEAK void cb_uwbapp_rx1_sfd_detected_irqcb(void)
{
}

/**
 * @brief Internal callback for UWB RX2 done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx2_done_irqcb(void)
{
  cb_uwbapp_rx2_done_irqcb();
}

/**
 * @brief Application callback for UWB RX2 done event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 2 done event.
 */
__WEAK void cb_uwbapp_rx2_done_irqcb(void)
{
}

/**
 * @brief Internal callback for UWB RX2 preamble detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx2_preamble_detected_irqcb(void)
{
  cb_uwbapp_rx2_preamble_detected_irqhandler();
}

/**
 * @brief Application callback for UWB RX2 preamble detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 2 preamble detection event.
 */
__WEAK void cb_uwbapp_rx2_preamble_detected_irqhandler(void)
{
}

/**
 * @brief Internal callback for UWB RX2 SFD detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx2_sfd_detected_irqcb(void)
{
  cb_uwbapp_rx2_sfd_detected_irqcb();
}

/**
 * @brief Application callback for UWB RX2 SFD detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 2 SFD detection event.
 */
__WEAK void cb_uwbapp_rx2_sfd_detected_irqcb(void)
{
}

/**
 * @brief Internal callback for UWB RX STS CIR end event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx_sts_cir_end_irqcb(void)
{
  cb_uwbapp_rx_sts_cir_end_irqhandler();
}

/**
 * @brief Application callback for UWB RX STS CIR end event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB RX STS CIR end event.
 */
__WEAK void cb_uwbapp_rx_sts_cir_end_irqhandler(void)
{
}

/**
 * @brief Internal callback for UWB RX PHR detection event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx_phr_detected_irqcb(void)
{
  cb_uwbapp_rx_phr_detected_irqhandler();
}

/**
 * @brief Application callback for UWB RX PHR detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB RX PHR detection event.
 */
__WEAK void cb_uwbapp_rx_phr_detected_irqhandler(void)
{
}

/**
 * @brief Internal callback for general UWB RX done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_rx_done_irqcb(void)
{
  cb_uwbapp_rx_done_irqhandler();
}

/**
 * @brief Application callback for general UWB RX done event
 *
 * Weak function that can be overridden by applications to handle
 * the general UWB RX done event.
 */
__WEAK void cb_uwbapp_rx_done_irqhandler(void)
{
}

/**
 * @brief Internal callback for UWB TX done event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_tx_done_irqcb(void)
{
  cb_uwbapp_tx_done_irqhandler();
}

/**
 * @brief Application callback for UWB TX done event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB TX done event.
 */
__WEAK void cb_uwbapp_tx_done_irqhandler(void)
{
}

/**
 * @brief Internal callback for UWB TX SFD mark event
 *
 * Called by the IRQ handler to invoke the application-specific callback.
 */
void cb_uwb_tx_sfd_mark_irqcb(void)
{
  cb_uwbapp_tx_sfd_mark_irqhandler();
}

/**
 * @brief Application callback for UWB TX SFD mark event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB TX SFD mark event.
 */
__WEAK void cb_uwbapp_tx_sfd_mark_irqhandler(void)
{
}
