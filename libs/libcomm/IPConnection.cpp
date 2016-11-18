#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../libutils/strutils.h"
#include "IPConnection.h"
#include "IPSupervisor.h"

#ifdef WIN32
#define SHUT_RDWR SD_BOTH
//	#define EAGAIN WSAEWOULDBLOCK 
//	#define EWOULDBLOCK WSAEWOULDBLOCK 
#endif

CIPConnection::CIPConnection(SOCKET socket, string Name)
{
	m_Socket = socket;
	m_Connected = (socket!=INVALID_SOCKET);
	m_ConnectString = "@"+Name+"{"+itoa(socket)+"}";
}

CIPConnection::~CIPConnection(void)
{
	CLog::Default()->Printf(5, "Delete %s connection %p", (m_Socket == INVALID_SOCKET ? "closed":"active"), this);

	if (m_Socket != INVALID_SOCKET)
		Disconnect();

}

void CIPConnection::Connect(string ConnectString)
{
	m_ConnectString = ConnectString;
	string addr, port;
	int pos = ConnectString.find(':');
	if (pos==string::npos)
		throw CHaException(CHaException::ErrConnectStringError, "Bad connect string (%s)", ConnectString.c_str());

	addr = ConnectString.substr(0, pos);
	port = ConnectString.substr(pos+1);

	//----------------------
	// Create a SOCKET for connecting to server
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_Socket == INVALID_SOCKET) 
		throw CHaException(CHaException::ErrCreateSocketError, "Error at socket(): %ld", WSAGetLastError());

	
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_port = htons( atoi(port.c_str()) );

// If the user input is an alpha name for the host, use gethostbyname()
// If not, get host by addr (assume IPv4)
    if (isalpha(addr.c_str()[0])) 
	{        /* host address is a name */
		//getaddrinfo()
		struct hostent *remoteHost = gethostbyname(addr.c_str());
		if (remoteHost == NULL) 
		{
			throw CHaException(CHaException::ErrConnectError, "Cannot resolve <%s>", addr.c_str());
		}

		if (remoteHost->h_addrtype!= AF_INET)
		{
			throw CHaException(CHaException::ErrConnectError, "Invalid address type <%s>", addr.c_str());
		}

		clientService.sin_addr.s_addr = *(u_long *) remoteHost->h_addr_list[0];
    } 
	else 
	{
        clientService.sin_addr.s_addr = inet_addr(addr.c_str());
        if (clientService.sin_addr.s_addr == INADDR_NONE) 
			throw CHaException(CHaException::ErrConnectError, "The IPv4 address <%s> entered must be a legal address. ", addr.c_str());
    }


	//----------------------
	// Connect to server.
	if ( connect( m_Socket, (sockaddr*) &clientService, sizeof(clientService) ) == SOCKET_ERROR)
		throw CHaException(CHaException::ErrConnectError, "Error at connect(): %ld", WSAGetLastError());

	if (m_Timeout )
	{
		struct timeval tv;
		tv.tv_sec = m_Timeout;
		tv.tv_usec = 0;
		setsockopt (m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof (tv) ); 
	}

	int param = 1;
	setsockopt (m_Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&param, sizeof (param) ); 
	setsockopt (m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&param, sizeof (param) ); 
	m_Connected = true;
}

void CIPConnection::Disconnect()
{
	CLog::Default()->Printf(5, "Disconnect %s connection %p/%d", (m_Socket == INVALID_SOCKET ? "closed":"active"), this, m_Socket);

	shutdown(m_Socket, SHUT_RDWR);
#ifdef WIN32
	closesocket(m_Socket);
#else
	close(m_Socket);
#endif
	m_Socket=INVALID_SOCKET;
	m_Connected = false;
}


void CIPConnection::Send(const char *Buffer, size_t Size)
{
	int l = send(m_Socket, Buffer, Size, 0);

	if (l!=Size)
		throw CHaException(CHaException::ErrSendError, "Error at send(): %ld", WSAGetLastError());
}


void CIPConnection::Recv(char *Buffer, size_t &Size, bool bWaitAll)
{
	if (false && m_Timeout>0)
	{
		fd_set fd_recv;
		FD_ZERO(&fd_recv);
		FD_SET(m_Socket, &fd_recv);
		timeval t;
		t.tv_sec=m_Timeout;
		t.tv_usec=0;
		
		int sel = select(m_Socket+1, &fd_recv, NULL, NULL, &t);
		if (sel==SOCKET_ERROR)
			throw CHaException(CHaException::ErrRecvError, "Error at select(): %ld", WSAGetLastError());
		else if (sel==0)
		{
			Size=0;
			return;
		}
	}

	int l = recv(m_Socket, Buffer, Size, bWaitAll?MSG_WAITALL:0);

	if (l==SOCKET_ERROR)
	{
#ifdef WIN32
		if (WSAGetLastError()==WSAETIMEDOUT)
#else
		if (WSAGetLastError()==EAGAIN || WSAGetLastError()==0)
#endif
		{
			if (m_Timeout==0)
				Disconnect();

			Size = 0;
			return;
		}

		throw CHaException(CHaException::ErrRecvError, "Error at recv(%s): %ld (%s)", m_ConnectString.c_str(), WSAGetLastError(), strerror(WSAGetLastError()));
	}

	if (l==0)
	{
		int err = WSAGetLastError();
#ifdef WIN32
		if (err==WSAETIMEDOUT)
#else
		if (err==EAGAIN || err==0)
#endif
		{
			Disconnect();
			Size = 0;
			return;
		}

		throw CHaException(CHaException::ErrRecvError, "Error at recv(%s) l==0 : %ld (%s)", m_ConnectString.c_str(), WSAGetLastError(), strerror(WSAGetLastError()));
	}

	Size = l;
}


void CIPConnection::OnSelect(CSupervisor *superviser)
{
	superviser->OnRecieveEx(this);
}


void CIPConnection::SetTimeout(int Timeout)
{
	CConnection::SetTimeout(Timeout);

	if (m_Socket!=SOCKET_ERROR )
	{
		struct timeval tv;
		tv.tv_sec = m_Timeout;
		tv.tv_usec = 0;
		setsockopt (m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof tv ); 
	}
}


string CIPConnection::GetPeerName()
{
	sockaddr_in addr;
	socklen_t size= sizeof(addr);
	if (getpeername(getSocket(), (sockaddr*)&addr, &size)==0)
	{
		char Buffer[500];
		snprintf(Buffer, sizeof(Buffer), "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		return Buffer;
	}
	else
		return "<FAIL>";

}
