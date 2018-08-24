/*
 * ui.c
 *
 * Created: 2/24/2017 11:14:08 PM
 *  Author: adeck
 */ 
#include <asf.h>
#include "ui.h"

//This file just contains the code which sets the LED colors. It is self-explainitory 
void ui_init(void)
{
	/* Initialize LEDs */
	LED_Off(PC_LED_RED);
	LED_Off(PC_LED_GREEN);
	LED_Off(PC_LED_BLUE);
	
	LED_On(POWER_LED_RED);
	LED_Off(POWER_LED_GREEN);
	LED_Off(POWER_LED_BLUE);
	
	LED_Off(VEHICLE_LED_RED);
	LED_Off(VEHICLE_LED_GREEN);
	LED_Off(VEHICLE_LED_BLUE);
}

void ui_powerdown(void)
{
	LED_Off(PC_LED_RED);
	LED_Off(PC_LED_GREEN);
	LED_Off(PC_LED_BLUE);
	
	LED_On(POWER_LED_RED);
	LED_Off(POWER_LED_GREEN);
	LED_Off(POWER_LED_BLUE);
	
	LED_Off(VEHICLE_LED_RED);
	LED_Off(VEHICLE_LED_GREEN);
	LED_Off(VEHICLE_LED_BLUE);
}

void ui_wakeup(void)
{
	
}

void ui_com_open(uint8_t port)
{
	UNUSED(port);
	LED_On(PC_LED_GREEN);
}

void ui_com_close(uint8_t port)
{
	UNUSED(port);
	LED_Off(PC_LED_GREEN);
}

void ui_com_rx_start(void)
{
	LED_On(PC_LED_BLUE);
}

void ui_com_rx_stop(void)
{
	LED_Off(PC_LED_BLUE);
}

void ui_com_tx_start(void)
{
	LED_On(PC_LED_RED);
	
}

void ui_com_tx_stop(void)
{
	LED_Off(PC_LED_RED);
	
}

void ui_com_error(void)
{
}

void ui_com_overflow(void)
{
}

void ui_com_process(uint16_t framenumber)
{
	//USB framenumber from PC, used to blink the USB LED at a given rate. 
	//Indicates a successfull connection to the PC.
	if ((framenumber % 1000) == 0) {
		LED_On(PC_LED_BLUE);
	}
	if ((framenumber % 1000) == 500) {
		LED_Off(PC_LED_BLUE);
	}
}

void ui_power_good()
{
	LED_On(POWER_LED_RED);
}

void ui_vehicle_enable_vpw()
{
	LED_On(VEHICLE_LED_BLUE);
}

void ui_vehicle_disable_vpw()
{
	LED_Off(VEHICLE_LED_BLUE);
}

void ui_vehicle_vpw_tx_notify()
{
	LED_Toggle(VEHICLE_LED_RED);
}

void ui_vehicle_vpw_rx_notify()
{
	LED_Toggle(VEHICLE_LED_GREEN);
}

void ui_vehicle_vpw_rx_notify_off()
{
	LED_Off(VEHICLE_LED_GREEN);
}

void ui_vehicle_vpw_tx_notify_off()
{
	LED_Off(VEHICLE_LED_RED);	
}

void ui_vehicle_enable_can()
{
	LED_On(VEHICLE_LED_GREEN);
}

void ui_vehicle_disable_can()
{
	LED_Off(VEHICLE_LED_GREEN);
}