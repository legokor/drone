#include "adc/adc.h"

#include "stm32f4xx_hal_adc.h"

void adc_init(adc_Adc* adc, uint32_t pin, bool oneshot) {}

float adc_getVoltage(adc_Adc* adc) {
    // TODO: this simple??
    return HAL_ADC_GetValue(adc->hadc);
}
