/******************************************************************************
 * File: uart.h
 * Module: UART (Universal Asynchronous Receiver/Transmitter)
 * Description: Header file for TM4C123GH6PM UART5 Driver (TivaWare)
 * 
 * Configuration:
 *   - UART5 (PE4: RX, PE5: TX)
 *   - Baud Rate: 115200
 *   - Data: 8 bits
 *   - Parity: None
 *   - Stop: 1 bit
 ******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 *                          Function Prototypes                                *
 ******************************************************************************/

/*
 * UART5_Init
 * Initializes UART5 with 115200 baud rate, 8N1 configuration using TivaWare.
 * Uses PE4 (RX) and PE5 (TX).
 * System clock is assumed to be 16 MHz.
 */
void UART5_Init(void);

/*
 * UART5_SendChar
 * Transmits a single character through UART5.
 * Blocks until the transmit FIFO is ready.
 * 
 * Parameters:
 *   data - Character to transmit
 */
void UART5_SendChar(char data);

/*
 * UART5_ReceiveChar
 * Receives a single character from UART5.
 * Blocks until a character is available in the receive FIFO.
 * 
 * Returns:
 *   Received character
 */
char UART5_ReceiveChar(void);

/*
 * UART5_SendString
 * Transmits a null-terminated string through UART5.
 * 
 * Parameters:
 *   str - Pointer to null-terminated string to transmit
 */
void UART5_SendString(const char *str);

/*
 * UART5_IsDataAvailable
 * Checks if data is available in the receive FIFO.
 * 
 * Returns:
 *   1 if data is available, 0 otherwise
 */
uint8_t UART5_IsDataAvailable(void);

#endif /* UART_H_ */
