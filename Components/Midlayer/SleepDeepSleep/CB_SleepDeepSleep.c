/**
 * @file    CB_Sleep_DeepSleep.c
 * @brief   Implementation of sleep and deep sleep functions
 * @details This file contains the implementation of functions related to sleep and deep sleep modes.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_CompileOption.h"
#include "CB_scr.h"
#include "CB_wdt.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include "NonLIB_sharedUtils.h"
#include "CB_SleepDeepSleep.h"
#if (GC_SLEEP_FUNC_AES_SUPPORT_ENABLE == CB_TRUE)
#include "CB_qspi.h"
#endif

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define CB_SLEEPDEEPSLEEP_UARTPRINT_ENABLE CB_FALSE
#if (CB_SLEEPDEEPSLEEP_UARTPRINT_ENABLE == CB_TRUE)
  #include "app_uart.h"
  #define cb_sleep_deepsleep_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define cb_sleep_deepsleep_print(...)
#endif


//-------------------------------
// DEFINE SECTION
//-------------------------------
#define SCR_BASE_ADDR                 (0x40020000)
#define NOP_50_CPU_CYCLES             "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" \
                                      "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" \
                                      "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" \
                                      "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" \
                                      "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" \

#define NOP_100_CPU_CYCLES            NOP_50_CPU_CYCLES NOP_50_CPU_CYCLES
#define NOP_1000_CPU_CYCLES           NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES NOP_100_CPU_CYCLES
#define DEF_MAX_VALUE_OF_SLEEPTIME    0xFFFFF
#if (GC_SLEEP_FUNC_AES_SUPPORT_ENABLE == CB_TRUE)
#define DEF_MASK_OF_AESEN_AESENREGSEL 0x30UL
#define DEF_MASK_OF_AES_EN_REG_SEL    0x20UL 
#endif


#define DEF_DEEPSLEEP_RC_CompensateRatio 					1.9f
#define DEF_DEEPSLEEP_CAL_BYPASS_CODE_VALUE 			0xFFUL //CalCode 0xFF for tested wake up from deep sleep. Default value is 0x68.
#define DEF_DEEPSLEEP_IDC_VALUE 									0xFUL //IDC value.

//-------------------------------

// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stSCR_TypeDef  *pSCR  = (stSCR_TypeDef* ) SCR_BASE_ADDR;
float RC_CompensateRatio = 1.0f;

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
#if (GC_SLEEP_FUNC_AES_SUPPORT_ENABLE == CB_TRUE)
void cb_scr_sleep_operation(void)__attribute__((section("SPECIFIC_SLEEPFUNC_TEST")));
#else
void cb_scr_sleep_operation(void);
#endif

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/**
 * @brief Performs a workaround for sleep mode within a critical section.
 *
 * This function performs a workaround for sleep mode within a critical section.
 */
void cb_scr_sleep_operation(void)
{
  /*Disable Ibuf */
  pSCR->cpu_ctrl |= SCR_CPU_CTRL_MASK_skip_init_ibuf; 
  /*Enter Sleep*/
  pSCR->sleep_mode |= SCR_forcesleep; // More than 640 CPU Cycles Delay Required for SCR Enter Sleep to take effect.    
  __asm volatile(
    NOP_1000_CPU_CYCLES
    //NOP_1000_CPU_CYCLES
    //NOP_1000_CPU_CYCLES
  );
  /****************************************************
  *****************************************************
  **********************Sleeping***********************
  *****************************************************
  *****************************************************/
  
  
  /*****************Waked up**************************/
  
  /*Active Memory after wake up, SCR Sleep Mode required delay to take effect*/
  //  pSCR->sleep_mode &= ~(SCR_forcesleep);
  /*0x4002000C-SleepModeRegister: sleep_time[20:1]0xFFF(default),force_sleep[0] 0:Clear SleepMode to active memory access, 1:Sleep Mode*/
  *((volatile uint32_t *)(0x4002000C)) = 0x1FFEUL; //this line hardcoded, because the memory is restricted from being access in this moment. 
  __asm volatile(
    NOP_50_CPU_CYCLES
  );
   
  /* Enable Ibuf */
  pSCR->cpu_ctrl &= ~SCR_CPU_CTRL_MASK_skip_init_ibuf; 
  return;
}

/**
 * @brief Enters sleep mode for a specified duration.
 *
 * This function enters sleep mode for the specified duration in milliseconds.
 *
 * @param slpduration_in_ms The duration of sleep mode in milliseconds.
 *
 * @return CB_STATUS The status of the operation (CB_PASS or CB_FAIL).
 */
