/**

 * @file    AppUwbRadar.h

 * @brief   Header file for the Radar Example App.

 * @details This header file declares radar example functions.

 * @author  Chipsbank

 * @date    2024

 */

#ifndef __APP_RADAR_H

#define __APP_RADAR_H

//-------------------------------

// INCLUDE SECTION

//-------------------------------

#include "APP_common.h"

#include "CB_system_types.h"

//-------------------------------

// FUNCTION PROTOTYPE SECTION

//-------------------------------



/**

 * @brief Validates UART args, caches runtime config, and initializes UWB/radar driver.

 *

 * Clamps invalid inputs and prints the final resolved config on UART when enabled.

 * Does not start TX — APP_RadarStart() does that.

 *

 * @param[in] mode_PRF             P[0]: PRF packet mode (0=BPRF/M5P1, 1=HPRF/M4P2, 2=LG4A/M1P2).

 * @param[in] power_code           P[1]: TX power code (0-62).

 * @param[in] scale_bit            P[2]: RX scale bit (recommended 4-6).

 * @param[in] gain_idx             P[3]: RX gain index.

 * @param[in] CIR_tap_start        P[4]: first CIR tap index in UART output (valid 0..240, <= tap_end).

 * @param[in] CIR_tap_end          P[5]: last CIR tap index (inclusive, valid 0..240).

 * @param[in] sensing_mode         P[6]: 0=continuous, 1=frame-based.

 * @param[in] sensing_burst_num    P[7]: bursts averaged per output (clamped to max formula).

 * @param[in] sensing_interval_ms  P[8]: CIR interval in slow-time (ms); valid range 1..10000.

 * @param[in] frame_interval_ms    P[9]: full frame period in mode 1 (ms).

 * @param[in] num_cir_per_frame    P[10]: CIR rows per frame in mode 1 (max 32).

 * @param[in] deep_sleep_en        P[11]: deep sleep during idle (mode 1 only).

 * @param[in] num_rx_mode          P[12]: 1=RX2 only, 2=RX1+RX2.

 * @param[in] preamble_index       P[13]: preamble code index (mode_PRF dependent).
 * @return APP_TRUE if init succeeded, APP_FALSE if validation failed.
 */
uint8_t APP_RadarInit(uint32_t mode_PRF, uint32_t power_code, uint32_t scale_bit, uint32_t gain_idx,

                   uint32_t CIR_tap_start, uint32_t CIR_tap_end, uint32_t sensing_mode,

                   uint32_t sensing_burst_num, uint32_t sensing_interval_ms, uint32_t frame_interval_ms,

                   uint32_t num_cir_per_frame, uint32_t deep_sleep_en, uint32_t num_rx_mode,

                   uint32_t preamble_index);



/**

 * @brief Starts the radar operation.

 *

 * Blocks in a TX-burst loop until APP_RadarStop(), or until one frame completes when

 * deep_sleep_en is set (sensing mode 1 only). CIR read/accumulate runs in

 * cb_uwbapp_tx_sfd_mark_irqhandler(); row finalize and UART output run here in task context.

 */

void APP_RadarStart(void);



/**

 * @brief Stops the radar operation.

 *

 * Clears radarTask_execute so APP_RadarStart() exits its burst loop.

 */

void APP_RadarStop(void);



/**

 * @brief TX SFD mark IRQ — called once per radar burst when TX SFD is sent.

 *

 * Sequence:

 *   1. Stop the radar frame (cb_framework_radar_stop).

 *   2. Read CIR from HW into resultRx1/2 (post-TX capture path).

 *   3. Accumulate I/Q into running sums for burst averaging.

 *   4. Pulse RX reset (cb_framework_radar_reset).

 *   5. Set radarDoneFlag so APP_RadarStart unblocks (row finalize uses radarCirBurstCounter in main loop).

 */

void cb_uwbapp_tx_sfd_mark_irqhandler(void);



#endif /* __APP_RADAR_H */


