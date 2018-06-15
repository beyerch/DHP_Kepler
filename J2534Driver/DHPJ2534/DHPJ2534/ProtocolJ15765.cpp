#include "stdafx.h"
#include "ProtocolISO15765.h"
#include "helper.h"
#include "Kepler.h"

int time;
CProtocolJ15765::CProtocolJ15765(int ProtocolID)
	:CProtocol(ProtocolID)
{
}


CProtocolJ15765::~CProtocolJ15765(void)
{
}

int CProtocolJ15765::Connect(unsigned long channelId, unsigned long Flags, unsigned long Baudrate)
{
	LOG(PROTOCOL, "CProtocolJ15765::Connect - flags: 0x%x", Flags);

	unsigned char CANMode[] = { 0x02, 0x00, 0x03, 0xA0, 0x02, 0x02 };
	Kepler::Send(CANMode, 6, 1000);

	// call base class implementation for general settings
	return CProtocol::Connect(channelId, Flags, 0);

}

int CProtocolJ15765::Disconnect()
{
	/*unsigned char VpwMode[] = { 0x02, 0x00, 0x02, 0xA0, 0xFF };
	Kepler::Send(VpwMode, 5,1000);*/
	return CProtocol::Disconnect();
}

int CProtocolJ15765::WriteMsg(PASSTHRU_MSG * pMsg, unsigned long Timeout)
{

	char MessageIndex = 0;
	PASSTHRU_MSG rtnMsg;

	LOG(PROTOCOL_MSG, "CProtocolCAN::DoWriteMsg - timeout %d", Timeout);
	LOG(ERR, "CProtocolCAN::DoWriteMsg  --- FIXME -- We ignore Timeout for now - call will be blocking");

	if (pMsg->ProtocolID != ProtocolID())
	{
		LOG(ERR, "CProtocolCAN::DoWriteMsg - invalid protocol id %d != J1850VPW", pMsg->ProtocolID);
		return ERR_MSG_PROTOCOL_ID;
	}

	char can_29bit_id = (pMsg->TxFlags & 0x100) >> 8;

	if (can_29bit_id)
	{
		LOG(ERR, "CProtocolCAN::DoWriteMsg  --- 29 bit CAN ID currently unsupported");
		return ERR_NOT_SUPPORTED;
	}

	char iso15765_addr_type = (pMsg->TxFlags & 0x80) >> 7;
	char iso15765_frame_pad = (pMsg->TxFlags & 0x40) >> 6;
	LOG(PROTOCOL_MSG, "CProtocolCAN::DoWriteMsg - 29 Bit: %d Addr Type: %d Frame Pad: %d", can_29bit_id, iso15765_addr_type, iso15765_frame_pad)

	unsigned char* message;
	unsigned int messageSize = 0;

	message = new (std::nothrow) unsigned char[pMsg->DataSize + 4];

	if (!message)
	{
		LOG(ERR, "CProtocolCAN:DoWriteMsg - Could not allocate memory for message");
		return ERR_FAILED;
	}

	char LenH = ((pMsg->DataSize + 3) & 0xFF00) >> 8;
	char LenL = (pMsg->DataSize + 3) & 0x00FF;

	message[MessageIndex++] = 0x02;
	message[MessageIndex++] = LenH;
	message[MessageIndex++] = LenL;
	message[MessageIndex++] = 0xA1; //Network Message
	message[MessageIndex++] = 0x01; //ISO15765 Message
	if (iso15765_addr_type)
	{
		message[MessageIndex++] = 1;
		message[MessageIndex++] = pMsg->Data[4];
		memcpy(message + MessageIndex, pMsg->Data, 4);
		memcpy(message + MessageIndex +4, pMsg->Data + 5, pMsg->DataSize - 5);
	
	}
	else
	{
		message[MessageIndex++] = 0;
		memcpy(message + MessageIndex, pMsg->Data, pMsg->DataSize);
	}

	
		//memcpy(rtnMsg.Data, pMsg->Data, 4);
		//rtnMsg.RxStatus =  9;
		//rtnMsg.DataSize = 4;
		//rtnMsg.ExtraDataIndex = 0;
		////rtnMsg.Timestamp = time++;
		//AddToRXBuffer(&rtnMsg);
	


	if (Kepler::Send(message, pMsg->DataSize + 6, Timeout) != pMsg->DataSize + 6)
	{
		LOG(ERR, "CProtocol:DoWriteMsg - sending message failed!");
		free(message);
		return ERR_FAILED;
	}
	

		
	
	return STATUS_NOERROR;

}

bool CProtocolJ15765::HandleMsg(PASSTHRU_MSG * pMsg, char * flags)
{
	return true;
}
