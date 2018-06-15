#include "stdafx.h"
#include "Kepler.h"
#include "helper.h"
#include <stdio.h>
#include "Benaphore.h"
#include "SerialCommunication.h"
#include <WinSock.h>
#include <thread>
#include <queue>
#include <mutex>

#define MAX_COLLECTION_BUF_SIZE 1024
#define MAX_READ_BUF_SIZE 256
#define MAX_LISTENERS 8

namespace Kepler
{
	Benaphore init_wait_lock;

	Benaphore init_lock;
	//Benaphore listener_lock;

	//Benaphore write_lock;

	std::mutex myMutex;
	std::mutex myListener;
	std::mutex myInit;



	std::queue<TX_QUEUE_MESSAGE*> TransmitQueue;

	bool isConnected = false;

	HANDLE hCommPort = INVALID_HANDLE_VALUE;

	OVERLAPPED read_overlap;
	OVERLAPPED write_overlap;
	OVERLAPPED comm_event_overlap;

	HANDLE ghWriteCompleteEvent;

	DWORD dwCommEventMask;

	typedef struct {
		LPKEPLERLISTENER callback;
		void * data;
	} listener_struct;

	listener_struct listeners[MAX_LISTENERS];
	int listeners_count = 0;

	char collectionbuf[MAX_COLLECTION_BUF_SIZE + 1];
	char buffer[MAX_READ_BUF_SIZE];
	int cbufi = 0;

	int RegisterListener(LPKEPLERLISTENER listener, void *data)
	{
		LOG(MAINFUNC, "Kepler::RegisterListener");
		std::lock_guard<std::mutex> guard(myListener);

		if (listeners_count < MAX_LISTENERS)
		{
			listeners[listeners_count].callback = listener;
			listeners[listeners_count].data = data;
			listeners_count++;

			LOG(MAINFUNC, "Kepler::RegisterListener - added succesfully");
		}
		else
		{
			LOG(ERR, "Kepler::RegisterListener - too many listeners!");
			return -1;
		}
		return 0;
	}

	void RemoveListener(LPKEPLERLISTENER listener)
	{
		LOG(MAINFUNC, "Kepler::RemoveListener");

		std::lock_guard<std::mutex> guard(myListener);

		int i = 0;
		while ((i < listeners_count) && (listeners[i].callback != listener))
			i++;
		if (listeners[i].callback == listener)
		{
			// remove entry so that entries after this are simply moved 1 entry lower
			while (i < listeners_count - 1)
			{
				listeners[i].callback = listeners[i + 1].callback;
				listeners[i].data = listeners[i + 1].data;
				i++;
			}
			listeners_count--;

			LOG(MAINFUNC, "Kepler::RemoveListener - removed successfully");
		}
		else
		{

			LOG(ERR, "Kepler::RemoveListener - no such listener found!");
		}
	}

	bool IsConnected()
	{
		LOG(HELPERFUNC, "Kepler::IsConnected %d", isConnected);

		std::lock_guard<std::mutex> guard(myInit);

		bool isconnected = isConnected;

		return isconnected;
	}

	int blockingWrite(unsigned char * buf, unsigned int len)
	{

		std::lock_guard<std::mutex> guard(myMutex);

		DWORD dwwritten = 0, dwErr;
		memset(&write_overlap, 0, sizeof(write_overlap));
		write_overlap.hEvent = ghWriteCompleteEvent; // CreateEvent(NULL, FALSE, FALSE, NULL); // TRUE, NULL); // for some reason event 
		unsigned int fSuccess = WriteFile(hCommPort, buf, len, &dwwritten, &write_overlap);
		if (!fSuccess)
		{
			dwErr = GetLastError();
			if (dwErr != ERROR_IO_PENDING)
			{
				LOG(ERR, "Kepler::SendMsg - Write failed (%d)\n", GetLastError());
				return -1;
			}
			// Pending IO -> Wait for the result
			if (!GetOverlappedResult(hCommPort, &write_overlap, &dwwritten, TRUE))
			{
				LOG(ERR, "Kepler::SendMsg - Error waiting for write to finish (%d)\n", GetLastError());
				return -1;
			}
		}
		return dwwritten;
	}

