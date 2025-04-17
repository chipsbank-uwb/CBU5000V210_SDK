/**
 * @file    CB_timer.c
 * @brief   Timer module driver implementation.
 * @details This file contains the implementation of functions for configuring,
 *          controlling, and handling interrupts for the Timer module.
 *          It defines constants and structures related to Timer module registers
 *          and provides functions to initialize timers, configure timer settings,
 *          start, pause, resume, and disable timers, as well as read timer values.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_timer.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include "CB_scr.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-variable"
#endif

#define TARGET_TIMER  (arrTimerList + enTimer)->TIMER
#define DEF_TIMER_MS  (1000u)
#define DEF_TIMER_US  (1000000u)
#define GC_SYSTEM_CLOCK_CHIPSET_64MHZ 64000000
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{
  enTimer        enTimer;
  TIMER_TypeDef* TIMER;
} timerTable;
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static TIMER_TypeDef *TIMER0 = (TIMER_TypeDef* ) TIMER0_BASE_ADDR;
static TIMER_TypeDef *TIMER1 = (TIMER_TypeDef* ) TIMER1_BASE_ADDR;
static TIMER_TypeDef *TIMER2 = (TIMER_TypeDef* ) TIMER2_BASE_ADDR;
static TIMER_TypeDef *TIMER3 = (TIMER_TypeDef* ) TIMER3_BASE_ADDR;

static volatile uint32_t TimerMode = 0x00000000;
static uint8_t TOE0;
static uint8_t TOE1;
static uint8_t TOE2;
static uint8_t TOE3;

static const timerTable  arrTimerList[] = {
  { EN_TIMER_0,  (TIMER_TypeDef* )TIMER0_BASE_ADDR },
  { EN_TIMER_1,  (TIMER_TypeDef* )TIMER1_BASE_ADDR },
  { EN_TIMER_2,  (TIMER_TypeDef* )TIMER2_BASE_ADDR },
  { EN_TIMER_3,  (TIMER_TypeDef* )TIMER3_BASE_ADDR },  
};

// Define a global variable to store user-set interrupt handlers
static void (*rc_callback_timer_irq_handler)(void) = NULL;
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void cb_timer_0_irq_callback(void);
void cb_timer_1_irq_callback(void);
void cb_timer_2_irq_callback(void);
void cb_timer_3_irq_callback(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Initializes the specified Timer module.
 * @param   enTimer Timer module enumeration.
 */
void cb_timer_init(enTimer enTimer)
{
    // Get the pointer to the specified timer
    TIMER_TypeDef* timer = arrTimerList[enTimer].TIMER;

    // Enable the specified timer module
    switch (enTimer) {
        case EN_TIMER_0:
            cb_scr_timer0_module_on();
            break;
        case EN_TIMER_1:
            cb_scr_timer1_module_on();
            break;
        case EN_TIMER_2:
            cb_scr_timer2_module_on();
            break;
        case EN_TIMER_3:
            cb_scr_timer3_module_on();
            break;
        default:
            // Handle invalid timer enumeration if necessary
            return;
    }

    // Disable timer interrupt initially
    NVIC_DisableIRQ((TIMER_0_IRQn + enTimer));

    // Clear all registers
    cb_timer_deinit(enTimer);

    // Enable timer interrupt after configuration
    NVIC_EnableIRQ((TIMER_0_IRQn + enTimer));

    // Temporary patch as TIMER's INT_EN default is enabled in SOC
    timer->INT_EN = 0;
}

/**
 * @brief   Configures a Timer module based on the provided setup parameters.
 * @param   TimerSetUp Pointer to the structure containing timer setup parameters.
 */
