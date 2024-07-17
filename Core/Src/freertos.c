/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"

#include "Keypad.h"
#include "BME280.h"
#include "ds3231_for_stm32_hal.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct
{
	float temperature;
	float preasurePA;
	float preasureHPA;
	float preasureMMHG;
	float humidity;
	float altitude;
} BMEValues;

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} DateTime;

typedef struct
{
	uint16_t raw_adc_value;
	uint8_t voltage_integer_part;
	uint8_t voltage_float_part;
	float voltage;
} BatteryVoltage;

typedef enum
{
	BatteryMenu, TemperatureMenu, TimeMenu
} Menu;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MAX_MSG_STRING_LENGTH 100

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

char msg_buffer[MAX_MSG_STRING_LENGTH];
Key key_pressed = None;
Key previos_key = None;

uint8_t menu_kursor = 2;
Menu menu_pages[] =
{ BatteryMenu, TemperatureMenu, TimeMenu };

BatteryVoltage battery =
{ 0, 0, 0.0f };

DateTime date_time =
{ 0, 0, 0, 0, 0, 0 };

BMEValues bme_values =
{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

/* USER CODE END Variables */
/* Definitions for readBattVoltage */
osThreadId_t readBattVoltageHandle;
const osThreadAttr_t readBattVoltage_attributes =
{ .name = "readBattVoltage", .stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for renderUI */
osThreadId_t renderUIHandle;
const osThreadAttr_t renderUI_attributes =
{ .name = "renderUI", .stack_size = 128 * 4, .priority = (osPriority_t) osPriorityAboveNormal, };
/* Definitions for pollKeypad */
osThreadId_t pollKeypadHandle;
const osThreadAttr_t pollKeypad_attributes =
{ .name = "pollKeypad", .stack_size = 128 * 4, .priority = (osPriority_t) osPriorityAboveNormal, };
/* Definitions for readBMEValues */
osThreadId_t readBMEValuesHandle;
const osThreadAttr_t readBMEValues_attributes =
{ .name = "readBMEValues", .stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for readDateTime */
osThreadId_t readDateTimeHandle;
const osThreadAttr_t readDateTime_attributes =
{ .name = "readDateTime", .stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for blinkStatusLED */
osThreadId_t blinkStatusLEDHandle;
const osThreadAttr_t blinkStatusLED_attributes =
{ .name = "blinkStatusLED", .stack_size = 128 * 4, .priority = (osPriority_t) osPriorityHigh, };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void showBatteryMenu();
void showTemperatureMenu();
void showTimeMenu();

/* USER CODE END FunctionPrototypes */

void StartReadBattVoltageTask(void *argument);
void StartRenderUITask(void *argument);
void StartPollKeypadTask(void *argument);
void StartReadBMEValuesTask(void *argument);
void StartReadDateTimeTask(void *argument);
void StartBlinkStatusLEDTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of readBattVoltage */
	readBattVoltageHandle = osThreadNew(StartReadBattVoltageTask, NULL, &readBattVoltage_attributes);

	/* creation of renderUI */
	renderUIHandle = osThreadNew(StartRenderUITask, NULL, &renderUI_attributes);

	/* creation of pollKeypad */
	pollKeypadHandle = osThreadNew(StartPollKeypadTask, NULL, &pollKeypad_attributes);

	/* creation of readBMEValues */
	readBMEValuesHandle = osThreadNew(StartReadBMEValuesTask, NULL, &readBMEValues_attributes);

	/* creation of readDateTime */
	readDateTimeHandle = osThreadNew(StartReadDateTimeTask, NULL, &readDateTime_attributes);

	/* creation of blinkStatusLED */
	blinkStatusLEDHandle = osThreadNew(StartBlinkStatusLEDTask, NULL, &blinkStatusLED_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartReadBattVoltageTask */
/**
 * @brief  Function implementing the readBattVoltage thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReadBattVoltageTask */
void StartReadBattVoltageTask(void *argument)
{
	/* USER CODE BEGIN StartReadBattVoltageTask */
	/* Infinite loop */

	for (;;)
	{
		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2, 10);
		battery.raw_adc_value = HAL_ADC_GetValue(&hadc2);
		HAL_ADC_Stop(&hadc2);
		battery.voltage = (float) (battery.raw_adc_value * (3.3f / 4096.0f));
		battery.voltage_integer_part = (int) battery.voltage;
		battery.voltage_float_part = (int) ((battery.voltage - battery.voltage_integer_part) * 100); // тут костыль надо будет разобратся
//		HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
		osDelay(100);

	}
	/* USER CODE END StartReadBattVoltageTask */
}

/* USER CODE BEGIN Header_StartRenderUITask */
/**
 * @brief Function implementing the renderUI thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartRenderUITask */
void StartRenderUITask(void *argument)
{
	/* USER CODE BEGIN StartRenderUITask */

	/* Infinite loop */
	for (;;)
	{
		switch (menu_pages[menu_kursor])
		{
		case BatteryMenu:
			showBatteryMenu();
			break;
		case TemperatureMenu:
			showTemperatureMenu();
			break;
		case TimeMenu:
			showTimeMenu();
			break;
		default:
			showBatteryMenu();
			break;
		}

		osDelay(25); // refresh rate
	}
	/* USER CODE END StartRenderUITask */
}

/* USER CODE BEGIN Header_StartPollKeypadTask */
/**
 * @brief Function implementing the pollKeypad thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartPollKeypadTask */
void StartPollKeypadTask(void *argument)
{
	/* USER CODE BEGIN StartPollKeypadTask */
	/* Infinite loop */
	for (;;)
	{
		key_pressed = pollKeypad();

		if ((previos_key != key_pressed) && (key_pressed != None))
		{
			previos_key = key_pressed;

			// print key for debugging
			sprintf(msg_buffer, "Key pressed: %s\r\n", keyToString(key_pressed));
			HAL_UART_Transmit(&huart2, (uint8_t*) msg_buffer, strlen(msg_buffer), 1000);

			// make beep sound
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500);
			osDelay(100);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);

			// change if needed menu cursor
			if (key_pressed == Left && menu_kursor > 0)
			{
//				if (menu_kursor == 0)
//				{
//					menu_kursor = sizeof(menu_pages) / sizeof(menu_pages[0]);
//				}
				menu_kursor--;
			}
			else if (key_pressed == Right && menu_kursor < (sizeof(menu_pages) / sizeof(menu_pages[0])))
			{
//				if (menu_kursor == (sizeof(menu_pages) / sizeof(menu_pages[0]) - 1))
//				{
//					menu_kursor = 0;
//				}
				menu_kursor++;
			}

		}
		osDelay(100);
	}
	/* USER CODE END StartPollKeypadTask */
}

/* USER CODE BEGIN Header_StartReadBMEValuesTask */
/**
 * @brief Function implementing the readBMEValues thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReadBMEValuesTask */
void StartReadBMEValuesTask(void *argument)
{
	/* USER CODE BEGIN StartReadBMEValuesTask */
	/* Infinite loop */
	for (;;)
	{
		// temperature
		bme_values.temperature = BME280_ReadTemperature();

		// preasures
//		bme_values.preasurePA = BME280_ReadPressure();
//		bme_values.preasureHPA = bme_values.preasurePA / 1000.0f;
//		bme_values.preasureMMHG = bme_values.preasurePA * 0.000750061683f;
//
//		// altitude
//		bme_values.altitude = BME280_ReadAltitude(SEALEVELPRESSURE_PA);
//
//		// humidity
//		bme_values.humidity = BME280_ReadHumidity();

		osDelay(2000);
	}
	/* USER CODE END StartReadBMEValuesTask */
}

/* USER CODE BEGIN Header_StartReadDateTimeTask */
/**
 * @brief Function implementing the readDateTime thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReadDateTimeTask */
void StartReadDateTimeTask(void *argument)
{
	/* USER CODE BEGIN StartReadDateTimeTask */
	/* Infinite loop */
	for (;;)
	{
		date_time.hour = DS3231_GetHour();
		date_time.minute = DS3231_GetMinute();
		date_time.second = DS3231_GetSecond();
		sprintf(msg_buffer, "%d : %d : %d\r\n", date_time.hour, date_time.minute, date_time.second);
		HAL_UART_Transmit(&huart2, (uint8_t*) msg_buffer, strlen(msg_buffer), 1000);
		osDelay(1000);
	}
	/* USER CODE END StartReadDateTimeTask */
}

/* USER CODE BEGIN Header_StartBlinkStatusLEDTask */
/**
 * @brief Function implementing the blinkStatusLED thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartBlinkStatusLEDTask */
void StartBlinkStatusLEDTask(void *argument)
{
	/* USER CODE BEGIN StartBlinkStatusLEDTask */
	/* Infinite loop */
	for (;;)
	{
		HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
		osDelay(1000);
	}
	/* USER CODE END StartBlinkStatusLEDTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void showBatteryMenu()
{
	ssd1306_Fill(Black); // clear display
	ssd1306_SetCursor(20, 5);
	sprintf(msg_buffer, "%d.%02d V", battery.voltage_integer_part, battery.voltage_float_part);
	ssd1306_WriteString(msg_buffer, Font_16x26, White);
	ssd1306_UpdateScreen();
}

void showTemperatureMenu()
{
	ssd1306_Fill(Black); // clear display
	ssd1306_SetCursor(0, 0);
	sprintf(msg_buffer, "%d *C", (int) bme_values.temperature);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}

void showTimeMenu()
{
	ssd1306_Fill(Black); // clear display
	ssd1306_SetCursor(0, 0);
	sprintf(msg_buffer, "%d : %d : %d", date_time.hour, date_time.minute, date_time.second);
	ssd1306_WriteString(msg_buffer, Font_7x10, White);
	ssd1306_UpdateScreen();
}

/* USER CODE END Application */

