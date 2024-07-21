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
#include "Float_transform/Float_transform.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct
{
	float temperature;
	float preassurePA;
	float preassureHPA;
	float preassureMMHG;
	float humidity;
	float altitude;
} BMEValues_t;

typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} DateTime_t;

typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} SetDateTime_t;

typedef struct
{
	uint16_t raw_adc_value;
	uint8_t voltage_integer_part;
	uint8_t voltage_float_part;
	float voltage;
	float percentage;
} BatteryVoltage_t;

typedef enum
{
	BatteryMenu, TemperatureMenu, TimeMenu, HumidityMenu, PreassureMenu, AltitudeMenu, SettingsMenu
} Menu_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MAX_MSG_STRING_LENGTH 100
#define MENU_PAGES_LENGTH 7

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

char msg_buffer[MAX_MSG_STRING_LENGTH];

Key key_pressed = None;
Key previos_key = None;

uint8_t menu_kursor = 0;
uint8_t temp_menu_kursor = 0;
uint8_t press_menu_kursor = 0;
uint8_t batt_menu_kursor = 0;
uint8_t settings_menu_cursor = 0;
uint8_t set_time_cursor = 0;

bool soundOn = true;
bool setTimeMode = false;

bool editYearMode = false;
bool editMonthMode = false;
bool editDayMode = false;
bool editHourMode = false;
bool editMinuteMode = false;
bool editSecondMode = false;

Menu_t menu_pages[] =
{ TimeMenu, TemperatureMenu, HumidityMenu, PreassureMenu, AltitudeMenu, BatteryMenu, SettingsMenu };

BatteryVoltage_t battery =
{ 0, 0, 0.0f, 0.0f };

DateTime_t date_time =
{ 0, 0, 0, 0, 0, 0 };

SetDateTime_t set_date_time =
{ 2000, 1, 1, 0, 0, 0 };

BMEValues_t bme_values =
{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

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

static void showBatteryMenuVolts();
static void showBatteryMenuPercent();

static void showTemperatureMenu();
static void showTemperatureMenuF();

static void showTimeMenu();
static void showSetTimeMenu();

static void showHumidityMenu();
static void showAltitudeMenu();

static void showPressureMenuHPA();
static void showPressureMenuMMHG();

static void showSettingsMenu();

static void processKey(Key key);

static void makeBeepSound();

static void setNewTime();

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
//				vTaskSuspend(readDateTimeHandle);
				showSetTimeMenu();
			}
			else
			{
//				vTaskResume(readDateTimeHandle);
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

static void showBatteryMenuVolts()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Battery voltage", Font_7x10, White);

	ssd1306_SetCursor(0, 14);
	sprintf(msg_buffer, "%d.%02d V", battery.voltage_integer_part, battery.voltage_float_part);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}

static void showBatteryMenuPercent()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Battery percentage", Font_7x10, White);

	ssd1306_SetCursor(0, 14);
	Float_transform(battery.percentage, 1, &sign_number, &integer_number, &fractional_number);
	sprintf(msg_buffer, "%d.%01ld %%", integer_number, fractional_number);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}

static void showTemperatureMenu()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Temperature", Font_7x10, White);

	ssd1306_SetCursor(0, 14);
	Float_transform(bme_values.temperature, 1, &sign_number, &integer_number, &fractional_number);
	sprintf(msg_buffer, "%d.%.01ld *C", integer_number, fractional_number);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}

static void showTemperatureMenuF()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Temperature", Font_7x10, White);

	ssd1306_SetCursor(0, 14);
	Float_transform(bme_values.temperature * (9.0f / 5.0f) + 32, 1, &sign_number, &integer_number, &fractional_number);
	sprintf(msg_buffer, "%d.%.01ld *F", integer_number, fractional_number);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}

static void showTimeMenu()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	sprintf(msg_buffer, "%d/%d/%d", date_time.day, date_time.month, date_time.year);
	ssd1306_WriteString(msg_buffer, Font_7x10, White);

	ssd1306_SetCursor(0, 12);
	sprintf(msg_buffer, "%d:%d:%d", date_time.hour, date_time.minute, date_time.second);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);

	ssd1306_UpdateScreen();
}

