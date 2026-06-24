#include "adc/adc.h"

#include "config.h"

static volatile uint32_t _adc_voltages[2];

void adc_init(ADC_HandleTypeDef* const adc) {
    HAL_ADC_Start_DMA(adc, (uint32_t*) _adc_voltages, 2);
}

float adc_getBatteryVoltage(void) {
    return _adc_voltages[0] * config_BATTERY_ADC_CONVERSION_FACTOR;
}

float adc_getESCTotalCurrent(void) {
    return _adc_voltages[1] * config_ESC_ADC_CONVERSION_FACTOR;
}
