/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM4_Init(void);
static void MX_IWDG_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

#define PULSE_TIME_MS 100

#define SAMPLE_UPDATE_TICK (uint16_t)((0.3)*(1000/PULSE_TIME_MS))

#define NETWORK_UPDATE_TICK (uint16_t)((45)*(1000/PULSE_TIME_MS))
#define DHT11_UPDATE_TICK (uint16_t)((0.8)*(1000/PULSE_TIME_MS))
#define LED_UPDATE_TICK (uint16_t)((0.5)*(1000/PULSE_TIME_MS))
#define ESP_DEBUG

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t debug=0;
uint16_t counter;
uint8_t ESP8266_STATUS=0;
void delay_us(uint32_t i)
{
  counter=i&0xffff;
	debug=counter;

	TIM4->CNT=counter+1;
	HAL_TIM_Base_Start(&htim4);
	while(TIM4->CNT>0);
	HAL_TIM_Base_Stop(&htim4);
}

void HOME_SCREEN(void)
{
	LCD_Clean();
	if(ESP8266_STATUS==1)
		l_print("IntelliSw",0,Middle);
	else if(ESP8266_STATUS==0)
		l_print("Fail To Trans",0,Middle);
	else if(ESP8266_STATUS==2)
		l_print("RECONF NETWORK",0,Middle);
	else if(ESP8266_STATUS==3)
		l_print("NoLinkToServer",0,Middle);
	l_print("Wek HH:mm:ss",1,Middle);
	l_print("TEMP:",2,Left);
	l_print("QNH:",3,Left);
	l_print("ILLU:",4,Left);
	l_print("HUMI:",5,Left);
}

#define SSID_ADDR 0x08020000

void FlashRead(void)
{
	__IO uint8_t* temp=(__IO uint8_t*)(SSID_ADDR);
	memset(ESP_SSID,0,sizeof(ESP_SSID));
	memset(ESP_PASSWD,0,sizeof(ESP_PASSWD));
	memset(ESP_TCP_IPADDR,0,sizeof(ESP_TCP_IPADDR));
	memset(ESP_TCP_PORT,0,sizeof(ESP_TCP_PORT));
	
	memcpy(ESP_SSID,(uint8_t*)temp,16);
	memcpy(ESP_PASSWD,(uint8_t*)(temp+16),20);
	memcpy(ESP_TCP_IPADDR,(uint8_t*)(temp+16+20),20);
	memcpy(ESP_TCP_PORT,(uint8_t*)(temp+16+20+20),8);
}

void FlashWrite(void)
{
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef f={0};
	f.TypeErase=FLASH_TYPEERASE_SECTORS;
	f.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	f.Sector=FLASH_SECTOR_5;
	f.NbSectors=1;
	uint32_t error=0;
	HAL_FLASHEx_Erase(&f,&error);
	for(uint8_t i=0;i<16;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,SSID_ADDR+i,ESP_SSID[i]);
	for(uint8_t i=0;i<20;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,SSID_ADDR+16+i,ESP_PASSWD[i]);
	for(uint8_t i=0;i<20;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,SSID_ADDR+16+20+i,ESP_TCP_IPADDR[i]);
	for(uint8_t i=0;i<8;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,SSID_ADDR+16+20+20+i,ESP_TCP_PORT[i]);
	
	HAL_FLASH_Lock();
}


//format $24.71:52:1013.52:100.0
uint8_t databuff[32];

void MakeData(void)
{
	memset(databuff,0,sizeof(databuff));
	sprintf((char*)databuff,"$%.2f:%hhu:%.2f:%.1f#",bmp_280_temperature,DHT11_HUMIDITY,bmp_280_atmospressure,illuminance);
	
}