void cb_timer_configure_timer(stTimerSetUp* TimerSetUp)
{
    enTimer enTimer;
    enTimer=TimerSetUp->Timer;  
    extern uint32_t SystemCoreClock;
    //Reset Timer interrupt bitfield
    TARGET_TIMER->INT_EN =(TARGET_TIMER->INT_EN &~(TimerSetUp->stTimeOut.TimerTimeoutEvent));
    if(TimerSetUp->TimerInterrupt==EN_TIMER_INTERUPT_DISABLE)
    {
        TARGET_TIMER->INT_EN &=~(TimerSetUp->stTimeOut.TimerTimeoutEvent);
    }
    else if(TimerSetUp->TimerInterrupt==EN_TIMER_INTERUPT_ENABLE)
    {
        TARGET_TIMER->INT_EN |=TimerSetUp->stTimeOut.TimerTimeoutEvent;
    }
    TARGET_TIMER->PS = 0x00000000;        // Prescalar - 0  
    // Reset TimeOutEvent's values
    TARGET_TIMER->TOV0 = 0x00000000;
    TARGET_TIMER->TOV1 = 0x00000000;
    TARGET_TIMER->TOV2 = 0x00000000;
    TARGET_TIMER->TOV3 = 0x00000000;
    // Reset TimerMode RunType, evt type, and AutoClear bitfield
    TimerMode = TimerMode&(uint32_t)(~(1<<DEF_POS_BIT_RUN_TYPE));
    TimerMode = TimerMode&(uint32_t)(~(1<<DEF_POS_BIT_EVT_TYPE));
    TimerMode = TimerMode&(uint32_t)(~(1<<DEF_POS_BIT_AUTO_CLEAR));
    // Configure Timer's RunType
    switch(TimerSetUp->TimerMode)
    {
        case EN_TIMER_MODE_FREERUN: TimerMode |= 0x00000803;  break;
                                                                // 0: run_type - Free running mode 
                                                                // 1: evt_type - timeout event will be auto clear if another timeout event occurs
                                                                // 11: auto_clear - counter will be clear once it reaches the maximum timeout value 
        case EN_TIMER_MODE_ONE_SHOT: TimerMode |= 0x00000802;  break;
                                                                // 0: run_type - one shot mode mode 
                                                                // 1: evt_type - timeout event will be auto clear if another timeout event occurs
                                                                // 11: auto_clear - counter will be clear once it reaches the maximum timeout value 
        default: break;
    }

    // Reset Timer's Evt Common mode bitfield
    TimerMode = TimerMode&(uint32_t)(~(3<<DEF_POS_BIT_EVT_COM));   
    // Configure Timer's Evt Common mode
    switch(TimerSetUp->TimerEvtComMode)
    {
        case EN_TIMER_EVTCOM_MODE_00:  TimerMode |= 0x00000000; break;
        case EN_TIMER_EVTCOM_MODE_01:  TimerMode |= 0x00000100; break;
        case EN_TIMER_EVTCOM_MODE_10:  TimerMode |= 0x00000200; break;
        default:  break;
    }
    // Configure Timeout pulse width
    if(TimerSetUp->stPulseWidth.TimerPulseWidth==EN_TIMER_PULSEWIDTH_ENABLE)
    {
        if(TimerSetUp->stPulseWidth.NumberOfCycles>DEF_MAX_CYCLE_OF_PULSE_WIDTH)
        {
            TimerSetUp->stPulseWidth.NumberOfCycles=DEF_MAX_CYCLE_OF_PULSE_WIDTH;
        }
        TimerMode = TimerMode&(uint32_t)(~(63<<DEF_POS_BIT_EVT_WIDTH)); //63 represents 6 bits for clearing the event width (evt_width).
        TimerMode = TimerMode&(uint32_t)(~(1<<DEF_POS_BIT_EVT_TYPE));   //clear evt_type.
        TimerMode |= ((uint32_t)TimerSetUp->stPulseWidth.NumberOfCycles << DEF_POS_BIT_EVT_WIDTH);   
    }
    // Apply TimerMode var to Timer's mode register
    TARGET_TIMER->MODE= TimerMode;
    // Configure Timeout Events
    enTimerTimeUnit Unit = TimerSetUp->TimeUnit;
    uint32_t ClockScale;
    if (Unit == EN_TIMER_MS)
    {
        ClockScale = (GC_SYSTEM_CLOCK_CHIPSET_64MHZ/DEF_TIMER_MS);
    }
    else
    {
        ClockScale = (GC_SYSTEM_CLOCK_CHIPSET_64MHZ/DEF_TIMER_US);  
    }
    *(&TARGET_TIMER->TOV0)  = ClockScale * TimerSetUp->stTimeOut.timeoutVal[0];//fix GC_SYSTEM_CLOCK_CHIPSET_64MHZ sicne the maximum timer frequency is 64MHZ.
    *(&TARGET_TIMER->TOV1)  = ClockScale * TimerSetUp->stTimeOut.timeoutVal[1];  
    *(&TARGET_TIMER->TOV2)  = ClockScale * TimerSetUp->stTimeOut.timeoutVal[2];  
    *(&TARGET_TIMER->TOV3)  = ClockScale * TimerSetUp->stTimeOut.timeoutVal[3];
    // Enable Timer
    TARGET_TIMER->EN      = 0x00000001;
    // Reset Timeout bitfield,and Enable Timeout 0,1,2,3
    TARGET_TIMER->TO_EN= (TARGET_TIMER->TO_EN&~(TimerSetUp->stTimeOut.TimerTimeoutEvent));
    TARGET_TIMER->TO_EN|=TimerSetUp->stTimeOut.TimerTimeoutEvent; 
    if(TimerSetUp->TimerEvtComEnable==EN_TIMER_EVTCOM_ENABLE)
    {
        TARGET_TIMER->TO_EN = ((TARGET_TIMER->TO_EN )&(uint32_t)(~(1<<DEF_POS_BIT_TM_TOE_COM)));
        TARGET_TIMER->TO_EN  |= 0x00000010;
    }

    if(TimerSetUp->AutoStartTimer==EN_START_TIMER_ENABLE)
    {
        TARGET_TIMER->START = 0x00000001;     // Start timer 0/1/2/3
    }
}

