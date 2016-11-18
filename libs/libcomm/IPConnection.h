#pragma once
#include "Connection.h"

class CSupervisor;



class LIBCOMM_API CIPConnection :
	public CConnection
{
protected:
	SOCKET m_Socket;
public:
	CIPConnection(SOCKET socket=INVALID_SOCKET, string Name="");
	virtual ~CIPConnection(void);

	virtual void Connect(string ConnectString);
	virtual void Disconnect();
	virtual void Send(const char *Buffer, size_t Size);
	virtual void Recv(char *Buffer, size_t &Size, bool bWaitAll );
	virtual void SetTimeout(int Timeout);

	virtual SOCKET getSocket(){return m_Socket;};
	virtual string GetPeerName();
	virtual void OnSelect(CSupervisor *superviser);
};
