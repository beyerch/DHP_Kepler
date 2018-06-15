#pragma once
#include "protocol.h"
#include "stdafx.h"
#include "Protocol.h"

class CProtocolJ15765 :
	public CProtocol
{
public:
	CProtocolJ15765(int ProtocolID);
	~CProtocolJ15765(void);

	
	int Connect(unsigned long channelId, unsigned long Flags, unsigned long Baudrate);
	int Disconnect();
	bool HandleMsg(PASSTHRU_MSG * pMsg, char * flags);
	
	int WriteMsg(PASSTHRU_MSG * pMsg, unsigned long Timeout);
	

protected:

private:



};
