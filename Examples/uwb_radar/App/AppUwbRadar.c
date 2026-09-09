/**
 * @file    AppUwbRadar.c
 * @brief   UWB radar example — CIR capture, burst averaging, and UART output.
 *
 * @details
 * High-level flow:
 *   1. UART command (AppSysUartCommander) calls APP_RadarInit(); on success sets g_task_f_execute.
 *   2. TaskHandler() calls APP_RadarStart(), which runs the main radar loop until stopped.
 *   3. Each burst (APP_RadarLaunchBurst):
 *        a) cb_framework_radar_config() then cb_framework_radar_start()
 *        b) TX SFD IRQ (cb_uwbapp_tx_sfd_mark_irqhandler) stops the frame, reads CIR,
 *           accumulates sums, resets RX, and sets radarDoneFlag.
 *   4. a) When sensing mode is 0 (continuous mode), averaged CIR is copied to result buffers and sent on UART
 *      b) When sensing mode is 1 (frame-based mode), averaged CIR is copied to result buffers and 
 *            only sent on UART once the entire frame is complete.
 *
 * Buffer roles:
 *   - resultRx1/2           : CIR staging for UART (IRQ: latest burst; main: finalized row copy)
 *   - accumulatingSumRx1/2  : int32 running sums while averaging bursts within one row
 *   - accumulatedResultRx1/2: finalized int16 row averages (sensing mode 0 uses row 0; sensing mode 1 up to 32 rows)
 *
 * IRQ vs main loop:
 *   The TX SFD ISR stops the frame, reads CIR, and accumulates sums. Row finalize
 *   (divide sums, store row) runs in APP_RadarStart when radarCirBurstCounter
 *   reaches sensing_burst_num.
 *
 * Timing parameters (P[8], P[9]):
 *   - sensing_interval_ms : mode 0 = gap between UART CIR outputs; mode 1 = gap between rows
 *     Valid range 1 .. 10000 ms (10 s hard limit).
 *   - frame_interval_ms   : full multi-row frame period (sensing mode 1 only)
 *
 * Sensing modes (P[6] sensing_mode):
 *   - Mode 0: runs continuously, each CIR cycle is spaced by sensing_interval_ms (deep sleep disabled). Each CIR cycle is sent to UART immediately.
 *   - Mode 1: collects one frame of CIRs arranged in a two-dimensional array. Each row of the array corresponds to one CIR. 
 * 			   Each column corresponds to the CIR taps. The total number of rows is "num_cir_per_frame".
 *             Each row has an interval of sensing_interval_ms.
			   Optional deep sleep after one full frame (P[11] deep_sleep_en).
 *   - For both modes, one CIR is an average of multiple bursts if sensing_burst_num>1.		
 *
 * Burst averaging (P[7] sensing_burst_num):
 *   - Bursts within one row run back-to-back; max count is clamped using:
 *       T_burst_us = ceil((10400 + (2800 + 57*N)*num_rx_mode) / 100)
 *       max = floor(sensing_interval_ms * 1000 / T_burst_us)
 *     where N = export tap count and num_rx_mode is 1 (1T1R) or 2 (1T2R).
 *
 * Tap indices (P[4], P[5]):
 *   - CIR_tap_start: first tap in UART export (inclusive); valid 0..tap_end, max index 240
 *   - CIR_tap_end: last tap for driver align and UART export (inclusive); valid 0..240
 *     (APP_RADAR_MAX_CIR_TAP_INDEX). Export uses s_CIR_tap_start..s_CIR_tap_end.
 *
 * Data pipeline (per burst):
 *   cb_framework_radar_getcir() -> resultRx1/2 -> accumulatingSumRx1/2
 *   -> (finalize row) accumulatedResultRx1/2[row] -> resultRx1/2 -> APP_RadarOutputCirUart()
 * 
 * ModePRF (P[0]):
 *   - 0 = BPRF (M5P1), 1 = HPRF (M4P2), 2 = LG4A / M1P2
 * Preamble Index (P[13]):
 *   - mode_PRF 0 (M5P1 / BPRF): preamble 9-24
 *   - mode_PRF 1 (M4P2 / HPRF): preamble 25-32
 *   - mode_PRF 2 (M1P2 / LG4A): preamble 1-8
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "AppUwbRadar.h"
#include "string.h"
#include "NonLIB_sharedUtils.h"
#include "CB_system.h"
#include "CB_uwbframework.h"
#include "CB_UwbDrivers.h"
#include "CB_SleepDeepSleep.h"
#include "AppRadarPersist.h"
#include "AppRadarGpio.h"
#include "TaskHandler.h"
#include "APP_CompileOption.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#ifndef APP_UWB_RADAR_UARTPRINT_ENABLE
#define APP_UWB_RADAR_UARTPRINT_ENABLE APP_TRUE  /* APP_FALSE strips all UART status prints */
#endif

