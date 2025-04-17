#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "CB_scr.h"

float cb_eadc_measure_ain_voltage(void);

uint16_t cb_eadc_get_original_value(enEADCGain gain);

#endif
