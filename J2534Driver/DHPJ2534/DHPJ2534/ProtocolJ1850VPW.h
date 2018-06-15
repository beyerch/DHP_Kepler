
#pragma once
#include "protocol.h"
class CProtocolJ1850VPW :
	public CProtocol
{
public:
	CProtocolJ1850VPW(int ProtocolID);
	~CProtocolJ1850VPW(void);
	
	enum baud_rate
	{
		J1850_VPW_1X,
		J1850_VPW_4X
	};

	int Connect(unsigned long channelId, unsigned long Flags, unsigned long Baudrate);
	int Disconnect();
	//int ReadMsgs(PASSTHRU_MSG * pMsg, unsigned long * pNumMsgs, unsigned long Timeout);
	int WriteMsg(PASSTHRU_MSG * pMsg, unsigned long Timeout);
	bool HandleMsg(PASSTHRU_MSG * pMsg, char * flags);
	//	int IOCTL(unsigned long IoctlID, void *pInput, void *pOutput);

	int IOCTL(unsigned long IoctlID, void *pInput, void *pOutput);
	//int GetIOCTLParam(SCONFIG * pConfig);
	int SetIOCTLParam(SCONFIG * pConfig);
protected:

private:

	baud_rate CurrBaudRate;

};