#ifndef APP_RADAR_ENABLE_ALIGN_DETECT
#define APP_RADAR_ENABLE_ALIGN_DETECT  APP_FALSE  /* APP_FALSE = skip ratio detect in get_cir (align still applied) */
#endif

#if (APP_UWB_RADAR_UARTPRINT_ENABLE == APP_TRUE)
#include "app_uart.h"
#define APP_UWB_RADAR_PRINT(...)  app_uart_printf(__VA_ARGS__)
#define APP_UWB_RADAR_CIR_PRINT(...)  app_uart_printf(__VA_ARGS__)
#else
#define APP_UWB_RADAR_PRINT(...)
#define APP_UWB_RADAR_CIR_PRINT(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define APP_RADAR_MAX_CIR_PER_FRAME 32U   /* max CIR rows per frame in sensing mode 1 */
#define APP_RADAR_MIN_SENSING_INTERVAL_MS 1U
#define APP_RADAR_MAX_SENSING_INTERVAL_MS 10000U /* 10 s hard limit for P[8] */
#define APP_RADAR_MAX_CIR_TAP 236U        /* CIR array slots (indices 0..235) */
#define APP_RADAR_MAX_CIR_TAP_INDEX (APP_RADAR_MAX_CIR_TAP - 1U) /* max tap index accepted by APP_RadarInit (240) */
static uint8_t s_CIR_tap_end = 47;       /* last tap index for driver align + UART export (inclusive) */
static uint8_t s_CIR_tap_start = 0;      /* first tap index included in UART export only */
static uint8_t s_num_cir_per_frame = 32;  /* P[10]: rows per frame when sensing_mode == 1 */

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/* Packed 16-bit I/Q for binary UART export (active #if 1 path in OutputCirUart) */
typedef struct {
    uint16_t Q_data;
    uint16_t I_data;
  } __attribute__((aligned(4))) printformat_cir_st;

/* Running sum of I/Q while averaging bursts within one CIR row */
typedef struct {
    int32_t I_data;
    int32_t Q_data;
  } __attribute__((aligned(4))) cir_sum_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
/* Latest/finalized CIR staging for UART output */
static cb_uwbsystem_rx_cir_iqdata_st resultRx1[APP_RADAR_MAX_CIR_TAP];
static cb_uwbsystem_rx_cir_iqdata_st resultRx2[APP_RADAR_MAX_CIR_TAP];

/* Packed I/Q staging for binary UART export (big-endian 16-bit samples) */
static printformat_cir_st print_resultcirrx1[APP_RADAR_MAX_CIR_TAP];
static printformat_cir_st print_resultcirrx2[APP_RADAR_MAX_CIR_TAP];
	
static uint8_t print_buffer[APP_RADAR_MAX_CIR_TAP*2];

static const uint8_t UartDataStartMarker_A[7] = {0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0x00};

/* --- Burst averaging accumulators (summed in IRQ, divided in FinalizeActiveRow) --- */
static cir_sum_st accumulatingSumRx1[APP_RADAR_MAX_CIR_TAP];
static cir_sum_st accumulatingSumRx2[APP_RADAR_MAX_CIR_TAP];

/* --- Per-frame storage: one averaged CIR row per index (sensing mode 1) --- */
static cb_uwbsystem_rx_cir_iqdata_st accumulatedResultRx1[APP_RADAR_MAX_CIR_PER_FRAME][APP_RADAR_MAX_CIR_TAP];
static cb_uwbsystem_rx_cir_iqdata_st accumulatedResultRx2[APP_RADAR_MAX_CIR_PER_FRAME][APP_RADAR_MAX_CIR_TAP];

/* --- Runtime config P[0]..P[13] (set in APP_RadarInit from UART args) --- */
static uint32_t s_sensing_interval_ms = 0;  /* mode 0: CIR output gap; mode 1: row gap */
static volatile uint8_t radarDoneFlag = APP_FALSE;   /* set by IRQ when one burst completes */
static uint8_t radarTask_execute = APP_FALSE;        /* main loop exit flag                  */
static uint32_t s_gain_idx;
static uint32_t packetSeqNum = 0;   /* increments on each UART CIR output              */
static uint32_t frameNum = 0;       /* low 8 bits packed into binary UART header */
static uint32_t s_sensing_burst_num = 1;   /* bursts averaged into one CIR row      */
static uint32_t radarCirBurstCounter = 0;   /* bursts collected for current row (IRQ) */
static uint32_t radarNumSensingCounter = 0; /* finalized CIR rows in current frame     */
static uint32_t s_frame_interval_ms = 100;  /* mode 1: full frame period            */
static uint8_t s_deep_sleep_en = 0;         /* mode 1 only: 1 = one frame then deep sleep */
static uint8_t s_mode_PRF = 0;
static uint8_t s_sensing_mode = 0;         /* 0 = continuous; 1 = frame-based multi-row */
static uint8_t s_power_code = 0;
static uint8_t s_scale_bit = 0;
static uint8_t s_num_rx_mode = CB_DRIVER_RADAR_USE_1T2R;  /* 1T1R or 1T2R */
static uint8_t s_preamble_index = EN_UWB_PREAMBLE_CODE_IDX_9;

