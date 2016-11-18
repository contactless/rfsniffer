#pragma once
#include "Connection.h"

class LIBCOMM_API CSerialConnection :
	public CConnection
{
#ifdef WIN32
	HANDLE m_hCom;
#else
	int m_hCom;
#endif

    int m_nBits;
    char m_nEvent;
    int m_nStop;
    int m_baurate;

	int setOpt(void);


public:
	CSerialConnection(void);
	virtual ~CSerialConnection(void);

	virtual void Connect(string ConnectString);
	virtual void Disconnect();
	virtual void Send(const char *Buffer, size_t Size);
	virtual void Recv(char *Buffer, size_t &Size, bool bWaitAll );
	virtual void SetTimeout(int Timeout);
	virtual string GetPeerName(){return "SERIAL PORT";};

};