/**
 * @brief   De-initializes the specified Timer module.
 * @param   enTimer Timer module enumeration.
 */
void cb_timer_deinit(enTimer enTimer)
{
    // Get the pointer to the specified timer
    TIMER_TypeDef* timer = arrTimerList[enTimer].TIMER;

    // Disable timer interrupt
    NVIC_DisableIRQ((TIMER_0_IRQn + enTimer));

    // Clear all registers
    timer->EN      = 0x00000000;  // Disable Timer
    timer->START   = 0x00000000;  // Stop Timer
    timer->CTRL    = 0x00000000;  // Reset Control Register
    timer->TVL     = 0x00000000;  // Reset Time Value Low
    timer->INT_EN  = 0x00000000;  // Disable Interrupts
    timer->INT_CLR = 0x00000000;  // Clear Interrupt Flags
    timer->PS      = 0x00000000;  // Reset Prescaler
    timer->MODE    = 0x00000000;  // Reset Mode Register
    timer->TO_EN   = 0x00000000;  // Disable Timeout Events
    timer->TOV0    = 0x00000000;  // Reset the timeout0 value registers
    timer->TOV1    = 0x00000000;  // Reset the timeout1 value registers
    timer->TOV2    = 0x00000000;  // Reset the timeout2 value registers
    timer->TOV3    = 0x00000000;  // Reset the timeout3 value registers
    timer->TOE     = 0x00000000;  // Clear Timeout Event Flags
}

/**
 * @brief   Initializes a timer setup structure with default values.
 * @param   TimerSetUp Pointer to the timer setup structure to be initialized.
 */