uint8_t rev_cnt=0;
uint8_t display_esp=0;
uint8_t LED_ON=1;
uint8_t rev_char=0;
uint8_t rev_buff[128];
uint8_t rev_char_uart2=0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==huart1.Instance)
	{
		//#ifndef ESP_DEBUG
		if(ESP8266_STATUS==1)
		{
			if(rev_char=='$')
			{
				rev_buff[rev_cnt++]=rev_char;
			}
			else if(rev_char=='#')
			{
				display_esp=1;
			}
			else if(rev_buff[0]=='$')
			{
				rev_buff[rev_cnt++]=rev_char;
			}
		}
		//#else
		//不对命令做任何解析 直接返回串口2
		//HAL_UART_Transmit(&huart2,&rev_char,1,100);
		//#endif
		ESP_GETS(rev_char);
		HAL_UART_Receive_IT(&huart1,&rev_char,1);
	}
	else if(huart->Instance==huart2.Instance)
	{
		HAL_UART_Transmit(&huart1,&rev_char_uart2,1,100);
		HAL_UART_Receive_IT(&huart2,&rev_char_uart2,1);
	}
}


uint8_t rtc_time[14]="Wed 15:58:35";
uint8_t week[3]="Wed";
char timebuff[14];

RTC_TimeTypeDef server_Time = {0};//服务器更新的时间
RTC_TimeTypeDef now_Time = {0};//当前时间

void RTC_Update(void)
{
	//012345 len=6
	//$DTWed 16:35:27
	uint8_t hour,minute,second;
	uint8_t temp_buff[14];
	memset(temp_buff,0,sizeof(temp_buff));
	memcpy(temp_buff,rev_buff+3,strlen((const char*)rev_buff)-3);
	
	sscanf((char*)temp_buff,"%s %hhu:%hhu:%hhu",week,&hour,&minute,&second);
	server_Time.Hours = hour;
  server_Time.Minutes = minute;
  server_Time.Seconds = second;
  server_Time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  server_Time.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &server_Time, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
}



void Display_Time(void)
{
	//3
	
	uint8_t hour,minute,second;
	RTC_DateTypeDef sDate={0};
	HAL_RTC_GetTime(&hrtc,&now_Time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
	hour=now_Time.Hours;minute=now_Time.Minutes;second=now_Time.Seconds;
	
	memset(timebuff,0,sizeof(timebuff));
	sprintf(timebuff,"%s %02hhu:%02hhu:%02hhu",week,hour,minute,second);
	l_print(timebuff,1,Middle);
	ESP8266_WatchDog();
}

//用户应用抢占屏幕显示
uint8_t AppMode=0;

void AppHandler_Char(void)
{
	AppMode=1;
	char scrbuffer[14];
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3,14);
	l_print(scrbuffer,0,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14,14);
	l_print(scrbuffer,1,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14+14,14);
	l_print(scrbuffer,2,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14+14+14,14);
	l_print(scrbuffer,3,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14+14+14+14,14);
	l_print(scrbuffer,4,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14+14+14+14+14,14);
	l_print(scrbuffer,5,Left);
}

void AppHandler_Pixel(void)
{
	AppMode=1;
	char scrbuffer[14];
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3,14);
	l_print(scrbuffer,0,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14,14);
	l_print(scrbuffer,1,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14+14,14);
	l_print(scrbuffer,2,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14+14+14,14);
	l_print(scrbuffer,3,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14+14+14+14,14);
	l_print(scrbuffer,4,Left);
	
	memset(scrbuffer,0,sizeof(scrbuffer));
	memcpy(scrbuffer,rev_buff+3+14+14+14+14+14,14);
	l_print(scrbuffer,5,Left);
}

