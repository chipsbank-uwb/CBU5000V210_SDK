#include "CB_Common.h"
#include "CB_PeripheralPhyAddrDataBase.h"
#include "CB_adc.h"
#include "CB_scr.h"

/**
 * @brief Calculate the voltage from the AIN pin using the specified gain.
 * 
 * This function measures the ground reference voltage (GNDValue), internal bandgap voltage (VbgValue),
 * and the voltage from the AIN pin (AinValue) using the specified gain. It then calculates the actual
 * voltage based on these measurements.
 * 
 * @param gain The gain setting for the ADC.
 * @return The calculated voltage value.
 */
static float cb_eadc_get_ain_voltage(enEADCGain gain) 
{
    uint16_t GNDValue; 
    uint16_t VbgValue; 
    uint16_t AinValue;
    float res;

    // start measure ground EADC value
    GNDValue = cb_scr_eadc_measure_dft(EN_EADC_DFT_MEASURE_GND, gain);

    // start measure Vbg value
    VbgValue = cb_scr_eadc_measure_dft(EN_EADC_DFT_MEASURE_VBG, gain);

    // setup to measure voltage from AIN pin
    AinValue = cb_scr_eadc_measure_ain(gain);

    // get the voltage
    if (AinValue >= VbgValue)
    {
      res = 0.7f + (0.7f * ((1.0f * (AinValue - VbgValue)) / (1.0f * (VbgValue - GNDValue))));
    }

    else 
    {
      res = 0.7f - (0.7f * ((1.0f * (VbgValue - AinValue)) / (1.0f * (VbgValue - GNDValue))));
    }

    return res;
}

/**
 * @brief Measure the voltage from the AIN pin with adaptive gain.
 * 
 * This function initially measures the voltage from the AIN pin using a default gain setting.
 * If the measured voltage falls within specific ranges, it adjusts the gain to improve resolution
 * for lower voltage levels.
 * 
 * @return The measured voltage value.
 */
float cb_eadc_measure_ain_voltage(void) 
{
    float res;
    res = cb_eadc_get_ain_voltage(EN_EADC_GAIN_0);

    // compensating for voltage-resolution
    // the lower the voltage measured, the higher the resolution
    if (res > 1.8f && res <= 2.5f) 
    {
      res = cb_eadc_get_ain_voltage(EN_EADC_GAIN_1);
      return res;
    }
    else if (res > 1.5f && res <= 1.8f) 
    {
      res = cb_eadc_get_ain_voltage(EN_EADC_GAIN_2);
      return res;
    }
    else if (res > 1.2f && res <= 1.5f) 
    {
      res = cb_eadc_get_ain_voltage(EN_EADC_GAIN_3);
      return res;
    }
    else if (res > 0.9f && res <= 1.2f)
    {
      res = cb_eadc_get_ain_voltage(EN_EADC_GAIN_4);
      return res;
    }
    else if (res >= 0.0f && res <= 0.9f) 
    {
      res = cb_eadc_get_ain_voltage(EN_EADC_GAIN_5);
      return res;
    }
    else 
    {
      return res;
    }
}


uint16_t cb_eadc_get_original_value(enEADCGain gain)
{
  uint16_t res = 0;
  res = cb_scr_eadc_measure_ain(gain);
  return res;
}