void cb_timer_struct_init(stTimerSetUp* TimerSetUp) 
{
    // Set the timer mode to free run mode
    TimerSetUp->TimerMode                      = EN_TIMER_MODE_FREERUN;
    // Set the time unit to milliseconds
    TimerSetUp->TimeUnit                       = EN_TIMER_MS; 
    // Set the timeout values
    TimerSetUp->stTimeOut.timeoutVal[0]        = 0;  // Timeout value for event 0
    TimerSetUp->stTimeOut.timeoutVal[1]        = 0;  // Timeout value for event 1
    TimerSetUp->stTimeOut.timeoutVal[2]        = 0;  // Timeout value for event 2
    TimerSetUp->stTimeOut.timeoutVal[3]        = 0;  // Timeout value for event 3 
    // Set the timeout event
    TimerSetUp->stTimeOut.TimerTimeoutEvent    = DEF_TIMER_TIMEOUT_EVENT_0;
    // Set the event common mode
    TimerSetUp->TimerEvtComMode                = EN_TIMER_EVTCOM_MODE_00;
    // Enable auto start timer
    TimerSetUp->AutoStartTimer                 = EN_START_TIMER_ENABLE;
    // Disable event common enable
    TimerSetUp->TimerEvtComEnable              = EN_TIMER_EVTCOM_DISABLE;
    // Enable timer interrupt
    TimerSetUp->TimerInterrupt                 = EN_TIMER_INTERUPT_ENABLE;
    // Set the pulse width parameters
    TimerSetUp->stPulseWidth.NumberOfCycles    = 0;
    TimerSetUp->stPulseWidth.TimerPulseWidth   = EN_TIMER_PULSEWIDTH_DISABLE;
}

/**
 * @brief   Sets the timeout value for a specific timer event and resets other timeout events to zero.
 * @param   enTimer Timer module enumeration.
 * @param   timeoutEvent The specific timeout event to modify (0-3).
 * @param   timeoutVal The new timeout value in milliseconds.
 */
void cb_set_timeout_expiration(enTimer enTimer, uint8_t timeoutEvent, uint32_t timeoutVal)
{
    // Define the time unit as milliseconds
    enTimerTimeUnit Unit = EN_TIMER_MS;
    uint32_t ClockScale;

    // Calculate the clock scale based on the time unit
    if (Unit == EN_TIMER_MS) {
            ClockScale = (GC_SYSTEM_CLOCK_CHIPSET_64MHZ / DEF_TIMER_MS); // Clock scale for milliseconds
    } else {
            ClockScale = (GC_SYSTEM_CLOCK_CHIPSET_64MHZ / DEF_TIMER_US); // Clock scale for microseconds
    }

    // Calculate the new timeout value in clock cycles
    uint32_t newTimeoutVal = ClockScale * timeoutVal;

    // Modify the specified timeout value
    switch (timeoutEvent) {
            case 0: TARGET_TIMER->TOV0 = newTimeoutVal; break;
            case 1: TARGET_TIMER->TOV1 = newTimeoutVal; break;
            case 2: TARGET_TIMER->TOV2 = newTimeoutVal; break;
            case 3: TARGET_TIMER->TOV3 = newTimeoutVal; break;
            default: return; // Invalid timeout event number
    }
}

/**
 * @brief   Starts the specified Timer module.
 * @param   enTimer Timer module enumeration.
 */
void cb_timer_enable(enTimer enTimer)
{
    TARGET_TIMER->START  = 0x00000001;  
}

/**
 * @brief   Pauses the specified Timer module.
 * @param   enTimer Timer module enumeration.
 */
void cb_timer_pause_timer(enTimer enTimer)
{
    TARGET_TIMER->CTRL  =  TARGET_TIMER->CTRL&(uint32_t)(~(1<<DEF_POS_BIT_PAUSE));
    TARGET_TIMER->CTRL  |= 0x00000001;  
}

/**
 * @brief   Resumes the specified Timer module.
 * @param   enTimer Timer module enumeration.
 */
void cb_timer_resume_timer(enTimer enTimer)
{
    TARGET_TIMER->CTRL  =  TARGET_TIMER->CTRL&(uint32_t)(~(1<<DEF_POS_BIT_PAUSE));
    TARGET_TIMER->CTRL  |= 0x00000000;  
}

/**
 * @brief   Latches the counter value to the Timeout Value register for the specified Timer module.
 * @param   enTimer Timer module enumeration.
 * @param   TimerTimeoutEvent Timer timeout event enumeration.
 */
void cb_timer_latch_counter_to_tov(enTimer enTimer,uint32_t TimerTimeoutEvent)
{
    TARGET_TIMER->TVL =  TimerTimeoutEvent;
}

