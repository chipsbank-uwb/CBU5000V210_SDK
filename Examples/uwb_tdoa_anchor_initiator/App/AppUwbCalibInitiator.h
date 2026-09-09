/**
 * @file    AppUwbCalibInitiator.h
 * @brief   TDOA initiator boot calibration interface (DS-TWR responder side).
 * @details The TDOA initiator acts as the DS-TWR *responder* (answering party)
 *          during boot calibration.  On receipt of a CAL_REQ the main TDOA loop
 *          hands control here; we send CAL_ACK, then answer each CAL_POLL with
 *          a deferred CAL_RESP, open RX for CAL_FINAL, and finally send CAL_DATA
 *          carrying our own DS-TWR tround/treply so the TDOA responder (DS-TWR
 *          initiator/driver) can close the distance locally.  When the responder
 *          finishes it broadcasts CAL_END with the trimmed-mean bias; we print
 *          lockedSumTpBiasCm and resume the normal TDOA round loop.
 *
 *          Frame sequence  (B = this device / TDOA initiator,
 *                           C = TDOA responder / DS-TWR initiator):
 *
 *            C → B  CAL_REQ  (0x11, non-deferred)
 *            B → C  CAL_ACK  (0x18, non-deferred; TDOA paused)
 *            C → B  CAL_POLL (0x12, non-deferred)
 *            B → C  CAL_RESP (0x13, deferred: POLL SFD + 700 us)
 *            C → B  CAL_FINAL(0x14, deferred: RESP TX_DONE + 500 us on C's side)
 *            B → C  CAL_DATA (0x15, deferred: FINAL SFD + 700 us;
 *                             carries B's tround/treply + bias)
 *            C → B  CAL_END  (0x17, non-deferred; carries lockedSumTpBiasCm)
 *
 *          Absolute-timer schedule (serial reuse of EN_UWB_ABSOLUTE_TIMER_0):
 *            CAL_RESP TX : EN_UWBEVENT_17_RX0_SFD_DET (POLL SFD)  + 700 us → TX
 *            CAL_FINAL RX: EN_UWBEVENT_28_TX_DONE      (RESP done) + 500 us → RX
 *            CAL_DATA TX : EN_UWBEVENT_17_RX0_SFD_DET (FINAL SFD) + 700 us → TX
 *
 *          Watchdog: Timer1, 50 ms one-shot, re-armed at each WAIT_POLL_RX entry.
 *          TX-done guard (P1): delayed TX offset = 1 ms → deadline = tx_start + 3 ms.
 *                              non-deferred TX         → deadline = tx_start + 2 ms.
 *          On guard expiry: tx_end, clear TxDone (P8), resend.
 *          Delayed TX that missed its trigger is resent non-deferred (DS-TWR
 *          distance still closes from measured timestamps; at most one sample lost).
 * @author  Chipsbank
 * @date    2026
 */

#ifndef __APP_UWB_CALIB_INITIATOR_H
#define __APP_UWB_CALIB_INITIATOR_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

/* Lifecycle / control. */
void    app_tdoa_calib_init(void);
void    app_tdoa_calib_start(uint8_t responder_id, uint8_t seq);
void    app_tdoa_calib_stop(void);
void    app_tdoa_calib_process(void);

/* Status queries. */
uint8_t app_tdoa_calib_is_active(void);
uint8_t app_tdoa_calib_is_done(void);

/* IRQ callbacks routed from the main file's UWB/timer handlers. */
void    app_tdoa_calib_tx_done_irqcb(void);
void    app_tdoa_calib_rx_done_irqcb(void);
void    app_tdoa_calib_initiator_timer_irqcb(void);  /* Routed from cb_timer_1_app_irq_callback when calib is active. */

#endif /* __APP_UWB_CALIB_INITIATOR_H */
