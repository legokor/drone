#include "adc/adc.h"

#include "config.h"

static volatile uint32_t _adc_voltages[2];

void adc_init(ADC_HandleTypeDef* adc) {
    // hadc = adc;
    HAL_ADC_Start_DMA(adc, (uint32_t*) _adc_voltages, 2);
    // TODO: continuos
}

float adc_getBatteryVoltage(void) {
    return _adc_voltages[0] * config_BATTERY_ADC_CONVERSION_FACTOR;
}

float adc_getESCTotalCurrent(void) {
    return _adc_voltages[1];
}
