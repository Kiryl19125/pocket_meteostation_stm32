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

#include "pocket_meteostation/pocket_meteostation.h"
#include "Float_transform/Float_transform.h"
#include "Keypad/Keypad.h"
#include "BME280/BME280.h"
#include "ds3231/ds3231_for_stm32_hal.h"

#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

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
/* USER CODE BEGIN Variables */

// variables for processing float numbers
extern uint8_t sign_number;
extern int integer_number;
extern uint32_t fractional_number;

extern BatteryVoltage_t battery;
extern BMEValues_t bme_values;
extern DateTime_t date_time;


extern Menu_t menu_pages[];
extern uint8_t menu_kursor;
extern uint8_t temp_menu_kursor;
extern uint8_t press_menu_kursor;
extern uint8_t batt_menu_kursor;
extern uint8_t settings_menu_cursor;
extern uint8_t set_time_cursor;

extern bool setTimeMode;

extern Key key_pressed;
extern Key previos_key;

/* USER CODE END Variables */
/* Definitions for readBattVoltage */
osThreadId_t readBattVoltageHandle;
const osThreadAttr_t readBattVoltage_attributes =
{ .name = "readBattVoltage", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for renderUI */
osThreadId_t renderUIHandle;
const osThreadAttr_t renderUI_attributes =
{ .name = "renderUI", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityAboveNormal, };
/* Definitions for pollKeypad */
osThreadId_t pollKeypadHandle;
const osThreadAttr_t pollKeypad_attributes =
{ .name = "pollKeypad", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityAboveNormal, };
/* Definitions for readBMEValues */
osThreadId_t readBMEValuesHandle;
const osThreadAttr_t readBMEValues_attributes =
{ .name = "readBMEValues", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for readDateTime */
osThreadId_t readDateTimeHandle;
const osThreadAttr_t readDateTime_attributes =
{ .name = "readDateTime", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for blinkStatusLED */
osThreadId_t blinkStatusLEDHandle;
const osThreadAttr_t blinkStatusLED_attributes =
{ .name = "blinkStatusLED", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityHigh, };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */


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
		battery.percentage = (100.0f * battery.raw_adc_value) / 4095.0f;
		Float_transform(battery.voltage, 2, &sign_number, &integer_number, &fractional_number);
		battery.voltage_integer_part = integer_number;
		battery.voltage_float_part = fractional_number;
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
			if (batt_menu_kursor == 0)
				showBatteryMenuPercent();
			else if (batt_menu_kursor == 1)
				showBatteryMenuVolts();
			break;
		case TemperatureMenu:
			if (temp_menu_kursor == 0)
				showTemperatureMenu();
			else if (temp_menu_kursor == 1)
				showTemperatureMenuF();
			break;
		case TimeMenu:
			showTimeMenu();
			break;
		case HumidityMenu:
			showHumidityMenu();
			break;
		case PreassureMenu:
			if (press_menu_kursor == 0)
				showPressureMenuMMHG();
			else if (press_menu_kursor == 1)
				showPressureMenuHPA();
			break;
		case AltitudeMenu:
			showAltitudeMenu();
			break;
		case SettingsMenu:
			if (setTimeMode)
			{
				vTaskSuspend(readDateTimeHandle);
				showSetTimeMenu();
			}
			else
			{
				vTaskResume(readDateTimeHandle);
				showSettingsMenu();
			}
			break;
		default:
			showTimeMenu();
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

		if (key_pressed != None)
		{
			previos_key = key_pressed;

			processKey(key_pressed);
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
		bme_values.preassurePA = BME280_ReadPressure();
		bme_values.preassureHPA = bme_values.preassurePA / 1000.0f;
		bme_values.preassureMMHG = bme_values.preassurePA * 0.000750061683f;
//
//		// altitude
		bme_values.altitude = BME280_ReadAltitude(SEALEVELPRESSURE_PA);
//
//		// humidity
		bme_values.humidity = BME280_ReadHumidity();

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

		date_time.year = DS3231_GetYear();
		date_time.month = DS3231_GetMonth();
		date_time.day = DS3231_GetDate();
		osDelay(500);
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
		osDelay(500);
	}
	/* USER CODE END StartBlinkStatusLEDTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


/* USER CODE END Application */

