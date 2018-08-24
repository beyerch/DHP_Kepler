/*
 * runtimer.h
 *
 * Created: 2/26/2017 12:56:38 PM
 *  Author: adeck
 */ 


#ifndef RUNTIMER_H_
#define RUNTIMER_H_

#include "Timers.h"
#include <asf.h>

static unsigned long long RunTimeMilliseconds = 0;
void StartRunTimer(void);
void millis(void);




#endif /* RUNTIMER_H_ */