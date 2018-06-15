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

#include "StdAfx.h"
#include "Protocol.h"
#include <assert.h>
#include "helper.h"
#include "Kepler.h"
#include "shim_debug.h"
#include <string.h>
#include <new>
#include <thread>

#define MAX_FLAGS_LEN 32

bool WINAPI KeplerListener(char * msg, int len, void * data)
{
	CProtocol * me = (CProtocol*)data;
	if (me->IsListening())
	{
		
		if (msg[3] == (char)0xaa)
		{
			return me->ParseMsg(msg, len);
		}
		else
		{
			LOG(ERR, "CProtocol::KeplerListener: Ignoring [%s]", msg);
			return false;
		}
	}
	return false;
}

CProtocol::CProtocol(int ProtocolID)
{
	protocolID = ProtocolID;

	// we are using J2534-2 PIN switching mode
	if (protocolID & 0x8000)
		SetPinSwitched(true);

	_dummy_filter_id = 0xf1f10001;
	rollingPeriodicMsgId = 0xbebe0001;

	rxBufferSize = 0;
	rxBufferOverflow = false;
	hiIndex = lowIndex = 0;
	datarate = SARDINE_DEFAULT_CAN_BAUD_RATE;
	listening = false;
	loopback = false;
	Kepler::RegisterListener((LPKEPLERLISTENER)KeplerListener, this);
	periodicMsgHandler = NULL;
	//std::thread SenderThraed(std::bind(&CProtocol::SendMessages, this));
}

CProtocol::~CProtocol(void)
{
	Kepler::RemoveListener((LPKEPLERLISTENER)KeplerListener);
	if (periodicMsgHandler)
		delete periodicMsgHandler;
	
}

// callback from CPeriodicMsgCallback, when timer has gone off in one of CPeriodicMsg instances
int CProtocol::SendPeriodicMsg(PASSTHRU_MSG * pMsg, unsigned long Id)
{
	LOG(PROTOCOL, "CProtocol::SendPeriodicMsg: msg id 0x%x", Id);
	if (!IsConnected())
	{
		LOG(ERR, "CProtocol::SendPeriodicMsg - not connected!");
		return ERR_DEVICE_NOT_CONNECTED;
	}
	while (!Kepler::ready)
	{

	}
	// Write message (blocking)
	return WriteMsg(pMsg, 0);
}

bool CProtocol::ParseMsg(char * msg, int len)
{
	PASSTHRU_MSG * pMsg = new (std::nothrow) PASSTHRU_MSG;

	if (!pMsg)
	{
		LOG(ERR, "Cprotocol:ParseMsg - Could not create PASSTHU_MSG object. Out of memory!");
		return false;
	}

	memcpy(pMsg->Data, msg+4, len - 5);
	pMsg->DataSize = len - 5;
	pMsg->Timestamp = GetTime();
	pMsg->ProtocolID = this->protocolID;
	pMsg->RxStatus = 0;
	pMsg->ExtraDataIndex = pMsg->DataSize;
	
	char flags[MAX_FLAGS_LEN + 1];

		if (HandleMsg(pMsg, flags))
		{
			LOG(PROTOCOL, "CProtocol::ParseMsg: Message accepted - adding to rx buffer");
			LogMessage(pMsg, RECEIVED, channelId, "");
			AddToRXBuffer(pMsg);
		}
		else
		{
			LOG(PROTOCOL, "CProtocol::ParseMsg: Message ignored");

			if ((pMsg->ProtocolID == J1850VPW) || (pMsg->ProtocolID == J1850VPW))
			{
				LogMessage(pMsg, ISO15765_RECV, channelId, " ignored before final assembly");
				// ISO15765 handler copied the contents from this msg. We can delete this, since we don't push it to rxbuffer 
				delete pMsg;
			}
			else
			{
				// not handled by this protocol, do not log.
				delete pMsg;
				return false;
			}
		}
	return true;
}

int CProtocol::ProtocolID()
{
	return protocolID;
}

void CProtocol::SetPinSwitched(bool pinSwitchedMode)
{
	pinSwitched = pinSwitchedMode;
}

bool CProtocol::IsPinSwitched()
{
	return pinSwitched;
}

int CProtocol::StopPeriodicMessages()
{
	LOG(PROTOCOL, "CProtocol::StopPeriodicMessages");
	if (periodicMsgHandler)
		periodicMsgHandler->RemoveAllPeriodicMessages();
	else
		return ERR_FAILED;
	return STATUS_NOERROR;
}

