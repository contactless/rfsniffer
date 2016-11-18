#pragma once
#include "libcomm.h"

class CBuffer;

#ifndef WIN32
typedef int SOCKET;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;

inline int WSAGetLastError() { return errno; };
#endif

class CSupervisor;

class LIBCOMM_API CConnection
{
protected:
	int m_Timeout;
	string m_ConnectString;
	bool m_AutoDelete;
	bool m_Connected;
public:
	CConnection(void);
	virtual ~CConnection(void);

	virtual void Connect(string ConnectString) = 0;
	virtual void Disconnect() = 0;
	virtual void Reconnect();
	virtual SOCKET getSocket()  = 0;
	virtual void SendString(string s){Send(s.c_str(), s.length());};
	virtual void Send(const char *Buffer, size_t Size) = 0;
	virtual void SendBuffer(CBuffer &buffer);
	virtual void Recv(char *Buffer, size_t &Size, bool bWaitAll ) = 0;
	virtual void SetTimeout(int Timeout);
	virtual string GetPeerName() = 0;
	void SetAutoDelete(bool AutoDelete=true){m_AutoDelete=AutoDelete;};
	void AutoDelete(){if (m_AutoDelete) delete this;};
	virtual bool isConnected(){ return m_Connected; };
	virtual void OnSelect(CSupervisor *superviser) = 0;

};
