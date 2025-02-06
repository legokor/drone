#ifndef BAR_H
#define BAR_H

#include <stdint.h>

typedef enum bar_Mode { Standby, Command, Background } bar_Mode;

typedef struct bar_Bar {
    bar_Mode mode;
} bar_Bar;

void bar_Init(bar_Bar* bar);

bar_Mode bar_GetMode(bar_Bar* bar);
void bar_SetMode(bar_Bar* bar, bar_Mode mode);

uint32_t bar_GetPressure(bar_Bar* bar);
uint32_t bar_GetTemperature(bar_Bar* bar);

void bar_SetPressureMeasurementRate(bar_Bar* bar, uint8_t r);
void bar_SetTemperatureMeasurementRate(bar_Bar* bar, uint8_t r);

void bar_SetPressurePrecision(bar_Bar* bar, uint8_t p);
void bar_SetTemperaturePrecision(bar_Bar* bar, uint8_t p);

float bar_CalculateAltitude(uint32_t p, uint32_t t);

#endif // BAR_H
