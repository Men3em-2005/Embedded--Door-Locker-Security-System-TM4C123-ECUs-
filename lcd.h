/*****************************************************************************
 * File: lcd.h
 * Description: Header for 16x2 LCD Driver (4-bit mode)
 * 
 * Pin Configuration:
 *   RS  -> PB0 (Register Select: 0=Command, 1=Data)
 *   EN  -> PB1 (Enable signal)
 *   D4  -> PB2 (Data bit 4)
 *   D5  -> PB3 (Data bit 5)
 *   D6  -> PB4 (Data bit 6)
 *   D7  -> PB5 (Data bit 7)
 *****************************************************************************/

#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/******************************************************************************
 *                              LCD Commands                                   *
 ******************************************************************************/

#define LCD_CLEAR           0x01    /* Clear display */
#define LCD_HOME            0x02    /* Return home */
#define LCD_ENTRY_MODE      0x06    /* Entry mode: increment cursor, no shift */
#define LCD_DISPLAY_ON      0x0C    /* Display ON, cursor OFF, blink OFF */
#define LCD_DISPLAY_OFF     0x08    /* Display OFF */
#define LCD_CURSOR_ON       0x0E    /* Display ON, cursor ON, blink OFF */
#define LCD_CURSOR_BLINK    0x0F    /* Display ON, cursor ON, blink ON */
#define LCD_SHIFT_LEFT      0x18    /* Shift display left */
#define LCD_SHIFT_RIGHT     0x1C    /* Shift display right */
#define LCD_4BIT_MODE       0x28    /* 4-bit mode, 2 lines, 5x8 font */
#define LCD_8BIT_MODE       0x38    /* 8-bit mode, 2 lines, 5x8 font */

/* Line addresses for 16x2 LCD */
#define LCD_LINE1           0x80    /* First line address */
#define LCD_LINE2           0xC0    /* Second line address */

/******************************************************************************
 *                          Function Prototypes                                *
 ******************************************************************************/

/*
 * LCD_Init
 * Initializes the LCD in 4-bit mode.
 * Must be called before any other LCD function.
 */
void LCD_Init(void);

/*
 * LCD_SendCommand
 * Sends a command to the LCD (RS = 0).
 * Parameters: command - LCD command byte
 */
void LCD_SendCommand(uint8_t command);

/*
 * LCD_SendData
 * Sends a data byte to the LCD (RS = 1).
 * Parameters: data - Character to display
 */
void LCD_SendData(uint8_t data);

/*
 * LCD_Clear
 * Clears the LCD display and returns cursor to home position.
 */
void LCD_Clear(void);

/*
 * LCD_SetCursor
 * Sets the cursor position on the LCD.
 * Parameters: 
 *   row - Row number (0 or 1)
 *   col - Column number (0 to 15)
 */
void LCD_SetCursor(uint8_t row, uint8_t col);

/*
 * LCD_WriteString
 * Writes a string to the LCD at the current cursor position.
 * Parameters: str - Pointer to null-terminated string
 */
void LCD_WriteString(const char *str);

/*
 * LCD_WriteChar
 * Writes a single character to the LCD at the current cursor position.
 * Parameters: c - Character to display
 */
void LCD_WriteChar(char c);

#endif /* LCD_H */
