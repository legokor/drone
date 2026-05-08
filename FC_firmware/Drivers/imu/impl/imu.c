#include "imu/imu.h"
#include "err/err.h"
#include "imu/impl/imu_registers.h"
#include "irq/irq.h"
#include "log/log.h"
#include "utils/utils.h"

#include "stm32f4xx_hal.h"

#include "arm_math.h"

#include <string.h>

#define _imu_SPI_TIMEOUT 3

#ifdef ASYNC_IMU
static void _imu_spiTransmitCpltCallback(void* context) {
    imu_Imu* imu = (imu_Imu*) context;

    HAL_SPI_Receive_DMA(imu->hspi, (uint8_t*) imu->imuBuffer, sizeof(imu->imuBuffer));
}

static void _imu_spiReceiveCpltCallback(void* context) {
    imu_Imu* imu = (imu_Imu*) context;

    memcpy((void*) imu->imuData, (void*) imu->imuBuffer, sizeof(imu->imuBuffer));

    imu->readInProgress = false;

    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 1);

    imu->newData = true;
}

static void _imu_timPeriodElapsedCallback(void* context) {
    imu_Imu* imu = (imu_Imu*) context;
    if (!imu->readEnabled || imu->readInProgress)
        return;

    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 0);

    imu->readInProgress = true;
    imu->readMemAddress = IMU_ACCEL_XOUT_H | 0x80;

    HAL_SPI_Transmit_IT(imu->hspi, &imu->readMemAddress, 1);
}

bool imu_newDataAvailable(imu_Imu* imu) {
    bool tmp = imu->newData;

    if (tmp)
        imu->newData = false;

    return tmp;
}
#endif

[[nodiscard]]
static bool _imu_writeBlocking(imu_Imu* imu, uint8_t regAddress, uint8_t data) {
#ifdef ASYNC_IMU
    imu->readEnabled = false;
    while (imu->readInProgress) {
    }
#endif

    bool ok = false;
    uint8_t txData[] = { regAddress, data };
    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 0);

    while (true) {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(imu->hspi, txData, 2, _imu_SPI_TIMEOUT);

        if (status != HAL_BUSY) {
            ok = status == HAL_OK;
            break;
        }
    }

    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 1);

#ifdef ASYNC_IMU
    imu->readEnabled = true;
#endif

    return ok;
}

[[nodiscard]]
static bool _imu_readBlocking(imu_Imu* imu, uint8_t regAddress, uint8_t numBytes, volatile uint8_t* buffer) {
#ifdef ASYNC_IMU
    imu->readEnabled = false;
    while (imu->readInProgress) {
    }
#endif

    bool ok = false;
    regAddress |= 0x80;
    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 0);

    while (true) {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(imu->hspi, &regAddress, 1, _imu_SPI_TIMEOUT);

        if (status != HAL_BUSY) {
            ok = status == HAL_OK;
            break;
        }
    }

    if (ok)
        ok = HAL_SPI_Receive(imu->hspi, (uint8_t*) buffer, numBytes, _imu_SPI_TIMEOUT * numBytes) == HAL_OK;

    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 1);

#ifdef ASYNC_IMU
    imu->readEnabled = true;
#endif

    return ok;
}