/**
 * @brief   Reads the value of the Timeout Value register for the specified Timer module.
 * @param   enTimer Timer module enumeration.
 * @param   TimerTimeoutEvent Timer timeout event enumeration.
 * @return  Timeout value.
 */
uint32_t cb_timer_read_tov_value(enTimer enTimer,uint8_t TimerTimeoutEvent)
{
    return *(&TARGET_TIMER->TOV0 + TimerTimeoutEvent);
}

/**
 * @brief   Disables the specified Timer module.
 * @param   enTimer Timer module enumeration.
 */
void cb_timer_disable_timer(enTimer enTimer)
{
    TARGET_TIMER->INT_EN  = 0x00000000;
    TARGET_TIMER->TO_EN   = 0x00000000; // Disable Timer, Timeout 0,1,2,3
    TARGET_TIMER->INT_CLR = 0x00000000;
    TARGET_TIMER->MODE    = 0x00000000;
    TARGET_TIMER->CTRL   |= 0x00000002; //Clear and stop timer can be setted when the timer is enable
    TARGET_TIMER->EN      = 0x00000000;  // Disable Timer
}

/**
 * @brief   Disables interrupts for all timers.
 * @details This function disables interrupts for all timers by setting the interrupt enable registers to 0.
 */
void cb_timer_disable_interrupt(void)
{
    TIMER0->INT_EN = 0;
    TIMER1->INT_EN = 0;
    TIMER2->INT_EN = 0;
    TIMER3->INT_EN = 0;
}

/**
 * @brief   Reads the value of the Timeout Enable register for the specified Timer module.
 * @param   enTimer Timer module enumeration.
 * @return  Timeout enable value.
 */
uint32_t cb_timer_read_toe_value(enTimer enTimer )
{
    return TARGET_TIMER->TOE;
}

unTimerTimeout cb_timer_read_timer_timeout_event(enTimer timer)
{
    unTimerTimeout timerTimeout = {0};
    switch (timer)
    {
        case EN_TIMER_0: {
            if (TIMER0->TOE & 0x00000001)  { timerTimeout.event_0 = CB_TRUE;  break; }
            if (TIMER0->TOE & 0x00000002)  { timerTimeout.event_1 = CB_TRUE;  break; }
            if (TIMER0->TOE & 0x00000004)  { timerTimeout.event_2 = CB_TRUE;  break; }
            if (TIMER0->TOE & 0x00000008)  { timerTimeout.event_3 = CB_TRUE;  break; }
            break;
        }
        case EN_TIMER_1: {
            if (TIMER1->TOE & 0x00000001) { timerTimeout.event_0 = CB_TRUE;  break; }
            if (TIMER1->TOE & 0x00000002) { timerTimeout.event_1 = CB_TRUE;  break; }
            if (TIMER1->TOE & 0x00000004) { timerTimeout.event_2 = CB_TRUE;  break; }
            if (TIMER1->TOE & 0x00000008) { timerTimeout.event_3 = CB_TRUE;  break; }
            break;
        }
        case EN_TIMER_2: {
            if (TIMER2->TOE & 0x00000001) { timerTimeout.event_0 = CB_TRUE;  break; }
            if (TIMER2->TOE & 0x00000002) { timerTimeout.event_1 = CB_TRUE;  break; } 
            if (TIMER2->TOE & 0x00000004) { timerTimeout.event_2 = CB_TRUE;  break; } 
            if (TIMER2->TOE & 0x00000008) { timerTimeout.event_3 = CB_TRUE;  break; }     
            break;
        }
        case EN_TIMER_3: {
            if (TIMER3->TOE & 0x00000001) { timerTimeout.event_0 = CB_TRUE;   break; }
            if (TIMER3->TOE & 0x00000002) { timerTimeout.event_1 = CB_TRUE;   break; }
            if (TIMER3->TOE & 0x00000004) { timerTimeout.event_2 = CB_TRUE;   break; }
            if (TIMER3->TOE & 0x00000008) { timerTimeout.event_3 = CB_TRUE;   break; }    
            break;
        }
    }

    return timerTimeout;
}

