/*
 * console.c
 *
 * Created: 3/14/2017 10:50:24 AM
 *  Author: adeck
 */ 
#include "console.h"

//Uses a UART port as a debug console. Any information can be written out to the console for debugging. 
//The UART pins for this are hooked up to the ALDL hardware.


void InitalizeDebugConsole(void)
{
#ifdef DEBUG_OUTPUT_ENABLED
	ioport_set_pin_dir(PIO_PA10_IDX, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(PIO_PA9_IDX, IOPORT_DIR_INPUT);
	pmc_enable_periph_clk(ID_UART0);
		const sam_uart_opt_t uart_serial_options = 
		{
			.ul_baudrate = USART_CONSOLE_BAUDRATE,
			.ul_mck =  sysclk_get_cpu_hz(),
			.ul_mode = USART_CONSOLE_MODE
		};
		uint32_t EnableSuccess=uart_init(CONSOLE_UART, &uart_serial_options);
		uint32_t SendSuccess = 0;
		uart_enable(CONSOLE_UART);	
#endif
}

void WriteChar(uint8_t data)
{
	
#ifdef DEBUG_OUTPUT_ENABLED
	while(!(CONSOLE_UART->UART_SR & UART_SR_TXRDY));
	uart_write(CONSOLE_UART, data);
	
#endif
}

void WriteString(uint8_t * string)
{
#ifdef DEBUG_OUTPUT_ENABLED
	
	uint32_t strLen = strlen(string);
	for(int i = 0; i < strLen; i++)
	{
		while(!(CONSOLE_UART->UART_SR & UART_SR_TXRDY));
		uart_write(CONSOLE_UART,string[i]);
	}
	
#endif
}

void WriteLine(uint8_t * string)
{
	
#ifdef DEBUG_OUTPUT_ENABLED
	
	WriteString(string);
	while(!(CONSOLE_UART->UART_SR & UART_SR_TXRDY));
	uart_write(CONSOLE_UART,0x0A);
	
#endif
}

void WriteCharArr(uint8_t *buf, uint16_t size)
{
	
}



