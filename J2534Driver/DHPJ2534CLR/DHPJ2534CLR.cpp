#include <stdafx.h>
#include "DHPJ2534clr.h"
#include <string>

int LastError;
///////////////////////////////////// PassThruFunctions /////////////////////////////////////////////////
DllExport PassThruOpen(void *pName, unsigned long *pDeviceID)
{
	pDeviceID = 0;
	DHPJ2534Sharp::J2534Container::CreateJ5234Object();
	DHPJ2534Sharp::J2534Container::J2534Handle->PassThruOpen();


	return LastError = STATUS_NOERROR;
}

DllExport PassThruClose(unsigned long DeviceID)
{
	//TODO: Shutdown serial and clean up stuffs

	/*ManagedGlobals::J2534Wrapper->PassThruClose(DeviceID);*/
	DHPJ2534Sharp::J2534Container::J2534Handle->Shutdown();
	return  LastError = STATUS_NOERROR;
}

DllExport PassThruConnect(unsigned long DeviceID, unsigned long ProtocolID, unsigned long Flags, unsigned long Baudrate, unsigned long *pChannelID)
{
	if (ProtocolID != 1 && ProtocolID != 5 && ProtocolID != 6)
	{
		return LastError = ERR_NOT_SUPPORTED;
	}
	int _ChannelID = DHPJ2534Sharp::J2534Container::J2534Handle->PassThruConnect(DeviceID, ProtocolID, Flags, Baudrate);
	*pChannelID = _ChannelID;
	return  LastError = STATUS_NOERROR;
}

DllExport PassThruDisconnect(unsigned long ChannelID)
{
	DHPJ2534Sharp::J2534Container::J2534Handle->PassThruDisconnect(ChannelID);
	return  LastError = STATUS_NOERROR;
}

DllExport PassThruReadMsgs(unsigned long ChannelID, PASSTHRU_MSG *pMsg, unsigned long *pNumMsgs, unsigned long Timeout)
{
	
	System::Collections::Generic::List<DHPJ2534Sharp::KeplerCommand^>^ ReadMessages = 
		DHPJ2534Sharp::J2534Container::J2534Handle->PassThruReadMsgs(ChannelID, *pNumMsgs, Timeout);

	for (int i = 0; i < ReadMessages->Count; i++)
	{
		PASSTHRU_MSG * pDestMsg = &pMsg[i];
		//DHPJ2534Sharp::KeplerCommand^ receievedMessage = DHPJ2534Sharp::J2534Container::J2534Handle->PassThruReadMsg(ChannelID, Timeout);
		if (ReadMessages[i] == nullptr)
		{
			//TODO: FLAG ERROR HERE
	
			//This is here for a sanity check
			break;
		}

		//We got a message, we need to bring it from the managed world to the unmanged world so we can return it to the caller
		//Pin the managed array in memory
		pin_ptr<System::Byte> ptrBuffer = &ReadMessages[i]->Message[ReadMessages[i]->Message->GetLowerBound(0)];
		//Copy the managed array data to the unmanaged array 
		memcpy(pDestMsg->Data, ptrBuffer, ReadMessages[i]->Message->Length);
		//Copy the rest of the important information
		pDestMsg->Timestamp = ReadMessages[i]->Timestamp;
		pDestMsg->DataSize = ReadMessages[i]->DataSize;
		pDestMsg->ExtraDataIndex = ReadMessages[i]->ExtraDataIndex;
		pDestMsg->ProtocolID = ReadMessages[i]->ProtocolID;
		pDestMsg->RxStatus = ReadMessages[i]->RxStatus;
	}
	*pNumMsgs = ReadMessages->Count;
	if (*pNumMsgs <= 0)
	{
		return LastError = ERR_BUFFER_EMPTY;
	}
	return LastError = STATUS_NOERROR;

}

DllExport PassThruWriteMsgs(unsigned long ChannelID, PASSTHRU_MSG *pMsg, unsigned long *pNumMsgs, unsigned long Timeout)
{

	for (int i = 0; i < *pNumMsgs; i++)
	{
		//We need to get the unmnaged message data to a managed array. so:
		//Create the managed array
		array<System::Byte>^ Data = gcnew array<System::Byte>(pMsg->DataSize);
		//Pin the managed array in memory so we can use a fast memcpy
		pin_ptr<System::Byte> ptrBuffer = &Data[Data->GetLowerBound(0)];
		//Copy the unmanged array to the managed array
		memcpy(ptrBuffer, pMsg->Data, pMsg->DataSize);
		//Call Write and let .NET handle the rest!
		DHPJ2534Sharp::J2534Container::J2534Handle->PassThruWriteMsg(ChannelID, Data, pMsg->DataSize, pMsg->TxFlags);
		//Tell user application we sent one message
		*pNumMsgs = i+1;
	}

	return  LastError = STATUS_NOERROR;
}

