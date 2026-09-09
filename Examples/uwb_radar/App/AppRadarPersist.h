/**
 * @file    AppRadarPersist.h
 * @brief   Non-volatile storage for radar UART configuration and auto-run state.
 * @details Persists the last UART `f` command parameters to flash so the device can
 *          auto-resume radar operation after deep sleep when GPIO7 is high.
 */
#ifndef __APP_RADAR_PERSIST_H
#define __APP_RADAR_PERSIST_H
#include <stdint.h>
/** @brief Record contains valid radar configuration. */
#define RADAR_NV_FLAG_VALID      0x01U
/** @brief Auto-start radar on boot when GPIO run gate is enabled. */
#define RADAR_NV_FLAG_AUTORUN    0x02U
/** @brief Saved configuration uses deep-sleep frame timing. */
#define RADAR_NV_FLAG_DEEPSLEEP  0x04U
/**
 * @brief Flash-backed radar configuration snapshot (P[0]..P[10], num_rx_mode).
 */
typedef struct
{
  uint16_t magic;              /**< Must be @c 0x55AA for a valid record. */
  uint8_t  flags;              /**< @ref RADAR_NV_FLAG_VALID and related flags. */
  uint8_t  num_rx_mode;        /**< P[12]: 1=1T1R, 2=1T2R; 0 defaults to 1T2R on load. */
  uint32_t mode_PRF;           /**< P[0] */
  uint32_t power_code;         /**< P[1] */
  uint32_t scale_bit;          /**< P[2] */
  uint32_t gain_idx;           /**< P[3] */
  uint32_t CIR_tap_start;      /**< P[4] */
  uint32_t CIR_tap_end;        /**< P[5] */
  uint32_t sensing_mode;       /**< P[6] */
  uint32_t sensing_burst_num;  /**< P[7] */
  uint32_t sensing_interval_ms;/**< P[8] */
  uint32_t frame_interval_ms;  /**< P[9] */
  uint32_t num_cir_per_frame;  /**< P[10] */
  uint32_t preamble_index;     /**< P[13] */
} radar_nv_config_st;
/**
 * @brief Initializes flash access and loads the cached NV mirror.
 */
void AppRadarPersist_Init(void);
/**
 * @brief Builds and saves an NV record from UART `f` command arguments.
 *
 * No-op unless @p deep_sleep_en is 1.
 */
void AppRadarPersist_SaveFromUartArgs(uint32_t mode_PRF, uint32_t power_code, uint32_t scale_bit,
                                    uint32_t gain_idx, uint32_t CIR_tap_start, uint32_t CIR_tap_end,
                                    uint32_t sensing_mode, uint32_t sensing_burst_num,
                                    uint32_t sensing_interval_ms, uint32_t frame_interval_ms,
                                    uint32_t num_cir_per_frame, uint32_t deep_sleep_en, uint32_t num_rx_mode,
                                    uint32_t preamble_index);
/** @brief Clears auto-run and deep-sleep flags in the stored record. */
void AppRadarPersist_DisablePersistMode(void);
/**
 * @brief Loads NV config, calls @ref APP_RadarInit, and sets @c g_task_f_execute.
 * @return 1 if auto-start was armed, otherwise 0.
 */
uint8_t AppRadarPersist_TryAutoStart(void);
#endif /* __APP_RADAR_PERSIST_H */
