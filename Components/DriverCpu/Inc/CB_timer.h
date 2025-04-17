/**
 * @file    CB_timer.h
 * @brief   Timer module driver header.
 * @details This file contains the declarations of data structures, enums, and function prototypes
 *          related to the Timer module driver. It provides constants for Timer registers and
 *          definitions for Timer modes, timeout events, interrupt control, and timer setup parameters.
 *          Additionally, it declares functions for initializing timers, configuring timer settings,
 *          starting, pausing, resuming, and disabling timers, as well as reading timer values.
 * @author  Chipsbank
 * @date    2024
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define   DEF_TIMER_TIMEOUT_EVENT_0  0x01
#define   DEF_TIMER_TIMEOUT_EVENT_1  0x02 
#define   DEF_TIMER_TIMEOUT_EVENT_2  0x04  
#define   DEF_TIMER_TIMEOUT_EVENT_3  0x08  

#define DEF_NUM_OF_TIMEOUT_EVENT  4

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief Enumeration defining timer instances.
 */
typedef enum
{
  EN_TIMER_0 = 0,  /**< Timer 0 */
  EN_TIMER_1,     /**< Timer 1 */
  EN_TIMER_2,     /**< Timer 2 */
  EN_TIMER_3,     /**< Timer 3 */
} enTimer;

/**
 * @brief Enumeration defining timer modes.
 */
typedef enum
{
  EN_TIMER_MODE_ONE_SHOT = 0,  /**< One-shot mode */
  EN_TIMER_MODE_FREERUN,      /**< Free-run mode */
} enTimerMode;

/**
 * @brief Enumeration defining timeout events for timers.
 */
typedef enum
{
  EN_TIMER_TIMEOUT_EVENT_0 = 0,   /**< Timeout event 0 */
  EN_TIMER_TIMEOUT_EVENT_1,       /**< Timeout event 1 */
  EN_TIMER_TIMEOUT_EVENT_2,       /**< Timeout event 2 */
  EN_TIMER_TIMEOUT_EVENT_3,       /**< Timeout event 3 */
} enTimerTimeoutEvent;

/**
 * @brief Enumeration defining event compare modes for timers.
 */
typedef enum
{
 EN_TIMER_EVTCOM_MODE_00 = 0,     /**< Event compare mode 00 */
 EN_TIMER_EVTCOM_MODE_01,         /**< Event compare mode 01 */
 EN_TIMER_EVTCOM_MODE_10,         /**< Event compare mode 10 */
}enTimerEvtComMode;

/**
 * @brief Enumeration defining timer interrupt enable/disable options.
 */
typedef enum
{ 
  EN_TIMER_INTERUPT_ENABLE,       /**< Enable timer interrupt */
  EN_TIMER_INTERUPT_DISABLE,      /**< Disable timer interrupt */
}enTimerInterrupt;

/**
 * @brief Enumeration defining options to enable/disable starting timer.
 */
typedef enum
{
  EN_START_TIMER_ENABLE,          /**< Enable starting timer */
  EN_START_TIMER_DISABLE,         /**< Disable starting timer */
}enStartTimer;

/**
 * @brief Enumeration defining options to enable/disable timer event compare.
 */
typedef enum
{
  EN_TIMER_EVTCOM_ENABLE,          /**< Enable timer event compare */
  EN_TIMER_EVTCOM_DISABLE,         /**< Disable timer event compare */
}enTimerEvtComEnable;

/**
 * @brief Enumeration defining options to enable/disable timer pulse width.
 */
typedef enum
{
  EN_TIMER_PULSEWIDTH_ENABLE,      /**< Enable timer pulse width */
  EN_TIMER_PULSEWIDTH_DISABLE,     /**< Disable timer pulse width */
}enTimerPulseWidth;

/**
 * @brief Enumeration defining timer time units.
 */
typedef enum
{
  EN_TIMER_MS = (0u),              /**< Milliseconds, default */
  EN_TIMER_US,                     /**< Microseconds */
}enTimerTimeUnit;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @brief Structure defining timer timeout events and their durations.
 */