int CProtocol::DeleteFilters()
{
	unsigned char DeleteFilters[] = { 0x02, 0x00, 0x01, 0xC3};
	Kepler::Send(DeleteFilters, 4, 1000);

	return 0;
}

int CProtocol::GetDatarate(unsigned long * rate)
{
	LOG(PROTOCOL, "CProtocol::GetDatarate: %d", datarate);
	LOG(ERR, "CProtocol::GetDatarate: --- does not actually retrive datarate from device! FIXME!");
	*rate = datarate;
	return STATUS_NOERROR;
}

bool CProtocol::IsListening()
{
	return listening;
}

void CProtocol::SetToListen(bool listen)
{
	listening = listen;
}

void CProtocol::ClearRXBuffer()
{
	LOG(PROTOCOL, "CProtocol::ClearRXBuffer");
	rx_lock.Lock();
	rxBufferOverflow = false;
	lowIndex = 0;
	hiIndex = 0;
	rxBufferSize = 0;
	rx_lock.Unlock();
}

void CProtocol::ClearTXBuffer()
{
	LOG(PROTOCOL, "CProtocol::ClearTXBuffer -- FIXME: not implemented!---");
}

int CProtocol::DoAddToRXBuffer(PASSTHRU_MSG * pMsg)
{
	if (rxBufferSize >= MAX_RX_BUFFER_SIZE)
	{
		LOG(ERR, "CProtocol::DoAddToRXBuffer -- buffer overflow!---");
		rxBufferOverflow = true;

		// we get rid of our oldest message
		PASSTHRU_MSG * oldMsg = rxbuffer[lowIndex];
		delete oldMsg;
		lowIndex++;
		lowIndex %= MAX_RX_BUFFER_SIZE;

		rxbuffer[hiIndex] = pMsg;
		hiIndex++;
		hiIndex %= MAX_RX_BUFFER_SIZE;
		return ERR_BUFFER_OVERFLOW;
	}
	else
	{
		rxbuffer[hiIndex] = pMsg;
		hiIndex++;
		hiIndex %= MAX_RX_BUFFER_SIZE;
		rxBufferSize++;
	}
	return STATUS_NOERROR;
}

int CProtocol::AddToRXBuffer(PASSTHRU_MSG * pMsg)
{
	LOG(PROTOCOL, "CProtocol::AddToRXBuffer");
	rx_lock.Lock();
	unsigned int ret = DoAddToRXBuffer(pMsg);
	rx_lock.Unlock();
	return ret;
}

void CProtocol::SetRXBufferOverflow(bool status)
{
	rx_lock.Lock();
	LOG(HELPERFUNC, "CProtocol::SetRXBufferOverflow %d", status);
	rxBufferOverflow = status;
	rx_lock.Unlock();
}

int CProtocol::GetRXMessageCount()
{
	rx_lock.Lock();
	LOG(HELPERFUNC, "CProtocol::GetRXMessageCount: %d", rxBufferSize);

	if ((rxBufferSize == ((hiIndex - lowIndex) % MAX_RX_BUFFER_SIZE)) == 0)
	{
		LOG(ERR, "CProtocol::GetRXMessageCount: %d", rxBufferSize);
		LOG(ERR, "CProtocol::GetRXMessageCount: hiIndex %d", hiIndex);
		LOG(ERR, "CProtocol::GetRXMessageCount: lowIndex %d", lowIndex);
	}

	assert(rxBufferSize == ((hiIndex - lowIndex) % MAX_RX_BUFFER_SIZE));	// sanity check
	int s = rxBufferSize;
	rx_lock.Unlock();
	return s;
}

PASSTHRU_MSG * CProtocol::PopMessage()
{
	LOG(PROTOCOL, "CProtocol::PopMessage");
	rx_lock.Lock();
	PASSTHRU_MSG * msg = rxbuffer[lowIndex++];
	lowIndex %= MAX_RX_BUFFER_SIZE;
	rxBufferSize--;
	rxBufferOverflow = false;
	rx_lock.Unlock();
	return msg;
}

bool CProtocol::IsRXBufferOverflow()
{
	rx_lock.Lock();
	LOG(HELPERFUNC, "CProtocol::IsBufferOverflow");
	bool rxoverflow = rxBufferOverflow;
	rx_lock.Unlock();
	return rxoverflow;
}

