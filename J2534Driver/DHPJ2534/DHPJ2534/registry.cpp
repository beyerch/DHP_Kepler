/*
**
** Copyright (C) 2012 Olaf @ Hacking Volvo blog (hackingvolvo.blogspot.com)
** Author: Olaf <hackingvolvo@gmail.com>
**
** This library is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published
** by the Free Software Foundation, either version 3 of the License, or (at
** your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, <http://www.gnu.org/licenses/>.
**
*/

#include "stdafx.h"
#include <malloc.h>
#include <string>
#include <list>
#include "helper.h"


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383


namespace DHPJ2534Registry {

	

	int Registry_GetString(HKEY hKey, LPCTSTR szValueName, LPTSTR * lpszResult) {
		DWORD dwType = 0, dwDataSize = 0, dwBufSize = 0;
		int res;

		if (lpszResult != NULL) *lpszResult = NULL;
		if (hKey == NULL || lpszResult == NULL) return E_INVALIDARG;

		// get value size
		res = RegQueryValueEx(hKey, szValueName, 0, &dwType, NULL, &dwDataSize);
		if (res != ERROR_SUCCESS)
			return res;

		// check the type
		if (dwType != REG_SZ)
			return ERROR_DATATYPE_MISMATCH;

		dwBufSize = dwDataSize + (1 * sizeof(TCHAR));
		*lpszResult = (LPTSTR)malloc(dwBufSize);
		if (*lpszResult == NULL)
			return ERROR_NOT_ENOUGH_MEMORY;

		// get the value
		res = RegQueryValueEx(hKey, szValueName, 0, &dwType, (LPBYTE)*lpszResult, &dwDataSize);
		if (res != ERROR_SUCCESS)
		{
			free(*lpszResult);
			return res;
		}
		(*lpszResult)[(dwBufSize / sizeof(TCHAR)) - 1] = TEXT('\0');

		return ERROR_SUCCESS;
	}


	// =====================================================================================
	int Registry_GetDWord(HKEY hKey, LPCTSTR szValueName, DWORD * lpdwResult) {

		int res;
		DWORD dwDataSize = sizeof(DWORD);
		DWORD dwType = 0;

		if (hKey == NULL || lpdwResult == NULL) return E_INVALIDARG;
		res = RegQueryValueEx(hKey, szValueName, 0, &dwType, (LPBYTE)lpdwResult, &dwDataSize);
		if (res != ERROR_SUCCESS)
			return res;
		else
			if (dwType != REG_DWORD) return ERROR_DATATYPE_MISMATCH;

		return ERROR_SUCCESS;
	}


	// Get settings from Registry: If a value cannot be found in registry, original values (referenced with parameters) will not be altered