DllExport PassThruStartPeriodicMsg(unsigned long ChannelID, PASSTHRU_MSG *pMsg, unsigned long *pMsgID, unsigned long TimeInterval)
{
	//We need to get the unmnaged message data to a managed array. so:
	//Create the managed array
	array<System::Byte>^ Data = gcnew array<System::Byte>(pMsg->DataSize);
	//Pin the managed array in memory so we can use a fast memcpy
	pin_ptr<System::Byte> ptrBuffer = &Data[Data->GetLowerBound(0)];
	//Copy the unmanged array to the managed array
	memcpy(ptrBuffer, pMsg->Data, pMsg->DataSize);
	//Start the periodic message on the .NET side
	*pMsgID = DHPJ2534Sharp::J2534Container::J2534Handle->PassThruStartPeriodicMsg(ChannelID, Data, pMsg->TxFlags, TimeInterval);

	return  LastError = STATUS_NOERROR;
}

DllExport PassThruStopPeriodicMsg(unsigned long ChannelID, unsigned long MsgID)
{
	DHPJ2534Sharp::J2534Container::J2534Handle->PassThruStopPeriodicMsg(MsgID, ChannelID);
	return LastError = STATUS_NOERROR;
}

DllExport PassThruStartMsgFilter(unsigned long ChannelID, unsigned long FilterType, PASSTHRU_MSG *pMaskMsg, PASSTHRU_MSG *pPatternMsg, PASSTHRU_MSG *pFlowControlMsg, unsigned long *pFilterID)
{
	//We need to get the unmnaged message data to a managed array. so:

	//Create the managed array
	array<System::Byte>^ MaskMsg = gcnew array<System::Byte>(pMaskMsg->DataSize);
	//Pin the managed array in memory so we can use a fast memcpy
	pin_ptr<System::Byte> ptrBuffer = &MaskMsg[MaskMsg->GetLowerBound(0)];
	//Copy the unmanged array to the managed array
	memcpy(ptrBuffer, pMaskMsg->Data, pMaskMsg->DataSize);

	//Repeat for Pattern Msg
	array<System::Byte>^ PatternMsg = gcnew array<System::Byte>(pPatternMsg->DataSize);
	ptrBuffer = &PatternMsg[PatternMsg->GetLowerBound(0)];
	memcpy(ptrBuffer, pPatternMsg->Data, pPatternMsg->DataSize);

	//Repeat for Flow control message if not null
	array<System::Byte>^ FlowControlMsg = gcnew array<System::Byte>(pMaskMsg->DataSize);

	if (pFlowControlMsg != NULL)
	{
		ptrBuffer = &FlowControlMsg[FlowControlMsg->GetLowerBound(0)];
		memcpy(ptrBuffer, pFlowControlMsg->Data, pFlowControlMsg->DataSize);
	}

	*pFilterID = DHPJ2534Sharp::J2534Container::J2534Handle->PassThruStartMsgFilter(ChannelID, FilterType, MaskMsg, PatternMsg, FlowControlMsg);
	return  LastError = STATUS_NOERROR;
}

DllExport PassThruStopMsgFilter(unsigned long ChannelID, unsigned long FilterID)
{
	//DHPJ2534Sharp::J2534Container::J2534Handle->PassThruStopMsgFilter(ChannelID, FilterID);
	return LastError = STATUS_NOERROR;
}

DllExport PassThruSetProgrammingVoltage(unsigned long DeviceID, unsigned long PinNumber, unsigned long Voltage)
{
	return LastError = ERR_NOT_SUPPORTED;
}

DllExport PassThruReadVersion(unsigned long DeviceID, char *pFirmwareVersion, char *pDllVersion, char *pApiVersion)
{
	//DHPJ2534Sharp::KeplerVersion^ version = DHPJ2534Sharp::J2534Container::J2534Handle->PassThruReadVersion(DeviceID);
	
	pFirmwareVersion = "01.17";
	pDllVersion = "01.00";
	pApiVersion = "04.04";

	return  LastError = STATUS_NOERROR;
}


// We don't alter any last_errors in this function
DllExport PassThruGetLastError(char *pErrorDescription)
{
	return LastError;
}

int 
GetIOCTLParam(SCONFIG * pConfig, unsigned long ChannelID)
{
	//LOGW(PROTOCOL, _T("CProtocol::GetIOCTLParam - parameter %d [%s]"), pConfig->Parameter, dbug_param2str(pConfig->Parameter));
	switch (pConfig->Parameter)
	{
	case DATA_RATE:
	//	GetDatarate(&pConfig->Value);
		return STATUS_NOERROR;
	case LOOPBACK:
		pConfig->Value = (unsigned long)DHPJ2534Sharp::J2534Container::J2534Handle->kepler->GetLoopback(ChannelID);
		//LOG(PROTOCOL, "CProtocol::GetIOCTLParam -get loopback: %d", pConfig->Value);
		return STATUS_NOERROR;
	case J1962_PINS:
		//LOG(PROTOCOL, "CProtocol::SetIOCTLParam - setting J1962 pins: pin1: %d, pin2: %d", (pConfig->Value >> 8) & 0xFF, pConfig->Value & 0xFF);
		return ERR_NOT_SUPPORTED;
	case STMIN_TX:
		return 0;
		break;
	default:
		//LOG(ERR, "CProtocol::GetIOCTLParam - Parameter not supported ! --- FIXME?");
		return STATUS_NOERROR;
	}

	return STATUS_NOERROR;
}