/**
 * @brief   Configures the interrupt for the specified Timer module.
 * @param   enTimer Timer module enumeration.
 * @param   timeoutEvent Specific interrupt event to configure.
 * @param   state Interrupt enable state. Set to CB_TRUE to enable, CB_FALSE to disable.
 */
void cb_timer_irq_config(enTimer enTimer, enTimerTimeoutEvent timeoutEvent, uint32_t state)
{
    // Get the pointer to the specified timer
    TIMER_TypeDef* timer = arrTimerList[enTimer].TIMER;

    if (state == CB_TRUE)
    {
            timer->INT_EN |= (1<<timeoutEvent);  // Enable the specified interrupt event
    }
    else
    {
            timer->INT_EN &= ~(1<<timeoutEvent);  // Disable the specified interrupt event
    }
}

/**
 * @brief   Get the timeout event flags for the specified Timer module.
 * @param   enTimer Timer module enumeration.
 * @return  Timeout event flags.
 */
uint32_t cb_timer_get_timeout_event_flags(enTimer enTimer)
{
    // Return the timeout event flags
    return TARGET_TIMER->TOE;
}

/**
 * @brief   Clears the specified timeout event flags for the Timer module.
 * @param   enTimer Timer module enumeration.
 * @param   timeoutEventFlags Timeout event flags to be cleared. Valid values are 0-3, representing Timeout Event 0 to Timeout Event 3.
 */
void cb_timer_clear_timeout_event_flags(enTimer enTimer, uint32_t timeoutEventFlags)
{
    // Ensure timeoutEventFlags is within the valid range (0-3)
    if (timeoutEventFlags > 3) {
        return; // or handle the error appropriately
    }

    // Clear the specified timeout event flags
    TARGET_TIMER->INT_CLR =  (uint32_t)(0x02<<timeoutEventFlags) | 0x01 ; // Assuming INT_CLR is used to clear timeout events as well
}

/**
 * @brief   Handler function for TIMER 0 interrupt.
 * @details This function is called when an interrupt is triggered by TIMER 0.
 *          It clears the interrupt flags based on the timeout events and performs necessary actions.
 */
void cb_timer_0_irqhandler(void)
{
    cb_timer_0_irq_callback();

    if (TIMER0->TOE & 0x00000001)      // timer 0 Event 0 timeout event
    {
        TIMER0->INT_CLR = 0x00000003;  
    }  
    if (TIMER0->TOE & 0x00000002)      // timer 0 Event 1 timeout event
    {  
        TIMER0->INT_CLR = 0x00000005;  
    }   
    if (TIMER0->TOE & 0x00000004)      // timer 0 Event 2 timeout event
    {
        TIMER0->INT_CLR = 0x00000009;
    }  
    if (TIMER0->TOE & 0x00000008)      // timer 0 Event 3 timeout event
    {
        TIMER0->INT_CLR = 0x00000011;
    } 
}

/**
 * @brief   Handler function for TIMER 1 interrupt.
 * @details This function is called when an interrupt is triggered by TIMER 1.
 *          It clears the interrupt flags based on the timeout events and performs necessary actions.
 */
void cb_timer_1_irqhandler (void)
{
    cb_timer_1_irq_callback();

    if (TIMER1->TOE & 0x00000001)      // timer 1 Event 0 timeout event
    {    
        TIMER1->INT_CLR = 0x00000003;
    }  
    if (TIMER1->TOE & 0x00000002)      // timer 1 Event 1 timeout event
    {
        TIMER1->INT_CLR = 0x00000005;  
    }   
    if (TIMER1->TOE & 0x00000004)      // timer 1 Event 2 timeout event
    { 
        TIMER1->INT_CLR = 0x00000009;   
    }  
    if (TIMER1->TOE & 0x00000008)      // timer 1 Event 3 timeout event
    {
        TIMER1->INT_CLR = 0x00000011; 
    } 
}