//-------------------------------
// LOCAL HELPERS
//-------------------------------
/** @return 1 if configured for 1T1R (RX2 only), 0 for 1T2R (RX1 + RX2). */
static uint8_t APP_RadarIs1T1R(void)
{
  return (s_num_rx_mode == CB_DRIVER_RADAR_USE_1T1R) ? 1U : 0U;
}

/* Tap count for driver getcir: (s_CIR_tap_end + 1), capped at APP_RADAR_MAX_CIR_TAP_INDEX + 1 (241) */
static uint32_t APP_RadarGetAlignSampleCount(void)
{
  uint32_t sample_count = (uint32_t)s_CIR_tap_end + 1U;

  if (sample_count > (APP_RADAR_MAX_CIR_TAP_INDEX + 1U))
  {
    sample_count = APP_RADAR_MAX_CIR_TAP_INDEX + 1U;
  }

  return sample_count;
}

/* Number of taps actually printed on UART (CIR_tap_start..CIR_tap_end inclusive) */
static uint32_t APP_RadarGetExportTapCount(void)
{
  return (uint32_t)s_CIR_tap_end - (uint32_t)s_CIR_tap_start + 1U;
}

//-------------------------------
// FUNCTION PROTOTYPE SECTION — CIR pipeline (see function bodies for flow)
//-------------------------------
static void APP_RadarGetCirFromHw(void);
static void APP_RadarAccumulateCir(void);
static void APP_RadarFinalizeActiveRow(uint32_t row);
static void APP_RadarFillResultFromAccumulated(uint32_t row);
static void APP_RadarOutputCirUart(uint32_t row);

static void APP_RadarLaunchBurst(cb_uwbsystem_tx_irqenable_st *stTxIrqEnable);