void Data_Receive(void)
{
	//$DTWed 15:09:22
	if(rev_buff[1]=='c'&&rev_buff[2]=='h')
	{
		//设备号一定要是个位数！！
		uint8_t deviceID=rev_buff[3]-48;
		if(deviceID==0)
		{
			LED_ON=rev_buff[5]-48;
		}
		else if(deviceID==1)
		{
			if(rev_buff[5]-48==0)
				HAL_GPIO_WritePin(IOT_OUTPUT_1_GPIO_Port,IOT_OUTPUT_1_Pin,GPIO_PIN_RESET);
			else
				HAL_GPIO_WritePin(IOT_OUTPUT_1_GPIO_Port,IOT_OUTPUT_1_Pin,GPIO_PIN_SET);
		}
		else if(deviceID==2)
		{
			if(rev_buff[5]-48==0)
				HAL_GPIO_WritePin(IOT_OUTPUT_2_GPIO_Port,IOT_OUTPUT_2_Pin,GPIO_PIN_RESET);
			else
				HAL_GPIO_WritePin(IOT_OUTPUT_2_GPIO_Port,IOT_OUTPUT_2_Pin,GPIO_PIN_SET);
		}
		else if(deviceID==3)
		{
			if(rev_buff[5]-48==0)
				HAL_GPIO_WritePin(IOT_OUTPUT_3_GPIO_Port,IOT_OUTPUT_3_Pin,GPIO_PIN_RESET);
			else
				HAL_GPIO_WritePin(IOT_OUTPUT_3_GPIO_Port,IOT_OUTPUT_3_Pin,GPIO_PIN_SET);
		}
		else if(deviceID==4)
		{
			if(rev_buff[5]-48==0)
				HAL_GPIO_WritePin(IOT_OUTPUT_4_GPIO_Port,IOT_OUTPUT_4_Pin,GPIO_PIN_RESET);
			else
				HAL_GPIO_WritePin(IOT_OUTPUT_4_GPIO_Port,IOT_OUTPUT_4_Pin,GPIO_PIN_SET);
		}
		else if(deviceID==5)
		{
			if(rev_buff[5]-48==0)
				HAL_GPIO_WritePin(IOT_OUTPUT_5_GPIO_Port,IOT_OUTPUT_5_Pin,GPIO_PIN_RESET);
			else
				HAL_GPIO_WritePin(IOT_OUTPUT_5_GPIO_Port,IOT_OUTPUT_5_Pin,GPIO_PIN_SET);
		}
	}
	else if(rev_buff[1]=='D'&&rev_buff[2]=='T')
	{
		RTC_Update();
	}
	else if(rev_buff[1]=='A'&&rev_buff[2]=='P')
	{
		AppHandler_Char();
		//$AF00000000000000 
		//   11111111111111 
		//   AAAAAAAAAAAAAA 
		//   zzzzzzzzzzzzzz
		//   LCD5110TEST233
		//   OKOKOKOKOKOKOK#
		//$AP0000000000000011111111111111AAAAAAAAAAAAAAzzzzzzzzzzzzzzLCD5110TEST233OKOKOKOKOKOKOK# 
	}
	else if(rev_buff[1]=='A'&&rev_buff[2]=='p')
	{
		AppHandler_Char();
		//$AF00000000000000 
		//   11111111111111 
		//   AAAAAAAAAAAAAA 
		//   zzzzzzzzzzzzzz
		//   LCD5110TEST233
		//   OKOKOKOKOKOKOK#
		//$AP0000000000000011111111111111AAAAAAAAAAAAAAzzzzzzzzzzzzzzLCD5110TEST233OKOKOKOKOKOKOK# 
	}
	else if(rev_buff[1]=='E'&&rev_buff[2]=='A')
	{
		//关闭用户抢占模式，恢复屏幕显示
		AppMode=0;
		HOME_SCREEN();
	}
	else if(rev_buff[1]=='E'&&rev_buff[2]=='R'&&rev_buff[3]=='S')
	{
		//$ERS#
		ESP_SendAT("AT+RESTORE");
		HAL_Delay(500);
		ESP_ClientInit();
		HOME_SCREEN();
	}
	else if(rev_buff[1]=='F'&&rev_buff[2]=='R'&&rev_buff[3]=='S')
	{
		//$FRS# 复位
		HAL_NVIC_SystemReset();
	}
	else if(rev_buff[1]=='F'&&rev_buff[2]=='R'&&rev_buff[3]=='S')
	{
		//$SRS# 屏幕初始化
		LCD_Init();
		HOME_SCREEN();
	}
}



