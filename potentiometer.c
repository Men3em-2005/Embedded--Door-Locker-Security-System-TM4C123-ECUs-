/******************************************************************************
 * File: potentiometer.c
 * Module: Potentiometer HAL (Hardware Abstraction Layer)
 * Description: Source file for Potentiometer HAL on TM4C123GH6PM
 ******************************************************************************/

#include "potentiometer.h"
#include "adc.h"

/******************************************************************************
 *                         Function Definitions                                *
 ******************************************************************************/

/*
 * Description: Initializes the potentiometer (ADC on PE3)
 * Parameters: None
 * Returns: None
 */
void POT_Init(void)
{
    /* Initialize ADC with channel 0 (PE3 = AIN0) */
    ADC_Init(POT_ADC_CHANNEL);
}

/*
 * Description: Reads the raw ADC value from the potentiometer
 * Parameters: None
 * Returns: 12-bit ADC value (0-4095)
 */
uint16_t POT_ReadRaw(void)
{
    return ADC_Read();
}

/*
 * Description: Reads the potentiometer voltage in millivolts
 * Parameters: None
 * Returns: Voltage in millivolts (0-3300)
 */
uint32_t POT_ReadMillivolts(void)
{
    uint16_t rawValue = ADC_Read();
    return ADC_ToMillivolts(rawValue);
}

/*
 * Description: Reads the potentiometer position as a percentage
 * Parameters: None
 * Returns: Position percentage (0-100)
 */
uint8_t POT_ReadPercentage(void)
{
    uint16_t rawValue = ADC_Read();
    /* Convert to percentage: (rawValue * 100) / 4095 */
    return (uint8_t)((rawValue * 100UL) / 4095UL);
}

/*
 * Description: Maps the potentiometer reading to a custom range
 * Parameters:
 *   - min: Minimum value of output range
 *   - max: Maximum value of output range
 * Returns: Mapped value within the specified range
 */
uint32_t POT_ReadMapped(uint32_t min, uint32_t max)
{
    uint16_t rawValue = ADC_Read();
    
    /* Map from 0-4095 to min-max range */
    /* Formula: min + (rawValue * (max - min)) / 4095 */
    return min + ((rawValue * (max - min)) / 4095UL);
}
