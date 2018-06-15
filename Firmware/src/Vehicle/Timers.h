/*
 * Timers.h
 *
 * Created: 2/25/2017 1:13:45 AM
 *  Author: adeck
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_

#define VPW_TIMER TC0
#define RUN_TIMER TC0
#define TP_TIMEOUT_TIMER TC1

#define VPW_TX_TIMER_ID ID_TC0
#define VPW_RX_TIMER_ID ID_TC1
#define RUN_TIMER_ID ID_TC2
#define TP_TIMEOUT_TIMER_ID ID_TC3

#define VPW_TX_TIMER_CHANNEL 0
#define VPW_RX_TIMER_CHANNEL 1
#define RUN_TIMER_CHANNEL	 2

#define TP_TIMEOUT_TIMER_CHANNEL	0

#endif /* TIMERS_H_ */