/**
 * @brief   Handler function for TIMER 2 interrupt.
 * @details This function is called when an interrupt is triggered by TIMER 2.
 *          It clears the interrupt flags based on the timeout events and performs necessary actions.
 */
void cb_timer_2_irqhandler (void)
{
    cb_timer_2_irq_callback();

    if (TIMER2->TOE & 0x00000001)      // timer 2 Event 0 timeout event
    {     
        TIMER2->INT_CLR = 0x00000003;  
    }  
    if (TIMER2->TOE & 0x00000002)      // timer 2 Event 1 timeout event
    {  
        TIMER2->INT_CLR = 0x00000005;  
    }   
    if (TIMER2->TOE & 0x00000004)      // timer 2 Event 2 timeout event
    {       
        TIMER2->INT_CLR = 0x00000009;         
    }  
    if (TIMER2->TOE & 0x00000008)      // timer 2 Event 3 timeout event
    {   
        TIMER2->INT_CLR = 0x00000011;       
    }  
}

/**
 * @brief   Handler function for TIMER 3 interrupt.
 * @details This function is called when an interrupt is triggered by TIMER 3.
 *          It clears the interrupt flags based on the timeout events and performs necessary actions.
 */
void cb_timer_3_irqhandler (void)
{
    cb_timer_3_irq_callback();

    if (TIMER3->TOE & 0x00000001)      // timer 3 Event 0 timeout event
    {   
        TIMER3->INT_CLR = 0x00000003; 
    }  
    if (TIMER3->TOE & 0x00000002)      // timer 3 Event 1 timeout event
    {   
        TIMER3->INT_CLR = 0x00000005;
    }   
    if (TIMER3->TOE & 0x00000004)      // timer 3 Event 2 timeout event
    {   
        TIMER3->INT_CLR = 0x00000009; 
    }  
    if (TIMER3->TOE & 0x00000008)      // timer 3 Event 3 timeout event
    {
        TIMER3->INT_CLR = 0x00000011;        
    }  
}


/**
 * @brief   Weak callback function for TIMER 0 interrupt.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 *
 * @param   TimeOutEventNumber: The timeout event number triggering the interrupt.
 */
void cb_timer_0_irq_callback(void)
{
    cb_timer_0_app_irq_callback();
}

__WEAK void cb_timer_0_app_irq_callback(void)
{
}

/**
 * @brief   Weak callback function for TIMER 1 interrupt.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 *
 * @param   TimeOutEventNumber: The timeout event number triggering the interrupt.
 */
void cb_timer_1_irq_callback(void)
{
    cb_timer_1_app_irq_callback();
}

__WEAK void cb_timer_1_app_irq_callback(void)
{
}

/**
 * @brief   Weak callback function for TIMER 2 interrupt.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 *
 * @param   TimeOutEventNumber: The timeout event number triggering the interrupt.
 */
void cb_timer_2_irq_callback(void)
{
    cb_timer_2_app_irq_callback();
}

__WEAK void cb_timer_2_app_irq_callback(void)
{
}

/**
 * @brief   Weak callback function for TIMER 3 interrupt.
 * @details This function serves as a weak callback that is invoked when an interrupt is triggered by TIMER 0.
 *          It is intended to be overridden by the user with a custom implementation.
 *
 * @param   TimeOutEventNumber: The timeout event number triggering the interrupt.
 */
void cb_timer_3_irq_callback(void)
{
    cb_timer_3_app_irq_callback();

    if(rc_callback_timer_irq_handler != NULL)
    {
        rc_callback_timer_irq_handler();
    }
}

__WEAK void cb_timer_3_app_irq_callback(void)
{
}

/**
 * @brief Set the timer midalyer interrupt handler.
 * @param handler The callback function to be called on timer midalyer interrupt.
 */
void cb_timer_3_rc_irq_callback(void(*handler)(void))
{
    rc_callback_timer_irq_handler = handler;
}

/**
 * @brief Clear the timer midalyer interrupt handler.
 */
void cb_timer_3_clear_irq_handler(void)
{
    rc_callback_timer_irq_handler = NULL;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