	bool GetSettingsFromRegistry(const char * DeviceName, int * ComPort, int * BaudRate, int * disableDTR, unsigned long * deviceId)
	{
		HKEY hKeySoftware, hKeyPTS0404, hKeySardineCAN;
		//	DWORD KeyType, KeySize;
		DWORD dwVal;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software"), 0, KEY_READ, &hKeySoftware) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: SOFTWARE");
			return FALSE;
		}
		if (RegOpenKeyEx(hKeySoftware, L"PassThruSupport.04.04", 0, KEY_READ, &hKeyPTS0404) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: PassThruSupport.04.04!");
			RegCloseKey(hKeySoftware);
			return FALSE;
		}
		RegCloseKey(hKeySoftware);


		WCHAR DeviceNameW[256];
		if (DeviceName != NULL)
		{
			size_t converted = 0;
			int ret = mbstowcs_s(&converted, DeviceNameW, strlen(DeviceName) + 1, DeviceName, _TRUNCATE);
			if ((ret != 0) || (converted != (strlen(DeviceName) + 1)))
			{
				LOG(ERR, "GetSettingsFromRegistry: Invalid DeviceName");
				return false;
			}
		}
		else
		{
			wcscpy_s(DeviceNameW, 80, L"Kepler");
		}

		//		if (RegOpenKeyEx(hKeyPTS0404, L"Sardine CAN", 0, KEY_READ , &hKeySardineCAN) != ERROR_SUCCESS)
		if (RegOpenKeyEx(hKeyPTS0404, DeviceNameW, 0, KEY_READ, &hKeySardineCAN) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: Couldn't find Kepler entry in registry!");
			RegCloseKey(hKeyPTS0404);
			return FALSE;
		}
		RegCloseKey(hKeyPTS0404);

		if (Registry_GetDWord(hKeySardineCAN, TEXT("COM_PORT"), &dwVal) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: No Com port entry!");
		}
		else
			*ComPort = dwVal;

		if (Registry_GetDWord(hKeySardineCAN, TEXT("BAUD_RATE"), &dwVal) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: No baud rate entry!");
		}
		else
			*BaudRate = dwVal;

		if (Registry_GetDWord(hKeySardineCAN, TEXT("DISABLE_DTR"), &dwVal) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: No disableDTR entry!");
		}
		else
			*disableDTR = dwVal;

		if (Registry_GetDWord(hKeySardineCAN, TEXT("DeviceID"), &dwVal) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: No Device ID entry!");
		}
		else
			*deviceId = dwVal;

		RegCloseKey(hKeySardineCAN);
		return true;
	}



	bool GetValueFromRegistry(HKEY previousKey, TCHAR * valueName, unsigned long * value)
	{
		HKEY hKeySoftware, hKeyPTS0404, hKeySardineCAN;
		//	DWORD KeyType, KeySize;
		DWORD dwVal;
		DWORD ret;

		if (previousKey == NULL)
		{
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software"), 0, KEY_READ, &hKeySoftware) != ERROR_SUCCESS)
			{
				LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: SOFTWARE");
				return FALSE;
			}
			if (RegOpenKeyEx(hKeySoftware, L"PassThruSupport.04.04", 0, KEY_READ, &hKeyPTS0404) != ERROR_SUCCESS)
			{
				LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: PassThruSupport.04.04!");
				RegCloseKey(hKeySoftware);
				return FALSE;
			}
			RegCloseKey(hKeySoftware);

			if (RegOpenKeyEx(hKeyPTS0404, L"Kepler", 0, KEY_READ, &hKeySardineCAN) != ERROR_SUCCESS)
			{
				LOG(ERR, "GetSettingsFromRegistry: Couldn't find Kepler entry in registry!");
				RegCloseKey(hKeyPTS0404);
				return FALSE;
			}
			RegCloseKey(hKeyPTS0404);
		}
		else
			hKeySardineCAN = previousKey;

		ret = Registry_GetDWord(hKeySardineCAN, valueName, &dwVal);
		if (previousKey == NULL)
			RegCloseKey(hKeySardineCAN);

		if (ret != ERROR_SUCCESS)
		{
			LOGW(ERR, L"GetSettingsFromRegistry: No %s entry!", valueName);
			return false;
		}
		else
			*value = dwVal;
		return true;
	}

	std::list<LPTSTR> GetVirtualSerialPortName(const char * PID, const char * VID)
	{
		std::list<LPTSTR> ports;
		HKEY hKeySoftware,hKeySoftwareCurrentControlSet, hKeyEnum, hKeyUSB, hKeyDevice, hKeyDeviceParameters, hKeySubKeyName;
		TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
		DWORD    cbName;                   // size of name string 
		TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
		DWORD    cchClassName = MAX_PATH;  // size of class string 
		DWORD    cSubKeys = 0;               // number of subkeys 
		DWORD    cbMaxSubKey;              // longest subkey size 
		DWORD    cchMaxClass;              // longest class string 
		DWORD    cValues;              // number of values for key 
		DWORD    cchMaxValue;          // longest value name 
		DWORD    cbMaxValueData;       // longest value data 
		DWORD    cbSecurityDescriptor; // size of security descriptor 
		FILETIME ftLastWriteTime;      // last write time 
		LPTSTR retVal;
		LPTSTR hRegSzPortName = L"Port Name";
		DWORD i,retCode;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM"), 0, KEY_READ, &hKeySoftware) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: SYSTEM");
			return ports;
		}

		if (RegOpenKeyEx(hKeySoftware, L"CurrentControlSet", 0, KEY_READ, &hKeySoftwareCurrentControlSet) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: Current Control Set");
			RegCloseKey(hKeySoftware);
			return ports;
		}
		RegCloseKey(hKeySoftware);

		if (RegOpenKeyEx(hKeySoftwareCurrentControlSet, L"Enum", 0, KEY_READ, &hKeyEnum) != ERROR_SUCCESS)
		{
			LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: Enum");
			RegCloseKey(hKeySoftware);
			return ports;
		}

		RegCloseKey(hKeySoftwareCurrentControlSet);


		if (RegOpenKeyEx(hKeyEnum, L"USB", 0, KEY_READ, &hKeyUSB) != ERROR_SUCCESS) //TODO: GENERALIZE THIS TO USE PARAMETERS
		{
			LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: USB");
			RegCloseKey(hKeySoftwareCurrentControlSet);
			return ports;
		}
		RegCloseKey(hKeyEnum);

		if (RegOpenKeyEx(hKeyUSB, L"VID_03EB&PID_2404", 0, KEY_READ, &hKeyDevice) != ERROR_SUCCESS) //TODO: GENERALIZE THIS TO USE PARAMETERS
		{
			LOG(ERR, "GetSettingsFromRegistry: Cannot open device key from PID & VID");
			RegCloseKey(hKeySoftware);
			return ports;
		}
		RegCloseKey(hKeyUSB);

		retCode = RegQueryInfoKey(
			hKeyDevice,              // key handle 
			achClass,                // buffer for class name 
			&cchClassName,           // size of class string 
			NULL,                    // reserved 
			&cSubKeys,               // number of subkeys 
			&cbMaxSubKey,            // longest subkey size 
			&cchMaxClass,            // longest class string 
			&cValues,                // number of values for this key 
			&cchMaxValue,            // longest value name 
			&cbMaxValueData,         // longest value data 
			&cbSecurityDescriptor,   // security descriptor 
			&ftLastWriteTime);       // last write time 
									 // Enumerate the subkeys, until RegEnumKeyEx fails.
		LOG(HELPERFUNC, "DHPJ2534Registry::GetVirtualSerialPortName - Enumerating subkeys for PID & VID");
		if (cSubKeys)
		{
			LOG(HELPERFUNC,"DHPJ2534Registry::GetVirtualSerialPortName Number of subkeys: %d", cSubKeys);

			for (i = 0; i<cSubKeys; i++)
			{
				cbName = MAX_KEY_LENGTH;
				retCode = RegEnumKeyEx(hKeyDevice, i,
					achKey,
					&cbName,
					NULL,
					NULL,
					NULL,
					&ftLastWriteTime);
				if (retCode == ERROR_SUCCESS)
				{
					//_tprintf(TEXT("(%d) %s\n"), i + 1, achKey);

					if (RegOpenKeyEx(hKeyDevice, achKey, 0, KEY_READ, &hKeySubKeyName) != ERROR_SUCCESS) //TODO: GENERALIZE THIS TO USE PARAMETERS
					{
						LOG(ERR, "GetSettingsFromRegistry: Cannot open subkey: %d Name: %s", i, achKey);
						RegCloseKey(hKeyDevice);
						return ports;
					}

					if (RegOpenKeyEx(hKeySubKeyName, L"Device Parameters", 0, KEY_READ, &hKeyDeviceParameters) == ERROR_SUCCESS) //TODO: GENERALIZE THIS TO USE PARAMETERS
					{
						Registry_GetString(hKeyDeviceParameters, L"PortName", &retVal);
						if (retVal != NULL)
						{
							ports.push_back(retVal);
							LOGW(HELPERFUNC, L"DHPJ2534Registry::GetVirtualSerialPortName Port found: %s", retVal);
						}
						else
						{
							LOG(ERR, "DHPJ2534Registry::GetVirtualSerialPortName REG_SZ PortName -> NULL");
						}
					}
					else
					{
						LOG(ERR, "GetSettingsFromRegistry: Cannot open registry key: Device Parameters");
					}
				}
			}
		}

		return ports;
	}

	

}