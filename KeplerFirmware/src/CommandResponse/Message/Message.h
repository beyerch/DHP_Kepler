/*
 * Message.h
 *
 * Created: 2/24/2017 11:45:20 AM
 *  Author: Aaron
 */ 


#ifndef MESSAGE_H_
#define MESSAGE_H_

//Message struct which contains a buffer pointer and a byte count
typedef struct {
	unsigned char* buf;
	short Size;
} Message_t;

#endif /* MESSAGE_H_ */