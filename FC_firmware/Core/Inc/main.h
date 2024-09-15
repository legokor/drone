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
#define BAR_CSB_Pin GPIO_PIN_13
#define BAR_CSB_GPIO_Port GPIOC
#define ADC_U_Pin GPIO_PIN_1
#define ADC_U_GPIO_Port GPIOC
#define ADC_I_Pin GPIO_PIN_2
#define ADC_I_GPIO_Port GPIOC
#define SBUS_TX_Pin GPIO_PIN_0
#define SBUS_TX_GPIO_Port GPIOA
#define SBUS_RX_Pin GPIO_PIN_1
#define SBUS_RX_GPIO_Port GPIOA
#define GPS_TX_Pin GPIO_PIN_2
#define GPS_TX_GPIO_Port GPIOA
#define GPS_RX_Pin GPIO_PIN_3
#define GPS_RX_GPIO_Port GPIOA
#define LORA_NSS_Pin GPIO_PIN_4
#define LORA_NSS_GPIO_Port GPIOA
#define LORA_RST_Pin GPIO_PIN_5
#define LORA_RST_GPIO_Port GPIOA
#define ESC_1_Pin GPIO_PIN_6
#define ESC_1_GPIO_Port GPIOA
#define ESC_2_Pin GPIO_PIN_7
#define ESC_2_GPIO_Port GPIOA
#define LORA_IRQ_Pin GPIO_PIN_4
#define LORA_IRQ_GPIO_Port GPIOC
#define ESC_RX_Pin GPIO_PIN_5
#define ESC_RX_GPIO_Port GPIOC
#define ESC_3_Pin GPIO_PIN_0
#define ESC_3_GPIO_Port GPIOB
#define ESC_4_Pin GPIO_PIN_1
#define ESC_4_GPIO_Port GPIOB
#define GEN_MOSI_Pin GPIO_PIN_2
#define GEN_MOSI_GPIO_Port GPIOB
#define GEN_SCL_Pin GPIO_PIN_10
#define GEN_SCL_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_12
#define LED_GPIO_Port GPIOB
#define LORA_BAR_SCK_Pin GPIO_PIN_13
#define LORA_BAR_SCK_GPIO_Port GPIOB
#define LORA_BAR_MISO_Pin GPIO_PIN_14
#define LORA_BAR_MISO_GPIO_Port GPIOB
#define LORA_BAR_MOSI_Pin GPIO_PIN_15
#define LORA_BAR_MOSI_GPIO_Port GPIOB
#define GEN_TX_Pin GPIO_PIN_6
#define GEN_TX_GPIO_Port GPIOC
#define GEN_RX_Pin GPIO_PIN_7
#define GEN_RX_GPIO_Port GPIOC
#define GEN_TIM8_3_Pin GPIO_PIN_8
#define GEN_TIM8_3_GPIO_Port GPIOC
#define GEN_TIM8_4_Pin GPIO_PIN_9
#define GEN_TIM8_4_GPIO_Port GPIOC
#define GEN_TIM1_1_Pin GPIO_PIN_8
#define GEN_TIM1_1_GPIO_Port GPIOA
#define DEBUG_TX_Pin GPIO_PIN_9
#define DEBUG_TX_GPIO_Port GPIOA
#define DEBUG_RX_Pin GPIO_PIN_10
#define DEBUG_RX_GPIO_Port GPIOA
#define GEN_SCK_Pin GPIO_PIN_10
#define GEN_SCK_GPIO_Port GPIOC
#define GEN_MISO_Pin GPIO_PIN_11
#define GEN_MISO_GPIO_Port GPIOC
#define GEN_SDA_Pin GPIO_PIN_12
#define GEN_SDA_GPIO_Port GPIOC
#define IMU_IRQ_Pin GPIO_PIN_2
#define IMU_IRQ_GPIO_Port GPIOD
#define IMU_SCK_Pin GPIO_PIN_3
#define IMU_SCK_GPIO_Port GPIOB
#define IMU_MISO_Pin GPIO_PIN_4
#define IMU_MISO_GPIO_Port GPIOB
#define IMU_MOSI_Pin GPIO_PIN_5
#define IMU_MOSI_GPIO_Port GPIOB
#define GEN_TIM4_1_Pin GPIO_PIN_6
#define GEN_TIM4_1_GPIO_Port GPIOB
#define GEN_TIM4_2_Pin GPIO_PIN_7
#define GEN_TIM4_2_GPIO_Port GPIOB
#define MAG_SCL_Pin GPIO_PIN_8
#define MAG_SCL_GPIO_Port GPIOB
#define MAG_SDA_Pin GPIO_PIN_9
#define MAG_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
