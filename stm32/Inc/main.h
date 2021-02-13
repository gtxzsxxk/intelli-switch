/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "LCD_Driver.h"
#include "esp8266_Driver.h"
#include "bmp280.h"
#include "illuminanceMeas.h"
#include "DHT11.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern I2C_HandleTypeDef hi2c1;
extern uint8_t AppMode;
extern RTC_TimeTypeDef now_Time;
extern RTC_TimeTypeDef server_Time;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim3;
void HOME_SCREEN(void);

void FlashRead(void);
void FlashWrite(void);
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void delay_us(uint32_t i);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_TEST_Pin GPIO_PIN_13
#define LED_TEST_GPIO_Port GPIOC
#define LCD_SCK_Pin GPIO_PIN_5
#define LCD_SCK_GPIO_Port GPIOA
#define LCD_DIN_Pin GPIO_PIN_7
#define LCD_DIN_GPIO_Port GPIOA
#define LCD_CE_Pin GPIO_PIN_0
#define LCD_CE_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_1
#define LCD_RST_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_2
#define LCD_DC_GPIO_Port GPIOB
#define IOT_OUTPUT_1_Pin GPIO_PIN_12
#define IOT_OUTPUT_1_GPIO_Port GPIOB
#define IOT_OUTPUT_2_Pin GPIO_PIN_13
#define IOT_OUTPUT_2_GPIO_Port GPIOB
#define IOT_OUTPUT_5_Pin GPIO_PIN_8
#define IOT_OUTPUT_5_GPIO_Port GPIOA
#define IOT_OUTPUT_3_Pin GPIO_PIN_11
#define IOT_OUTPUT_3_GPIO_Port GPIOA
#define IOT_OUTPUT_4_Pin GPIO_PIN_12
#define IOT_OUTPUT_4_GPIO_Port GPIOA
#define DHT11_PORT_Pin GPIO_PIN_4
#define DHT11_PORT_GPIO_Port GPIOB
#define ESP8266_RST_Pin GPIO_PIN_5
#define ESP8266_RST_GPIO_Port GPIOB
#define Netconf_Pin GPIO_PIN_9
#define Netconf_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