/** Configure TX SFD IRQ, apply radar config, and start one burst. */
static void APP_RadarLaunchBurst(cb_uwbsystem_tx_irqenable_st *stTxIrqEnable)
{
  cb_system_uwb_configure_tx_irq(stTxIrqEnable);

  cb_framework_radar_config(s_scale_bit, s_mode_PRF,
                            (cb_uwbsystem_preamblecodeidx_en)s_preamble_index);
  cb_framework_radar_start(s_gain_idx);
}

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Validates UART args, caches runtime config, and initializes UWB/radar driver.
 *
 * Clamps invalid inputs (tap range, burst count, num_cir_per_frame, deep_sleep_en in mode 0).
 * Prints the final resolved config on UART, then calls cb_framework_radar_on().
 * UWB stack init (cb_framework_uwb_init) is done in main() before radar starts.
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
                   uint32_t preamble_index)
{
  if (sensing_interval_ms < APP_RADAR_MIN_SENSING_INTERVAL_MS ||
      sensing_interval_ms > APP_RADAR_MAX_SENSING_INTERVAL_MS)
  {
    APP_UWB_RADAR_PRINT("Invalid sensing_interval_ms:%u, valid range %u..%u\n",
                        sensing_interval_ms,
                        APP_RADAR_MIN_SENSING_INTERVAL_MS,
                        APP_RADAR_MAX_SENSING_INTERVAL_MS);
    return APP_FALSE;
  }

	/* --- Cache runtime config from UART args --- */
	s_power_code = (uint8_t)power_code;
	s_scale_bit = (uint8_t)scale_bit;
  s_sensing_interval_ms = sensing_interval_ms;
  s_gain_idx = gain_idx;
  s_sensing_mode = (uint8_t)sensing_mode;

  s_frame_interval_ms = frame_interval_ms;
  s_num_rx_mode = (num_rx_mode == CB_DRIVER_RADAR_USE_1T1R) ? CB_DRIVER_RADAR_USE_1T1R : CB_DRIVER_RADAR_USE_1T2R;

  if (CIR_tap_end <= APP_RADAR_MAX_CIR_TAP_INDEX)
  {
    s_CIR_tap_end = (uint8_t)CIR_tap_end;
  }
  else
  {
    APP_UWB_RADAR_PRINT("Invalid CIR_tap_end:%d, ..%u, keeping %d\n",
                        CIR_tap_end, APP_RADAR_MAX_CIR_TAP_INDEX, s_CIR_tap_end);
		
		return APP_FALSE;
  }

  if (CIR_tap_start <= s_CIR_tap_end && CIR_tap_start <= APP_RADAR_MAX_CIR_TAP_INDEX)
  {
    s_CIR_tap_start = (uint8_t)CIR_tap_start;
  }
  else
  {
    s_CIR_tap_start = 0U;
    APP_UWB_RADAR_PRINT("Invalid CIR_tap_start:%d, valid range 0..%u, using 0\n",
                        CIR_tap_start, APP_RADAR_MAX_CIR_TAP_INDEX);
		
		return APP_FALSE;
  }

  /* T_burst_us = ceil((10400 + (2800 + 57*N)*num_rx_mode) / 100); max = floor(sensing_interval_ms*1000 / T_burst_us) */
  uint32_t maxAllowedSensingBurstNum;
  {
    uint32_t export_tap_count = APP_RadarGetExportTapCount();
    uint32_t t_burst_us;

    t_burst_us = (10400U + ((2800U + (57U * export_tap_count)) * (uint32_t)s_num_rx_mode) + 99U) / 100U;
    if (t_burst_us == 0U)
    {
      maxAllowedSensingBurstNum = 1U;
    }
    else
    {
      maxAllowedSensingBurstNum = (sensing_interval_ms * 1000U) / t_burst_us;
    }
    if (maxAllowedSensingBurstNum < 1U)
    {
      maxAllowedSensingBurstNum = 1U;
    }
  }

  if (sensing_burst_num < 1)
  {
    s_sensing_burst_num = 1;
    APP_UWB_RADAR_PRINT("Invalid sensing_burst_num:%d, using default value:%d\n", sensing_burst_num, s_sensing_burst_num);
  }
  else if (sensing_burst_num > maxAllowedSensingBurstNum)
  {
    s_sensing_burst_num = maxAllowedSensingBurstNum;
    APP_UWB_RADAR_PRINT("Invalid sensing_burst_num:%d, using default value:%d\n", sensing_burst_num, s_sensing_burst_num);
  }
  else
  {
    s_sensing_burst_num = sensing_burst_num;
  }

  if (num_cir_per_frame < 1 || num_cir_per_frame > APP_RADAR_MAX_CIR_PER_FRAME)
  {
    s_num_cir_per_frame = APP_RADAR_MAX_CIR_PER_FRAME;
    APP_UWB_RADAR_PRINT("Invalid num_cir_per_frame:%d, using default value:%d\n", num_cir_per_frame, s_num_cir_per_frame);
  }
  else
  {
    s_num_cir_per_frame = (uint8_t)num_cir_per_frame;
  }  
  
  /* Reset accumulation buffers for a fresh run */
  memset(accumulatingSumRx2, 0, sizeof(accumulatingSumRx2));
  memset(accumulatedResultRx2, 0, sizeof(accumulatedResultRx2));
  if (APP_RadarIs1T1R() == 0U)
  {
    memset(accumulatingSumRx1, 0, sizeof(accumulatingSumRx1));
    memset(accumulatedResultRx1, 0, sizeof(accumulatedResultRx1));
  }

  s_deep_sleep_en = (deep_sleep_en == 1U) ? 1U : 0U;
  if (s_sensing_mode == 0U)
  {
    /* Deep sleep is only supported in sensing mode 1 */
    if (s_deep_sleep_en != 0U)
    {
      APP_UWB_RADAR_PRINT("deep_sleep_en ignored in sensing_mode 0\n");
    }
    s_deep_sleep_en = 0U;
  }

  if (mode_PRF == 0U)
  {
    if ((preamble_index < EN_UWB_PREAMBLE_CODE_IDX_9) || (preamble_index > EN_UWB_PREAMBLE_CODE_IDX_24))
    {
      APP_UWB_RADAR_PRINT("Invalid preamble_index:%u for mode_PRF 0 (M5P1), valid range 9-24\n", preamble_index);
      return APP_FALSE;
    }
  }
  else if (mode_PRF == 1U)
  {
    if ((preamble_index < EN_UWB_PREAMBLE_CODE_IDX_25) || (preamble_index > EN_UWB_PREAMBLE_CODE_IDX_32))
    {
      APP_UWB_RADAR_PRINT("Invalid preamble_index:%u for mode_PRF 1 (M4P2), valid range 25-32\n", preamble_index);
      return APP_FALSE;
    }
  }
  else if (mode_PRF == 2U)
  {
    if (preamble_index < EN_UWB_PREAMBLE_CODE_IDX_1 || preamble_index  > EN_UWB_PREAMBLE_CODE_IDX_8)
    {
      APP_UWB_RADAR_PRINT("Invalid preamble_index:%u for mode_PRF 2 (M1P2), valid value 1-8\n", preamble_index);
      return APP_FALSE;
    }
  }
  else
  {
    APP_UWB_RADAR_PRINT("Invalid mode_PRF:%u, valid values 0-2\n", mode_PRF);
    return APP_FALSE;
  }

  s_mode_PRF = (uint8_t)mode_PRF;
  s_preamble_index = (uint8_t)preamble_index;

  /* --- Final config after validation --- */
  APP_UWB_RADAR_PRINT("mode_PRF value: %d\n", s_mode_PRF);
  APP_UWB_RADAR_PRINT("power_code value: %d\n", s_power_code);
  APP_UWB_RADAR_PRINT("scale_bit value: %d\n", s_scale_bit);
  APP_UWB_RADAR_PRINT("gain_idx value: %u\n", s_gain_idx);
  APP_UWB_RADAR_PRINT("CIR_tap_start value: %d\n", s_CIR_tap_start);
  APP_UWB_RADAR_PRINT("CIR_tap_end value: %d\n", s_CIR_tap_end);
  APP_UWB_RADAR_PRINT("sensing_mode value: %d\n", s_sensing_mode);
  APP_UWB_RADAR_PRINT("sensing_burst_num value: %u\n", s_sensing_burst_num);
  APP_UWB_RADAR_PRINT("sensing_interval_ms value: %u\n", s_sensing_interval_ms);
  APP_UWB_RADAR_PRINT("frame_interval_ms value: %u\n", s_frame_interval_ms);
  APP_UWB_RADAR_PRINT("num_cir_per_frame value: %d\n", s_num_cir_per_frame);
  APP_UWB_RADAR_PRINT("deep_sleep_en value: %d\n", s_deep_sleep_en);
  APP_UWB_RADAR_PRINT("num_rx_mode value: %u\n", s_num_rx_mode);
  APP_UWB_RADAR_PRINT("preamble_index value: %u\n", s_preamble_index);

  /*
   * IndexRef: CIR tap index for zero-range after first-CIR shift/truncate (new release).
   *
   *   RefIndexRegister = 21 + 24 = 45
   *   PD_SYNC_IDX        = 285 - RefIndexRegister = 240  (RX_TOP->MODE_RADAR[pd_sync_idx])
   *   IndexRef           = 270 - PD_SYNC_IDX = 30
   *
   * Same IndexRef from align math (example diffRx = 17):
   *   IndexRef = RefIndexRegister + diffRx + 9 - (diffRx - 8) - 32
   *            = 45 + 17 - 32 = 30
   */

  APP_UWB_RADAR_PRINT("IndexRef value: 30\n");

  cb_framework_radar_on(s_power_code, s_num_rx_mode);

  return APP_TRUE;
}

