/*
 * runtimer.c
 *
 * Created: 2/26/2017 2:56:50 PM
 *  Author: Aaron
 */ 
 #include "runtimer.h"
//Starts a timer on boot up of the interface. The timer counts in Milliseconds. 
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
	 tc_write_rc(RUN_TIMER, RUN_TIMER_CHANNEL, cpu_ms_2_cy(1,96000000));
	 tc_enable_interrupt(RUN_TIMER, RUN_TIMER_CHANNEL,  TC_IER_CPCS);
	 NVIC_EnableIRQ(TC2_IRQn);
	 NVIC_SetPriority(TC2_IRQn,0);
	 tc_start(RUN_TIMER, RUN_TIMER_CHANNEL);
 }
//Gets the current run time in milliseconds
 unsigned long long millis()
 {
	 return RunTimeMilliseconds;
 }
//Timer interrupt.
 void TC2_Handler()
 {
	if ((tc_get_status(RUN_TIMER, RUN_TIMER_CHANNEL) & TC_SR_CPCS) == TC_SR_CPCS)
	{
		NVIC_ClearPendingIRQ(TC2_IRQn);
		RunTimeMilliseconds++;
	}
 }