bool CProtocol::IsConnected()
{
	return Kepler::IsConnected();
}

int CProtocol::WriteMsgs(PASSTHRU_MSG * pMsgs, unsigned long * pNumMsgs, unsigned long Timeout)
{
	LOG(PROTOCOL, "CProtocol::WriteMsgs - num_msgs: %d, timeout %d", *pNumMsgs, Timeout);
	unsigned int err;

	if (!IsConnected())
	{
		LOG(ERR, "CProtocol::WriteMsgs - not connected");
		return ERR_DEVICE_NOT_CONNECTED;
	}

	if (*pNumMsgs > MAX_J2534_MESSAGES)
	{
		LOG(ERR, "CProtocol::WriteMsgs - tried sending too many messages (according to specs limit is 10)");
		return ERR_EXCEEDED_LIMIT;
	}

	for (unsigned int i = 0; i<*pNumMsgs; i++)
	{
		PASSTHRU_MSG * curr_msg = &pMsgs[i];
		if (curr_msg->ProtocolID != ProtocolID())
		{
#ifndef ENFORCE_PROTOCOL_IDS_IN_MSGS
			LOG(ERR, "CProtocol::WriteMsgs - invalid protocol id in message #%d: %d != %d (expected protocol) -- ignoring for now", i, curr_msg->ProtocolID, ProtocolID());
#else
			LOG(ERR, "CProtocol::WriteMsgs - invalid protocol id in message #%d: %d != %d (expected protocol)", i, curr_msg->ProtocolID, ProtocolID());
			return ERR_MSG_PROTOCOL_ID;
#endif
		}
		err = WriteMsg(curr_msg, Timeout);
		if (err != STATUS_NOERROR)
		{
			LOG(ERR, "CPRotocol::WriteMsgs - error while writing msg -> aborting!");
			return err;
		}

	}
	LOG(PROTOCOL_VERBOSE, "CProtocol::WriteMsgs: writing %d messages successful!", *pNumMsgs);
	return STATUS_NOERROR;
}

int CProtocol::GetIOCTLParam(SCONFIG * pConfig)
{
	LOGW(PROTOCOL, _T("CProtocol::GetIOCTLParam - parameter %d [%s]"), pConfig->Parameter, dbug_param2str(pConfig->Parameter));
	switch (pConfig->Parameter)
	{
	case DATA_RATE:
		GetDatarate(&pConfig->Value);
		return STATUS_NOERROR;
	case LOOPBACK:
		pConfig->Value = IsLoopback();
		LOG(PROTOCOL, "CProtocol::GetIOCTLParam -get loopback: %d", pConfig->Value);
		return STATUS_NOERROR;
	case J1962_PINS:
		LOG(PROTOCOL, "CProtocol::SetIOCTLParam - setting J1962 pins: pin1: %d, pin2: %d", (pConfig->Value >> 8) & 0xFF, pConfig->Value & 0xFF);
		return SetJ1962Pins((pConfig->Value >> 8) & 0xFF, pConfig->Value & 0xFF);
	default:
		LOG(ERR, "CProtocol::GetIOCTLParam - Parameter not supported ! --- FIXME?");
		return ERR_NOT_SUPPORTED;
	}

	return STATUS_NOERROR;
}

int CProtocol::SetIOCTLParam(SCONFIG * pConfig)
{
	LOGW(PROTOCOL, _T("CProtocol::SetIOCTLParam - parameter %d [%s]"), pConfig->Parameter, dbug_param2str(pConfig->Parameter));
	switch (pConfig->Parameter)
	{
	case DATA_RATE:
#ifdef IGNORE_SILENTLY_UNIMPLEMENTED_FEATURES
		LOG(ERR, "CProtocol::SetIOCTLParam - ignoring set DATA_RATE --- FIXME");
		return STATUS_NOERROR;
#else
		LOG(ERR, "CProtocol::SetIOCTLParam - unsupported parameter! --- FIXME");
		return ERR_NOT_SUPPORTED;
#endif
	case LOOPBACK:
		if (pConfig->Value == 0)
		{
			LOG(PROTOCOL, "CProtocol::SetIOCTLParam - disabling loopback");
		}
		else
		{
			LOG(PROTOCOL, "CProtocol::SetIOCTLParam - enabling loopback");
		}
		SetLoopback((pConfig->Value == 1) ? true : false);
		return STATUS_NOERROR;
	case J1962_PINS:
		LOG(PROTOCOL, "CProtocol::SetIOCTLParam - setting J1962 pins: pin1: %d, pin2: %d", (pConfig->Value >> 8) & 0xFF, pConfig->Value & 0xFF);
		return SetJ1962Pins((pConfig->Value >> 8) & 0xFF, pConfig->Value & 0xFF);
	case CAN_MIXED_FORMAT:
		return STATUS_NOERROR;
	break;
	default:
		LOG(ERR, "CProtocol::SetIOCTLParam - Parameter not supported! --- FIXME?");
		return ERR_NOT_SUPPORTED;
	}

	return STATUS_NOERROR;
}