/**
 * @brief Read CIR taps from the driver into resultRx1/2 after a burst (ISR path).
 *
 * Calls cb_framework_radar_getcir() for RX1/RX2 (or RX2 only in 1T1R).
 * alignSampleCount is APP_RadarGetAlignSampleCount() — (s_CIR_tap_end + 1), max 241.
 * The driver applies first-CIR alignment internally; return status is ignored here.
 */
static void APP_RadarGetCirFromHw(void)
{
  uint32_t alignSampleCount = APP_RadarGetAlignSampleCount();

  if (APP_RadarIs1T1R() != 0U)
  {
    (void)cb_framework_radar_getcir(&resultRx2[0], EN_UWB_RX_2, alignSampleCount, APP_RADAR_ENABLE_ALIGN_DETECT);
  }
  else
  {
    (void)cb_framework_radar_getcir(&resultRx1[0], EN_UWB_RX_1, alignSampleCount, APP_RADAR_ENABLE_ALIGN_DETECT);
    (void)cb_framework_radar_getcir(&resultRx2[0], EN_UWB_RX_2, alignSampleCount, APP_RADAR_ENABLE_ALIGN_DETECT);
  }
}

/** Add latest resultRx* burst into accumulatingSum* (called from ISR). */
static void APP_RadarAccumulateCir(void)
{
  uint32_t i;

  if (APP_RadarIs1T1R() == 0U)
  {
    for (i = (uint32_t)s_CIR_tap_start; i <= (uint32_t)s_CIR_tap_end; ++i)
    {
      accumulatingSumRx1[i].I_data += (int32_t)resultRx1[i].I_data;
      accumulatingSumRx1[i].Q_data += (int32_t)resultRx1[i].Q_data;
    }
  }

  for (i = (uint32_t)s_CIR_tap_start; i <= (uint32_t)s_CIR_tap_end; ++i)
  {
    accumulatingSumRx2[i].I_data += (int32_t)resultRx2[i].I_data;
    accumulatingSumRx2[i].Q_data += (int32_t)resultRx2[i].Q_data;
  }
}

/**
 * @brief Divide accumulated sums by burst count and store into accumulatedResult[row].
 *
 * Called when sensing_burst_num bursts have been collected for one CIR row.
 * Clears the sum buffers afterward so the next row starts fresh.
 */