#ifdef ASYNC_IMU
bool imu_init(imu_Imu* imu,
              SPI_HandleTypeDef* hspi,
              GPIO_TypeDef* csPort,
              uint16_t csPin,
              IRQn_Type readIrq,
              TIM_HandleTypeDef* htim) {

    imu->readIrq = readIrq;
    imu->newData = false;
    imu->readEnabled = false;
    imu->readInProgress = false;

#else
bool imu_init(imu_Imu* imu, SPI_HandleTypeDef* hspi, GPIO_TypeDef* csPort, uint16_t csPin) {
#endif

    imu->hspi = hspi;
    imu->csPort = csPort;
    imu->csPin = csPin;

    imu->gyroOffset = (imu_Vec3) { .x = 0, .y = 0, .z = 0 };
    imu->useGyroOffsets = false;

    log_debug("Initializing IMU...");

    // Disable I2C and enable SPI
    err_try(_imu_writeBlocking(imu, IMU_USER_CTRL, 0x10));
    HAL_Delay(10);

    // RESET, enable all sensors
    err_try(_imu_writeBlocking(imu, IMU_PWR_MGMT_1, 0x00));
    HAL_Delay(10);

    // Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
    err_try(_imu_writeBlocking(imu, IMU_PWR_MGMT_1, 0x01));
    HAL_Delay(10);

    err_try(imu_detectImu(imu));

#ifdef ASYNC_IMU
    irq_subscribeToIrq(irq_SPI_RX_CPLT, _imu_spiReceiveCpltCallback, imu, hspi);
    irq_subscribeToIrq(irq_SPI_TX_CPLT, _imu_spiTransmitCpltCallback, imu, hspi);
    irq_subscribeToIrq(irq_TIM_PERIOD_ELAPSED, _imu_timPeriodElapsedCallback, imu, htim);
#endif

    log_debug("IMU initialized");

    return true;
}

#define _imu_GYRO_OFFSET_SAMPLE_SIZE 0xff

bool imu_calculateGyroOffset(imu_Imu* imu) {
    bool prevEnabled = imu->useGyroOffsets;
    imu->useGyroOffsets = false;

    imu->gyroOffset = (imu_Vec3) { .x = 0, .y = 0, .z = 0 };
    for (int p = 0; p < _imu_GYRO_OFFSET_SAMPLE_SIZE; p++) {
        imu_Vec3 res;
        err_try(imu_readGyroData(imu, &res));

        imu->gyroOffset.x += res.x;
        imu->gyroOffset.y += res.y;
        imu->gyroOffset.z += res.z;

        HAL_Delay(10);
    }

    imu->gyroOffset.x /= _imu_GYRO_OFFSET_SAMPLE_SIZE;
    imu->gyroOffset.y /= _imu_GYRO_OFFSET_SAMPLE_SIZE;
    imu->gyroOffset.z /= _imu_GYRO_OFFSET_SAMPLE_SIZE;

    imu->useGyroOffsets = prevEnabled;

    return true;
}

void imu_enableGyroOffsetSubtraction(imu_Imu* imu, bool enabled) {
    imu->useGyroOffsets = enabled;
}

bool imu_detectImu(imu_Imu* imu) {
    uint8_t whoAmI;
    err_try(_imu_readBlocking(imu, IMU_WHO_AM_I_MPU9250, 1, &whoAmI));

    return whoAmI == 0x73;
}

bool imu_setDefaultSettings(imu_Imu* imu) {
    // Set gyro full scale range (+-2000DPS)
    err_try(imu_setGyroSensitivity(imu, 3));

    // Enable DLPF for the gyro and temp sensors (set fchoice_b's to 0 -> fchoice's to 1)
    err_try(imu_enableGyroAndTempDLPF(imu, true));

    // Set gyro and temp DLPF to 41Hz (results in a 5.9ms delay and a 1kHz sample rate)
    err_try(imu_setGyroAndTempDLPF(imu, 6));

    // Set accelerometer sensitivity to +-4g
    err_try(imu_setAccSensitivity(imu, 3));

    // Enable DLPF for accelerometer (set fchoice_b to 0 -> fchoice to 1)
    err_try(imu_enableAccDLPF(imu, true));

    // Set accelerometer DLPF to 44.8Hz (results in a 4.88ms delay and a 1kHz sample rate)
    err_try(imu_setAccDLPF(imu, 3));

    // Set the sample rate divider to 4+1=5 (so that the gyro/temp and accelerometer data rate is 200Hz)
    err_try(imu_setSampleRateDivider(imu, 0));

    return true;
}

bool imu_setSampleRateDivider(imu_Imu* imu, uint8_t divider) {
    err_try(_imu_writeBlocking(imu, IMU_SMPLRT_DIV, divider));

    return true;
}

bool imu_enableAccDLPF(imu_Imu* imu, bool enable) {
    uint8_t accConfigTmp;
    err_try(_imu_readBlocking(imu, IMU_ACCEL_CONFIG2, 1, &accConfigTmp));
    accConfigTmp &= 0xf7;

    if (enable)
        err_try(_imu_writeBlocking(imu, IMU_ACCEL_CONFIG2, accConfigTmp | 0x00));
    else
        err_try(_imu_writeBlocking(imu, IMU_ACCEL_CONFIG2, accConfigTmp | 0x08));

    return true;
}

bool imu_enableGyroAndTempDLPF(imu_Imu* imu, bool enable) {
    uint8_t gyroConfigTmp;
    err_try(_imu_readBlocking(imu, IMU_GYRO_CONFIG, 1, &gyroConfigTmp));
    gyroConfigTmp &= 0xfc;

    if (enable)
        err_try(_imu_writeBlocking(imu, IMU_GYRO_CONFIG, gyroConfigTmp | 0x00));
    else
        err_try(_imu_writeBlocking(imu, IMU_GYRO_CONFIG, gyroConfigTmp | 0x03));

    return true;
}

bool imu_setAccDLPF(imu_Imu* imu, uint8_t value) {
    uint8_t accConfigTmp;
    err_try(_imu_readBlocking(imu, IMU_ACCEL_CONFIG2, 1, &accConfigTmp));
    accConfigTmp &= 0xf8;

    err_try(_imu_writeBlocking(imu, IMU_ACCEL_CONFIG2, accConfigTmp | value));

    return true;
}

bool imu_setGyroAndTempDLPF(imu_Imu* imu, uint8_t value) {
    uint8_t configTmp;
    err_try(_imu_readBlocking(imu, IMU_CONFIG, 1, &configTmp));
    configTmp &= 0xf8;

    err_try(_imu_writeBlocking(imu, IMU_CONFIG, configTmp | value));

    return true;
}

bool imu_setAccSensitivity(imu_Imu* imu, uint8_t sensitivity) {
    err_assert(sensitivity <= 3);

    uint8_t accConfigTmp;
    err_try(_imu_readBlocking(imu, IMU_ACCEL_CONFIG, 1, &accConfigTmp));
    accConfigTmp &= 0xe7;

    err_try(_imu_writeBlocking(imu, IMU_ACCEL_CONFIG, (sensitivity << 3) | accConfigTmp));
    imu->accSensitivity = (2 << sensitivity) / (float) (1 << 15);

    return true;
}

bool imu_setGyroSensitivity(imu_Imu* imu, uint8_t sensitivity) {
    err_assert(sensitivity <= 3);

    uint8_t gyroConfigTmp;
    err_try(_imu_readBlocking(imu, IMU_GYRO_CONFIG, 1, &gyroConfigTmp));
    gyroConfigTmp &= 0xe7;

    const int SENSITIVITY_TABLE[4] = { 250, 500, 1000, 2000 };

    err_try(_imu_writeBlocking(imu, IMU_GYRO_CONFIG, (sensitivity << 3) | gyroConfigTmp));
    imu->gyroSensitivity = SENSITIVITY_TABLE[sensitivity] / (float) (1 << 15);

    return true;
}

bool imu_readGyroData(imu_Imu* imu, imu_Vec3* data) {

#ifdef ASYNC_IMU
    HAL_NVIC_DisableIRQ(imu->readIrq);
#else
    err_try(_imu_readBlocking(imu, IMU_GYRO_XOUT_H, 6, imu->imuData + 8));
#endif

    int16_t x = imu->imuData[8] << 8 | imu->imuData[9];
    int16_t y = imu->imuData[10] << 8 | imu->imuData[11];
    int16_t z = imu->imuData[12] << 8 | imu->imuData[13];

#ifdef ASYNC_IMU
    HAL_NVIC_EnableIRQ(imu->readIrq);
#endif

    imu_Vec3 tmp = (imu_Vec3) { .x = x, .y = y, .z = z };

    arm_scale_f32(tmp.arr, (utils_PI / 180) * imu->gyroSensitivity, data->arr, 3);

    if (imu->useGyroOffsets) {
        arm_sub_f32(data->arr, imu->gyroOffset.arr, tmp.arr, 3);

        // TODO: jobb?
        *data = tmp;
    }

    return true;
}

bool imu_readAccData(imu_Imu* imu, imu_Vec3* out) {

#ifdef ASYNC_IMU
    HAL_NVIC_DisableIRQ(imu->readIrq);
#else
    err_try(_imu_readBlocking(imu, IMU_ACCEL_XOUT_H, 6, imu->imuData));
#endif

    int16_t x = imu->imuData[0] << 8 | imu->imuData[1];
    int16_t y = imu->imuData[2] << 8 | imu->imuData[3];
    int16_t z = imu->imuData[4] << 8 | imu->imuData[5];

#ifdef ASYNC_IMU
    HAL_NVIC_EnableIRQ(imu->readIrq);
#endif

    imu_Vec3 tmp = (imu_Vec3) { .x = x, .y = y, .z = z };

    arm_scale_f32(tmp.arr, imu->accSensitivity, out->arr, 3);

    return true;
}

float imu_readTempData(imu_Imu* imu) {

#ifdef ASYNC_IMU
    HAL_NVIC_DisableIRQ(imu->readIrq);
#else
    _imu_readBlocking(imu, IMU_TEMP_OUT_H, 2, imu->imuData + 6);
#endif

    int16_t t = imu->imuData[6] << 8 | imu->imuData[7];

#ifdef ASYNC_IMU
    HAL_NVIC_EnableIRQ(imu->readIrq);
#endif

    return (t - 21.0) / 333.87 + 21.0;
}
