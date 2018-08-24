/*
 * fifo.c
 *
 * Created: 2/24/2017 11:42:39 AM
 *  Author: Aaron
 */ 
 #include "fifo.h"
 

 //This initializes the FIFO structure with the given buffer and size
 void fifo_init(fifo_t * f){
	 f->head = 0;
	 f->tail = 0;
	 f->size = FIFO_DEPTH;
 }

 bool fifo_empty(fifo_t * f){

	return f->tail == f-> head ? true : false;
 }

//Reads a message from the fifo
 Message_t * fifo_read(fifo_t * f){
	 Message_t * msg;
		 if( f->tail != f->head ){ //see if any data is available
			 msg = (f->buf[f->tail]);  //grab a message from the buffer
			 f->tail++;
			  //increment the tail

			 if( f->tail == f->size ){  //check for wrap-around
				 f->tail = 0;
			 }
			 } else {
			 return NULL; 
		 }
	 return msg; 
 }

 //Writes a message to the fifo
 bool fifo_write(fifo_t * f, Message_t * message){
		 //first check to see if there is space in the buffer
		 if( (f->head + 1 == f->tail) || ( (f->head + 1 == f->size) && (f->tail == 0) ) ){
				return false;
			 } else {
			 f->buf[f->head] = message;
			 f->head++;  //increment the head
			 if( f->head == f->size ){  //check for wrap-around
				 f->head = 0;
			 }
		 }
		 return true;
	 }