static void showSetTimeMenu()
{

//	ssd1306_DrawRectangle(0, 0, 127, 31, White);
//	ssd1306_SetCursor(4, 2);
//
//	set_date_time.year = date_time.year;
//	set_date_time.month = date_time.month;
//	set_date_time.day = date_time.day;
//	set_date_time.hour = date_time.hour;
//	set_date_time.minute = date_time.minute;
//	set_date_time.second = date_time.second;

	ssd1306_Fill(Black); // clear display

	if (set_time_cursor >= 0 && set_time_cursor < 3)
	{
		// first page
		ssd1306_SetCursor(10, 0);
		sprintf(msg_buffer, "Year: %d", set_date_time.year);
		ssd1306_WriteString(msg_buffer, Font_7x10, White);

		ssd1306_SetCursor(10, 11);
		sprintf(msg_buffer, "Month: %d", set_date_time.month);
		ssd1306_WriteString(msg_buffer, Font_7x10, White);

		ssd1306_SetCursor(10, 22);
		sprintf(msg_buffer, "Day: %d", set_date_time.day);
		ssd1306_WriteString(msg_buffer, Font_7x10, White);

		ssd1306_SetCursor(0, (set_time_cursor * 10) + 1);
		ssd1306_WriteString(">", Font_7x10, White);

		if (editYearMode || editMonthMode || editDayMode || editHourMode || editMinuteMode || editSecondMode)
		{
			ssd1306_SetCursor(100, (set_time_cursor * 10) + 1);
			ssd1306_WriteString("<--", Font_7x10, White);
		}
		else
		{
			ssd1306_SetCursor(120, (set_time_cursor * 10) + 1);
			ssd1306_WriteString("<", Font_7x10, White);
		}

	}
	else if (set_time_cursor >= 3 && set_time_cursor < 6)
	{
		// second page
		ssd1306_SetCursor(10, 0);
		sprintf(msg_buffer, "Hour: %d", set_date_time.hour);
		ssd1306_WriteString(msg_buffer, Font_7x10, White);

		ssd1306_SetCursor(10, 11);
		sprintf(msg_buffer, "Minute: %d", set_date_time.minute);
		ssd1306_WriteString(msg_buffer, Font_7x10, White);

		ssd1306_SetCursor(10, 22);
		sprintf(msg_buffer, "Second: %d", set_date_time.second);
		ssd1306_WriteString(msg_buffer, Font_7x10, White);

		ssd1306_SetCursor(0, ((set_time_cursor - 3) * 10) + 1);
		ssd1306_WriteString(">", Font_7x10, White);

		if (editYearMode || editMonthMode || editDayMode || editHourMode || editMinuteMode || editSecondMode)
		{
			ssd1306_SetCursor(100, ((set_time_cursor - 3) * 10) + 1);
			ssd1306_WriteString("<--", Font_7x10, White);
		}
		else
		{
			ssd1306_SetCursor(120, ((set_time_cursor - 3) * 10) + 1);
			ssd1306_WriteString("<", Font_7x10, White);
		}

	}

	ssd1306_UpdateScreen();
//
//	sprintf(msg_buffer, "%d/%d/%d", set_date_time.day, set_date_time.month, set_date_time.year);
//	ssd1306_WriteString(msg_buffer, Font_7x10, White);
//
//	ssd1306_SetCursor(4, 11);
//	sprintf(msg_buffer, "%d:%d:%d", set_date_time.hour, set_date_time.minute, set_date_time.second);
//	ssd1306_WriteString(msg_buffer, Font_11x18, White);
//	ssd1306_UpdateScreen();
}

static void showHumidityMenu()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Humidity", Font_7x10, White);

	ssd1306_SetCursor(0, 14);
	Float_transform(bme_values.humidity, 1, &sign_number, &integer_number, &fractional_number);
	sprintf(msg_buffer, "%d.%.01ld %%", integer_number, fractional_number);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}

