/**
 * @file    CB_Wdt.h
 * @brief   Header file for Watchdog Timer.
 * @details This file defines structures, constants, and function prototypes for Watchdog Timer.
 * @author  Chipsbank
 * @date    2024
 */
#ifndef INC_WDT_H_
#define INC_WDT_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @brief Enumeration defining Watchdog Modes.
 */
typedef enum {
  EN_WDT_RESET = 1,           /**<Watchdog Resets CPU after timing out */
  EN_WDT_INTERVAL = 0,        /**<Watchdog Doesn't Reset CPU after time out */
} enWdtMode;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

/**
 * @brief Structure defining Watchdog configuration.
 */
typedef struct {
  uint8_t WdtMode;             /**<Watchdog Timer Mode - RESET or INTERVAL */
  uint8_t WdtRunInSleep;       /**<Watchdog pauses(0) or run(1) while cpu is sleeping */
  uint8_t WdtRunInHalt;        /**<Watchdog pauses(0) or run(1) while cpu is halted by debugger */
  uint16_t GraceAfterInt;      /**<Grace time before CPU reset after Watchdog timeouts, in msec*/
  uint32_t Interval;           /**<Watchdog timer interval, in milliseconds */
} stWdtConfig;
typedef void (*WdtCallback_t)(void);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/* Initialization and Configuration functions *********************************/
void cb_wdt_init(const stWdtConfig* const Config);   
void cb_wdt_deinit(void);    
void cb_wdt_set_timeout(uint32_t TimeoutVal);          
                   
/* Enable/Disable Control functions ******************************************/
void cb_wdt_enable(void);                                
void cb_wdt_disable(void);                               

/* Counter Management and reset functions ************************************/
void cb_wdt_refresh(void);  
void cb_wdt_force_reset(void);                               
uint32_t cb_wdt_get_current_count(void);                
                             
/* Interrupts and flags management functions ********************************/
void cb_wdt_irq_config(uint8_t enable);  
void cb_wdt_clear_irq(void);                             
uint32_t cb_wdt_get_irq_setting(void);  
uint8_t cb_wdt_is_running(void);                         

/* Callback functions ********************************************************/
void NMI_Handler(void);
void cb_wdt_nmi_app_irq_callback(void);                  
void cb_wdt_nmi_rc_irq_callback(void(*handler)(void));    // Dedicated to CB_system.c     
void cb_wdt_nmi_clear_irq_handler(void);                  // Dedicated to CB_system.c     

                      
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

#endif /** INC_WDT_H_ */
