
/*****************************************************************************
 * File: keypad.h
 * Description: Header for 4x4 Keypad Driver
 *****************************************************************************/

#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

/*
 * Keypad mapping array declaration.
 * Used to translate row/column indices to key values.
 */
extern const char keypad_codes[4][4];

/* Keypad dimensions */
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

/*
 * Initializes the keypad GPIO pins.
 * Must be called before using Keypad_GetKey.
 */
void Keypad_Init(void);

/*
 * Scans the keypad and returns the character of the pressed key.
 * Returns 0 if no key is pressed.
 */
char Keypad_GetKey(void);

#endif // KEYPAD_H
