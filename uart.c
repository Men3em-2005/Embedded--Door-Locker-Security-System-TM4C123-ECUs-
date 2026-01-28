/******************************************************************************
 * File: uart.c
 * Module: UART (Universal Asynchronous Receiver/Transmitter)
 * Description: Source file for TM4C123GH6PM UART5 Driver (TivaWare)
 * 
 * Configuration:
 *   - UART5 (PE4: RX, PE5: TX)
 *   - Baud Rate: 115200
 *   - Data: 8 bits
 *   - Parity: None
 *   - Stop: 1 bit
 *   - System Clock: 16 MHz
 * 
 * Note: This implementation uses TivaWare peripheral driver library.
 *       TivaWare functions simplify UART configuration and provide
 *       higher-level abstractions compared to direct register access.
 ******************************************************************************/

#include "uart.h"
#include <stdint.h>
#include <stdbool.h>

/* TivaWare includes */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/

#define SYSTEM_CLOCK    16000000    /* 16 MHz system clock */
#define BAUD_RATE       115200      /* Target baud rate */

/******************************************************************************
 *                          Function Implementations                           *
 ******************************************************************************/

/*
 * UART5_Init
 * Initializes UART5 with 115200 baud rate, 8N1 configuration using TivaWare.
 * 
 * TivaWare functions used:
 *   - SysCtlPeripheralEnable(): Enable peripheral clocks
 *   - GPIOPinConfigure(): Configure pin muxing
 *   - GPIOPinTypeUART(): Configure pins for UART alternate function
 *   - UARTConfigSetExpClk(): Configure UART parameters
 *   - UARTEnable(): Enable UART module
 */
void UART5_Init(void)
{
    /* 1. Enable peripheral clocks for UART5 and GPIOE */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    
    /* Wait for peripherals to be ready */
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART5));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    
    /* 2. Configure GPIO pins for UART functionality */
    /* PE4: U5RX (UART5 Receive) */
    /* PE5: U5TX (UART5 Transmit) */
    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    
    /* Set pin type to UART */
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    
    /* 3. Configure UART parameters */
    /* System clock, baud rate, 8 data bits, 1 stop bit, no parity */
    UARTConfigSetExpClk(UART5_BASE, SYSTEM_CLOCK, BAUD_RATE,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | 
                         UART_CONFIG_PAR_NONE));
    
    /* 4. Enable UART5 */
    UARTEnable(UART5_BASE);
}

/*
 * UART5_SendChar
 * Transmits a single character through UART5 using TivaWare.
 * Uses UARTCharPut() which blocks until FIFO has space.
 */
void UART5_SendChar(char data)
{
    /* UARTCharPut() blocks until space is available in TX FIFO */
    UARTCharPut(UART5_BASE, data);
}

/*
 * UART5_ReceiveChar
 * Receives a single character from UART5 using TivaWare.
 * Uses UARTCharGet() which blocks until data is available.
 */
char UART5_ReceiveChar(void)
{
    /* UARTCharGet() blocks until data is available in RX FIFO */
    return (char)UARTCharGet(UART5_BASE);
}

/*
 * UART5_SendString
 * Transmits a null-terminated string through UART5.
 */
void UART5_SendString(const char *str)
{
    while (*str != '\0')
    {
        UART5_SendChar(*str);
        str++;
    }
}

/*
 * UART5_IsDataAvailable
 * Checks if data is available in the receive FIFO using TivaWare.
 * Uses UARTCharsAvail() to check RX FIFO status.
 */
uint8_t UART5_IsDataAvailable(void)
{
    /* UARTCharsAvail() returns true if characters are available */
    return (UARTCharsAvail(UART5_BASE)) ? 1 : 0;
}
