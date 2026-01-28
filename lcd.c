/*****************************************************************************
 * File: lcd.c
 * Description: 16x2 LCD Driver Implementation (4-bit mode)
 * 
 * Pin Configuration:
 *   RS  -> PB0 (Register Select: 0=Command, 1=Data)
 *   EN  -> PB1 (Enable signal)
 *   D4  -> PB2 (Data bit 4)
 *   D5  -> PB3 (Data bit 5)
 *   D6  -> PB4 (Data bit 6)
 *   D7  -> PB5 (Data bit 7)
 *****************************************************************************/

#include "lcd.h"
#include "dio.h"
#include "systick.h"

/******************************************************************************
 *                            Pin Definitions                                  *
 ******************************************************************************/

#define LCD_PORT        PORTB
#define LCD_RS          PIN0
#define LCD_EN          PIN1
#define LCD_D4          PIN2
#define LCD_D5          PIN3
#define LCD_D6          PIN4
#define LCD_D7          PIN5

/******************************************************************************
 *                          Private Functions                                  *
 ******************************************************************************/

/*
 * LCD_EnablePulse
 * Generates a high-to-low pulse on the EN pin to latch data/command.
 */
static void LCD_EnablePulse(void)
{
    DIO_WritePin(LCD_PORT, LCD_EN, HIGH);
    DelayMs(1);  /* Enable pulse width */
    DIO_WritePin(LCD_PORT, LCD_EN, LOW);
    DelayMs(1);  /* Wait for command to execute */
}

/*
 * LCD_Send4Bits
 * Sends 4 bits of data to the LCD via D4-D7 pins.
 * Parameters: nibble - 4-bit value to send (bits 0-3 used)
 */
static void LCD_Send4Bits(uint8_t nibble)
{
    DIO_WritePin(LCD_PORT, LCD_D4, (nibble >> 0) & 0x01);
    DIO_WritePin(LCD_PORT, LCD_D5, (nibble >> 1) & 0x01);
    DIO_WritePin(LCD_PORT, LCD_D6, (nibble >> 2) & 0x01);
    DIO_WritePin(LCD_PORT, LCD_D7, (nibble >> 3) & 0x01);
    LCD_EnablePulse();
}

/******************************************************************************
 *                          Public Functions                                   *
 ******************************************************************************/

/*
 * LCD_Init
 * Initializes the LCD in 4-bit mode.
 */
void LCD_Init(void)
{
    /* Initialize GPIO pins as outputs */
    DIO_Init(LCD_PORT, LCD_RS, OUTPUT);
    DIO_Init(LCD_PORT, LCD_EN, OUTPUT);
    DIO_Init(LCD_PORT, LCD_D4, OUTPUT);
    DIO_Init(LCD_PORT, LCD_D5, OUTPUT);
    DIO_Init(LCD_PORT, LCD_D6, OUTPUT);
    DIO_Init(LCD_PORT, LCD_D7, OUTPUT);
    
    /* Initial state: RS = 0, EN = 0 */
    DIO_WritePin(LCD_PORT, LCD_RS, LOW);
    DIO_WritePin(LCD_PORT, LCD_EN, LOW);
    
    /* Wait for LCD to power up (>15ms after VCC reaches 4.5V) */
    DelayMs(50);
    
    /* Initialization sequence for 4-bit mode */
    /* Send 0x03 three times to ensure 8-bit mode is cleared */
    DIO_WritePin(LCD_PORT, LCD_RS, LOW);  /* Command mode */
    
    LCD_Send4Bits(0x03);
    DelayMs(5);
    
    LCD_Send4Bits(0x03);
    DelayMs(1);
    
    LCD_Send4Bits(0x03);
    DelayMs(1);
    
    /* Set to 4-bit mode */
    LCD_Send4Bits(0x02);
    DelayMs(1);
    
    /* Function set: 4-bit mode, 2 lines, 5x8 font */
    LCD_SendCommand(LCD_4BIT_MODE);
    
    /* Display ON, cursor ON, blink OFF */
    LCD_SendCommand(LCD_CURSOR_ON);
    
    /* Clear display */
    LCD_Clear();
    
    /* Entry mode: increment cursor, no display shift */
    LCD_SendCommand(LCD_ENTRY_MODE);
}

/*
 * LCD_SendCommand
 * Sends a command to the LCD (RS = 0).
 */
void LCD_SendCommand(uint8_t command)
{
    DIO_WritePin(LCD_PORT, LCD_RS, LOW);  /* Command mode */
    
    /* Send upper nibble */
    LCD_Send4Bits(command >> 4);
    
    /* Send lower nibble */
    LCD_Send4Bits(command & 0x0F);
    
    /* Wait for command to execute */
    if (command == LCD_CLEAR || command == LCD_HOME) {
        DelayMs(2);  /* Clear and home commands take longer */
    } else {
        DelayMs(1);
    }
}

/*
 * LCD_SendData
 * Sends a data byte to the LCD (RS = 1).
 */
void LCD_SendData(uint8_t data)
{
    DIO_WritePin(LCD_PORT, LCD_RS, HIGH);  /* Data mode */
    
    /* Send upper nibble */
    LCD_Send4Bits(data >> 4);
    
    /* Send lower nibble */
    LCD_Send4Bits(data & 0x0F);
    
    DelayMs(1);  /* Wait for data to be written */
}

/*
 * LCD_Clear
 * Clears the LCD display and returns cursor to home position.
 */
void LCD_Clear(void)
{
    LCD_SendCommand(LCD_CLEAR);
    DelayMs(2);  /* Clear command takes longer to execute */
}

/*
 * LCD_SetCursor
 * Sets the cursor position on the LCD.
 * Parameters: 
 *   row - Row number (0 or 1)
 *   col - Column number (0 to 15)
 */
void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t address;
    
    if (row == 0) {
        address = LCD_LINE1 + col;
    } else {
        address = LCD_LINE2 + col;
    }
    
    LCD_SendCommand(address);
}

/*
 * LCD_WriteString
 * Writes a string to the LCD at the current cursor position.
 */
void LCD_WriteString(const char *str)
{
    while (*str != '\0') {
        LCD_SendData(*str);
        str++;
    }
}

/*
 * LCD_WriteChar
 * Writes a single character to the LCD at the current cursor position.
 */
void LCD_WriteChar(char c)
{
    LCD_SendData(c);
}