int CProtocol::IOCTL(unsigned long IoctlID, void *pInput, void *pOutput)
{
	LOG(PROTOCOL, "CProtocol::IOCTL - ioctl command %d", IoctlID);
	unsigned int err;

	switch (IoctlID)
	{
	case GET_CONFIG:
	{
		if (pInput == NULL)
		{
			LOG(ERR, "CProtocol::IOCTL: pInput==NULL!");
			return ERR_NULL_PARAMETER;
		}
		SCONFIG_LIST * pList = (SCONFIG_LIST*)pInput;
		for (unsigned int i = 0; i<pList->NumOfParams; i++)
		{
			SCONFIG * pConfig = &pList->ConfigPtr[i];
			if (pConfig == NULL)
			{
				LOG(ERR, "CProtocol::IOCTL: pConfig==NULL!");
				return ERR_NULL_PARAMETER;
			}
			err = GetIOCTLParam(pConfig);
			if (err != STATUS_NOERROR)
			{
				LOG(ERR, "CProtocol::IOCTL: exiting !");
				return err;
			}
		}
		break;
	}
	case SET_CONFIG:
	{
		if (pInput == NULL)
		{
			LOG(ERR, "CProtocol::IOCTL: pInput==NULL!");
			return ERR_NULL_PARAMETER;
		}
		SCONFIG_LIST * pList = (SCONFIG_LIST*)pInput;
		for (unsigned int i = 0; i<pList->NumOfParams; i++)
		{
			SCONFIG * pConfig = &pList->ConfigPtr[i];
			if (pConfig == NULL)
			{
				LOG(ERR, "CProtocol::IOCTL: pConfig==NULL!");
				return ERR_NULL_PARAMETER;
			}
			err = SetIOCTLParam(pConfig);
			if (err != STATUS_NOERROR)
			{
				LOG(ERR, "CProtocol::IOCTL: exiting !");
				return err;
			}
		}
		break;
	}
	case CLEAR_TX_BUFFER:
		ClearTXBuffer();
		break;
	case CLEAR_RX_BUFFER:
		ClearRXBuffer();
		break;
	case CLEAR_PERIODIC_MSGS:
		return StopPeriodicMessages();
		break;
	case CLEAR_MSG_FILTERS:
		LOG(MAINFUNC, "CProtocol::IOCTL: Remove all filters")
		return DeleteFilters();
		break;
	case CAN_MIXED_FORMAT:
		return STATUS_NOERROR;
	case ISO15765_STMIN:
		return STATUS_NOERROR;

		break;
	default:
		LOG(MAINFUNC, "CProtocol::IOCTL----- NOT SUPPORTED -----");
		return ERROR_NOT_SUPPORTED;
		break;
	}
	return STATUS_NOERROR;
}

void CProtocol::SetLoopback(bool _loopback)
{
	rx_lock.Lock();
	loopback = _loopback;
	rx_lock.Unlock();
}

bool CProtocol::IsLoopback()
{
	rx_lock.Lock();
	bool _loopback = loopback;
	rx_lock.Unlock();
	return _loopback;
}

int CProtocol::SetJ1962Pins(unsigned long pin1, unsigned long pin2)
{
	LOG(PROTOCOL, "CPRotocol::SetJ1962Pins - pin1: %d, pin2: %d", pin1, pin2);
#ifdef IGNORE_SILENTLY_UNIMPLEMENTED_FEATURES
	LOG(ERR, "CPRotocol::SetJ1962Pins - pin switching not really implemented, just saving given values");
	J1962Pin1 = pin1;
	J1962Pin2 = pin2;
	return STATUS_NOERROR;
#else	
	LOG(ERR, "CPRotocol::SetJ1962Pins - pin switching not supported");
	return ERR_NOT_SUPPORTED;
#endif
}

