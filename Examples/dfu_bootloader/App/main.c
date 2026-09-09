//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ARMCM33_DSP_FP.h>
#include "dfu_uart.h"
#include "dfu_handler.h"
//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void DWT_Init(void);
extern volatile const unsigned char appBin[] __attribute__((aligned(4)));
extern volatile const unsigned char bootsettingBin[] __attribute__((aligned(4)));

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
void DWT_Init(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

int main(void) 
{
    volatile unsigned char embedded_app_probe = appBin[0];
    volatile unsigned char embedded_bootsetting_probe = bootsettingBin[0];
    (void)embedded_app_probe;
    (void)embedded_bootsetting_probe;

    DWT_Init();
    dfu_uart_init();
    dfu_boot_startup();
	while (1)
	{
        dfu_uart_polling();
	}
}
