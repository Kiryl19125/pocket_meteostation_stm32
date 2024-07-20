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
	if (HAL_GPIO_ReadPin(BTN_LEFT_GPIO_Port, BTN_LEFT_Pin) == GPIO_PIN_SET)
	{
		return Left;
	}

	// check up key
	if (HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin) == GPIO_PIN_SET)
	{
		return Up;
	}

	// check middle key
	if (HAL_GPIO_ReadPin(BTN_MIDDLE_GPIO_Port, BTN_MIDDLE_Pin) == GPIO_PIN_SET)
	{
		return Middle;
	}

	// check down key
	if (HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin) == GPIO_PIN_SET)
	{
		return Down;
	}

	// check right key
	if (HAL_GPIO_ReadPin(BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin) == GPIO_PIN_SET)
	{
		return Right;
	}

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
