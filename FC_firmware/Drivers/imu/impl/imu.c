#include "imu/imu.h"
#include <string.h>
#include "imu/impl/imu_registers.h"
#include "int/int.h"
#include "log/log.h"

#define _IMU_SPI_TIMEOUT 3

#ifdef ASYNC_IMU
void _imu_SpiTransmitCpltCallback(void* context) {
    imu_Imu* imu = (imu_Imu*) context;

    HAL_SPI_Receive_DMA(imu->hspi, (uint8_t*) imu->imuBuffer, sizeof(imu->imuBuffer));
}

void _imu_SpiReceiveCpltCallback(void* context) {
    imu_Imu* imu = (imu_Imu*) context;

    memcpy((void*) imu->imuData, (void*) imu->imuBuffer, sizeof(imu->imuBuffer));

    imu->readInProgress = false;

    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 1);

    imu->newData = true;
}

void _imu_TimPeriodElapsedCallback(void* context) {
    imu_Imu* imu = (imu_Imu*) context;
    if (!imu->readEnabled || imu->readInProgress)
        return;

    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 0);

    imu->readInProgress = true;
    imu->readMemAddress = IMU_ACCEL_XOUT_H | 0x80;

    HAL_SPI_Transmit_IT(imu->hspi, &imu->readMemAddress, 1);
}

bool imu_NewDataAvailable(imu_Imu* imu) {
    bool tmp = imu->newData;

    if (tmp)
        imu->newData = false;

    return tmp;
}
#endif

static bool _imu_WriteBlocking(imu_Imu* imu, uint8_t regAddress, uint8_t data) {
#ifdef ASYNC_IMU
    imu->readEnabled = false;
    while (imu->readInProgress) {
    }
#endif

    bool ok = false;
    uint8_t txData[] = { regAddress, data };
    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 0);

    while (true) {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(imu->hspi, txData, 2, _IMU_SPI_TIMEOUT);
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

static uint8_t _imu_ReadBlocking(imu_Imu* imu, uint8_t regAddress, uint8_t numBytes, volatile uint8_t* buffer) {
#ifdef ASYNC_IMU
    imu->readEnabled = false;
    while (imu->readInProgress) {
    }

#endif

    bool ok = false;
    regAddress |= 0x80;
    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 0);

    while (true) {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(imu->hspi, &regAddress, 1, _IMU_SPI_TIMEOUT);

        if (status != HAL_BUSY) {
            ok = status == HAL_OK;
            break;
        }
    }

    if (ok)
        ok = HAL_SPI_Receive(imu->hspi, (uint8_t*) buffer, numBytes, _IMU_SPI_TIMEOUT * numBytes) == HAL_OK;

    HAL_GPIO_WritePin(imu->csPort, imu->csPin, 1);

#ifdef ASYNC_IMU
    imu->readEnabled = true;
#endif

    return ok;
}

