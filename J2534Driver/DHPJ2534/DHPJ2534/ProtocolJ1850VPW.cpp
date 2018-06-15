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
#include "ProtocolJ1850VPW.h"
#include "helper.h"
#include "Kepler.h"

CProtocolJ1850VPW::CProtocolJ1850VPW(int ProtocolID)
	:CProtocol(ProtocolID)
{
}


CProtocolJ1850VPW::~CProtocolJ1850VPW(void)
{
}

bool CProtocolJ1850VPW::HandleMsg(PASSTHRU_MSG * pMsg, char * flags)
{
	return true;
}

int CProtocolJ1850VPW::Connect(unsigned long channelId, unsigned long Flags, unsigned long Baudrate)
{
	LOG(PROTOCOL, "CProtocolJ1850VPW::Connect - flags: 0x%x", Flags);

	unsigned char VpwMode[] = { 0x02, 0x00, 0x02, 0xA0, 0x00 };
	Kepler::Send(VpwMode, 5, 1000);

	if (Baudrate == 41666)
	{
		LOG(PROTOCOL, "CProtocolJ1850VPW:: Entering highspeed mode!", Flags);
		unsigned char HighSpeedMode[] = { 0x02, 0x00, 0x01, 0xB1 };
		Kepler::Send(HighSpeedMode, 4,10000);
	}

	// call base class implementation for general settings
	return CProtocol::Connect(channelId,Flags, Baudrate);

}

int CProtocolJ1850VPW::Disconnect()
{
	unsigned char VpwMode[] = { 0x02, 0x00, 0x02, 0xA0, 0xFF };
	Kepler::Send(VpwMode, 5,1000);
	return CProtocol::Disconnect();
}


int CProtocolJ1850VPW::WriteMsg(PASSTHRU_MSG * pMsg, unsigned long Timeout)
{
	long tmpDataSize;
	unsigned char* message;
	
	LOG(PROTOCOL_MSG, "CProtocolJ1850VPW::DoWriteMsg - timeout %d", Timeout);
	LOG(ERR, "CProtocolJ1850VPW::DoWriteMsg  --- FIXME -- We ignore Timeout for now - call will be blocking");

	
	if (pMsg->ProtocolID != ProtocolID())
	{
		LOG(ERR, "CProtocolJ1850VPW::DoWriteMsg - invalid protocol id %d != J1850VPW", pMsg->ProtocolID);
		return ERR_MSG_PROTOCOL_ID;
	}

	if (pMsg->DataSize < 4)
	{
		LOG(ERR, "CProtocolJ1850VPW::DoWriteMsg - invalid data length: %d", pMsg->DataSize);
		return ERR_INVALID_MSG;
	}

	LOG(PROTOCOL, "CProtocol::DoWriteMsg - timeout %d", Timeout);
	LOG(ERR, "CProtocol::DoWriteMsg  --- FIXME -- We ignore Timeout for now - call will be blocking");

	//dbug_printmsg(pMsg, _T("Msg"), 1, true);

	//LogMessage(pMsg, SENT, channelId, "");

	message = new (std::nothrow) unsigned char[pMsg->DataSize + 4];
	if (!message)
	{
		LOG(ERR, "CProtocol:DoWriteMsg - Could no allocate memory for message");
		return ERR_FAILED;
	}


	tmpDataSize = pMsg->DataSize + 1;

	char LenH = (tmpDataSize & 0xFF00) >> 8;
	char LenL = tmpDataSize & 0x00FF;

	message[0] = 0x02;
	message[1] = LenH;
	message[2] = LenL;
	message[3] = 0xA1;

	memcpy(message + 4, pMsg->Data, tmpDataSize - 1);
	
	if (Kepler::Send(message, tmpDataSize + 3, Timeout) != tmpDataSize + 3)
	{
		LOG(ERR, "CProtocol:DoWriteMsg - sending message failed!");
		free(message);
		return ERR_FAILED;
	}

	free(message);
	return STATUS_NOERROR;
}

int CProtocolJ1850VPW::SetIOCTLParam(SCONFIG * pConfig)
{
	LOGW(PROTOCOL, _T("CProtocolJ1850VPW::SetIOCTLParam - parameter %d [%s]"), pConfig->Parameter, dbug_param2str(pConfig->Parameter));
	switch (pConfig->Parameter)
	{
	case DATA_RATE:
		if (pConfig->Value == 41666)
		{
			LOG(PROTOCOL, "CProtocolJ1850VPW::SetIOCTLParam - Entering highspeed mode!");
			unsigned char HighSpeedMode[] = { 0x02, 0x00, 0x01, 0xB1 };
			Kepler::Send(HighSpeedMode, 4, 1000);
		}
		else
		{
			LOG(PROTOCOL, "CProtocolJ1850VPW::SetIOCTLParam - Entering lowspeed mode!");
			unsigned char HighSpeedMode[] = { 0x02, 0x00, 0x01, 0xB0 };
			Kepler::Send(HighSpeedMode, 4, 1000);
		}
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
	default:
		LOG(ERR, "CProtocol::SetIOCTLParam - Parameter not supported! --- FIXME?");
		return ERR_NOT_SUPPORTED;
	}

	return STATUS_NOERROR;
}


int CProtocolJ1850VPW::IOCTL(unsigned long IoctlID, void *pInput, void *pOutput)
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
	default:
		LOG(MAINFUNC, "CProtocol::IOCTL----- NOT SUPPORTED -----");
		return ERROR_NOT_SUPPORTED;
		break;
	}
	return STATUS_NOERROR;
}


/*
int CProtocolJ1850VPW::StartPeriodicMsg( PASSTHRU_MSG * pMsg, unsigned long * pMsgID, unsigned long TimeInterval)
{
return ERR_NOT_SUPPORTED;
}

int CProtocolJ1850VPW::StopPeriodicMsg( unsigned long pMsgID)
{
return ERR_NOT_SUPPORTED;
}

int CProtocolJ1850VPW::IOCTL(unsigned long IoctlID, void *pInput, void *pOutput)
{
#ifdef IGNORE_SILENTLY_UNIMPLEMENTED_FEATURES_WITH_DATAPRO
return STATUS_NOERROR;
#else
return ERR_NOT_SUPPORTED;
#endif
}

*/