int CProtocol::GetJ1962Pins(unsigned long * pin1, unsigned long * pin2)
{
	LOG(PROTOCOL, "CPRotocol::GetJ1962Pins - pin1: %d, pin2: %d", J1962Pin1, J1962Pin2);
#ifdef IGNORE_SILENTLY_UNIMPLEMENTED_FEATURES
	LOG(ERR, "CPRotocol::SetJ1962Pins - pin switching not really implemented, just replying with saved values");
	*pin1 = J1962Pin1;
	*pin2 = J1962Pin2;
	return STATUS_NOERROR;
#else	
	LOG(ERR, "CPRotocol::SetJ1962Pins - pin switching not supported");
	return ERR_NOT_SUPPORTED;
#endif
}

int CProtocol::Connect(unsigned long _channelId, unsigned long Flags, unsigned long Baudrate)
{
	LOG(PROTOCOL, "CProtocol::Connect - flags: 0x%x", Flags);

	channelId = _channelId;
	periodicMsgHandler = new CPeriodicMessageHandler();
	if (periodicMsgHandler == NULL)
		return ERR_FAILED;

	if (!periodicMsgHandler->createMsgHandlerThread(channelId))
	{
		delete periodicMsgHandler;
		periodicMsgHandler = NULL;
		return ERR_FAILED;
	}

	/*if (CInterceptor::UseInterceptor())
	{
		interceptor = new CInterceptor(this, protocolID);
	}*/

	// we are now receiving messages via HandleMsg
	SetToListen(true);

	// Delete existing message filters for this protocol and stop periodic messages, as specified in J2534-1
	//DeleteFilters();
	StopPeriodicMessages();

	return STATUS_NOERROR;
}

int CProtocol::Disconnect()
{
	LOG(PROTOCOL, "CProtocol::Disconnect");

	// Delete existing message filters for this protocol and stop periodic messages, as specified in J2534-1
	StopPeriodicMessages();
	DeleteFilters();

	delete periodicMsgHandler;
	periodicMsgHandler = NULL;

	return STATUS_NOERROR;
}

int CProtocol::DoReadMsgs(PASSTHRU_MSG * pMsgs, unsigned int count, bool overflow)
{
	for (unsigned int i = 0; i<count; i++)
	{
		LOG(PROTOCOL_VERBOSE, "CProtocol::DoReadMsg - reading msg #%d", i);
		PASSTHRU_MSG * curr_msg = PopMessage();
	//	dbug_printmsg(curr_msg, _T("ReadMsg"), 1, false);
		PASSTHRU_MSG * pDestMsg = &pMsgs[i];
		LOG(PROTOCOL_VERBOSE, "CProtocol::DoReadMsg - copying msg #%d", i);
		memcpy(pDestMsg, curr_msg, sizeof(PASSTHRU_MSG));
		
	}
	if (overflow)
	{
		LOG(ERR, "CProtocol::ReadMsgs: we had buffer overflow");
		return ERR_BUFFER_OVERFLOW;
	}
	else
	{
		LOG(PROTOCOL, "CProtocol::DoReadMsgs success");
		return STATUS_NOERROR;
	}
}

int CProtocol::ReadMsgs(PASSTHRU_MSG * pMsgs, unsigned long * pNumMsgs, unsigned long Timeout)
{
	LOG(PROTOCOL, "CProtocol::ReadMsgs: timeout %d", Timeout);

	unsigned long count = GetRXMessageCount();
	if ((count == 0) && (Timeout == 0))
	{
		LOG(PROTOCOL_VERBOSE, "CProtocol::ReadMsgs: No messages and timeout==0 ");
		*pNumMsgs = 0;
		return ERR_BUFFER_EMPTY;
	}

	bool overflow = IsRXBufferOverflow();
	if (count >= (*pNumMsgs))
	{
		LOG(PROTOCOL_VERBOSE, "CProtocol::ReadMsgs: Enough messages in buffer: sending %d msgs", *pNumMsgs);
		// we have (more than) enough of buffered messages, read just the amount requested
		return DoReadMsgs(pMsgs, *pNumMsgs, overflow);
	}
	else if ((Timeout == 0) && (count>0))
	{
		LOG(PROTOCOL_VERBOSE, "CProtocol::ReadMsgs: timeout==0 and we have >0 msgs in buffer: sending %d msgs", count);
		*pNumMsgs = count;
		return DoReadMsgs(pMsgs, count, overflow);
	}
	else
	{
		LOG(PROTOCOL_VERBOSE, "CProtocol::ReadMsgs: sleeping for %d milliseconds while waiting for new messages", Timeout);
		// sleep here
		SleepEx(Timeout, FALSE);	// FIXME: need better time handling: sleep MAXIMUM of timeout, in case enough messages will come before that
		count = GetRXMessageCount();
		if (count>0)
		{
			// read MIN(*pNumMsgs,count) messages
			*pNumMsgs = (count > *pNumMsgs) ? *pNumMsgs : count;
			return DoReadMsgs(pMsgs, *pNumMsgs, overflow);
		}
		else
			return ERR_BUFFER_EMPTY;
	}
	// we shouldn't reach this
	LOG(PROTOCOL_VERBOSE, "CProtocol::ReadMsgs: Shouldn't hit this... ever...", Timeout); 
	assert(0);
	return ERR_NOT_SUPPORTED;
}

