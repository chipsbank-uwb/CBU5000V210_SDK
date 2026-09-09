/**
 * @file    AppUwbCalibResponder.h
 * @brief   TDOA responder boot calibration interface.
 * @details The responder drives the DS-TWR calibration exchange (as initiator
 *          side), opens a fixed 3-second sample window after the first CAL_ACK,
 *          computes distance locally, and locks `sum_tp_bias_cm` for normal
 *          TDOA mode. The TDOA initiator acts as the DS-TWR responder side.
 * @author  Chipsbank
 * @date    2026
 */

#ifndef __APP_UWB_CALIB_RESPONDER_H
#define __APP_UWB_CALIB_RESPONDER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>

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
void    app_tdoa_calib_init                 (float default_sum_tp_bias_cm);
void    app_tdoa_calib_process              (void);

uint8_t app_tdoa_calib_is_active            (void);
uint8_t app_tdoa_calib_is_done              (void);
uint8_t app_tdoa_calib_is_locked            (void);

float   app_tdoa_calib_get_sum_tp_bias_cm   (void);

void    app_tdoa_calib_tx_done_irqcb        (void);
void    app_tdoa_calib_rx_done_irqcb        (void);
void    app_tdoa_calib_timer_irqcb          (void);

#endif /* __APP_UWB_CALIB_RESPONDER_H */
