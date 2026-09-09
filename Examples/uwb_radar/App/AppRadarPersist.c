/**
 * @file    AppRadarPersist.c
 * @brief   Flash persistence for radar configuration (Radar_sentinel-style addresses).
 */

#include "AppRadarPersist.h"
#include "AppUwbRadar.h"
#include "AppRadarGpio.h"
#include "TaskHandler.h"
#include "CB_flash.h"

#define RADAR_NV_MAGIC         0x55AAU
#define RADAR_NV_ADDR          0x0007E000U

static radar_nv_config_st s_nvMirror;
static uint8_t s_nvMirrorValid = 0U;

static uint8_t AppRadarPersist_IsValid(const radar_nv_config_st *cfg)
{
  return (cfg != 0) && (cfg->magic == RADAR_NV_MAGIC) && ((cfg->flags & RADAR_NV_FLAG_VALID) != 0U);
}

static void AppRadarPersist_EraseAndWrite(const radar_nv_config_st *cfg)
{
  radar_nv_config_st local = *cfg;

  (void)cb_flash_erase_sector((uint16_t)(RADAR_NV_ADDR / 0x1000U));
  (void)cb_flash_program_by_addr(RADAR_NV_ADDR, (uint8_t *)&local, (uint16_t)sizeof(local));
}

static uint8_t AppRadarPersist_Load(radar_nv_config_st *cfg)
{
  if (cfg == 0)
  {
    return 0U;
  }

  (void)cb_flash_read_by_addr(RADAR_NV_ADDR, (uint8_t *)cfg, (uint16_t)sizeof(radar_nv_config_st));
  return AppRadarPersist_IsValid(cfg) ? 1U : 0U;
}

static void AppRadarPersist_Save(const radar_nv_config_st *cfg)
{
  if ((cfg == 0) || !AppRadarPersist_IsValid(cfg))
  {
    return;
  }

  s_nvMirror = *cfg;
  s_nvMirrorValid = 1U;
  AppRadarPersist_EraseAndWrite(cfg);
}

/**
 * @brief Initializes flash access and loads the cached NV mirror.
 */
void AppRadarPersist_Init(void)
{
  (void)cb_flash_init();
  s_nvMirrorValid = AppRadarPersist_Load(&s_nvMirror);
}

void AppRadarPersist_SaveFromUartArgs(uint32_t mode_PRF, uint32_t power_code, uint32_t scale_bit,
                                      uint32_t gain_idx, uint32_t CIR_tap_start, uint32_t CIR_tap_end,
                                      uint32_t sensing_mode, uint32_t sensing_burst_num,
                                      uint32_t sensing_interval_ms, uint32_t frame_interval_ms,
                                      uint32_t num_cir_per_frame, uint32_t deep_sleep_en, uint32_t num_rx_mode,
                                      uint32_t preamble_index)
{
  radar_nv_config_st cfg = {0};

  if (deep_sleep_en != 1U)
  {
    return;
  }

  cfg.magic = RADAR_NV_MAGIC;
  cfg.flags = RADAR_NV_FLAG_VALID | RADAR_NV_FLAG_AUTORUN | RADAR_NV_FLAG_DEEPSLEEP;

  cfg.mode_PRF = mode_PRF;
  cfg.num_rx_mode = (num_rx_mode == 0U) ? 2U : (uint8_t)num_rx_mode;
  cfg.power_code = power_code;
  cfg.scale_bit = scale_bit;
  cfg.gain_idx = gain_idx;
  cfg.CIR_tap_start = CIR_tap_start;
  cfg.CIR_tap_end = CIR_tap_end;
  cfg.sensing_mode = sensing_mode;
  cfg.sensing_burst_num = sensing_burst_num;
  cfg.sensing_interval_ms = sensing_interval_ms;
  cfg.frame_interval_ms = frame_interval_ms;
  cfg.num_cir_per_frame = num_cir_per_frame;
  cfg.preamble_index = (preamble_index == 0U) ? EN_UWB_PREAMBLE_CODE_IDX_9 : preamble_index;

  AppRadarPersist_Save(&cfg);
}

/** @brief Clears auto-run and deep-sleep flags in the stored record. */
void AppRadarPersist_DisablePersistMode(void)
{
  if (s_nvMirrorValid == 0U)
  {
    if (AppRadarPersist_Load(&s_nvMirror) == 0U)
    {
      return;
    }
    s_nvMirrorValid = 1U;
  }

  s_nvMirror.flags &= (uint8_t)~(RADAR_NV_FLAG_AUTORUN | RADAR_NV_FLAG_DEEPSLEEP);
  AppRadarPersist_Save(&s_nvMirror);
}

static uint8_t AppRadarPersist_ApplyInit(const radar_nv_config_st *cfg)
{
  uint32_t deep_sleep_en = ((cfg->flags & RADAR_NV_FLAG_DEEPSLEEP) != 0U) ? 1U : 0U;
  uint32_t num_rx_mode = (cfg->num_rx_mode == 0U) ? 2U : cfg->num_rx_mode;
  uint32_t preamble_index = (cfg->preamble_index == 0U) ? EN_UWB_PREAMBLE_CODE_IDX_9 : cfg->preamble_index;

  return APP_RadarInit(cfg->mode_PRF, cfg->power_code, cfg->scale_bit, cfg->gain_idx,
                       cfg->CIR_tap_start, cfg->CIR_tap_end, cfg->sensing_mode, cfg->sensing_burst_num,
                       cfg->sensing_interval_ms, cfg->frame_interval_ms, cfg->num_cir_per_frame,
                       deep_sleep_en, num_rx_mode, preamble_index);
}

/**
 * @brief Loads NV config, calls @ref APP_RadarInit, and sets @c g_task_f_execute.
 * @return 1 if auto-start was armed, otherwise 0.
 */
uint8_t AppRadarPersist_TryAutoStart(void)
{
  radar_nv_config_st cfg;

  if (AppRadarGpio_IsRunAllowed() == 0U)
  {
    return 0U;
  }

  if (AppRadarPersist_Load(&cfg) == 0U)
  {
    return 0U;
  }

  if ((cfg.flags & (RADAR_NV_FLAG_AUTORUN | RADAR_NV_FLAG_DEEPSLEEP)) !=
      (RADAR_NV_FLAG_AUTORUN | RADAR_NV_FLAG_DEEPSLEEP))
  {
    return 0U;
  }

  s_nvMirror = cfg;
  s_nvMirrorValid = 1U;
  g_task_f_execute = AppRadarPersist_ApplyInit(&cfg);
  return (g_task_f_execute == APP_TRUE) ? 1U : 0U;
}
