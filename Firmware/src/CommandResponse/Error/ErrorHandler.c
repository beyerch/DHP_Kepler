/*
 * ErrorHandler.c
 *
 * Created: 2/24/2017 11:57:17 PM
 *  Author: adeck
 */ 

#include "ErrorHandler.h"

void ThrowError(Error_T *currError)
{
	unsigned char error[] = {START_BYTE, 0x00, 0x04, ERROR_RESPONSE, currError->ThrowerID, currError->ErrorMajor, currError->ErrorMinor};
		
	Message_t errorMessage;
	errorMessage.buf = error;
	errorMessage.Size = ERROR_MESSAGE_LENGTH+2;
	
	WriteMessage(&errorMessage);
	
}