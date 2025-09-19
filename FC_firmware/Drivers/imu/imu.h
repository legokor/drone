#ifndef IMU_H
#define IMU_H

#define ASYNC_IMU

#include <stdbool.h>

#include "stm32f4xx_hal.h"

typedef union [[nodiscard]] imu_Vec3 {
    struct {
        float x, y, z;
    };

    struct {
        float roll, pitch, yaw;
    };

    float arr[3];
} imu_Vec3;

/**
 * @brief MPU9250 IMU and compass IC I2C driver for the STM32 microcontroller.
 * The implementation heavily relies on the HAL STM32 drivers.
 */
typedef struct [[nodiscard]] imu_Imu {
    SPI_HandleTypeDef* hspi;

    float accSensitivity, gyroSensitivity;

    imu_Vec3 gyroOffset;
    bool useGyroOffsets;

    volatile uint8_t imuData[14];

    GPIO_TypeDef* csPort;
    uint16_t csPin;

#ifdef ASYNC_IMU
    uint8_t readMemAddress;

    volatile uint8_t imuBuffer[14];
    volatile bool newData;

    IRQn_Type readIrq;

    volatile bool readEnabled;
    volatile bool readInProgress;
#endif
} imu_Imu;

#ifdef ASYNC_IMU
/**
 * @brief Initializes the MPU9250 IMU driver for async data retrieval using DMA.
 * @param imu - the IMU instance
 * @param hspi - the SPI handle for the IMU
 * @param dmaIrq - the IRQn_Type handle of the SPI event interrupt
 * @param htim - the timer handle for the periodical data read
 *
 * @return 1 on success, else 0
 */
[[nodiscard]] bool imu_init(
    imu_Imu* imu,
    SPI_HandleTypeDef* hspi,
    GPIO_TypeDef* csPort,
    uint16_t csPin,
    IRQn_Type readIrq,
    TIM_HandleTypeDef* htim
);
#else
/**
 * @brief Initializes the MPU9250 IMU driver for sync (blocking) data retrieval.
 * @param imu - the IMU instance
 * @param hspi - the SPI handle for the IMU
 *
 * @return 1 on success, else 0
 */
[[nodiscard]] bool imu_init(
    imu_Imu* imu,
    SPI_HandleTypeDef* hspi,
    GPIO_TypeDef* csPort,
    uint16_t csPin
);
#endif

/**
 * @brief Checks whether the IMU (MPU9250) was detected.
 * @param imu - the IMU instance
 *
 * @return true, if the IMU (MPU9250) was detected.
 */
[[nodiscard]] bool imu_detectImu(imu_Imu* imu);

/**
 * @brief Sets the IMU to the default settings, which will suite most applications:
 * 	- The magnetometer is set to 16 bit resolution with a 100Hz sample rate
 * 	- The accelerometer is set to +-4g range with a 200Hz sample rate and a 44.8Hz DLPF
 * 	- The gyro and thermometer are set to 2000DPS range (in case of the gyro) with a 200Hz sample rate and a 41Hz DLPF
 * @param imu - the IMU instance
 *
 */
[[nodiscard]] bool imu_setDefaultSettings(imu_Imu* imu);

/**
 * @brief Calculates the gyro offsets while at rest using a lot of samples.
 * @param imu - the IMU instance
 */
[[nodiscard]] bool imu_calculateGyroOffset(imu_Imu* imu);

/**
 * @brief Enables the calculated gyro offset subtraction from the results
 * @param imu - the IMU instance
 * @param enabled - 1 to enable, 0 to disable
 */
void imu_enableGyroOffsetSubtraction(imu_Imu* imu, bool enabled);

/**
 * @brief Sets the sample rate divider.
 * The divider is only active when DLPF is enabled and set to 1-6 (both in case of the gyro/temp and the accelerometer).
 * @param imu - the IMU instance
 * @param divider - the divider that divides the internal (1kHz sample rate) by (1 + divider).
 */
[[nodiscard]] bool imu_setSampleRateDivider(imu_Imu* imu, uint8_t divider);

/**
 * @brief Enable DLPF for the accelerometer (set fchoice_b to 0 -> fchoice to 1).
 * @param imu - the IMU instance
 * @param enable - whether to enable the DLPF for the accelerometer.
 */
[[nodiscard]] bool imu_enableAccDLPF(imu_Imu* imu, bool enable);

/**
 * @brief Enable DLPF for the gyro and temp sensors (set fchoice_b's to 0 -> fchoice's to 1).
 * @param imu - the IMU instance
 * @param enable - whether to enable the DLPF for the gyro and thermometer.
 */
[[nodiscard]] bool imu_enableGyroAndTempDLPF(imu_Imu* imu, bool enable);

/**
 * @brief Details about the values: https://invensense.tdk.com/wp-content/uploads/2015/02/RM-MPU-9250A-00-v1.6.pdf
 * 	- table on page 15
 * @param imu - the IMU instance
 * @param value - the accelerometer DLPF value
 */
[[nodiscard]] bool imu_setAccDLPF(imu_Imu* imu, uint8_t value);

/**
 * @brief Details about the values: https://invensense.tdk.com/wp-content/uploads/2015/02/RM-MPU-9250A-00-v1.6.pdf
 * 	- table on page 13
 * @param imu - the IMU instance
 * @param value - the gyro and thermometer DLPF value
 */
[[nodiscard]] bool imu_setGyroAndTempDLPF(imu_Imu* imu, uint8_t value);

/**
 * @brief Sets the gyro's sensitivity.
 * @param imu - the IMU instance
 * @param sensitivity - possible (+/-) values are:
 * 	- 0 - 250DPS
 * 	- 1 - 500DPS
 * 	- 2 - 1000DPS
 * 	- 3 - 2000DPS
 */
[[nodiscard]] bool imu_setGyroSensitivity(imu_Imu* imu, uint8_t sensitivity);

/**
 * @brief Sets the accelerometer's sensitivity.
 * @param imu - the IMU instance
 * @param sensitivity - possible (+/-) values are:
 * 	- 0 - 2G
 * 	- 1 - 4G
 * 	- 2 - 8G
 * 	- 3 - 16G
 */
[[nodiscard]] bool imu_setAccSensitivity(imu_Imu* imu, uint8_t sensitivity);

#ifdef ASYNC_IMU
/**
 * @brief This function is only present when using the async version of the driver.
 * You can check whether new data is available to read using it.
 *
 * @return true, if new data is available (and clears the internal new data flag).
 */
[[nodiscard]] bool imu_newDataAvailable(imu_Imu* imu);
#endif

/**
 * @brief Returns a vector containing the gyro data.
 * @param imu - the IMU instance
 * @param out - the result gyro data
 *
 * @return the gyro data in rad/s.
 */
[[nodiscard]] bool imu_readGyroData(imu_Imu* imu, imu_Vec3* out);

/**
 * @brief Returns a vector containing the accelerometer data.
 * @param imu - the IMU instance
 * @param out - the result acc data
 *
 * @return the accelerometer data in g's.
 */
[[nodiscard]] bool imu_readAccData(imu_Imu* imu, imu_Vec3* out);

/**
 * @brief Returns the temperature data.
 * @param imu - the IMU instance
 *
 * @return the temperature in °C.
 */
[[nodiscard]] float imu_readTempData(imu_Imu* imu);

#endif // IMU_H
