/*
 * Keypad.c
 *
 *  Created on: Jul 17, 2024
 *      Author: kiryl
 */

#include "Keypad.h"
#include "main.h"

Key pollKeypad()
{
	// check left key
	HAL_GPIO_WritePin(Keypad_C1_OUT_GPIO_Port, Keypad_C1_OUT_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(Keypad_R2_IN_GPIO_Port, Keypad_R2_IN_Pin) == GPIO_PIN_SET)
	{
		return Left;
	}
	HAL_GPIO_WritePin(Keypad_C1_OUT_GPIO_Port, Keypad_C1_OUT_Pin, GPIO_PIN_RESET);

	// check up key
	HAL_GPIO_WritePin(Keypad_C2_OUT_GPIO_Port, Keypad_C2_OUT_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(Keypad_R1_IN_GPIO_Port, Keypad_R1_IN_Pin))
	{
		return Up;
	}
	HAL_GPIO_WritePin(Keypad_C2_OUT_GPIO_Port, Keypad_C2_OUT_Pin, GPIO_PIN_RESET);

	// check middle key
	HAL_GPIO_WritePin(Keypad_C2_OUT_GPIO_Port, Keypad_C2_OUT_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(Keypad_R2_IN_GPIO_Port, Keypad_R2_IN_Pin))
	{
		return Middle;
	}
	HAL_GPIO_WritePin(Keypad_C2_OUT_GPIO_Port, Keypad_C2_OUT_Pin, GPIO_PIN_RESET);

	// check down key
	HAL_GPIO_WritePin(Keypad_C2_OUT_GPIO_Port, Keypad_C2_OUT_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(Keypad_R3_IN_GPIO_Port, Keypad_R3_IN_Pin))
	{
		return Down;
	}
	HAL_GPIO_WritePin(Keypad_C2_OUT_GPIO_Port, Keypad_C2_OUT_Pin, GPIO_PIN_RESET);

	// check right key
	HAL_GPIO_WritePin(Keypad_C3_OUT_GPIO_Port, Keypad_C3_OUT_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(Keypad_R2_IN_GPIO_Port, Keypad_R2_IN_Pin))
	{
		return Right;
	}
	HAL_GPIO_WritePin(Keypad_C3_OUT_GPIO_Port, Keypad_C3_OUT_Pin, GPIO_PIN_RESET);

	return None;
}

const char* keyToString(Key key)
{
	switch (key)
	{
	case Up:
		return "Up";
	case Middle:
		return "Middle";
	case Down:
		return "Down";
	case Left:
		return "Left";
	case Right:
		return "Right";
	default:
		return "None";
	}
}
