#ifndef BAR_H
#define BAR_H

#include <stdint.h>
#include "spi.h"

typedef enum [[nodiscard]] bar_Mode { Standby, Command, Background } bar_Mode;

typedef struct [[nodiscard]] bar_Bar {
    SPI_HandleTypeDef* hspi;

    bar_Mode mode;
} bar_Bar;

void bar_init(bar_Bar* bar);

[[nodiscard]] bar_Mode bar_getMode(bar_Bar* bar);
void bar_setMode(bar_Bar* bar, bar_Mode mode);

[[nodiscard]] uint32_t bar_getPressure(bar_Bar* bar);
[[nodiscard]] uint32_t bar_getTemperature(bar_Bar* bar);

void bar_setPressureMeasurementRate(bar_Bar* bar, uint8_t r);
void bar_setTemperatureMeasurementRate(bar_Bar* bar, uint8_t r);

void bar_setPressurePrecision(bar_Bar* bar, uint8_t p);
void bar_setTemperaturePrecision(bar_Bar* bar, uint8_t p);

[[nodiscard]] float bar_calculateAltitude(uint32_t p, uint32_t t);

#endif // BAR_H
