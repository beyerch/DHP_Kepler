#include "stdafx.h"
#include "ProtocolCAN.h"
#include "helper.h"
#include "Kepler.h"

CProtocolCAN::CProtocolCAN(int ProtocolID) : CProtocol(ProtocolID)
{
}


CProtocolCAN::~CProtocolCAN()
{
}

int CProtocolCAN::Connect(unsigned long channelId, unsigned long Flags, unsigned long Baudrate)
{
	LOG(PROTOCOL, "CProtocolCAN::Connect - flags: 0x%x", Flags);

	unsigned char CANMode[] = { 0x02, 0x00, 0x03, 0xA0, 0x02, 0x02 };
	Kepler::Send(CANMode, 6, 1000);

	// call base class implementation for general settings
	return CProtocol::Connect(channelId, Flags, 0);
}

int CProtocolCAN::Disconnect()
{
	/*unsigned char VpwMode[] = { 0x02, 0x00, 0x02, 0xA0, 0xFF };
	Kepler::Send(VpwMode, 5, 1000);*/
	return CProtocol::Disconnect();
}

int CProtocolCAN::WriteMsg(PASSTHRU_MSG * pMsg, unsigned long Timeout)
{

	char MessageIndex = 0;

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

	char LenH = ((12 + 3) & 0xFF00) >> 8;
	char LenL = (12 + 3) & 0x00FF;

	message[MessageIndex++] = 0x02;
	message[MessageIndex++] = LenH;
	message[MessageIndex++] = LenL;
	message[MessageIndex++] = 0xA1; //Network Message
	message[MessageIndex++] = 0x00; //CAN Message
	if (iso15765_addr_type)
	{
		return ERR_FAILED;
	}
	else
	{
		message[MessageIndex++] = 0;
		memcpy(message + MessageIndex, pMsg->Data, pMsg->DataSize);
		memset(message + MessageIndex + pMsg->DataSize, 0x00, 12 - pMsg->DataSize);
	}

	if (Kepler::Send(message, pMsg->DataSize + 6, Timeout) != pMsg->DataSize + 6)
	{
		LOG(ERR, "CProtocol:DoWriteMsg - sending message failed!");
		free(message);
		return ERR_FAILED;
	}

	if (this->IsLoopback())
	{
		pMsg->RxStatus = 0x01;
		pMsg->DataSize = 12;
		pMsg->ExtraDataIndex = 12;

		AddToRXBuffer(pMsg);
	}

	return STATUS_NOERROR;

}

bool CProtocolCAN::HandleMsg(PASSTHRU_MSG * pMsg, char * flags)
{

	return true;
}