CB_STATUS cb_sleep_control(uint32_t slpduration_in_ms)
{
    uint8_t wdt_irq_temp_buf = 0;
    uint32_t sleep_value_to_set = 0;

    if (slpduration_in_ms > DEF_MAX_VALUE_OF_SLEEPTIME)
    {
        /* Sleep Time Set exceed the limit. Max = 0xFFFFF */
        return CB_FAIL;
    }

    float raw_compensated_sleep_value = ((float)slpduration_in_ms) / RC_CompensateRatio;
    sleep_value_to_set = (uint32_t)raw_compensated_sleep_value; // integer part
    float fractional_compensated_sleep_value = raw_compensated_sleep_value - (float)sleep_value_to_set; // fractional part

    if (fractional_compensated_sleep_value >= 0.5f) // Rounding consideration to minimize the precision lost
    {
        sleep_value_to_set += 1;
    }
    else if (sleep_value_to_set == 0) // Make sure it will not be 0
    {
        sleep_value_to_set = 1;
    }
    else if (sleep_value_to_set > DEF_MAX_VALUE_OF_SLEEPTIME)
    {
        sleep_value_to_set = DEF_MAX_VALUE_OF_SLEEPTIME;
    }

    __disable_irq(); // Disabling all the "maskable" irq. To prevent memory accessing during memory deactivated period

    if (cb_wdt_is_running() == CB_TRUE)
    {
        wdt_irq_temp_buf = (uint8_t)cb_wdt_get_irq_setting(); // Store NMI irq status before sleep
        cb_wdt_irq_config(0); // Disabling for NMI irq
    }

    pSCR->cpu_ctrl |= SCR_CPU_CTRL_MASK_ALL_pson_in_dsleep;
    /* Setting Sleep time */
    pSCR->sleep_mode = (sleep_value_to_set << SCR_sleeptime_Pos) & SCR_sleeptime_Msk;
    /* Disable Icache */
    pSCR->cpu_ctrl |= SCR_CPU_CTRL_MASK_icache_bypass;

#if (GC_SLEEP_FUNC_AES_SUPPORT_ENABLE == CB_TRUE)
    volatile uint32_t* p_QSPI_SETTING = ((volatile uint32_t *)(0x40030000 + 0x24));
    *p_QSPI_SETTING &= ~DEF_MASK_OF_AESEN_AESENREGSEL;
    *p_QSPI_SETTING |= DEF_MASK_OF_AES_EN_REG_SEL;
#endif

    __asm volatile(
        NOP_50_CPU_CYCLES
    );

    cb_scr_sleep_operation();

#if (GC_SLEEP_FUNC_AES_SUPPORT_ENABLE == CB_TRUE)
    *p_QSPI_SETTING &= ~DEF_MASK_OF_AESEN_AESENREGSEL;
#endif

    /* Enable Icache */
    pSCR->cpu_ctrl &= ~SCR_CPU_CTRL_MASK_icache_bypass;

    /* Wait Ibuf & I-cache stable */
    __asm volatile(
        NOP_50_CPU_CYCLES
    );

    if (cb_wdt_is_running() == CB_TRUE)
    {
        cb_wdt_irq_config(wdt_irq_temp_buf); // Resume NMI irq status before sleep
    }

    __enable_irq(); // Enable back all the irq

    /* Back to Normal Operation */
    return CB_PASS;
}
#if defined(__clang__)
    // Suppress warnings for Clang/LLVM (Arm Compiler 6)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunreachable-code-return"
#endif

/**
 * @brief Enters deep sleep mode for a specified duration.
 *
 * This function enters deep sleep mode for the specified duration in milliseconds.
 *
 * @param slpduration_in_ms The duration of deep sleep mode in milliseconds.
 *
 * @return CB_STATUS The status of the operation (CB_PASS or CB_FAIL).
 */
CB_STATUS cb_deep_sleep_control(uint32_t slpduration_in_ms)
{
    uint32_t sleep_value_to_set = 0;

    if (slpduration_in_ms > DEF_MAX_VALUE_OF_SLEEPTIME)
    {
      /*Sleep Time Set exceed the limit. Max =0xFFFFF*/
      return CB_FAIL; 
    }

    /*Adjust Rc Clock to more stable & accurate*/
    pSCR->rc32 &= ~(SCR_RC32_idc_rc32k|SCR_RC32_cal_bypass_code);
    cb_hal_delay_in_us(100);// Wait for at least 20 microseconds to ensure that the register bit is cleared.
    pSCR->rc32 |=(((DEF_DEEPSLEEP_IDC_VALUE<<SCR_RC32_idc_rc32k_pos)&SCR_RC32_idc_rc32k_msk)|((DEF_DEEPSLEEP_CAL_BYPASS_CODE_VALUE<<SCR_RC32_cal_bypass_code_pos)&SCR_RC32_cal_bypass_code_msk));
    cb_hal_delay_in_us(100);

    float raw_compensated_sleep_value = ((float)slpduration_in_ms) / DEF_DEEPSLEEP_RC_CompensateRatio;
    sleep_value_to_set = (uint32_t)raw_compensated_sleep_value;// integer part
    float fractional_compensated_sleep_value = raw_compensated_sleep_value - (float)sleep_value_to_set;  //fractional part

    if (fractional_compensated_sleep_value >= 0.5f) //Rounding consideration to minimize the precision lost.
    {
      sleep_value_to_set += 1;
    }
    else if(sleep_value_to_set == 0) //Make sure it will not be 0.
    {
      sleep_value_to_set = 1;
    }
    else if (sleep_value_to_set > DEF_MAX_VALUE_OF_SLEEPTIME)
    {
      sleep_value_to_set = DEF_MAX_VALUE_OF_SLEEPTIME;
    }

    pSCR->cpu_ctrl &= ~(SCR_CPU_CTRL_MASK_ALL_pson_in_dsleep);
    pSCR->sleep_mode = (sleep_value_to_set<<SCR_sleeptime_Pos)&SCR_sleeptime_Msk;  

    /*Enter Sleep*/
    pSCR->sleep_mode |= SCR_forcesleep; // More than 640 CPU Cycles Delay Required for SCR Enter Sleep to take effect.

    while(1);//FW shall stop here. FW is expected to re-boot if enter DeepSleep. 
    /*Back to Normal Operation*/
    return CB_PASS;
}

#if defined(__clang__)
    // Re-enable warnings for Clang/LLVM
    #pragma clang diagnostic pop
#endif



