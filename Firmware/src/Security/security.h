/*
 * security.h
 *
 * Created: 2/25/2017 10:14:49 AM
 *  Author: adeck
 */ 


#ifndef SECURITY_H_
#define SECURITY_H_

#include "ErrorHandler.h"
#include "compiler.h"

uint32_t* GetUniqueID(void);
void EnterSecureMode(long key);
void ExitSecureMode(void);
unsigned long GetKey(void);
void UnlockBluetoothCommunications(long key);
unsigned long GetBluetoothKey(void);

#endif /* SECURITY_H_ */