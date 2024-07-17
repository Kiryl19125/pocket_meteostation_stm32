/*
 * Keypad.h
 *
 *  Created on: Jul 17, 2024
 *      Author: kiryl
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

typedef enum
{
	None, Up, Down, Left, Right, Middle
} Key;

Key pollKeypad();
const char* keyToString(Key key);

#endif /* INC_KEYPAD_H_ */
