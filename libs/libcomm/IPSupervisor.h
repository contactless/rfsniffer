#pragma once
#include "libcomm.h"
#include "../libutils/thread.h"
#include "../libutils/locks.h"
#include "IPConnection.h"

class CSupervisorClient;

typedef vector<CConnection*> CConnectionVector;
typedef map<CConnection*, CSupervisorClient*> CConnectionClientMap;
typedef map<CSupervisorClient*, int> CClientCountMap;


class LIBCOMM_API CSupervisor
	:public CThread
{
	CConnectionVector m_Connections, m_Connections2Add, m_Connections2Remove;
	CConnectionClientMap m_ClientsMap;
	CClientCountMap m_ClientCountMap;
	CLock m_Lock;
	bool m_Work, m_inThread;
	int m_InsidePool;

	void ProcessDelta();
public:
	CSupervisor(void);
	virtual ~CSupervisor(void);

	void AddConnection(CSupervisorClient *client, CConnection* connection);
	void RemoveConnection(CConnection* connection);
	void AddClient(CSupervisorClient *client);
	void RemoveClient(CSupervisorClient *client, bool bForce);
	void Stop(DWORD timeOut);

	virtual void OnRecieveEx(CConnection* Conn);
	virtual void OnNewConnectionEx(CConnection* ParentConn, CConnection* Conn);
	virtual void OnDeleteConnectionEx(CConnection* Conn);
	virtual void Poll(int timeoutSec, bool bInternal=false);
	virtual void OnIdle();
	virtual int ThreadProc();

};