static void APP_RadarFinalizeActiveRow(uint32_t row)
{
  int32_t divisor = (int32_t)s_sensing_burst_num;
  uint32_t i;

  if (divisor < 1)
  {
    divisor = 1;
  }

  for (i = (uint32_t)s_CIR_tap_start; i <= (uint32_t)s_CIR_tap_end; ++i)
  {
    if (APP_RadarIs1T1R() == 0U)
    {
      accumulatedResultRx1[row][i].I_data = (int16_t)(accumulatingSumRx1[i].I_data / divisor);
      accumulatedResultRx1[row][i].Q_data = (int16_t)(accumulatingSumRx1[i].Q_data / divisor);
    }

    accumulatedResultRx2[row][i].I_data = (int16_t)(accumulatingSumRx2[i].I_data / divisor);
    accumulatedResultRx2[row][i].Q_data = (int16_t)(accumulatingSumRx2[i].Q_data / divisor);
  }

  memset(accumulatingSumRx2, 0, sizeof(accumulatingSumRx2));
  if (APP_RadarIs1T1R() == 0U)
  {
    memset(accumulatingSumRx1, 0, sizeof(accumulatingSumRx1));
  }
}

/** Copy one finalized row from accumulatedResult into resultRx* for UART output. */
static void APP_RadarFillResultFromAccumulated(uint32_t row)
{
  uint32_t exportTapCount = APP_RadarGetExportTapCount();

  if (APP_RadarIs1T1R() == 0U)
  {
    memcpy(&resultRx1[s_CIR_tap_start], &accumulatedResultRx1[row][s_CIR_tap_start],
           exportTapCount * sizeof(resultRx1[0]));
  }

  memcpy(&resultRx2[s_CIR_tap_start], &accumulatedResultRx2[row][s_CIR_tap_start],
         exportTapCount * sizeof(resultRx2[0]));
}

/**
 * @brief Send current resultRx* CIR taps to UART.
 *
 * Active path (#if 1): binary stream for host-side parsing tools.
 *   [7-byte marker A][4-byte header][I taps RX1][Q taps RX1][I taps RX2][Q taps RX2]
 *   Header = (packetSeqNum[23:0] << 8) | frameNum[7:0], big-endian on wire.
 *   Each I/Q block is big-endian int16 for taps CIR_tap_start..CIR_tap_end.
 *   1T1R: RX2 data is mapped into the RX1 slot (no RX2 blocks sent).
 *
 * Alternate path (#else): human-readable comma-separated I/Q text.
 */
