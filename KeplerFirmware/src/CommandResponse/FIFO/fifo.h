/*
 * fifo.h
 *
 * Created: 2/24/2017 1:45:57 PM
 *  Author: Aaron
 */ 


#ifndef FIFO_H_
#define FIFO_H_

#include "Message.h"
#include "compiler.h"
#define FIFO_DEPTH 4

typedef struct {
	int head;
	int tail;
	int size;
	Message_t *buf[FIFO_DEPTH];
} fifo_t;

void fifo_init(fifo_t * f);
bool fifo_empty(fifo_t * f);
bool fifo_write(fifo_t * f, Message_t * message);
Message_t* fifo_read(fifo_t * f);
#endif /* FIFO_H_ */