volatile uint16_t sample_cnt=SAMPLE_UPDATE_TICK;
volatile uint16_t dht11cnt=DHT11_UPDATE_TICK,ledcnt=LED_UPDATE_TICK;
volatile uint16_t net_cnt=NETWORK_UPDATE_TICK;
volatile uint8_t intellisw_mode;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart1,&rev_char,1);
	HAL_UART_Receive_IT(&huart2,&rev_char_uart2,1);
	LCD_Init();
	intellisw_mode=HAL_GPIO_ReadPin(Netconf_GPIO_Port,Netconf_Pin);
	FlashRead();
	if(intellisw_mode)
		ESP8266_STATUS=ESP_ClientInit();
	else
		ESP_ServerInit();
	bmp280_init();
	HOME_SCREEN();
	HAL_Delay(200);
	//HomeScreen();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	MX_IWDG_Init();
  while (1)
  {
		if(sample_cnt>=SAMPLE_UPDATE_TICK)
		{
			bmp280_getTemperature();
			bmp280_getAtmosPressure();
			bh1750_getIlluminance();
			sample_cnt=0;
		}
		HAL_Delay(PULSE_TIME_MS);
		
		if(dht11cnt>=DHT11_UPDATE_TICK)
		{
			DHT11_Output();
			dht11cnt=0;
		}
		if(ledcnt>=LED_UPDATE_TICK)
		{
			if(LED_ON)
				HAL_GPIO_TogglePin(LED_TEST_GPIO_Port,LED_TEST_Pin);
			else
				HAL_GPIO_WritePin(LED_TEST_GPIO_Port,LED_TEST_Pin,GPIO_PIN_SET);
			ledcnt=0;
		}
		if(net_cnt>=NETWORK_UPDATE_TICK)
		{
			MakeData();
			ESP_SendData((char*)databuff);
			net_cnt=0;
		}
		if(display_esp==1)
		{
			Data_Receive();
			display_esp=0;
			rev_cnt=0;
			memset(rev_buff,0,sizeof(rev_buff));
		}
		if(!AppMode)
			Display_Time();
		
		dht11cnt++;
		net_cnt++;
		ledcnt++;
		sample_cnt++;
		
		HAL_IWDG_Refresh(&hiwdg);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 10000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 2000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 13;
  sTime.Minutes = 47;
  sTime.Seconds = 55;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 20;
  sDate.Year = 21;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 25202-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim3.Init.Period = 50000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 84-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim4.Init.Period = 1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 19200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_SCK_Pin|LCD_DIN_Pin|IOT_OUTPUT_5_Pin|IOT_OUTPUT_3_Pin 
                          |IOT_OUTPUT_4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CE_Pin|LCD_RST_Pin|LCD_DC_Pin|IOT_OUTPUT_1_Pin 
                          |DHT11_PORT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, IOT_OUTPUT_2_Pin|ESP8266_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_TEST_Pin */
  GPIO_InitStruct.Pin = LED_TEST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_TEST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_SCK_Pin LCD_DIN_Pin */
  GPIO_InitStruct.Pin = LCD_SCK_Pin|LCD_DIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CE_Pin LCD_RST_Pin LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_CE_Pin|LCD_RST_Pin|LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : IOT_OUTPUT_1_Pin IOT_OUTPUT_2_Pin ESP8266_RST_Pin */
  GPIO_InitStruct.Pin = IOT_OUTPUT_1_Pin|IOT_OUTPUT_2_Pin|ESP8266_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : IOT_OUTPUT_5_Pin IOT_OUTPUT_3_Pin IOT_OUTPUT_4_Pin */
  GPIO_InitStruct.Pin = IOT_OUTPUT_5_Pin|IOT_OUTPUT_3_Pin|IOT_OUTPUT_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : DHT11_PORT_Pin */
  GPIO_InitStruct.Pin = DHT11_PORT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_PORT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Netconf_Pin */
  GPIO_InitStruct.Pin = Netconf_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Netconf_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
	
  /* USER CODE BEGIN Callback 1 */
	else if(htim->Instance==htim3.Instance)
	{
		//ESP_REV_FLAG=1;
		HAL_TIM_Base_Stop_IT(&htim3);
	}
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