static void showAltitudeMenu()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Altitude", Font_7x10, White);

	ssd1306_SetCursor(0, 14);
	Float_transform(bme_values.altitude, 1, &sign_number, &integer_number, &fractional_number);
	sprintf(msg_buffer, "%d.%.01ld m", integer_number, fractional_number);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}

static void showPressureMenuHPA()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Pressure", Font_7x10, White);

	ssd1306_SetCursor(0, 14);
	Float_transform(bme_values.preassureHPA, 1, &sign_number, &integer_number, &fractional_number);
	sprintf(msg_buffer, "%d.%.01ld hPA", integer_number, fractional_number);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();

}

static void showPressureMenuMMHG()
{
	ssd1306_Fill(Black); // clear display

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Pressure", Font_7x10, White);

	ssd1306_SetCursor(0, 14);
	Float_transform(bme_values.preassureMMHG, 1, &sign_number, &integer_number, &fractional_number);
	sprintf(msg_buffer, "%d.%.01ld mmHg", integer_number, fractional_number);
	ssd1306_WriteString(msg_buffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}

static void showSettingsMenu()
{
	ssd1306_Fill(Black);

	ssd1306_SetCursor(10, 0);
	if (soundOn)
		ssd1306_WriteString("Sound: ON", Font_7x10, White);
	else if (!soundOn)
		ssd1306_WriteString("Sound: OFF", Font_7x10, White);

	ssd1306_SetCursor(10, 12);
	ssd1306_WriteString("Set time", Font_7x10, White);

	ssd1306_SetCursor(0, (settings_menu_cursor * 10) + 2);
	ssd1306_WriteString(">", Font_7x10, White);
	ssd1306_SetCursor(120, (settings_menu_cursor * 10) + 2);
	ssd1306_WriteString("<", Font_7x10, White);

	ssd1306_UpdateScreen();
}

static void makeBeepSound()
{
	if (soundOn)
	{
		// make beep sound
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500);
		osDelay(100);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
	}
}

static void setNewTime()
{
	DS3231_SetYear(set_date_time.year);
	DS3231_SetMonth(set_date_time.month);
	DS3231_SetDate(set_date_time.day);
	DS3231_SetHour(set_date_time.hour);
	DS3231_SetMinute(set_date_time.minute);
	DS3231_SetSecond(set_date_time.second);
}

