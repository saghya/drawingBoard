#ifndef __ADC_H
#define __ADC_H

#include "main.h"

#define ADC_PRECISION 4096

extern ADC_HandleTypeDef  hadc1;
extern ADC_HandleTypeDef  hadc2;

extern uint32_t adc1_values[ADC_PRECISION];
extern uint32_t adc2_values[ADC_PRECISION];

void startADCs();

#endif // __ADC_H

