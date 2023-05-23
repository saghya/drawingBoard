#include "adc.h"

uint32_t adc1_values[ADC_PRECISION] = {0};
uint32_t adc2_values[ADC_PRECISION] = {0};

void startADCs()
{
    HAL_ADC_Start_DMA(&hadc1, adc1_values, ADC_PRECISION);
    HAL_ADC_Start_DMA(&hadc2, adc2_values, ADC_PRECISION);
}

