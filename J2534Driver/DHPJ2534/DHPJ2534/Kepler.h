#pragma once

#define KEPLER_DEFAULT_COM_PORT 3
#define KEPLER_DEFAULT_BAUD_RATE 115200
#define KEPLER_DEFAULT_DTR_DISABLED 1

typedef BOOL(WINAPI *LPKEPLERLISTENER)(char * msg, int len, void * data);

namespace Kepler
{

#define KEPLER_INIT_OK 0
#define KEPLER_ALREADY_CONNECTED 1  // by us
#define KEPLER_IN_USE 2   // by some other process
#define KEPLER_OPEN_FAILED 3
#define KEPLER_GET_COMMSTATE_FAILED 4
#define KEPLER_SET_COMMSTATE_FAILED 5
#define KEPLER_SET_COMMMASK_FAILED 6
#define KEPLER_CREATE_EVENT_FAILED 7


	int OpenDevice(LPTSTR com_port, int baud_rate, int disable_DTR);
	bool IsConnected();
	int CloseDevice();
	int Listen(HANDLE CommEventHandle);	// non-blocking

	int RegisterListener(LPKEPLERLISTENER listener, void * data);
	void RemoveListener(LPKEPLERLISTENER listener);
	int Send(unsigned char * msg, unsigned short len, unsigned long Timeout);
	int Write(char * buf, unsigned int len);

	int HandleCommEvent();

static	bool ready = true;
}