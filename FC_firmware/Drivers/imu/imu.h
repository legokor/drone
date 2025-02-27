#ifndef IMU_H
#define IMU_H

#define ASYNC_IMU

#include "stm32f4xx_hal.h"

/**
 * @brief MPU9250 IMU and compass IC I2C driver for the STM32 microcontroller.
 * The implementation heavily relies on the HAL STM32 drivers.
 */
typedef struct {
	SPI_HandleTypeDef* hspi;
    TIM_HandleTypeDef* htim;

	float accSensitivity;
	float gyroSensitivity;

	float gyroOffsetX, gyroOffsetY, gyroOffsetZ;
	uint8_t useGyroOffsets;

	volatile uint8_t imuData[14];
    uint8_t readMemAddress;

#ifdef ASYNC_IMU
	volatile uint8_t imuBuffer[14];
	volatile uint8_t newData;

	IRQn_Type readIr;

	volatile uint8_t readEnabled;
	volatile uint8_t readInProgress;
#endif
} imu_Imu;

typedef struct {
	float x, y, z;
} imu_Vec3;

#ifdef ASYNC_IMU
/**
 * @brief Initializes the MPU9250 IMU driver for async data retrieval using DMA.
 * @param imu - the IMU instance
 * @param hspi - the SPI handle for the IMU
 * @param dmaIr - the IRQn_Type handle of the SPI event interrupt
 * @param htim - the timer handle for the periodical data read
 * 
 * @return 1 on success, else 0
 */
uint8_t imu_Init(imu_Imu* imu, SPI_HandleTypeDef* hspi, IRQn_Type readIr, TIM_HandleTypeDef* htim);
#else
/**
 * @brief Initializes the MPU9250 IMU driver for sync (blocking) data retrieval.
 * @param imu - the IMU instance
 * @param hspi - the SPI handle for the IMU
 * 
 * @return 1 on success, else 0
 */
uint8_t imu_Init(imu_Imu* imu, SPI_HandleTypeDef* hspi);
#endif

/**
 * @brief Checks whether the IMU (MPU9250) was detected.
 * @param imu - the IMU instance
 *
 * @return true, if the IMU (MPU9250) was detected.
 */
uint8_t imu_DetectImu(imu_Imu* imu);

/**
 * @brief Sets the IMU to the default settings, which will suite most applications:
 * 	- The magnetometer is set to 16 bit resolution with a 100Hz sample rate
 * 	- The accelerometer is set to +-4g range with a 200Hz sample rate and a 44.8Hz DLPF
 * 	- The gyro and thermometer are set to 2000DPS range (in case of the gyro) with a 200Hz sample rate and a 41Hz DLPF
 * @param imu - the IMU instance
 *
 */
void imu_SetDefaultSettings(imu_Imu* imu);

/**
 * @brief Calculates the gyro offsets while at rest using a lot of samples.
 * @param imu - the IMU instance
 */
void imu_CalculateGyroOffset(imu_Imu* imu);

/**
 * @brief Enables the calculated gyro offset subtraction from the results
 * @param imu - the IMU instance
 * @param enabled - 1 to enable, 0 to disable
 */
void imu_EnableGyroOffsetSubtraction(imu_Imu* imu, uint8_t enabled);

/**
 * @brief Sets the sample rate divider.
 * The divider is only active when DLPF is enabled and set to 1-6 (both in case of the gyro/temp and the accelerometer).
 * @param imu - the IMU instance
 * @param divider - the divider that divides the internal (1kHz sample rate) by (1 + divider).
 */
void imu_SetSampleRateDivider(imu_Imu* imu, uint8_t divider);

/**
 * @brief Enable DLPF for the accelerometer (set fchoice_b to 0 -> fchoice to 1).
 * @param imu - the IMU instance
 * @param enable - whether to enable the DLPF for the accelerometer.
 */
void imu_EnableAccDLPF(imu_Imu* imu, uint8_t enable);

/**
 * @brief Enable DLPF for the gyro and temp sensors (set fchoice_b's to 0 -> fchoice's to 1).
 * @param imu - the IMU instance
 * @param enable - whether to enable the DLPF for the gyro and thermometer.
 */
void imu_EnableGyroAndTempDLPF(imu_Imu* imu, uint8_t enable);

/**
 * @brief Details about the values: https://invensense.tdk.com/wp-content/uploads/2015/02/RM-MPU-9250A-00-v1.6.pdf
 * 	- table on page 15
 * @param imu - the IMU instance
 * @param value - the accelerometer DLPF value
 */
void imu_SetAccDLPF(imu_Imu* imu, uint8_t value);

/**
 * @brief Details about the values: https://invensense.tdk.com/wp-content/uploads/2015/02/RM-MPU-9250A-00-v1.6.pdf
 * 	- table on page 13
 * @param imu - the IMU instance
 * @param value - the gyro and thermometer DLPF value
 */
void imu_SetGyroAndTempDLPF(imu_Imu* imu, uint8_t value);

/**
 * @brief Sets the gyro's sensitivity.
 * @param imu - the IMU instance
 * @param sensitivity - possible (+/-) values are:
 * 	- 0 - 250DPS
 * 	- 1 - 500DPS
 * 	- 2 - 1000DPS
 * 	- 3 - 2000DPS
 */
void imu_SetGyroSensitivity(imu_Imu* imu, uint8_t sensitivity);

/**
 * @brief Sets the accelerometer's sensitivity.
 * @param imu - the IMU instance
 * @param sensitivity - possible (+/-) values are:
 * 	- 0 - 2G
 * 	- 1 - 4G
 * 	- 2 - 8G
 * 	- 3 - 16G
 */
void imu_SetAccSensitivity(imu_Imu* imu, uint8_t sensitivity);

#ifdef ASYNC_IMU
/**
 * @brief This function is only present when using the async version of the driver.
 * You can check whether new data is available to read using it.
 *
 * @return true, if new data is available (and clears the internal new data flag).
 */
uint8_t imu_NewDataAvailable(imu_Imu* imu);
#endif

/**
 * @brief Returns a vector containing the gyro data.
 * @param imu - the IMU instance
 * 
 * @return the gyro data in °/s.
 */
imu_Vec3 imu_ReadGyroData(imu_Imu* imu);

/**
 * @brief Returns a vector containing the accelerometer data.
 * @param imu - the IMU instance
 * 
 * @return the accelerometer data in g's.
 */
imu_Vec3 imu_ReadAccData(imu_Imu* imu);

/**
 * @brief Returns the temperature data.
 * @param imu - the IMU instance
 * 
 * @return the temperature in °C.
 */
float imu_ReadTempData(imu_Imu* imu);

#endif // IMU_H