	int Write(char * buf, unsigned int len)
	{
		DWORD dwwritten = 0, dwErr;

		std::lock_guard<std::mutex> guard(myMutex);

		memset(&write_overlap, 0, sizeof(write_overlap));
		write_overlap.hEvent = ghWriteCompleteEvent; // CreateEvent(NULL, FALSE, FALSE, NULL); // TRUE, NULL); // for some reason event 
		unsigned int fSuccess = WriteFile(hCommPort, buf, len, &dwwritten, &write_overlap);
		if (!fSuccess)
		{
			dwErr = GetLastError();
			if (dwErr != ERROR_IO_PENDING)
			{
				LOG(ERR, "Kepler::SendMsg - Write failed (%d)\n", GetLastError());
				return -1;
			}

			// Pending IO -> Wait for the result
			if (!GetOverlappedResult(hCommPort, &write_overlap, &dwwritten, TRUE))
			{
				LOG(ERR, "Kepler::SendMsg - Error waiting for write to finish (%d)\n", GetLastError());
				return -1;
			}

		}

		return dwwritten;
	}

	int OpenDevice(LPTSTR com_port, int baud_rate, int disable_DTR)
	{
		LOGW(MAINFUNC, L"Kepler::OpenDevice - com port %s, baud_rate %d, disable DTR %d", com_port, baud_rate, disable_DTR);

		if (isConnected)
		{
			LOG(ERR, "Kepler::OpenDevice - already connected!");
			return KEPLER_ALREADY_CONNECTED;
		}
		if (baud_rate == -1)
		{
			baud_rate = KEPLER_DEFAULT_BAUD_RATE;
			LOG(MAINFUNC, "Kepler::OpenDevice - defaulting to baud rate %d", baud_rate);
		}
		if (disable_DTR == -1)
		{
			disable_DTR = KEPLER_DEFAULT_DTR_DISABLED;
			LOG(MAINFUNC, "Kepler::OpenDevice - defaulting to dtr disabled=%d", disable_DTR);
		}

		
		long int err;
		BOOL fSuccess;
		DCB dcb;

		LOG(MAINFUNC, "Kepler::OpenDevice - creating file handle for com port");
		hCommPort = CreateFile(
			com_port,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE, // 0,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL
		);

		if (hCommPort == INVALID_HANDLE_VALUE)
		{
			err = GetLastError();
			if (err == ERROR_ALREADY_EXISTS)
			{
				LOG(ERR, "Kepler::OpenDevice - Already opened by other process!");
				
				return KEPLER_IN_USE;
			}
			else
			{
				LOG(ERR, "Kepler::OpenDevice - CreateFileA failed %d!", err);
				
				return KEPLER_OPEN_FAILED;
			}
		}

		LOG(MAINFUNC, "Kepler::OpenDevice - getting comm state");
		fSuccess = GetCommState(hCommPort, &dcb);

		if (!fSuccess)
		{
			LOG(ERR, "Kepler::OpenDevice - GetCommStateFailed %d", err = GetLastError());
			PrintError(err);
			CloseHandle(hCommPort);
			hCommPort = INVALID_HANDLE_VALUE;
			
			return KEPLER_GET_COMMSTATE_FAILED;
		}

		LOG(MAINFUNC, "Kepler::OpenDevice - setting comm state");
		dcb.BaudRate = baud_rate;
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.fDtrControl = (disable_DTR == 1 ? DTR_CONTROL_DISABLE : DTR_CONTROL_ENABLE);	// to prevent Kepler from reseting when first sending something

		fSuccess = SetCommState(hCommPort, &dcb);
		if (!fSuccess)
		{
			err = GetLastError();
			LOG(ERR, "Kepler::OpenDevice - SetCommStateFailed %d", err);
			CloseHandle(hCommPort);
			hCommPort = INVALID_HANDLE_VALUE;
			PrintError(err);

			return KEPLER_SET_COMMSTATE_FAILED;
		}


		LOG(MAINFUNC, "Kepler::OpenDevice - enable listening to comm events");
		DWORD      dwStoredFlags;
		// listen only for events regarding errors and receive buffer
		dwStoredFlags = /*EV_BREAK | EV_CTS |*/ EV_DSR | /* EV_ERR | EV_RING | EV_RLSD | */ EV_RXCHAR /* | EV_RXFLAG | EV_TXEMPTY*/;
		if (!SetCommMask(hCommPort, dwStoredFlags))
		{
			err = GetLastError();
			LOG(ERR, "Kepler::OpenDevice - Error setting communications mask (err %d); abort!", err);
			CloseHandle(hCommPort);
			hCommPort = INVALID_HANDLE_VALUE;
			PrintError(err);

			return KEPLER_SET_COMMMASK_FAILED;
		}

		// create event for write completed
		if ((ghWriteCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
		{
			LOG(ERR, "Kepler::OpenDevice - Create 'Write Complete Event' failed (err %d); abort!", GetLastError());
			return KEPLER_CREATE_EVENT_FAILED;
		}

		isConnected = true;

		LOG(MAINFUNC, "Kepler::OpenDevice - port configured");
		return KEPLER_INIT_OK;
	}

	int CloseDevice()
	{
		LOG(MAINFUNC, "Kepler::CloseDevice");

		CloseHandle(hCommPort);
		hCommPort = INVALID_HANDLE_VALUE;
		isConnected = 0;
		CloseHandle(ghWriteCompleteEvent);

		return 0;
	}

	
	int Send(unsigned char * data, unsigned short len, unsigned long Timeout)
	{
		LOG(KEPLER_MSG_VERBOSE, "Kepler::SendMsg - msg: [%s]", data);
		ready = false;
		
		//std::lock_guard<std::mutex> guard(myMutex);
		//write_lock.Lock();

		DWORD dwwritten = 0;
	
		if ((dwwritten = blockingWrite(data, len)) == -1)
		{
			LOG(ERR, "Kepler::SendMsg - Blocking write failed!");
			//write_lock.Unlock();
			return -1;
		}

		if (dwwritten != len)
		{
			//write_lock.Unlock();
			LOG(ERR, "Kepler::SendMsg - Write didn't finish (%d out of %d bytes sent)\n", dwwritten, len);
			DWORD   dwErrors;
			COMSTAT comStat;
			ClearCommError(hCommPort, &dwErrors, &comStat);
			LOG(ERR, "Kepler::SendMsg - ClearCommError: Error flags: 0x%x, bytes in output queue: %d\n", dwErrors, comStat.cbOutQue);
			return -1;
		}

		//write_lock.Unlock();

		LOG(KEPLER_MSG_VERBOSE, "Kepler::SendMsg - completed succefully: %d bytes written ", dwwritten);
		ready = true;

		return dwwritten;
	}

	int Listen(HANDLE CommEventHandle)
	{
		memset(&comm_event_overlap, 0, sizeof(comm_event_overlap));
		comm_event_overlap.hEvent = CommEventHandle; // comm event handle
		int ret;
		//	if (!ReadFileEx(hCommPort, buffer,1, &read_overlap, (LPOVERLAPPED_COMPLETION_ROUTINE)&ReadRequestCompleted))
		ret = WaitCommEvent(hCommPort, &dwCommEventMask, &comm_event_overlap);
		if (!ret)
		{
			ret = GetLastError();
			if (ret != ERROR_IO_PENDING)
			{
				LOG(ERR, "Kepler::WaitCommEvent error: %d", ret);
				return ret;
			}
		}
		return 0;
	}

	void MsgReceived(char * msg_buf, int len)
	{
		LOG(KEPLER_MSG, "Kepler::MsgReceived: read: %d bytes: [%s]", len, msg_buf);
		if (listeners_count == 0)
		{
			LOG(KEPLER_MSG, "Kepler::MsgReceived: No listeners");
			return;
		}
		// Callback for each listener
		int accepted = 0;
		for (int i = 0; i<listeners_count; i++)
		{
			if (listeners[i].callback(msg_buf, len, listeners[i].data))
				accepted++;
		}
		if (!accepted)
		{
			LOG(ERR, "Kepler::MsgReceived: Warning! None of the listeners accepted the message!");
		}

	}

	VOID CALLBACK ReadRequestCompleted(DWORD errorCode, DWORD bytesRead, LPVOID overlapped)
	{
		buffer[bytesRead] = 0;
		LOG(KEPLER_MSG_VERBOSE, "Kepler::ReadRequestCompleted: errorCode %d, read: %d bytes: [%s]\n", errorCode, bytesRead, buffer);
		if (bytesRead > 0)
		{
			unsigned int i = 0;

			while (i < bytesRead)
			{
				//Find the start byte
				if (buffer[i] == START_BYTE)
				{
					collectionbuf[cbufi++] = buffer[i++];
					break;
				}
			}
			collectionbuf[cbufi++] = buffer[i];
			collectionbuf[cbufi++] = buffer[i+1];


			//Next two bytes in buffer are length high and length low
			unsigned short messageLength = bytesRead - 2;//(collectionbuf[i] << 8) | collectionbuf[i+1];
			i += 2;
			//Adjust for offset
			messageLength += i -1;
			if (i > bytesRead)
			{
				LOG(ERR, "Kepler::ReadRequestCompleted: Buffer underflow. Need %d bytes, have %d", messageLength, bytesRead - i);
				return;
			}

			for ( i; i <= messageLength; i++)
			{	
				collectionbuf[cbufi++] = buffer[i];
			}

			MsgReceived(collectionbuf, i);
			cbufi = 0;
		}
	}

	int BlockingRead(int bytes)
	{
		LOG(HELPERFUNC, "Kepler::BlockingRead: read %d bytes", bytes);
		//	if (!ReadFileEx(hCommPort, buffer,1, &read_overlap, (LPOVERLAPPED_COMPLETION_ROUTINE)&ReadRequestCompleted))
		DWORD err = 0;
		DWORD bytesRead;
		DWORD bytesLeft = bytes;
		while ((bytesLeft > 0) && (!err))
		{
			DWORD bytesToRead = (bytesLeft > MAX_READ_BUF_SIZE ? MAX_READ_BUF_SIZE : bytesLeft);

			memset(&read_overlap, 0, sizeof(read_overlap));
			err = ReadFileEx(hCommPort, buffer, bytesToRead, &read_overlap, NULL);
			if (!err)
			{
				err = GetLastError();
				if (err != ERROR_IO_PENDING)
				{
					LOG(ERR, "Kepler::BlockingRead: ReadFileEx error! %d", err);
					return err;
				}
				else
					err = NULL; // omit ERROR_IO_PENDING since we want to continue reading
			}
			if (!GetOverlappedResult(hCommPort, &read_overlap, &bytesRead, TRUE))
			{
				LOG(ERR, "Kepler::BlockingRead: GetOverlappedResult error! %d", err = GetLastError());
				return err;
			}
			if (bytesRead != bytes)
			{
				LOG(ERR, "Kepler::BlockingRead: bytes read (%d) != bytes requested (%d)! Still handling the bytes we got..", bytesRead, bytes);
			}
			ReadRequestCompleted(0, bytesRead, &read_overlap);
			bytesLeft -= bytesRead;
		}
		return STATUS_NOERROR;
	}


	int HandleCommEvent()
	{
		// Get and clear current errors on the port.
		DWORD   dwErrors;
		COMSTAT comStat;
		DWORD ret;
		if (!ClearCommError(hCommPort, &dwErrors, &comStat))
		{
			LOG(ERR, "Kepler::HandleCommEvent - error calling ClearCommError: %d", ret = GetLastError());
			return ret;
		}
		if (dwErrors & CE_FRAME)
		{
			LOG(ERR, "Kepler::HandleCommEvent - hardware detected a framing error!");
		}
		if (dwErrors & CE_OVERRUN)
		{
			LOG(ERR, "Kepler::HandleCommEvent - A character-buffer overrun has occurred. The next character is lost!");
		}
		if (dwErrors & CE_RXOVER)
		{
			LOG(ERR, "Kepler::HandleCommEvent - An input buffer overflow has occurred!");
		}
		if (dwErrors & CE_RXPARITY)
		{
			LOG(ERR, "Kepler::HandleCommEvent - hardware detected a parity error!");
		}

		if (comStat.cbInQue > 0)
		{
			LOG(HELPERFUNC, "Kepler::HandleCommEvent - %d bytes in receiving buffer!", comStat.cbInQue);
			return BlockingRead(comStat.cbInQue);
		}
		return 0;
	}
}





