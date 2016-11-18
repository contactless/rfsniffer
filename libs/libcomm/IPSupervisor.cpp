#include "stdafx.h"
#include "IPSupervisor.h"
#include "SupervisorClient.h"
#include "../libutils/Exception.h"

CSupervisor::CSupervisor(void)
{
	m_inThread = false;
	m_InsidePool = 0;
}

CSupervisor::~CSupervisor(void)
{
	Stop(10000);

	LOCK(m_Lock);
	ProcessDelta();

	for (CConnectionVector::iterator i=m_Connections.begin();i!=m_Connections.end();i++)
	{
		// Failed to clean up connections
		//delete (*i);
	}

	m_Connections.clear();
}


void CSupervisor::AddConnection(CSupervisorClient *client, CConnection* connection)
{
	LOCK(m_Lock);

	m_Connections2Add.push_back(connection);
	m_ClientsMap[connection] = client;
	m_ClientCountMap[client]++;
}

void CSupervisor::RemoveConnection(CConnection* connection)
{
	LOCK(m_Lock);

	m_Connections2Remove.push_back(connection);

	for_each(CConnectionVector, m_Connections2Add, i)
		if ((*i)==connection)
		{
			m_Connections2Add.erase(i);
			return;
		}
}

void CSupervisor::AddClient(CSupervisorClient *client)
{
	LOCK(m_Lock);

	m_ClientCountMap[client]++;
}

void CSupervisor::RemoveClient(CSupervisorClient *client, bool bForce)
{
	LOCK(m_Lock);

	if (bForce)
	{
		ProcessDelta();

		for_each(CConnectionClientMap, m_ClientsMap, i)
			if (client==i->second)
				RemoveConnection(i->first);

		ProcessDelta();
	}

	if (m_ClientCountMap.find(client) == m_ClientCountMap.end())
		return;

	m_ClientCountMap[client]--;
	if (!m_ClientCountMap[client])
		m_ClientCountMap.erase(client);
}


void CSupervisor::ProcessDelta()
{
/*	if (m_InsidePool)
		return;
		*/
	LOCK(m_Lock);

	for (CConnectionVector::iterator conn2Remove=m_Connections2Remove.begin();conn2Remove!=m_Connections2Remove.end();conn2Remove++)
	{
		for (CConnectionVector::iterator i=m_Connections.begin();i!=m_Connections.end();i++)
			if ((*i)->getSocket()==(*conn2Remove)->getSocket())
			{
				CLog::GetLog("Main")->Printf(5, "CSupervisor::ProcessDelta close socket %d", (*i)->getSocket());
				OnDeleteConnectionEx(*i);
				RemoveClient(m_ClientsMap[*i], false);
				m_ClientsMap.erase(*i);				
				(*i)->AutoDelete();
				m_Connections.erase(i);
				break;
			}
	}
	m_Connections2Remove.clear();

	for_each (CConnectionVector, m_Connections2Add, conn2Add)
	{
		bool isDublicate = false;

		for_each (CConnectionVector, m_Connections, conn)
			if ((*conn)==(*conn2Add))
				isDublicate = true;

		if (!isDublicate)
			m_Connections.push_back(*conn2Add);
	}

	m_Connections2Add.clear();
}

void CSupervisor::Stop(DWORD timeOut)
{
	if (!m_Work)
		return;

	m_Work = false;
	Wait(timeOut);

	for (CConnectionVector::iterator i=m_Connections.begin();i!=m_Connections.end();i++)
		RemoveConnection(*i);

	ProcessDelta();
}

void CSupervisor::OnRecieveEx(CConnection* Conn)
{
	if (m_ClientsMap.find(Conn)!=m_ClientsMap.end())
		m_ClientsMap[Conn]->OnRecieve(Conn);
}

void CSupervisor::OnNewConnectionEx(CConnection* ParentConn, CConnection* Conn)
{
	AddConnection(m_ClientsMap[ParentConn], Conn);
}

void CSupervisor::OnDeleteConnectionEx(CConnection* Conn)
{
	if (m_ClientsMap.find(Conn)!=m_ClientsMap.end())
		m_ClientsMap[Conn]->OnDeleteConnection(Conn);
}

