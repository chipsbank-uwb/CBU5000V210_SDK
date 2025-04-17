/*
 * @file    Task.c
 * @brief   Implementation file for the task handler module.
 * @details This file contains the implementation of the TaskHandler function,
 *          which is responsible for handling tasks in the system. It includes
 *          necessary headers and defines the TaskHandler function.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "TaskHandler.h"
#include "AppUwbCommTrx.h"
#include "AppUwbDstwr.h"
#include "AppUwbPdoa.h"
#include "AppUwbRngAoa.h"
#if (APP_DEMO_ENABLE == APP_TRUE)
#include "App/AppDemo.h"
#endif
#if (APP_UWB_RADAR_ENABLE == APP_TRUE)
#include "AppUwbRadar.h"
#endif
#include <string.h>

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define TASKHANDLER_UARTPRINT_ENABLE APP_TRUE
#if (TASKHANDLER_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define taskhandler_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define taskhandler_print(...)
#endif
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
uint8_t g_task_a_tx_execute      = APP_FALSE;
uint8_t g_task_a_rx_execute      = APP_FALSE;
uint8_t g_task_b_ini_execute     = APP_FALSE;
uint8_t g_task_b_resp_execute    = APP_FALSE;
uint8_t g_task_c_ini_execute     = APP_FALSE;
uint8_t g_task_c_resp_execute    = APP_FALSE;
uint8_t g_task_d_ini_execute     = APP_FALSE;
uint8_t g_task_d_resp_execute    = APP_FALSE;
uint8_t g_task_e_execute         = APP_FALSE;
uint8_t g_task_f_execute         = APP_FALSE;
uint8_t g_task_g_execute         = APP_FALSE;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Handles tasks in the system.
 * 
 * This function is responsible for handling tasks in the system. It typically
 * serves as the main loop or scheduler of the application, where various tasks
 * are executed based on their priority or schedule. It may include tasks such as
 * reading sensor data, processing user inputs, updating display information, etc.
 * 
 * Tasks in the system are typically implemented as functions or modules, and this
 * function is responsible for coordinating their execution. Depending on the
 * application architecture, the TaskHandler may run periodically, in response to
 * interrupts, or in a continuous loop.
 * 
 * Example usage:
 * @code
 * int main() {
 *     while(1) {
 *         TaskHandler();
 *     }
 *     return 0;
 * }
 * @endcode
 * 
 * @note This function may need to be customized or extended based on the specific
 * requirements and architecture of the application.
 */
void TaskHandler(void)
{
  //------------------------
  // Task 'a_tx' execute
  //------------------------
  if(g_task_a_tx_execute == APP_TRUE) 
  {
    taskhandler_print("[app_uwb_commtx_main]\n");
    app_uwb_commtx_main();
    g_task_a_tx_execute = APP_FALSE;
  }  
  
  //------------------------
  // Task 'a_rx' execute
  //------------------------
  if(g_task_a_rx_execute == APP_TRUE) 
  {
    taskhandler_print("[app_uwb_commrx_main]\n");
    app_uwb_commrx_main();
    g_task_a_rx_execute = APP_FALSE;
  }  
  //------------------------
  // Task 'c_ini' execute
  //------------------------
  if(g_task_c_ini_execute == APP_TRUE) 
  {
    taskhandler_print("[app_pdoa_initiator]\n");
    app_pdoa_initiator();
    g_task_c_ini_execute = APP_FALSE;
  }  
  //------------------------
  // Task 'c_resp' execute
  //------------------------
  if(g_task_c_resp_execute == APP_TRUE) 
  {
    taskhandler_print("[app_pdoa_responder]\n");
    app_pdoa_responder();
    g_task_c_resp_execute = APP_FALSE;
  } 

  //------------------------
  // Task 'b_ini' execute
  //------------------------
  if(g_task_b_ini_execute == APP_TRUE) 
  {
    taskhandler_print("[app_dstwr_initiator]\n");
    app_dstwr_initiator();
    g_task_b_ini_execute = APP_FALSE;
  }  
  
  //------------------------
  // Task 'b_resp' execute
  //------------------------
  if(g_task_b_resp_execute == APP_TRUE) 
  {
    taskhandler_print("[app_dstwr_responder]\n");
    app_dstwr_responder();
    g_task_b_resp_execute = APP_FALSE;
  }  
  
  //------------------------
  // Task 'd_ini' execute
  //------------------------
  if(g_task_d_ini_execute == APP_TRUE) 
  {
    taskhandler_print("[app_rngaoa_initiator]\n");
    app_rngaoa_initiator();
    g_task_d_ini_execute = APP_FALSE;
  }  
  
  //------------------------
  // Task 'd_resp' execute
  //------------------------
  if(g_task_d_resp_execute == APP_TRUE) 
  {
    taskhandler_print("[app_rngaoa_responder]\n");
    app_rngaoa_responder();
    g_task_d_resp_execute = APP_FALSE;
  }  
  
  //------------------------
  // Task 'e' execute
  //------------------------  
  if(g_task_e_execute == APP_TRUE) 
  {
		taskhandler_print("[APP_UWB_PERIODIC_TRX]\n");
  }
  
#if (APP_UWB_RADAR_ENABLE == APP_TRUE)
  
  //------------------------
  // Task 'f' execute
  //------------------------
  if(g_task_f_execute == APP_TRUE) 
  {
    APP_RadarStart();
    g_task_f_execute = APP_FALSE;
  }
#endif
	//------------------------
  // Task 'g' execute
  //------------------------
  if(g_task_g_execute == APP_TRUE) 
  {
    taskhandler_print("[APP_UWB_PSR]\n");
  }  

#if (APP_DEMO_ENABLE == APP_TRUE)
  APP_DEMO_Run();
#endif
}