int SetIOCTLParam(SCONFIG * pConfig, unsigned long ChannelID)
{
	//LOGW(PROTOCOL, _T("CProtocol::SetIOCTLParam - parameter %d [%s]"), pConfig->Parameter, dbug_param2str(pConfig->Parameter));
	switch (pConfig->Parameter)
	{
	case DATA_RATE:
		
	case LOOPBACK:
		DHPJ2534Sharp::J2534Container::J2534Handle->kepler->SetLoopback(pConfig->Value, ChannelID);
		return STATUS_NOERROR;
	case J1962_PINS:
		return ERR_NOT_SUPPORTED;
	case CAN_MIXED_FORMAT:
		return STATUS_NOERROR;
		break;
	default:
		//LOG(ERR, "CProtocol::SetIOCTLParam - Parameter not supported! --- FIXME?");
		return STATUS_NOERROR;
	}

	return STATUS_NOERROR;
}
DllExport PassThruIoctl(unsigned long ChannelID, unsigned long IoctlID, void *pInput, void *pOutput)
{
	unsigned int err;
	// check and debug IOCTL commands, parameters and values. Handle here the general commands (non-protocol spesific)
	switch (IoctlID)
	{
	case GET_CONFIG:
	{
		if (pInput == NULL)
		{
			//LOG(ERR, "CProtocol::IOCTL: pInput==NULL!");
			return ERR_NULL_PARAMETER;
		}
		SCONFIG_LIST * pList = (SCONFIG_LIST*)pInput;
		for (unsigned int i = 0; i<pList->NumOfParams; i++)
		{
			SCONFIG * pConfig = &pList->ConfigPtr[i];
			if (pConfig == NULL)
			{
				//LOG(ERR, "CProtocol::IOCTL: pConfig==NULL!");
				return ERR_NULL_PARAMETER;
			}
			err = GetIOCTLParam(pConfig, ChannelID);
			if (err != STATUS_NOERROR)
			{
				//LOG(ERR, "CProtocol::IOCTL: exiting !");
				return err;
			}
		}
		break;
	}
	case SET_CONFIG:
	{
		if (pInput == NULL)
		{
			//LOG(ERR, "CProtocol::IOCTL: pInput==NULL!");
			return ERR_NULL_PARAMETER;
		}
		SCONFIG_LIST * pList = (SCONFIG_LIST*)pInput;
		for (unsigned int i = 0; i<pList->NumOfParams; i++)
		{
			SCONFIG * pConfig = &pList->ConfigPtr[i];
			if (pConfig == NULL)
			{
				//LOG(ERR, "CProtocol::IOCTL: pConfig==NULL!");
				return ERR_NULL_PARAMETER;
			}
			err = SetIOCTLParam(pConfig, ChannelID);
			if (err != STATUS_NOERROR)
			{
				//LOG(ERR, "CProtocol::IOCTL: exiting !");
				return err;
			}
		}
		break;
	}
		break;
	case READ_VBATT:
		*(unsigned long*)pOutput = 12 * 1000;
		return STATUS_NOERROR;
		break;
	case FIVE_BAUD_INIT:
		break;
	case FAST_INIT:
		break;
	case CLEAR_TX_BUFFER:
		DHPJ2534Sharp::J2534Container::J2534Handle->ClearTxBuffer();
		break;
	case CLEAR_RX_BUFFER:
		DHPJ2534Sharp::J2534Container::J2534Handle->ClearRxBuffer(ChannelID);

		break;
	case CLEAR_PERIODIC_MSGS:
		DHPJ2534Sharp::J2534Container::J2534Handle->ClearPeriodicMessages(ChannelID);
		break;
	case CLEAR_MSG_FILTERS:
		DHPJ2534Sharp::J2534Container::J2534Handle->ClearMessageFilters(ChannelID);
		break;
	case CLEAR_FUNCT_MSG_LOOKUP_TABLE:
		break;
	case ADD_TO_FUNCT_MSG_LOOKUP_TABLE:
		break;
	case DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE:
		break;
	case READ_PROG_VOLTAGE:
		return ERR_NOT_SUPPORTED;
		break;
	default:
		return ERR_INVALID_IOCTL_ID;
		break;
	}

	return STATUS_NOERROR;
}

