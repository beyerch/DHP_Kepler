#pragma once

#include "stdafx.h"
#include <string>
#include <list>

namespace DHPJ2534Registry {

	bool GetSettingsFromRegistry(const char * deviceName, int * ComPort, int * BaudRate, int * disableDTR, unsigned long * deviceId);
	bool GetValueFromRegistry(HKEY previousKey, TCHAR * valueName, unsigned long * value);
	std::list<LPTSTR> GetVirtualSerialPortName(const char * PID, const char * VID);
}