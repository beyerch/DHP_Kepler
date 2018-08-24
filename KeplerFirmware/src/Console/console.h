/*
 * console.h
 *
 * Created: 3/14/2017 10:50:38 AM
 *  Author: adeck
 */ 


#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "ioport.h"
#include "string.h"
#include <asf.h>

//#define DEBUG_OUTPUT_ENABLED

#define CONSOLE_UART				UART0
#define CONSOLE_UART_ID				ID_UART0

#define USART_CONSOLE				(CONSOLE_UART)
#define USART_CONSOLE_BAUDRATE		(115200)
#define USART_CONSOLE_MODE			(UART_MR_PAR_NO||UART_MR_CHMODE_NORMAL)


void InitalizeDebugConsole(void);
void WriteString(uint8_t * string);
void WriteLine(uint8_t * string);
void WriteChar(uint8_t data);
void WriteCharArr(uint8_t *buf, uint16_t size);

#endif /* CONSOLE_H_ */