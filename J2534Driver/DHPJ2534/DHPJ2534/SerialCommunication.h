#pragma once

#include "stdafx.h"
#include "Windows.h"

namespace SerialCommunication
{
	bool CreateCommThread();
	bool CloseCommThread();

	int WaitUntilInitialized(const char * deviceName, unsigned long timeout);

	void RequestInitialization(const char * deviceName);
	void SetInitReqComplete();
	bool IsConnected();

	int WaitForEvents();	// blocks until event occurs (read)

	const char * GetCommErrorMsg();

	unsigned long GetDeviceId();
 

}