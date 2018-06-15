// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "SerialCommunication.h"
#include "helper.h"

bool setup()
{
	LOG(INIT, "DHPJ2534 Kepler: setup");
	
	return SerialCommunication::CreateCommThread();
}

void exitdll()
{
	LOG(INIT, "DHPJ2534 Kepler: Exitdll");
	SerialCommunication::CloseCommThread();
	
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	
	LOG(INIT, "DHP J2534 PassThru Driver V 0.1");
	LOG(INIT, "(c) Digital Horsepower Inc.");
	LOG(INIT, "DllMain: %d", ul_reason_for_call);
	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (!setup())
			return FALSE;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		exitdll();
		break;
	}
	LOG(INIT, "DllMain exit OK");
	return TRUE;
}

