#ifndef ADC_H
#define ADC_H

#include "stm32f4xx_hal.h"

typedef struct [[nodiscard]] adc_Adc {
    ADC_HandleTypeDef* hadc;
    bool oneshot;
} adc_Adc;

void adc_init(adc_Adc* adc, uint32_t pin, bool oneshot);

[[nodiscard]]
float adc_getVoltage(adc_Adc* adc);

#endif