#ifdef ASYNC_IMU
bool imu_Init(imu_Imu* imu,
              SPI_HandleTypeDef* hspi,
              GPIO_TypeDef* csPort,
              uint16_t csPin,
              IRQn_Type readIr,
              TIM_HandleTypeDef* htim) {

    imu->readIr = readIr;
    imu->newData = false;
    imu->readEnabled = false;
    imu->readInProgress = false;

#else
bool imu_Init(imu_Imu* imu, SPI_HandleTypeDef* hspi, GPIO_TypeDef* csPort, uint16_t csPin) {
#endif

    imu->hspi = hspi;
    imu->csPort = csPort;
    imu->csPin = csPin;

    imu->gyroOffset = (imu_Vec3){ 0.0f, 0.0f, 0.0f };
    imu->useGyroOffsets = false;

    log_Debug("Initializing IMU...");

    bool ok = true;

    // Disable I2C and enable SPI
    if (ok) {
        ok = _imu_WriteBlocking(imu, IMU_USER_CTRL, 0x10);
        HAL_Delay(10);
    }

    // RESET, enable all sensors
    if (ok) {
        ok = _imu_WriteBlocking(imu, IMU_PWR_MGMT_1, 0x00);
        HAL_Delay(10);
    }

    // Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
    if (ok) {
        ok = _imu_WriteBlocking(imu, IMU_PWR_MGMT_1, 0x01);
        HAL_Delay(10);
    }

    if (ok) {
        ok = imu_DetectImu(imu);
    }

#ifdef ASYNC_IMU
    imu->readEnabled = ok;

    int_SubscribeToInt(INT_SPI_RX_CPLT, _imu_SpiReceiveCpltCallback, imu, hspi);
    int_SubscribeToInt(INT_SPI_TX_CPLT, _imu_SpiTransmitCpltCallback, imu, hspi);
    int_SubscribeToInt(INT_TIM_PERIOD_ELAPSED, _imu_TimPeriodElapsedCallback, imu, htim);
#endif

    if (ok)
        log_Debug("IMU initialized");
    else
        log_Error("IMU initialization failed");

    return ok;
}

#define _imu_GyroOffsetSampleSize 0xff

void imu_CalculateGyroOffset(imu_Imu* imu) {
    bool prevEnabled = imu->useGyroOffsets;
    imu->useGyroOffsets = false;

    imu->gyroOffset = (imu_Vec3){ 0.0f, 0.0f, 0.0f };
    for (int p = 0; p < _imu_GyroOffsetSampleSize; p++) {
        imu_Vec3 res = imu_ReadGyroData(imu);

        imu->gyroOffset.x += res.x;
        imu->gyroOffset.y += res.y;
        imu->gyroOffset.z += res.z;

        HAL_Delay(8);
    }

    imu->gyroOffset.x /= _imu_GyroOffsetSampleSize;
    imu->gyroOffset.y /= _imu_GyroOffsetSampleSize;
    imu->gyroOffset.z /= _imu_GyroOffsetSampleSize;

    imu->useGyroOffsets = prevEnabled;
}

void imu_EnableGyroOffsetSubtraction(imu_Imu* imu, bool enabled) {
    imu->useGyroOffsets = enabled;
}

bool imu_DetectImu(imu_Imu* imu) {
    uint8_t whoAmI;
    _imu_ReadBlocking(imu, IMU_WHO_AM_I_MPU9250, 1, &whoAmI);

    return whoAmI == 0x73;
}

void imu_SetDefaultSettings(imu_Imu* imu) {
    // Set gyro full scale range (+-2000DPS)
    imu_SetGyroSensitivity(imu, 3);

    // Enable DLPF for the gyro and temp sensors (set fchoice_b's to 0 -> fchoice's to 1)
    imu_EnableGyroAndTempDLPF(imu, true);

    // Set gyro and temp DLPF to 41Hz (results in a 5.9ms delay and a 1kHz sample rate)
    imu_SetGyroAndTempDLPF(imu, 3);

    // Set accelerometer sensitivity to +-4g
    imu_SetAccSensitivity(imu, 1);

    // Enable DLPF for accelerometer (set fchoice_b to 0 -> fchoice to 1)
    imu_EnableAccDLPF(imu, true);

    // Set accelerometer DLPF to 44.8Hz (results in a 4.88ms delay and a 1kHz sample rate)
    imu_SetAccDLPF(imu, 3);

    // Set the sample rate divider to 4+1=5 (so that the gyro/temp and accelerometer data rate is 200Hz)
    imu_SetSampleRateDivider(imu, 4);
}

void imu_SetSampleRateDivider(imu_Imu* imu, uint8_t divider) {
    _imu_WriteBlocking(imu, IMU_SMPLRT_DIV, divider);
}

void imu_EnableAccDLPF(imu_Imu* imu, bool enable) {
    uint8_t accConfigTmp;
    _imu_ReadBlocking(imu, IMU_ACCEL_CONFIG2, 1, &accConfigTmp);
    accConfigTmp &= 0xf7;

    if (enable)
        _imu_WriteBlocking(imu, IMU_ACCEL_CONFIG2, accConfigTmp | 0x00);
    else
        _imu_WriteBlocking(imu, IMU_ACCEL_CONFIG2, accConfigTmp | 0x08);
}

void imu_EnableGyroAndTempDLPF(imu_Imu* imu, bool enable) {
    uint8_t gyroConfigTmp;
    _imu_ReadBlocking(imu, IMU_GYRO_CONFIG, 1, &gyroConfigTmp);
    gyroConfigTmp &= 0xfc;

    if (enable)
        _imu_WriteBlocking(imu, IMU_GYRO_CONFIG, gyroConfigTmp | 0x00);
    else
        _imu_WriteBlocking(imu, IMU_GYRO_CONFIG, gyroConfigTmp | 0x03);
}

void imu_SetAccDLPF(imu_Imu* imu, uint8_t value) {
    uint8_t accConfigTmp;
    _imu_ReadBlocking(imu, IMU_ACCEL_CONFIG2, 1, &accConfigTmp);
    accConfigTmp &= 0xf8;

    _imu_WriteBlocking(imu, IMU_ACCEL_CONFIG2, accConfigTmp | value);
}

void imu_SetGyroAndTempDLPF(imu_Imu* imu, uint8_t value) {
    uint8_t configTmp;
    _imu_ReadBlocking(imu, IMU_CONFIG, 1, &configTmp);
    configTmp &= 0xf8;

    _imu_WriteBlocking(imu, IMU_CONFIG, configTmp | value);
}

void imu_SetAccSensitivity(imu_Imu* imu, uint8_t sensitivity) {
    uint8_t accConfigTmp;
    _imu_ReadBlocking(imu, IMU_ACCEL_CONFIG, 1, &accConfigTmp);
    accConfigTmp &= 0xe7;

    switch (sensitivity) {
        case 0:
            _imu_WriteBlocking(imu, IMU_ACCEL_CONFIG, (0x00 << 3) | accConfigTmp);
            imu->accSensitivity = 2.0 / 32768.0;
            break;
        case 1:
            _imu_WriteBlocking(imu, IMU_ACCEL_CONFIG, (0x01 << 3) | accConfigTmp);
            imu->accSensitivity = 4.0 / 32768.0;
            break;
        case 2:
            _imu_WriteBlocking(imu, IMU_ACCEL_CONFIG, (0x02 << 3) | accConfigTmp);
            imu->accSensitivity = 8.0 / 32768.0;
            break;
        case 3:
            _imu_WriteBlocking(imu, IMU_ACCEL_CONFIG, (0x03 << 3) | accConfigTmp);
            imu->accSensitivity = 16.0 / 32768.0;
            break;
    }
}

void imu_SetGyroSensitivity(imu_Imu* imu, uint8_t sensitivity) {
    uint8_t gyroConfigTmp;
    _imu_ReadBlocking(imu, IMU_GYRO_CONFIG, 1, &gyroConfigTmp);
    gyroConfigTmp &= 0xe7;

    switch (sensitivity) {
        case 0:
            _imu_WriteBlocking(imu, IMU_GYRO_CONFIG, (0x00 << 3) | gyroConfigTmp);
            imu->gyroSensitivity = 250.0 / 32768.0;
            break;
        case 1:
            _imu_WriteBlocking(imu, IMU_GYRO_CONFIG, (0x01 << 3) | gyroConfigTmp);
            imu->gyroSensitivity = 500.0 / 32768.0;
            break;
        case 2:
            _imu_WriteBlocking(imu, IMU_GYRO_CONFIG, (0x02 << 3) | gyroConfigTmp);
            imu->gyroSensitivity = 1000.0 / 32768.0;
            break;
        case 3:
            _imu_WriteBlocking(imu, IMU_GYRO_CONFIG, (0x03 << 3) | gyroConfigTmp);
            imu->gyroSensitivity = 2000.0 / 32768.0;
            break;
    }
}

imu_Vec3 imu_ReadGyroData(imu_Imu* imu) {

#ifdef ASYNC_IMU
    HAL_NVIC_DisableIRQ(imu->readIr);
#else
    _imu_ReadBlocking(imu, IMU_GYRO_XOUT_H, 6, imu->imuData + 8);
#endif

    int16_t x = imu->imuData[8] << 8 | imu->imuData[9];
    int16_t y = imu->imuData[10] << 8 | imu->imuData[11];
    int16_t z = imu->imuData[12] << 8 | imu->imuData[13];

#ifdef ASYNC_IMU
    HAL_NVIC_EnableIRQ(imu->readIr);
#endif

    imu_Vec3 data = { x = x * imu->gyroSensitivity, //
                      y = y * imu->gyroSensitivity, //
                      z = z * imu->gyroSensitivity };

    if (imu->useGyroOffsets) {
        data.x -= imu->gyroOffset.x;
        data.y -= imu->gyroOffset.y;
        data.z -= imu->gyroOffset.z;
    }

    return data;
}

imu_Vec3 imu_ReadAccData(imu_Imu* imu) {

#ifdef ASYNC_IMU
    HAL_NVIC_DisableIRQ(imu->readIr);
#else
    _imu_ReadBlocking(imu, IMU_ACCEL_XOUT_H, 6, imu->imuData);
#endif

    int16_t x = imu->imuData[0] << 8 | imu->imuData[1];
    int16_t y = imu->imuData[2] << 8 | imu->imuData[3];
    int16_t z = imu->imuData[4] << 8 | imu->imuData[5];

#ifdef ASYNC_IMU
    HAL_NVIC_EnableIRQ(imu->readIr);
#endif

    imu_Vec3 data = { x * imu->accSensitivity, //
                      y * imu->accSensitivity, //
                      z * imu->accSensitivity };

    return data;
}

float imu_ReadTempData(imu_Imu* imu) {

#ifdef ASYNC_IMU
    HAL_NVIC_DisableIRQ(imu->readIr);
#else
    _imu_ReadBlocking(imu, IMU_TEMP_OUT_H, 2, imu->imuData + 6);
#endif

    int16_t t = imu->imuData[6] << 8 | imu->imuData[7];

#ifdef ASYNC_IMU
    HAL_NVIC_EnableIRQ(imu->readIr);
#endif

    return (t - 21.0) / 333.87 + 21.0;
}
