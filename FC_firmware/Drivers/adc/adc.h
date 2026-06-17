#ifndef ADC_H
#define ADC_H

#include "stm32f4xx_hal.h"

void adc_init(ADC_HandleTypeDef* const hadc);

[[nodiscard]]
float adc_getBatteryVoltage(void);

[[nodiscard]]
float adc_getESCTotalCurrent(void);

#endif