static void APP_RadarOutputCirUart(uint32_t row)
{
  uint32_t printResultStartIndex = s_CIR_tap_start;
  uint32_t printResultEndIndex = (uint32_t)s_CIR_tap_end + 1U;
  uint16_t printByteCount = (uint16_t)((printResultEndIndex - printResultStartIndex) * 2);
  uint32_t packed;

  if (APP_RadarIs1T1R() == 0U)
  {
    memcpy(&print_resultcirrx1, &resultRx1, sizeof(resultRx1));
    memcpy(&print_resultcirrx2, &resultRx2, sizeof(resultRx2));
  }
  else
  {
    /* 1T1R: map RX2 into primary print slot so host parser sees one RX chain */
    memcpy(&print_resultcirrx1, &resultRx2, sizeof(resultRx2));
  }

	#if 1  /* Binary UART export — set to 0 for human-readable text output */

  app_uart_output_raw((uint8_t *)UartDataStartMarker_A, sizeof(UartDataStartMarker_A));

  packetSeqNum++;
  if (packetSeqNum >= 0x00800000)
  {
    packetSeqNum = 0;
  }

  if (frameNum >= 0x80)
  {
    frameNum = 0;
  }

  /* Pack sequence (24b) + frame (8b) into one big-endian u32 */
  packed = ((packetSeqNum & 0x00FFFFFF) << 8) | (frameNum & 0xFF);
  app_uart_print_u32_raw(packed);

  /* Primary RX: all I samples, then all Q samples (not interleaved) */
  for (uint32_t i = printResultStartIndex; i < printResultEndIndex; ++i)
  {
    uint32_t bufIdx = (i - printResultStartIndex) * 2U;
    uint16_t value = print_resultcirrx1[i].I_data;
    print_buffer[bufIdx]     = (uint8_t)((value >> 8) & 0xFF);
    print_buffer[bufIdx + 1] = (uint8_t)(value & 0xFF);
  }
  app_uart_output_raw((uint8_t *)print_buffer, printByteCount);

  for (uint32_t i = printResultStartIndex; i < printResultEndIndex; ++i)
  {
    uint32_t bufIdx = (i - printResultStartIndex) * 2U;
    uint16_t value = print_resultcirrx1[i].Q_data;
    print_buffer[bufIdx]     = (uint8_t)((value >> 8) & 0xFF);
    print_buffer[bufIdx + 1] = (uint8_t)(value & 0xFF);
  }
  app_uart_output_raw((uint8_t *)print_buffer, printByteCount);

  if (APP_RadarIs1T1R() == 0U)
  {
    /* Secondary RX (1T2R only): I block then Q block */
    for (uint32_t i = printResultStartIndex; i < printResultEndIndex; ++i)
    {
      uint32_t bufIdx = (i - printResultStartIndex) * 2U;
      uint16_t value = print_resultcirrx2[i].I_data;
      print_buffer[bufIdx]     = (uint8_t)((value >> 8) & 0xFF);
      print_buffer[bufIdx + 1] = (uint8_t)(value & 0xFF);
    }
    app_uart_output_raw((uint8_t *)print_buffer, printByteCount);

    for (uint32_t i = printResultStartIndex; i < printResultEndIndex; ++i)
    {
      uint32_t bufIdx = (i - printResultStartIndex) * 2U;
      uint16_t value = print_resultcirrx2[i].Q_data;
      print_buffer[bufIdx]     = (uint8_t)((value >> 8) & 0xFF);
      print_buffer[bufIdx + 1] = (uint8_t)(value & 0xFF);
    }
    app_uart_output_raw((uint8_t *)print_buffer, printByteCount);
  }
	
	#else  /* Alternate: human-readable I/Q text on UART (change #if 1 to #if 0 above to enable) */
	APP_UWB_RADAR_CIR_PRINT("Seq:%d\n", ++packetSeqNum);

	APP_UWB_RADAR_CIR_PRINT("I:  ");
	for (uint32_t i = printResultStartIndex; i < printResultEndIndex; ++i) {
		APP_UWB_RADAR_CIR_PRINT("%d,", (APP_RadarIs1T1R() != 0U) ? resultRx2[i].I_data : resultRx1[i].I_data);
	}
	APP_UWB_RADAR_CIR_PRINT("\n");

	APP_UWB_RADAR_CIR_PRINT("Q: ");
	for (uint32_t i = printResultStartIndex; i < printResultEndIndex; ++i) {
		APP_UWB_RADAR_CIR_PRINT("%d,", (APP_RadarIs1T1R() != 0U) ? resultRx2[i].Q_data : resultRx1[i].Q_data);
	}
	APP_UWB_RADAR_CIR_PRINT("\n");

  if (APP_RadarIs1T1R() == 0U)
  {
		APP_UWB_RADAR_CIR_PRINT("I2:  ");
		for (uint32_t i = printResultStartIndex; i < printResultEndIndex; ++i) {
			APP_UWB_RADAR_CIR_PRINT("%d,", resultRx2[i].I_data);
		}
		APP_UWB_RADAR_CIR_PRINT("\n");

		APP_UWB_RADAR_CIR_PRINT("Q2: ");
		for (uint32_t i = printResultStartIndex; i < printResultEndIndex; ++i) {
			APP_UWB_RADAR_CIR_PRINT("%d,", resultRx2[i].Q_data);
		}
		APP_UWB_RADAR_CIR_PRINT("\n");
	}

#endif
}

/**
 * @brief Main radar loop — blocks until radarTask_execute is cleared.
 *
 * Loop body (one iteration = one TX burst via APP_RadarLaunchBurst):
 *   1. APP_RadarLaunchBurst() — radar_config + radar_start.
 *   2. Spin until cb_uwbapp_tx_sfd_mark_irqhandler sets radarDoneFlag.
 *   3. When radarCirBurstCounter reaches sensing_burst_num, finalize the active row.
 *   4. When frame complete: copy rows to result buffers, print CIR, reset accumulators.
 *   5. Pace next row/frame using cb_hal_get_time_us() deadlines (bursts within a row are back-to-back).
 *
 * Mode 0 frame complete: radarCirBurstCounter >= sensing_burst_num (then runs continuously).
 * Mode 1 row complete:   radarCirBurstCounter >= sensing_burst_num
 * Mode 1 frame complete: radarNumSensingCounter >= num_cir_per_frame
 *
 * If deep_sleep_en (mode 1 only): exit loop after one frame, compute sleep budget, enter deep sleep.
 */
