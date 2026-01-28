/******************************************************************************
 * File: potentiometer.h
 * Module: Potentiometer HAL (Hardware Abstraction Layer)
 * Description: Header file for Potentiometer HAL on TM4C123GH6PM
 ******************************************************************************/

#ifndef POTENTIOMETER_H_
#define POTENTIOMETER_H_

#include <stdint.h>

/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/

/*
 * Potentiometer Configuration
 * Connected to PE3 (AIN0)
 */
#define POT_PIN             3       /* PE3 */
#define POT_ADC_CHANNEL     0       /* AIN0 */

/******************************************************************************
 *                         Function Prototypes                                 *
 ******************************************************************************/

/*
 * Description: Initializes the potentiometer (ADC on PE3)
 * Parameters: None
 * Returns: None
 */
void POT_Init(void);

/*
 * Description: Reads the raw ADC value from the potentiometer
 * Parameters: None
 * Returns: 12-bit ADC value (0-4095)
 */
uint16_t POT_ReadRaw(void);

/*
 * Description: Reads the potentiometer voltage in millivolts
 * Parameters: None
 * Returns: Voltage in millivolts (0-3300)
 */
uint32_t POT_ReadMillivolts(void);

/*
 * Description: Reads the potentiometer position as a percentage
 * Parameters: None
 * Returns: Position percentage (0-100)
 */
uint8_t POT_ReadPercentage(void);

/*
 * Description: Maps the potentiometer reading to a custom range
 * Parameters:
 *   - min: Minimum value of output range
 *   - max: Maximum value of output range
 * Returns: Mapped value within the specified range
 */
uint32_t POT_ReadMapped(uint32_t min, uint32_t max);

#endif /* POTENTIOMETER_H_ */
