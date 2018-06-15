#pragma once
#include "Protocol.h"
class CProtocolCAN :
	public CProtocol
{
public:
	CProtocolCAN(int ProtocolID);
	~CProtocolCAN();

	int Connect(unsigned long channelId, unsigned long Flags, unsigned long Baudrate);
	int Disconnect();
	bool HandleMsg(PASSTHRU_MSG * pMsg, char * flags);
	int WriteMsg(PASSTHRU_MSG * pMsg, unsigned long Timeout);
};

