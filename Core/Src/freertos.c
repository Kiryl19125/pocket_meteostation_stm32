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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct
{
	float_t temperature;
	float_t preasureHPA;
	float_t preasureMMHG;
	float_t humidity;
	float_t altitude;
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
	uint8_t voltage_integer_part;
	uint8_t voltage_float_part;
	float_t voltage;
} BatteryVoltage;
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

uint16_t raw_adc_value;
float voltage = 0.0f;
char msg_buffer[MAX_MSG_STRING_LENGTH];

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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartReadBattVoltageTask(void *argument);
void StartRenderUITask(void *argument);
void StartPollKeypadTask(void *argument);

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
		raw_adc_value = HAL_ADC_GetValue(&hadc2);
		HAL_ADC_Stop(&hadc2);
		voltage = (float) (raw_adc_value * (3.3f / 4096.0f));
		HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
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
		ssd1306_Fill(Black);
		ssd1306_SetCursor(0, 0);
		sprintf(msg_buffer, "Raw: %d", raw_adc_value);
		ssd1306_WriteString(msg_buffer, Font_7x10, White);

		ssd1306_SetCursor(0, 12);
		sprintf(msg_buffer, "V: %d.%02d", (int) voltage, (int) ((voltage - (int) (voltage)) * 100)); // тут костыль надо будет разобратся
		ssd1306_WriteString(msg_buffer, Font_7x10, White);
		ssd1306_UpdateScreen();
		osDelay(100);
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
		Key key_pressed = pollKeypad();
		sprintf(msg_buffer, "Key pressed: %s\r\n", keyToString(key_pressed));
		HAL_UART_Transmit(&huart2, (uint8_t*) msg_buffer, strlen(msg_buffer), 1000);
		if (key_pressed != None)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500);
			osDelay(100);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
		}
		osDelay(100);
	}
	/* USER CODE END StartPollKeypadTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

