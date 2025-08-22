#include "CB_adc.h"
#include "app_adc.h"
#include "app_uart.h"
#include "CB_system.h"


/**
 * @brief Initialize and test the ADC functionality.
 * 
 * This function initializes the UART for communication, prints a test message,
 * measures the voltage from the AIN pin and prints the result.
 */
void app_adc_demo_loop(void)
{
    float voltage = app_adc_get_adc_voltage();          //Obtain the voltage value of the conversion result
    uint16_t adc_value = app_adc_get_10bit();           //Obtain the 10bite quantization value                             
    app_uart_printf("voltage = %f  V,  10bit_value: %d \n", voltage, adc_value);    // Print the measured voltage
    cb_system_delay_in_ms(300);                                                    // Introduce a delay of 300 milliseconds
}