typedef struct
{
  uint32_t            TimerTimeoutEvent;                      /**< Timer timeout event */
  uint32_t            timeoutVal[DEF_NUM_OF_TIMEOUT_EVENT];  /**< Array of timeout durations */
} stTimeOut;

/**
 * @brief Structure defining timer pulse width and number of cycles.
 */
typedef struct
{
  enTimerPulseWidth  TimerPulseWidth;  /**< Timer pulse width */
  uint8_t            NumberOfCycles;   /**< Number of cycles */
}stPulseWidth;

/**
 * @brief Structure defining timer setup parameters.
 */
typedef struct
{
  enTimer                 Timer;              /**< Timer instance */
  enTimerMode             TimerMode;          /**< Timer mode */
  stTimeOut               stTimeOut;          /**< Timer timeout events and durations */
  enTimerTimeUnit         TimeUnit;           /**< Timer timeout unit, milliseconds */
  enTimerEvtComMode       TimerEvtComMode;    /**< Timer event compare mode */
  enStartTimer            AutoStartTimer;     /**< Option to automatically start timer */
  enTimerEvtComEnable     TimerEvtComEnable;  /**< Option to enable timer event compare */
  enTimerInterrupt        TimerInterrupt;     /**< Option to enable/disable timer interrupt */
  stPulseWidth            stPulseWidth;       /**< Timer pulse width and number of cycles */
}stTimerSetUp;

/**
 * @brief Union representing timer timeout events.
 */
typedef union
{
  struct{
    uint8_t event_0 :1;   /**< Event 0 */
    uint8_t event_1 :1;   /**< Event 1 */
    uint8_t event_2 :1;   /**< Event 2 */
    uint8_t event_3 :1;   /**< Event 3 */
    uint8_t reserved:4;   /**< Reserved bits */
  };
  uint8_t event;          /**< Timer timeout event */
} unTimerTimeout;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Initializes the specified Timer module.
 * @param   enTimer Timer module enumeration.
 */
void cb_timer_init(enTimer enTimer);

/**
 * @brief       Configures a Timer module based on the provided setup parameters.
 * @param[in]   TimerSetUp Pointer to the structure containing timer setup parameters.
 */
void cb_timer_configure_timer(stTimerSetUp* TimerSetUp);

/**
 * @brief   De-initializes the specified Timer module.
 * @param   enTimer Timer module enumeration.
 */
void cb_timer_deinit(enTimer enTimer);

/**
 * @brief   Initializes a timer setup structure with default values.
 * @param[in]   TimerSetUp Pointer to the timer setup structure to be initialized.
 */
void cb_timer_struct_init(stTimerSetUp* TimerSetUp);

/**
 * @brief   Sets the timeout value for a specific timer event and resets other timeout events to zero.
 * @param[in]   enTimer Timer module enumeration.
 * @param[in]   timeoutEvent The specific timeout event to modify (0-3).
 * @param[in]   timeoutVal The new timeout value in milliseconds.
 */
void cb_set_timeout_expiration(enTimer enTimer, uint8_t timeoutEvent, uint32_t timeoutVal);

/**
 * @brief       Disables the specified Timer module.
 * @param[in]   enTimer Timer module enumeration.
 */
void cb_timer_disable_timer(enTimer enTimer);

/**
 * @brief   Disables interrupts for all timers.
 * @details This function disables interrupts for all timers by setting the interrupt enable registers to 0.
 */
void cb_timer_disable_interrupt(void);

/**
 * @brief       Reads the value of the Timeout Enable register for the specified Timer module.
 * @param[in]   enTimer Timer module enumeration.
 * @return      Timeout enable value.
 */
uint32_t cb_timer_read_toe_value(enTimer enTimer);

/**
 * @brief      Starts the specified Timer module.
 * @param[in]   enTimer Timer module enumeration.
 */
void cb_timer_enable(enTimer enTimer);

/**
 * @brief   Pauses the specified Timer module.
 * @param[in]   enTimer Timer module enumeration.
 */
void cb_timer_pause_timer(enTimer enTimer);

/**
 * @brief   Resumes the specified Timer module.
 * @param[in]   enTimer Timer module enumeration.
 */