bool WINAPI KeplerSystemListener(char *msg, int len, void *data)
{
	CProtocol * me = (CProtocol*)data;
	if (me->IsListening())
	{

		if (msg[3] == (char)0xC0 || msg[3] == (char)0xEF)
		{
			return me->SetFilterSuccess(msg, len);
		}
		else
		{
			LOG(ERR, "CProtocol::KeplerSystemListener: Ignoring [%s]", msg);
			return false;
		}
	}
	return false;
}

int CProtocol::StartMsgFilter(unsigned long FilterType, PASSTHRU_MSG * pMaskMsg, PASSTHRU_MSG * pPatternMsg, PASSTHRU_MSG * pFlowControlMsg, unsigned long * pFilterID)
{
	char tmpFilterType;
	unsigned char * FilterMessage;
	USHORT FilterMessageLength;
	FilterSetSuccessfull = 0;
	LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::StartMsgFilter - Attempting to set a message filter");
	LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::StartMsgFilter - FilterType: %d", FilterType);

	if (FilterType > 3)
	{
		LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::StartMsgFilter - FilterType unsupported for this protocol!", FilterType);
		return ERR_FAILED;
	}

	LogMessage(pMaskMsg, FILTER, 1, "Filter Mask");
	LogMessage(pPatternMsg, FILTER, 1, "Pattern message");

	LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::StartMsgFilter - Registering system listener");


	Kepler::RegisterListener((LPKEPLERLISTENER)KeplerSystemListener, this);

	if (FilterType == 1)
	{
		tmpFilterType = 1;
	}
	else if (FilterType == 2)
	{
		tmpFilterType = 0;
	}
	else if (FilterType == 3)
	{
		tmpFilterType = 3;
	}

	FilterMessageLength = (pMaskMsg->DataSize * 3) + 7;
	FilterMessage = new (std::nothrow) unsigned char[FilterMessageLength];
	if (!FilterMessage)
	{
		LOG(ERR, "CProtocolJ1850VPW::StartMsgFilter - Out of memory!");
		return ERR_FAILED;
	}
	FilterMessage[0] = 0x02;
	FilterMessage[1] = ((FilterMessageLength - 3) & 0xFF00) >> 8;
	FilterMessage[2] = (FilterMessageLength - 3) & 0x00FF;
	FilterMessage[3] = 0xC0;
	FilterMessage[4] = (pMaskMsg->DataSize & 0xFF00) >> 8;
	FilterMessage[5] = (pMaskMsg->DataSize & 0x00FF);
	FilterMessage[6] = tmpFilterType;
	memcpy(FilterMessage + 7, pMaskMsg->Data, pMaskMsg->DataSize);
	memcpy(FilterMessage + 7 + pMaskMsg->DataSize, pPatternMsg->Data, pPatternMsg->DataSize);
	if (pFlowControlMsg != NULL)
	{
		memcpy(FilterMessage + 7 + pMaskMsg->DataSize + pMaskMsg->DataSize, pFlowControlMsg->Data, pFlowControlMsg->DataSize);
	}
	else
	{
		memset(FilterMessage + 7 + pMaskMsg->DataSize + pMaskMsg->DataSize, 0x00, pMaskMsg->DataSize);
	}

	Kepler::Send(FilterMessage, FilterMessageLength, 1000);

	LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::StartMsgFilter - Waiting for response FIXME: ADD TIMEOUT HERE (ProtocolJ1850VPW.cpp L177)");


	//TODO: Add timeout here
	while (FilterSetSuccessfull == 0);

	if (FilterSetSuccessfull == 2)
	{
		LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::StartMsgFilter - Failed");

		return ERR_FAILED;
	}
	LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::StartMsgFilter - Success");

	LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::StartMsgFilter - Removing system listener");
	Kepler::RemoveListener((LPKEPLERLISTENER)KeplerSystemListener);

	FilterMessage[0] = 0x02;
	FilterMessage[1] = ((FilterMessageLength - 8) & 0xFF00) >> 8;
	FilterMessage[2] = (FilterMessageLength - 8) & 0x00FF;
	FilterMessage[3] = 0xB6;
	FilterMessage[4] = (pMaskMsg->DataSize & 0xFF00) >> 8;
	FilterMessage[5] = (pMaskMsg->DataSize & 0x00FF);
	memcpy(FilterMessage + 6, pMaskMsg->Data, pMaskMsg->DataSize);
	memcpy(FilterMessage + 6 + pMaskMsg->DataSize, pPatternMsg->Data, pPatternMsg->DataSize);

	Kepler::Send(FilterMessage, 14, 1000);

	return STATUS_NOERROR;
}

