/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LORA_IRQN_Pin GPIO_PIN_13
#define LORA_IRQN_GPIO_Port GPIOC
#define LORA_RST_Pin GPIO_PIN_14
#define LORA_RST_GPIO_Port GPIOC
#define STATUS_LED_Pin GPIO_PIN_2
#define STATUS_LED_GPIO_Port GPIOC
#define VBAT_ADC_Pin GPIO_PIN_0
#define VBAT_ADC_GPIO_Port GPIOA
#define IBAT_ADC_Pin GPIO_PIN_1
#define IBAT_ADC_GPIO_Port GPIOA
#define GEN_UART2_TX_Pin GPIO_PIN_2
#define GEN_UART2_TX_GPIO_Port GPIOA
#define GEN_UART2_RX_Pin GPIO_PIN_3
#define GEN_UART2_RX_GPIO_Port GPIOA
#define GEN_TIM2_CH1_Pin GPIO_PIN_5
#define GEN_TIM2_CH1_GPIO_Port GPIOA
#define ESC_1_Pin GPIO_PIN_6
#define ESC_1_GPIO_Port GPIOA
#define ESC_2_Pin GPIO_PIN_7
#define ESC_2_GPIO_Port GPIOA
#define ESC_3_Pin GPIO_PIN_0
#define ESC_3_GPIO_Port GPIOB
#define ESC_4_Pin GPIO_PIN_1
#define ESC_4_GPIO_Port GPIOB
#define GEN_TIM2_CH4_Pin GPIO_PIN_2
#define GEN_TIM2_CH4_GPIO_Port GPIOB
#define GEN_TIM2_CH3_Pin GPIO_PIN_10
#define GEN_TIM2_CH3_GPIO_Port GPIOB
#define IMU_IRQN_Pin GPIO_PIN_12
#define IMU_IRQN_GPIO_Port GPIOB
#define IMU_BAR_SCK_Pin GPIO_PIN_13
#define IMU_BAR_SCK_GPIO_Port GPIOB
#define IMU_BAR_MISO_Pin GPIO_PIN_14
#define IMU_BAR_MISO_GPIO_Port GPIOB
#define IMU_BAR_MOSI_Pin GPIO_PIN_15
#define IMU_BAR_MOSI_GPIO_Port GPIOB
#define IMU_CS_Pin GPIO_PIN_6
#define IMU_CS_GPIO_Port GPIOC
#define BAR_CS_Pin GPIO_PIN_7
#define BAR_CS_GPIO_Port GPIOC
#define GEN_TIM8_CH3_Pin GPIO_PIN_8
#define GEN_TIM8_CH3_GPIO_Port GPIOC
#define GEN_I2C3_SDA_Pin GPIO_PIN_9
#define GEN_I2C3_SDA_GPIO_Port GPIOC
#define GEN_I2C3_SCL_Pin GPIO_PIN_8
#define GEN_I2C3_SCL_GPIO_Port GPIOA
#define DEBUG_UART1_TX_Pin GPIO_PIN_9
#define DEBUG_UART1_TX_GPIO_Port GPIOA
#define DEBUG_UART1_RX_Pin GPIO_PIN_10
#define DEBUG_UART1_RX_GPIO_Port GPIOA
#define GPS_TX_Pin GPIO_PIN_10
#define GPS_TX_GPIO_Port GPIOC
#define GPS_RX_Pin GPIO_PIN_11
#define GPS_RX_GPIO_Port GPIOC
#define UART5_TX_NC_Pin GPIO_PIN_12
#define UART5_TX_NC_GPIO_Port GPIOC
#define SBUS_RX_Pin GPIO_PIN_2
#define SBUS_RX_GPIO_Port GPIOD
#define LORA_SCK_Pin GPIO_PIN_3
#define LORA_SCK_GPIO_Port GPIOB
#define LORA_MISO_Pin GPIO_PIN_4
#define LORA_MISO_GPIO_Port GPIOB
#define LORA_MOSI_Pin GPIO_PIN_5
#define LORA_MOSI_GPIO_Port GPIOB
#define MAG_SCL_Pin GPIO_PIN_6
#define MAG_SCL_GPIO_Port GPIOB
#define MAG_SDA_Pin GPIO_PIN_7
#define MAG_SDA_GPIO_Port GPIOB
#define GEN_TIM10_CH1_Pin GPIO_PIN_8
#define GEN_TIM10_CH1_GPIO_Port GPIOB
#define GEN_TIM2_CH2_Pin GPIO_PIN_9
#define GEN_TIM2_CH2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
