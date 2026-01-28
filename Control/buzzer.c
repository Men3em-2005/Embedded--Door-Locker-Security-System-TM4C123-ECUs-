/******************************************************************************
 * File: buzzer.c
 * Module: Buzzer Driver
 * Description: Buzzer HAL implementation
 ******************************************************************************/

#include "buzzer.h"
#include "tm4c123gh6pm.h"

/******************************************************************************
 *                              Pin Configuration                              *
 ******************************************************************************/

/*
 * Buzzer pin connected to Port F
 * BUZZER -> PF1
 */
#define BUZZER_PIN          1
#define BUZZER_PIN_MASK     (1 << BUZZER_PIN)

/******************************************************************************
 *                          Function Definitions                               *
 ******************************************************************************/

/*
 * Buzzer_Init
 * Initializes PF1 as output pin for buzzer control.
 * Sets the pin to LOW initially (buzzer off).
 * Note: Preserves other Port F pins (motor uses PF0 and PF4)
 */
void Buzzer_Init(void) {
    volatile unsigned long delay;
    
    /* Enable clock for Port F (may already be enabled by motor) */
    SYSCTL_RCGCGPIO_R |= 0x20;  /* Enable clock for Port F */
    
    /* Small delay for clock stabilization */
    delay = SYSCTL_RCGCGPIO_R;
    delay = SYSCTL_RCGCGPIO_R;
    
    /* Wait for Port F to be ready */
    while((SYSCTL_PRGPIO_R & 0x20) == 0);
    
    /* Configure PF1 - use |= and &= to preserve other pins */
    GPIO_PORTF_DIR_R |= BUZZER_PIN_MASK;       /* Set PF1 as output (preserve PF0, PF4) */
    GPIO_PORTF_AFSEL_R &= ~BUZZER_PIN_MASK;    /* Disable alternate function */
    GPIO_PORTF_DEN_R |= BUZZER_PIN_MASK;       /* Enable digital function on PF1 */
    GPIO_PORTF_AMSEL_R &= ~BUZZER_PIN_MASK;    /* Disable analog function */
    GPIO_PORTF_PCTL_R &= ~0x000000F0;          /* Clear PCTL for PF1 (bits 7-4) */
    GPIO_PORTF_DATA_R &= ~BUZZER_PIN_MASK;     /* Start with buzzer off (LOW) */
}

/*
 * Buzzer_On
 * Turns the buzzer on by setting PF1 to HIGH.
 */
void Buzzer_On(void) {
    GPIO_PORTF_DATA_R |= BUZZER_PIN_MASK;
}

/*
 * Buzzer_Off
 * Turns the buzzer off by setting PF1 to LOW.
 */
void Buzzer_Off(void) {
    GPIO_PORTF_DATA_R &= ~BUZZER_PIN_MASK;
}

/*
 * Buzzer_Toggle
 * Toggles the buzzer state.
 */
void Buzzer_Toggle(void) {
    GPIO_PORTF_DATA_R ^= BUZZER_PIN_MASK;
}

/*
 * Buzzer_Beep
 * Makes the buzzer beep for a specified duration.
 * Generates a ~4kHz tone by toggling for passive buzzers.
 * Higher frequency = louder volume for most passive buzzers.
 */
void Buzzer_Beep(uint16_t duration_ms) {
    uint32_t i;
    /* Use 4kHz frequency for louder sound - 250us period = 125us high + 125us low */
    uint32_t toggles = duration_ms * 4000 / 1000;  /* 4000 toggles per second */
    
    /* Generate tone by toggling */
    for (i = 0; i < toggles; i++) {
        GPIO_PORTF_DATA_R ^= BUZZER_PIN_MASK;
        
        /* Delay for ~125us (half period of 4kHz) */
        /* At 16MHz, need ~2000 cycles for 125us */
        /* Each loop iteration ~4 cycles, so ~500 iterations */
        volatile uint32_t delay = 500;
        while(delay--);
    }
    
    /* Ensure buzzer is off after beep */
    GPIO_PORTF_DATA_R &= ~BUZZER_PIN_MASK;
}
