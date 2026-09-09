/**
 * @file    NonLIB_sharedUtils.c
 * @brief   Utility functions for precise delays in embedded systems
 * @details This file contains a set of functions used for precise time delays.
 *					This file is visible in the Non-lib level
 * @author  Chipsbank
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "NonLIB_sharedUtils.h"

//-------------------------------
// FUNCTION DECLARATIONS
//-------------------------------
/**
 * @brief   Waits for a specified number of CPU cycles.
 * 
 * @param[in] cpucycles  Number of CPU cycles to wait.
 * 
 * @note    Uses DWT cycle counter to perform the wait operation. Assumes the DWT unit is enabled.
 */
static void cb_hal_wait_cpu_cycle(uint32_t cpucycles);
static void cb_hal_wait_cpu_cycle(uint32_t cpucycles) 
{
#define CPU_CYCLES_PER_LOOP 3  // Each loop iteration takes exactly 3 CPU cycles

  if (cpucycles == 0) return;

  // Adjust for loop overhead (each loop iteration takes 3 cycles)
  cpucycles /= CPU_CYCLES_PER_LOOP;

  __ASM volatile 
  (
    "1: \n"
    "   SUBS %0, %0, #1 \n"  // 1 cycle: Decrement counter
    "   NOP \n"              // 1 cycle: No operation
    "   BNE 1b \n"           // 1 cycle: Branch back if not zero
    : "+r" (cpucycles)
    :
    : "cc"
  );
}

/**
 * @brief   Waits for a specific number of microseconds.
 * 
 * @param[in] microseconds  The delay time in microseconds.
 * 
 * @note    The function breaks large delays into 20-second chunks for efficiency.
 */
void cb_hal_delay_in_us(uint32_t microseconds)
{
  uint32_t cycles = 0;

  // Handle large microseconds in 20-second chunks
  while (microseconds >= 20000000) // 20 million microseconds = 20 seconds
  {
    cb_hal_wait_cpu_cycle(SystemCoreClock * 20);
    microseconds -= 20000000; // Subtract the chunk
  }

  // Handle remaining microseconds
  cycles = (SystemCoreClock / 1000000) * microseconds;
  cb_hal_wait_cpu_cycle(cycles);
}

/**
 * @brief   Waits for a specific number of milliseconds.
 * 
 * @param[in] milliseconds  The delay time in milliseconds.
 * 
 * @note    The function handles large delays by splitting them into 20-second chunks.
 */
void cb_hal_delay_in_ms(uint32_t milliseconds)
{
  uint32_t cycles = 0;

  // Handle large milliseconds in 20-second chunks
  while (milliseconds >= 20000) // 20,000 milliseconds = 20 seconds
  {
    cb_hal_wait_cpu_cycle(SystemCoreClock * 20);
    milliseconds -= 20000; // Subtract the chunk
  }

  // Handle remaining milliseconds
  cycles = (SystemCoreClock / 1000) * milliseconds;
  cb_hal_wait_cpu_cycle(cycles);
}

/**
 * @brief Get current tick value
 * @return Current tick value in milliseconds
 */
uint32_t cb_hal_get_time_ms(void) 
{
  return sysTickCounter;
}
/**
 * @brief Check if a timeout period has elapsed
 * @param start_tick Starting tick value
 * @param timeout_ms Timeout period in milliseconds
 * @return CB_STATUS The status of the operation (CB_PASS or CB_FAIL).
 */
CB_STATUS cb_hal_is_time_elapsed_ms(uint32_t start_tick, uint32_t timeout_ms) 
{
  if ((sysTickCounter - start_tick) >= timeout_ms)
  {
    return CB_PASS;
  }
  return CB_FAIL;
}

/**
 * @brief Returns system time in microseconds.
 *
 * Takes a stable SysTick snapshot to avoid inconsistent reads.
 * Timestamp is 32-bit and wraps every ~1.19 hours.
 *
 * @return Current time in microseconds.
 */
uint32_t cb_hal_get_time_us(void)
{
  uint32_t ms1, val1, ms2, val2;
  const uint32_t load = SysTick->LOAD + 1U;
  const uint32_t f_hz = (SysTick->CTRL & SysTick_CTRL_CLKSOURCE_Msk)
                        ? SystemCoreClock
                        : (SystemCoreClock / 8U);

  // Take a stable snapshot
  val1 = SysTick->VAL;  ms1 = sysTickCounter;
  val2 = SysTick->VAL;  ms2 = sysTickCounter;
  if (val2 > val1) { ms1 = ms2; val1 = val2; }

  const uint32_t sub_cycles = load - val1;

  // --- compute microseconds using scaled arithmetic (no 64-bit math) ---
  // (sub_cycles / f_hz) * 1e6  ==  (sub_cycles * 1000) / (f_hz / 1000)
  const uint32_t f_khz       = f_hz / 1000U;          // 64MHz ? 64000 kHz
  const uint32_t sub_us      = (sub_cycles * 1000U) / f_khz;
  const uint32_t us_per_tick = (load       * 1000U) / f_khz;

  return ms1 * us_per_tick + sub_us;
}

/**
 * @brief Checks if a timeout has elapsed.
 *
 * Uses unsigned subtraction, which safely handles 32-bit wraparound.
 *
 * @param start_us   Start timestamp.
 * @param timeout_us Timeout duration in microseconds.
 *
 * @return CB_PASS if elapsed >= timeout_us, else CB_FAIL.
 */
CB_STATUS cb_hal_is_time_elapsed_us(uint32_t start_us, uint32_t timeout_us)
{
  // Unsigned subtraction automatically handles wraparound
  return ((cb_hal_get_time_us() - start_us) >= timeout_us) ? CB_PASS : CB_FAIL;
}

/**
 * @brief Convert a value to its two's complement representation.
 *
 * This function takes an unsigned integer value and the number of bits, and converts the 
 * value to its signed integer representation using two's complement arithmetic. If the 
 * value exceeds the positive range of the specified bit width, it will be adjusted to 
 * reflect its negative counterpart.
 *
 * @param value The unsigned integer value to convert.
 * @param bit The number of bits for the representation.
 *
 * @return The signed integer representation of the value.
 */
int32_t cb_utils_twos_complement(uint32_t value, int32_t bit)
{
    // Cast to signed
    int32_t signedVal = (int32_t)value;

    if (signedVal >= (1 << (bit - 1)))
        signedVal -= (1 << bit);

    return signedVal;
}