bool CProtocol::SetFilterSuccess(char * msg, int len)
{
	if (msg[3] == (char)0xC0 && msg[4] == (char)0x01)
	{
		LOG(ERR, "CProtocol::SetFilterSuccess: Sucess");
		FilterSetSuccessfull = 1;
		return true;
	}

	if (msg[3] == (char)0xEF)
	{
		LOG(ERR, "CProtocol::SetFilterSuccess: Error!");
		FilterSetSuccessfull = 2;
		return true;
	}

	return false;
}

int CProtocol::StopMsgFilter(unsigned long FilterID)
{
	LOG(PROTOCOL, "CProtocol::StopMsgFilter - filter id 0x%x", FilterID);
#ifdef IGNORE_SILENTLY_UNIMPLEMENTED_FEATURES
	LOG(ERR, "CProtocol::StopMsgFilter - not yet implemented, ignored");
	return STATUS_NOERROR;
#else
	LOG(ERR, "CProtocol::StopMsgFilter  --- FIXME ! does not exist");
	return ERR_NOT_SUPPORTED;
#endif
}

int CProtocol::StartPeriodicMsg(PASSTHRU_MSG * pMsg, unsigned long * pMsgID, unsigned long TimeInterval)
{
	LOG(PROTOCOL, "CProtocol::StartPeriodicMsg - time interval %d milliseconds", TimeInterval);
	dbug_printmsg(pMsg, _T("PeriodicMsg"), 1, true);

	if (!IsConnected())
	{
		LOG(ERR, "CProtocol::StartPeriodicMsg - not connected");
		return ERR_DEVICE_NOT_CONNECTED;
	}

	if (pMsg->ProtocolID != ProtocolID())
	{
#ifdef ENFORCE_PROTOCOL_IDS_IN_MSGS
		LOG(ERR, "CProtocol::WriteMsgs - invalid protocol id: %d != %d (expected protocol) -- ignoring for now", pMsg->ProtocolID, ProtocolID());
#else
		LOG(ERR, "CProtocol::WriteMsgs - invalid protocol id: %d != %d (expected protocol)", pMsg->ProtocolID, ProtocolID());
		return ERR_MSG_PROTOCOL_ID;
#endif
	}


	CPeriodicMsg * msg = new CPeriodicMsg(this, rollingPeriodicMsgId, TimeInterval);
	if (msg == NULL)
		return ERR_FAILED;
	if (msg->AttachMessage(pMsg))
	{
		delete msg;
		return ERR_FAILED;
	}

	int ret;
	if ((ret = periodicMsgHandler->AddPeriodicMessage(msg)) != STATUS_NOERROR)
	{
		delete msg;
		return ret;
	}


	*pMsgID = rollingPeriodicMsgId++;

	return STATUS_NOERROR;
}

int CProtocol::StopPeriodicMsg(unsigned long MsgID)
{
	LOG(PROTOCOL, "CProtocol::StopPeriodicMsg - msg id 0x%x", MsgID);
	int ret = periodicMsgHandler->RemovePeriodicMessage(MsgID);
	return ret;
}
