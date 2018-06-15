/*
 * ErrorHandler.h
 *
 * Created: 2/24/2017 11:57:26 PM
 *  Author: adeck
 */ 


#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

#define ERROR_MESSAGE_LENGTH 5

#include "Error.h"
#include "Errors.h"
#include "Message.h"
#include "MessageHandler.h"

void ThrowError(Error_T *currError);


#endif /* ERRORHANDLER_H_ */