void cb_timer_resume_timer(enTimer enTimer);

/**
 * @brief   Latches the counter value to the Timeout Value register for the specified Timer module.
 * @param[in]   enTimer Timer module enumeration.
 * @param[in]   TimerTimeoutEvent Timer timeout event enumeration.
 */
void cb_timer_latch_counter_to_tov(enTimer enTimer,uint32_t TimerTimeoutEvent);

/**
 * @brief       Reads the value of the Timeout Value register for the specified Timer module.
 * @param[in]   enTimer Timer module enumeration.
 * @param[in]   TimerTimeoutEvent Timer timeout event enumeration.
 * @return      Timeout value.
 */
uint32_t cb_timer_read_tov_value(enTimer enTimer,uint8_t TimerTimeoutEvent);

/**
 * @brief       Reads the timeout event of the TOE register for the specified Timer module.
 * @param[in]   enTimer Timer module enumeration.
 * @return      unTimerTimeout enum indicate which timeout event has occured.
 */
unTimerTimeout cb_timer_read_timer_timeout_event(enTimer timer);

/**
 * @brief   Configures the interrupt for the specified Timer module.
 * @param   enTimer Timer module enumeration.
 * @param   timeoutEvent Specific interrupt event to configure.
 * @param   state Interrupt enable state. Set to CB_TRUE to enable, CB_FALSE to disable.
 */
void cb_timer_irq_config(enTimer enTimer, enTimerTimeoutEvent timeoutEvent, uint32_t state);

/**
 * @brief   Get the timeout event flags for the specified Timer module.
 * @param   enTimer Timer module enumeration.
 * @return  Timeout event flags.
 */
uint32_t cb_timer_get_timeout_event_flags(enTimer enTimer);

/**
 * @brief   Clears the specified timeout event flags for the Timer module.
 * @param   enTimer Timer module enumeration.
 * @param   timeoutEventFlags Timeout event flags to be cleared. Valid values are 0-3, representing Timeout Event 0 to Timeout Event 3.
 */
void cb_timer_clear_timeout_event_flags(enTimer enTimer, uint32_t timeoutEventFlags);

/**
 * @brief   Weak callback function for TIMER 0 interrupt at APP layer.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 */
void cb_timer_0_app_irq_callback(void);

/**
 * @brief   Weak callback function for TIMER 1 interrupt at APP layer.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 */
void cb_timer_1_app_irq_callback(void);

/**
 * @brief   Weak callback function for TIMER 2 interrupt at APP layer.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 */
void cb_timer_2_app_irq_callback(void);

/**
 * @brief   Weak callback function for TIMER 3 interrupt at APP layer.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 */
void cb_timer_3_app_irq_callback(void);

/**
 * @brief   Weak callback function for TIMER 3 interrupt at MID layer.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 */
void cb_timer_3_mid_irq_callback(void);

/**
 * @brief   Handler function for TIMER 0 interrupt.
 * @details This function is called when an interrupt is triggered by TIMER 0.
 *          It clears the interrupt flags based on the timeout events and performs necessary actions.
 */
void cb_timer_0_irqhandler(void);

/**
 * @brief   Handler function for TIMER 1 interrupt.
 * @details This function is called when an interrupt is triggered by TIMER 1.
 *          It clears the interrupt flags based on the timeout events and performs necessary actions.
 */
void cb_timer_1_irqhandler(void);

/**
 * @brief   Handler function for TIMER 2 interrupt.
 * @details This function is called when an interrupt is triggered by TIMER 2.
 *          It clears the interrupt flags based on the timeout events and performs necessary actions.
 */
void cb_timer_2_irqhandler(void);

/**
 * @brief   Handler function for TIMER 3 interrupt.
 * @details This function is called when an interrupt is triggered by TIMER 3.
 *          It clears the interrupt flags based on the timeout events and performs necessary actions.
 */
void cb_timer_3_irqhandler(void);


void cb_timer_3_rc_irq_callback(void(*handler)(void));  // Dedicated to CB_system.c  
void cb_timer_3_clear_irq_handler(void);                 // Dedicated to CB_system.c  

#endif
