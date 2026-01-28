/******************************************************************************
 * File: buzzer.h
 * Module: Buzzer Driver
 * Description: Header file for Buzzer HAL
 ******************************************************************************/

#ifndef BUZZER_H_
#define BUZZER_H_

#include <stdint.h>

/******************************************************************************
 * Function Prototypes
 * API for Buzzer control.
 ******************************************************************************/

/*
 * Buzzer_Init
 * Initializes the buzzer control pin (PF1) as output.
 * Must be called before using other buzzer functions.
 */
void Buzzer_Init(void);

/*
 * Buzzer_On
 * Turns the buzzer on.
 */
void Buzzer_On(void);

/*
 * Buzzer_Off
 * Turns the buzzer off.
 */
void Buzzer_Off(void);

/*
 * Buzzer_Toggle
 * Toggles the buzzer state.
 */
void Buzzer_Toggle(void);

/*
 * Buzzer_Beep
 * Makes the buzzer beep for a specified duration.
 * Works with both active and passive buzzers.
 * For passive buzzers, generates a tone by toggling.
 * 
 * Parameters:
 *   duration_ms - Duration of beep in milliseconds
 */
void Buzzer_Beep(uint16_t duration_ms);

#endif /* BUZZER_H_ */
