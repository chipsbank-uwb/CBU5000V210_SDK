#include "CB_adc.h"
#include "app_adc.h"
#include "app_uart.h"
#include "CB_system.h"


/**
 * @brief Initialize and test the ADC functionality.
 * 
 * This function initializes the UART for communication, prints a test message,
 * measures the voltage from the AIN pin with adaptive gain, and prints the result.
 */
void app_adc_test_adaptive(void)
{
    float ret = 0;
    ret = cb_eadc_measure_ain_voltage();  // Measure voltage from AIN pin with adaptive gain
    app_uart_printf("adc = %f \n", ret);  // Print the measured voltage
    cb_system_delay_in_ms(500);             // Introduce a delay of 500 milliseconds
}


/**
 * @brief Test the ADC functionality with a specific gain setting.
 * 
 * This function measures the voltage from the AIN pin using a specific gain setting.
 * It takes multiple readings, averages them, converts the result to a voltage value in the range of 0-3.3V,
 * and prints the result. It also introduces a delay of 500 milliseconds between tests.
 * 
 * @param gain The gain setting for the ADC.
 */
void app_adc_test_original_value(enEADCGain gain)
{
    float valtage = 0;
    for(uint8_t i = 0; i < 5; i++)
    {
        valtage += cb_eadc_get_original_value(gain);  // Measure voltage from AIN pin with gain
    }
    valtage = (float)(valtage / 1024 / 5) * 3.3;      // Convert to 0-3.3V
    app_uart_printf("Ain = %f \n", valtage);          // Print the measured voltage
    cb_system_delay_in_ms(500);                         // Introduce a delay of 500 milliseconds
}


