/**
 * @file CB_uwb.h
 * @brief Ultra-Wideband (UWB) interrupt handlers interface
 * 
 * This file defines the interface for handling UWB hardware interrupts and provides
 * weak callback functions that can be implemented by applications.
 * 
 * @author ChipsBank
 * @date 2024
 */

#ifndef INC_UWB_H_
#define INC_UWB_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

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
 * @brief IRQ handler for UWB RX0 done event
 *
 * This function is called when the UWB receiver port 0 has completed
 * a packet reception. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx0_done_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX0 preamble detection event
 *
 * This function is called when the UWB receiver port 0 has detected
 * a packet preamble. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx0_preamble_detected_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX0 SFD detection event
 *
 * This function is called when the UWB receiver port 0 has detected
 * a start frame delimiter (SFD). It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx0_sfd_detected_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX1 done event
 *
 * This function is called when the UWB receiver port 1 has completed
 * a packet reception. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx1_done_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX1 preamble detection event
 *
 * This function is called when the UWB receiver port 1 has detected
 * a packet preamble. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx1_preamble_detected_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX1 SFD detection event
 *
 * This function is called when the UWB receiver port 1 has detected
 * a start frame delimiter (SFD). It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx1_sfd_detected_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX2 done event
 *
 * This function is called when the UWB receiver port 2 has completed
 * a packet reception. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx2_done_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX2 preamble detection event
 *
 * This function is called when the UWB receiver port 2 has detected
 * a packet preamble. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx2_preamble_detected_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX2 SFD detection event
 *
 * This function is called when the UWB receiver port 2 has detected
 * a start frame delimiter (SFD). It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx2_sfd_detected_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX STS CIR end event
 *
 * This function is called when the UWB receiver has completed processing
 * the STS (Scrambled Timestamp Sequence) CIR (Channel Impulse Response).
 * It disables the corresponding event and CPU IRQs, then calls the application callback.
 */
void cb_uwb_rx_sts_cir_end_irqhandler(void);

/**
 * @brief IRQ handler for UWB RX PHR detection event
 *
 * This function is called when the UWB receiver has detected a PHR
 * (PHY Header). It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx_phr_detected_irqhandler(void);

/**
 * @brief IRQ handler for general UWB RX done event
 *
 * This function is called when the UWB receiver has completed
 * a packet reception across all ports. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_rx_done_irqhandler(void);

/**
 * @brief IRQ handler for UWB TX done event
 *
 * This function is called when the UWB transmitter has completed
 * sending a packet. It disables the corresponding event and CPU IRQs,
 * then calls the application callback.
 */
void cb_uwb_tx_done_irqhandler(void);

/**
 * @brief IRQ handler for UWB TX SFD mark event
 *
 * This function is called when the UWB transmitter has sent the SFD
 * (Start Frame Delimiter) portion of a packet. It disables the corresponding event
 * and CPU IRQs, then calls the application callback.
 */
void cb_uwb_tx_sfd_mark_irqhandler(void);

/**
 * @brief Application callback for UWB RX0 done event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 0 done event.
 */
void cb_uwbapp_rx0_done_irqcb(void);

/**
 * @brief Application callback for UWB RX0 preamble detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 0 preamble detection event.
 */
void cb_uwbapp_rx0_preamble_detected_irqcb(void);

/**
 * @brief Application callback for UWB RX0 SFD detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 0 SFD detection event.
 */
void cb_uwbapp_rx0_sfd_detected_irqcb(void);

/**
 * @brief Application callback for UWB RX1 done event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 1 done event.
 */
void cb_uwbapp_rx1_done_irqhandler(void);

/**
 * @brief Application callback for UWB RX1 preamble detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 1 preamble detection event.
 */
void cb_uwbapp_rx1_preamble_detected_irqhandler(void);

/**
 * @brief Application callback for UWB RX1 SFD detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 1 SFD detection event.
 */
void cb_uwbapp_rx1_sfd_detected_irqcb(void);

/**
 * @brief Application callback for UWB RX2 done event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 2 done event.
 */
void cb_uwbapp_rx2_done_irqcb(void);

/**
 * @brief Application callback for UWB RX2 preamble detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 2 preamble detection event.
 */
void cb_uwbapp_rx2_preamble_detected_irqhandler(void);

/**
 * @brief Application callback for UWB RX2 SFD detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB receiver port 2 SFD detection event.
 */
void cb_uwbapp_rx2_sfd_detected_irqcb(void);

/**
 * @brief Application callback for UWB RX STS CIR end event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB RX STS CIR end event.
 */
void cb_uwbapp_rx_sts_cir_end_irqhandler(void);

/**
 * @brief Application callback for UWB RX PHR detection event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB RX PHR detection event.
 */
void cb_uwbapp_rx_phr_detected_irqhandler(void);

/**
 * @brief Application callback for general UWB RX done event
 *
 * Weak function that can be overridden by applications to handle
 * the general UWB RX done event.
 */
void cb_uwbapp_rx_done_irqhandler(void);

/**
 * @brief Application callback for UWB TX done event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB TX done event.
 */
void cb_uwbapp_tx_done_irqhandler(void);

/**
 * @brief Application callback for UWB TX SFD mark event
 *
 * Weak function that can be overridden by applications to handle
 * the UWB TX SFD mark event.
 */
void cb_uwbapp_tx_sfd_mark_irqhandler(void);

#endif