void CSupervisor::Poll(int timeoutSec, bool bInternal)
{
	if (m_inThread && !bInternal)
		return;

	fd_set fd_recv, fd_err;
	size_t fd_count= 0;

	{
		LOCK(m_Lock);

		ProcessDelta();

		if (m_Connections.size()==0)
		{
			if (timeoutSec>0)
			{
				OnIdle();
#ifdef WIN32
				Sleep(500);
#else
				usleep(500);
#endif
			}

			return;
		}

		FD_ZERO(&fd_recv);
		FD_ZERO(&fd_err);

		for (CConnectionVector::iterator i=m_Connections.begin();i!=m_Connections.end();i++)
		{
			if ((*i)->getSocket()==INVALID_SOCKET)
				RemoveConnection(*i);
			else if ((*i)->isConnected())
			{
				FD_SET((*i)->getSocket(), &fd_recv);
				FD_SET((*i)->getSocket(), &fd_err);

				if (fd_count<(*i)->getSocket())
					fd_count=(*i)->getSocket();
			}
		}
	}
	timeval t;
	t.tv_sec=timeoutSec;
	t.tv_usec=0;
	int sel = select(fd_count+1, &fd_recv, NULL, &fd_err, &t);

	if (sel>0)
	{
		LOCK(m_Lock);
		m_InsidePool++;
		try
		{
			CConnectionVector m_ReadConnections, m_ErrorConnections;

			for_each(CConnectionVector, m_Connections, i)
			{
				if (FD_ISSET((*i)->getSocket(), &fd_err))
				{
					sel--;
					m_ErrorConnections.push_back(*i);
				}
				if (FD_ISSET((*i)->getSocket(), &fd_recv))
				{
					sel--;
					m_ReadConnections.push_back(*i);
				}
			}

			for_each(CConnectionVector, m_ErrorConnections, i)
			{
				CLog::GetLog("Main")->Printf(0, "CSupervisor::Poll(%d) FD_ERR IS SET", __LINE__);
				sel--;
				CConnection *conn = *i;
				conn->Disconnect();
				RemoveConnection(conn);
				delete conn;
			}

			for_each(CConnectionVector, m_ReadConnections, i)
			{
				(*i)->OnSelect(this);
			}
		}
		catch(CHaException)
		{
			m_InsidePool--;
			throw;
		}
		m_InsidePool--;

		if (sel && false)
		{
			throw CHaException(CHaException::ErrSyncFailed, "BAD fd_set. Sel=%d", sel);	
		}
	}
	else if (sel==SOCKET_ERROR )
	{

		for (CConnectionVector::iterator i = m_Connections.begin(); i != m_Connections.end(); i++)
		{
			if ((*i)->getSocket() == INVALID_SOCKET)
				RemoveConnection(*i);
			else
			{
				FD_ZERO(&fd_recv);
				FD_SET((*i)->getSocket(), &fd_recv);
				fd_count = (*i)->getSocket();
				timeval t;
				t.tv_sec = 0;
				t.tv_usec = 0;
				int sel = select(fd_count + 1, &fd_recv, NULL, NULL, &t);

				if (sel == SOCKET_ERROR)
				{
					CLog::GetLog("Main")->Printf(0, "CSupervisor::Poll(%d) found bad connection", __LINE__);
					(*i)->Disconnect();
					RemoveConnection(*i);
				}
			}
		}
	// don't error??

		//throw CHaException(CHaException::ErrSyncFailed, "select failed");
	}
	else // if (timeoutSec>0)  ???????? Need idle in call without timeout???
		OnIdle();
}


int CSupervisor::ThreadProc()
{
	m_inThread = m_Work = true;

	while (m_Work)
	{
		try
		{
			Poll(2, true);
		}
		catch (CHaException ex)
		{
			CLog::GetLog("Main")->Printf(0, "CSupervisor::ThreadProc runtime error. Error code %d, Message '%s'.\n", ex.GetCode(), ex.GetMessage().c_str());
		}
	}

	return 0;
}

void CSupervisor::OnIdle()
{
	for_each(CClientCountMap, m_ClientCountMap, i)
		if (i->first)
			i->first->OnIdle();
}
