
/*****************************************************************************
 * File: keypad.c
 * Description: 4x4 Keypad Driver for TM4C123GH6PM
 ******************************************************************************/

#include "keypad.h"
#include "dio.h"

/*
 * Keypad mapping array.
 * Each element represents the character returned for a specific key press.
 * The array is organized as [row][column], matching the physical keypad layout.
 */
const char keypad_codes[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

/*
 * Pin configuration for keypad interface.
 * Columns are connected to PortC pins PC4-PC7 (outputs).
 * Rows are connected to PortA pins PA2-PA5 (inputs with pull-up).
 */
#define KEYPAD_COL_PORT PORTC
#define KEYPAD_COL_PINS {PIN4, PIN5, PIN6, PIN7} // PC4-PC7

#define KEYPAD_ROW_PORT PORTA
#define KEYPAD_ROW_PINS {PIN2, PIN3, PIN4, PIN5} // PA2-PA5


/*
 * Keypad_Init
 * Initializes the GPIO pins for keypad operation.
 * - Rows are set as inputs with internal pull-up resistors (PortA).
 * - Columns are set as outputs and driven HIGH (PortB).
 * This function must be called before using Keypad_GetKey.
 */
void Keypad_Init(void) {
    uint8_t row_pins[4] = KEYPAD_ROW_PINS;
    uint8_t col_pins[4] = KEYPAD_COL_PINS;
    // Configure rows (PortA) as input with pull-up
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_ROW_PORT, row_pins[i], INPUT);
        DIO_SetPUR(KEYPAD_ROW_PORT, row_pins[i], ENABLE);
    }
    // Configure columns (PortB) as output and set HIGH
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_COL_PORT, col_pins[i], OUTPUT);
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[i], HIGH);
    }
}


/*
 * Keypad_GetKey
 * Scans the keypad and returns the character of the pressed key.
 * Returns 0 if no key is pressed.
 *
 * Scanning logic:
 *   1. Set each column LOW one at a time, others HIGH.
 *   2. Read all row inputs; if any row reads LOW, a key is pressed.
 *   3. Wait for key release (debounce).
 *   4. Return the mapped character from keypad_codes.
 */
char Keypad_GetKey(void) {
    uint8_t row_pins[4] = KEYPAD_ROW_PINS;
    uint8_t col_pins[4] = KEYPAD_COL_PINS;
    for (uint8_t col = 0; col < 4; col++) {
        // Set all columns HIGH (inactive)
        for (uint8_t c = 0; c < 4; c++) {
            DIO_WritePin(KEYPAD_COL_PORT, col_pins[c], HIGH);
        }
        // Set current column LOW (active)
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[col], LOW);
        // Small delay for signal to settle
        for (volatile int d = 0; d < 100; d++);
        // Scan rows for key press
        for (uint8_t row = 0; row < 4; row++) {
            uint8_t pin_val = DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]);
            if (pin_val == LOW) {
                // Key detected at (col, row)
                // Wait for key release (debounce)
                while (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW);
                // Return the mapped character from keypad_codes
                return keypad_codes[row][col];
            }
        }
    }
    return 0; // No key pressed
}