void APP_RadarStart(void)
{
  APP_UWB_RADAR_PRINT("APP_RadarStart\n");
 
  uint32_t sense_interval_us = s_sensing_interval_ms * 1000U;  /* mode 0: CIR gap; mode 1: row gap */
  uint32_t interval_us       = s_frame_interval_ms * 1000U;    /* full frame (mode 1) */
    
  radarTask_execute = APP_TRUE;
  packetSeqNum = 0;

  /* Fire cb_uwbapp_tx_sfd_mark_irqhandler on TX SFD (not on RX done) */
  cb_uwbsystem_tx_irqenable_st stTxIrqEnable = { CB_FALSE } ;
  stTxIrqEnable.txDone  = CB_FALSE;
  stTxIrqEnable.sfdDone = CB_TRUE;
  
  uint32_t sense_frame_start_us = cb_hal_get_time_us();  /* mode 1: per-row pacing anchor */
  uint32_t frame_start_us = cb_hal_get_time_us();        /* mode 0/1: frame pacing anchor  */
  uint32_t active_start_us = cb_hal_get_time_us();       /* mode 1 deep sleep: active-time budget */

  radarCirBurstCounter = 0U;

  while (radarTask_execute)
  {
    radarDoneFlag = APP_FALSE;
    APP_RadarLaunchBurst(&stTxIrqEnable);

    while (radarDoneFlag == APP_FALSE) { }

    if (s_sensing_mode == 0)
    {
      if (radarCirBurstCounter >= s_sensing_burst_num)
      {
        APP_RadarFinalizeActiveRow(0);
        APP_RadarFillResultFromAccumulated(0);
        APP_RadarOutputCirUart(0);

        radarCirBurstCounter = 0U;

        memset(accumulatedResultRx2, 0, sizeof(accumulatedResultRx2));
        if (APP_RadarIs1T1R() == 0U)
        {
          memset(accumulatedResultRx1, 0, sizeof(accumulatedResultRx1));
        }

        while (!cb_hal_is_time_elapsed_us(frame_start_us, sense_interval_us)) { /* spin */ }

        frame_start_us += sense_interval_us;
      }
    }
    else /* s_sensing_mode == 1 */
    {
      if (radarCirBurstCounter >= s_sensing_burst_num)
      {
        APP_RadarFinalizeActiveRow(radarNumSensingCounter);
        radarCirBurstCounter = 0U;
        ++radarNumSensingCounter;

        if (radarNumSensingCounter >= s_num_cir_per_frame)
        {
          if (s_deep_sleep_en == 1U)
          {
            radarTask_execute = APP_FALSE;
          }

          radarNumSensingCounter = 0;

          for (int j = 0; j < s_num_cir_per_frame; ++j)
          {
            APP_RadarFillResultFromAccumulated((uint32_t)j);
            APP_RadarOutputCirUart((uint32_t)j);
          }

          memset(accumulatedResultRx2, 0, sizeof(accumulatedResultRx2));
          if (APP_RadarIs1T1R() == 0U)
          {
            memset(accumulatedResultRx1, 0, sizeof(accumulatedResultRx1));
          }

          if (s_deep_sleep_en == 0)
          {
            while (!cb_hal_is_time_elapsed_us(frame_start_us, interval_us)) { /* spin */ }
          }

          frameNum++;
          frame_start_us += interval_us;
          sense_frame_start_us = cb_hal_get_time_us();
        }
        else if (s_deep_sleep_en == 0)
        {
          while (!cb_hal_is_time_elapsed_us(sense_frame_start_us, sense_interval_us)) { /* spin */ }
          sense_frame_start_us += sense_interval_us;
        }
      }
    }
  }

  cb_framework_radar_off();
  APP_UWB_RADAR_PRINT("Radar Off.\n");

  if (s_deep_sleep_en == 1U)
  {
		/* Sleep long enough that (active time + sleep) ~= frame_interval_ms */
		uint32_t time_taken_us = cb_hal_get_time_us() - active_start_us;
		uint32_t time_taken_ms = (time_taken_us + 999U) / 1000U;
		
    if (AppRadarGpio_IsRunAllowed() != 0U)
    {
			uint32_t deep_sleep_timing = 0;
			if (s_frame_interval_ms > time_taken_ms)
			{
				deep_sleep_timing = s_frame_interval_ms - time_taken_ms;
			}
			else
			{
				deep_sleep_timing = 1U; /* minimum 1 ms if frame work exceeded budget */
			}
			
			cb_deep_sleep_control(deep_sleep_timing);
      while (1) { /* chip reboots after deep sleep */ }
    }

    /* GPIO7 low: host held run pin low — skip sleep, return to idle task */
    APP_UWB_RADAR_PRINT("GPIO7 low: deep sleep skipped, awaiting UART.\n");
    g_task_f_execute = APP_FALSE;
  }
}

/**
 * @brief Stops the radar operation.
 *
 * Clears radarTask_execute so APP_RadarStart() exits its burst loop.
 */
void APP_RadarStop(void)
{
  APP_UWB_RADAR_PRINT("Stopped.\n");
  radarTask_execute = APP_FALSE;
}

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
void cb_uwbapp_tx_sfd_mark_irqhandler(void)
{
  cb_framework_radar_stop();

  APP_RadarGetCirFromHw();

  ++radarCirBurstCounter;

  APP_RadarAccumulateCir();

  cb_framework_radar_reset();   /* RX_RESET pulse */

  radarDoneFlag = APP_TRUE;
}
