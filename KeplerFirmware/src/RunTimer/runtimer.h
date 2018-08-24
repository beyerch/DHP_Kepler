/*
 * runtimer.h
 *
 * Created: 2/26/2017 2:56:56 PM
 *  Author: Aaron
 */ 


#ifndef RUNTIMER_H_
#define RUNTIMER_H_

#include "Timers.h"
#include <asf.h>

static unsigned long long RunTimeMilliseconds = 0;
void StartRunTimer(void);
unsigned long long millis(void);

#endif /* RUNTIMER_H_ */