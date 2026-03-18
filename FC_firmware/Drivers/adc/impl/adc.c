#include "adc/adc.h"

#include "adc/adc.h"

static ADC_HandleTypeDef* hadc;

void adc_init(ADC_HandleTypeDef* adc) {
    hadc = adc;
    HAL_ADC_Start(hadc);
    // TODO: continuos
}

float adc_getBatteryVoltage(void) {
    HAL_ADC_Start(hadc);
    return HAL_ADC_GetValue(hadc);
}

float adc_getESCTotalCurrent(void) {
    HAL_ADC_Start(hadc);
    return HAL_ADC_GetValue(hadc);
}
