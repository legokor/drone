#include "bar/bar.h"
#include "bar/impl/bar_regs.h"
#include "log/log.h"

#include <math.h>
#include <stddef.h>

void bar_init(bar_Bar* bar) {
    log_debug("Initalizing bar...");

    bar->mode = Standby;
}

bar_Mode bar_getMode(bar_Bar* bar) {
    return bar->mode;
}

void _spi_read(SPI_HandleTypeDef* spi, uint8_t reg, uint8_t* data, size_t len) {
    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 1);

    const uint8_t ctrl[] = { reg | 0x80 };
    HAL_SPI_Transmit(spi, ctrl, 1, 1000);

    HAL_SPI_Receive(spi, data, len, 1000);

    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 0);
}

void _spi_write(SPI_HandleTypeDef* spi, uint8_t reg, uint8_t* data, size_t len) {
    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 1);

    const uint8_t ctrl[] = { reg & (~0x80) };
    HAL_SPI_Transmit(spi, ctrl, 1, 1000);

    HAL_SPI_Transmit(spi, data, len, 1000);

    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 0);
}

uint8_t _spi_read8(SPI_HandleTypeDef* spi, uint8_t reg) {
    uint8_t ret;
    _spi_read(spi, reg, &ret, 1);
    return ret;
}

void _spi_write8(SPI_HandleTypeDef* spi, uint8_t reg, uint8_t data) {
    _spi_write(spi, reg, &data, 1);
}

void _spi_write_masked(SPI_HandleTypeDef* spi, uint8_t reg, uint8_t value, uint8_t mask) {
    uint8_t old = _spi_read8(spi, reg);

    uint8_t oldMasked = ~mask & old;
    uint8_t valMasked = value & mask;

    _spi_write8(spi, reg, oldMasked | valMasked);
}

void _spi_write_bits(SPI_HandleTypeDef* spi, uint8_t reg, uint8_t value, uint8_t offset, uint8_t count) {
    uint8_t n_bits = (1 << count) - 1;
    _spi_write_masked(spi, reg, value << offset, n_bits << offset);
}

void bar_setMode(bar_Bar* bar, bar_Mode mode) {
    // TODO: set mode
}

uint32_t bar_getPressure(bar_Bar* bar) {
    uint32_t data;
    _spi_read(bar->hspi, _bar_REG__PSR_B2, ((uint8_t*) &data) + 1, 3);
    return data & ((1 << 24) - 1);
}

uint32_t bar_getTemperature(bar_Bar* bar) {
    uint32_t data;
    _spi_read(bar->hspi, _bar_REG__TMP_B2, ((uint8_t*) &data) + 1, 3);
    return data & ((1 << 24) - 1);
}

void bar_setPressureMeasurementRate(bar_Bar* bar, uint8_t r) {
    _spi_write_bits(bar->hspi, _bar_REG__PRS_CFG, r, _bar_REG__PRS_CFG__RATE__OFFSET, _bar_REG__PRS_CFG__RATE__LENGTH);
}

void bar_setTemperatureMeasurementRate(bar_Bar* bar, uint8_t r) {
    _spi_write_bits(bar->hspi, _bar_REG__TMP_CFG, r, _bar_REG__TMP_CFG__RATE__OFFSET, _bar_REG__TMP_CFG__RATE__LENGTH);
}

void bar_setPressurePrecision(bar_Bar* bar, uint8_t p) {
    _spi_write_bits(bar->hspi, _bar_REG__PRS_CFG, p, _bar_REG__PRS_CFG__PREC__OFFSET, _bar_REG__PRS_CFG__PREC__LENGTH);
}

void bar_setTemperaturePrecision(bar_Bar* bar, uint8_t p) {
    _spi_write_bits(bar->hspi, _bar_REG__TMP_CFG, p, _bar_REG__TMP_CFG__PREC__OFFSET, _bar_REG__TMP_CFG__PREC__LENGTH);
}

float bar_calculateAltitude(uint32_t p, uint32_t t) {
    const float C_TO_K = 273.15f;
    const float P0 = 101325.f;

    // https://en.wikipedia.org/wiki/Hypsometric_equation
    return ((powf(P0 / p, 1 / 5.257f) - 1) * (t + C_TO_K)) / 0.0065f;

    // t ~= 14.25
    // vaia.com:   return (1 - powf(p / P0, 1 / 5.257)) * 44330;
    // betaflight: return (1 - powf(p / P0, 1 / 5.257)) * 4433000;
}
