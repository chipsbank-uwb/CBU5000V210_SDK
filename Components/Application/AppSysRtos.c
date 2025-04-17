
//-------------------------------
// INCLUDE SECTION
//-------------------------------
// Scheduler Header
#include "FreeRTOS.h"
#include "TaskHandler.h"
#include "semphr.h"
// Application Header
#include "APP_CompileOption.h"
#include "AppSysRtos.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_TASK_STACK_MONITOR_ENABLE       APP_FALSE //Monitor remaining task stack, this will tell us how many memory left in each task

#define APP_SYS_RTOS_UARTPRINT_ENABLE APP_TRUE
#if (APP_SYS_RTOS_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define APP_SYS_RTOS_PRINT(...) app_uart_printf(__VA_ARGS__)
#else
  #define APP_SYS_RTOS_PRINT(...)
#endif

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void APP_RTOS_vTaskHandler(const void *pvParameters) __attribute__((noreturn));

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_TASKHANDLER_STACK_SIZE ((uint16_t)512)   //in bytes = 512 * 4 = 2048

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

#if (APP_TASK_STACK_MONITOR_ENABLE == APP_TRUE)
UBaseType_t taskHandlerSizeRemain = (UBaseType_t)DEF_TASKHANDLER_STACK_SIZE;
#endif


//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void APP_RTOS_Init(void)
{
  xTaskCreate((TaskFunction_t)APP_RTOS_vTaskHandler, "Task: TaskHandler", DEF_TASKHANDLER_STACK_SIZE, NULL, 1, NULL);
}

void APP_RTOS_StartScheduler(void)
{
  // Start the scheduler
  vTaskStartScheduler();
}

void APP_RTOS_vTaskHandler(const void *pvParameters)
{
  while(1)
  {
    TaskHandler();
#if (APP_TASK_STACK_MONITOR_ENABLE == APP_TRUE)
    if(uxTaskGetStackHighWaterMark( NULL ) < taskHandlerSizeRemain) taskHandlerSizeRemain = uxTaskGetStackHighWaterMark( NULL );
#endif
		//vTaskDelay(1);
  }
}
