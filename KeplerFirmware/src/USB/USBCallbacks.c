/*
 * USBCallbacks.c
 *
 * Created: 2/24/2017 11:12:25 PM
 *  Author: adeck
 */ 
#include "USBCallbacks.h"


//ASF USB Callbacks. See ASF Documentation.

void main_suspend_action(void)
{
	ui_powerdown();
}

void main_resume_action(void)
{
	ui_wakeup();
}

void main_sof_action(void)
{
#ifdef LED_USB_FRAME_COUNTER
	//Flash USB LED
	ui_com_process(udd_get_frame_number());
#endif
}

bool main_msc_enable(void)
{
	return true;
}

void main_msc_disable(void)
{
	
}

bool main_cdc_enable(uint8_t port)
{
	ui_com_open(port);
	return true;
}

void main_cdc_disable(uint8_t port)
{
	//ui_com_close(port);
}

void main_cdc_set_dtr(uint8_t port, bool b_enable)
{

}