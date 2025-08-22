#include "CB_adc.h"
#include "CB_UwbDrivers.h"


/**
 * @brief Get ADC voltage value
 * @return ADC voltage in volts
 */
float app_adc_get_adc_voltage(void)
{
    float voltage = 0;
    // Read voltage from ADC channel 0
    voltage = cb_adc_read_AIN_voltage(0);  
    // Ensure voltage is not negative
    if(voltage < 0.0f)
    {
      voltage = 0.0f;
    }
    return voltage;
}

/**
 * @brief Get 10-bit ADC raw value
 * @return 10-bit ADC code
 */
uint16_t app_adc_get_10bit(void)
{
  // Read 10-bit ADC value from channel 0
  uint16_t adc_value = adc_read_AIN_10bit_code(0);
  return adc_value;
}