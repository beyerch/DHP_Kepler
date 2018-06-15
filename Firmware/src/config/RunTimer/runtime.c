/*
 * runtime.c
 *
 * Created: 2/26/2017 12:56:49 PM
 *  Author: adeck
 */ 
#include "runtimer.h"

void StartRunTimer()
{
	sysclk_enable_peripheral_clock(RUN_TIMER_ID);
	tc_stop(RUN_TIMER, RUN_TIMER_CHANNEL);
	tc_init(
	RUN_TIMER,
	RUN_TIMER_CHANNEL,
	TC_CMR_TCCLKS_TIMER_CLOCK4|
	TC_CMR_BURST_NONE|TC_CMR_CPCTRG
	);
	tc_write_rc(RUN_TIMER, RUN_TIMER_CHANNEL, 750);
	tc_enable_interrupt(RUN_TIMER, RUN_TIMER_CHANNEL,  TC_IER_CPCS);
	NVIC_EnableIRQ(TC2_IRQn);
	NVIC_SetPriority(TC2_IRQn,8);
	tc_start(RUN_TIMER, RUN_TIMER_CHANNEL);
}
void millis()
{
	return RunTimeMilliseconds;
}


void TC2_Handler()
{
	RunTimeMilliseconds++;
}