static void processKey(Key key)
{
	if (key == Left)
	{
		if (setTimeMode)
		{
			makeBeepSound();
			setTimeMode = false;
			setNewTime();
		}
		else
		{
			makeBeepSound();
			if (menu_kursor > 0)
			{
				menu_kursor--;
			}
			else if (menu_kursor == 0)
			{
				menu_kursor = MENU_PAGES_LENGTH - 1; // last element if manu_pages
			}
		}
	}
	else if (key == Right && !setTimeMode)
	{
		makeBeepSound();
		if (menu_kursor < MENU_PAGES_LENGTH - 1)
		{
			menu_kursor++;
		}
		else if (menu_kursor == MENU_PAGES_LENGTH - 1)
		{
			menu_kursor = 0;
		}
	}
	else if (key == Up)
	{
		if (setTimeMode)
		{
			makeBeepSound();
			if (editYearMode)
			{
				if (set_date_time.year >= 2000 && set_date_time.year < 2199)
					set_date_time.year++;
			}
			else if (editMonthMode)
			{
				if (set_date_time.month >= 1 && set_date_time.month < 12)
					set_date_time.month++;
			}
			else if (editDayMode)
			{
				if (set_date_time.day >= 1 && set_date_time.day < 31)
					set_date_time.day++;
			}
			else if (editHourMode)
			{
				if (set_date_time.hour >= 0 && set_date_time.hour < 23)
					set_date_time.hour++;
			}
			else if (editMinuteMode)
			{
				if (set_date_time.minute >= 0 && set_date_time.minute < 59)
					set_date_time.minute++;
			}
			else if (editSecondMode)
			{
				if (set_date_time.second >= 0 && set_date_time.second < 59)
					set_date_time.second++;
			}
			else
			{
				if (set_time_cursor > 0)
					set_time_cursor--;
			}

		}
		else
		{
			if (menu_pages[menu_kursor] == TemperatureMenu)
			{
				makeBeepSound();
				if (temp_menu_kursor == 0)
					temp_menu_kursor = 1;
				else if (temp_menu_kursor == 1)
					temp_menu_kursor = 0;
			}
			if (menu_pages[menu_kursor] == PreassureMenu)
			{
				makeBeepSound();
				if (press_menu_kursor == 0)
					press_menu_kursor = 1;
				else if (press_menu_kursor == 1)
					press_menu_kursor = 0;
			}
			if (menu_pages[menu_kursor] == BatteryMenu)
			{
				makeBeepSound();
				if (batt_menu_kursor == 0)
					batt_menu_kursor = 1;
				else if (batt_menu_kursor == 1)
					batt_menu_kursor = 0;
			}
			if (menu_pages[menu_kursor] == SettingsMenu)
			{
				makeBeepSound();
				if (settings_menu_cursor > 0)
					settings_menu_cursor--;
			}
		}

	}
	else if (key == Down)
	{
		if (setTimeMode)
		{
			makeBeepSound();
			if (editYearMode)
			{
				if (set_date_time.year > 2000 && set_date_time.year <= 2199)
					set_date_time.year--;
			}
			else if (editMonthMode)
			{
				if (set_date_time.month > 1 && set_date_time.month <= 12)
					set_date_time.month--;
			}
			else if (editDayMode)
			{
				if (set_date_time.day > 1 && set_date_time.day <= 31)
					set_date_time.day--;
			}
			else if (editHourMode)
			{
				if (set_date_time.hour > 0 && set_date_time.hour <= 23)
					set_date_time.hour--;
			}
			else if (editMinuteMode)
			{
				if (set_date_time.minute > 0 && set_date_time.minute <= 59)
					set_date_time.minute--;
			}
			else if (editSecondMode)
			{
				if (set_date_time.second > 0 && set_date_time.second <= 59)
					set_date_time.second--;
			}
			else
			{
				if (set_time_cursor < 5)
					set_time_cursor++;
			}
		}
		else
		{
			if (menu_pages[menu_kursor] == TemperatureMenu)
			{
				makeBeepSound();
				if (temp_menu_kursor == 0)
					temp_menu_kursor = 1;
				else if (temp_menu_kursor == 1)
					temp_menu_kursor = 0;
			}
			if (menu_pages[menu_kursor] == PreassureMenu)
			{
				makeBeepSound();
				if (press_menu_kursor == 0)
					press_menu_kursor = 1;
				else if (press_menu_kursor == 1)
					press_menu_kursor = 0;
			}
			if (menu_pages[menu_kursor] == BatteryMenu)
			{
				makeBeepSound();
				if (batt_menu_kursor == 0)
					batt_menu_kursor = 1;
				else if (batt_menu_kursor == 1)
					batt_menu_kursor = 0;
			}
			if (menu_pages[menu_kursor] == SettingsMenu)
			{
				makeBeepSound();
				if (settings_menu_cursor < 1)
					settings_menu_cursor++;
			}
		}

	}
	else if (key == Middle)
	{
		if (menu_pages[menu_kursor] == SettingsMenu)
		{
			makeBeepSound();
			if (setTimeMode)
			{
				// TODO
				switch (set_time_cursor)
				{
				case 0:
					editYearMode = !editYearMode;
					break;
				case 1:
					editMonthMode = !editMonthMode;
					break;
				case 2:
					editDayMode = !editDayMode;
					break;
				case 3:
					editHourMode = !editHourMode;
					break;
				case 4:
					editMinuteMode = !editMinuteMode;
					break;
				case 5:
					editSecondMode = !editSecondMode;
					break;
				}
			}
			else
			{
				if (settings_menu_cursor == 0)
					soundOn = !soundOn;
				else if (settings_menu_cursor == 1)
					setTimeMode = true;
			}
		}
	}
}
/* USER CODE END Application */

