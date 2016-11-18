#include "stdafx.h"
#include "IPListener.h"
#include "IPSupervisor.h"
#include "../libutils/strutils.h"

CIPListener::CIPListener(void)
{
	m_Name = "LISTEN null";
}

CIPListener::~CIPListener(void)
{
}

void CIPListener::Listen(int Port)
{
	m_Name = "LISTEN "+itoa(Port);
	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests.
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_Socket == INVALID_SOCKET) 
		throw CHaException(CHaException::ErrCreateSocketError, "Error at socket(): %ld\n", WSAGetLastError());

	const int on = 1;
	setsockopt (m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ); 
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(Port);


	if (bind( m_Socket, (sockaddr*) &service, sizeof(service)) == SOCKET_ERROR)
		throw CHaException(CHaException::ErrCreateSocketError, "bind() failed.\n");

	//----------------------
	// Listen for incoming connection requests 
	// on the created socket
	if (listen( m_Socket, SOMAXCONN ) == SOCKET_ERROR)
		throw CHaException(CHaException::ErrCreateSocketError, "listen() failed.\n");

	m_Connected = true;
}

void CIPListener::OnSelect(CSupervisor *superviser)
{
	SOCKET socket = accept(getSocket(), NULL, NULL);
	
	CIPConnection *conn = new CIPConnection(socket, "LISTEN ");

	superviser->OnNewConnectionEx(this, conn);
}