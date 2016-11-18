#include "stdafx.h"
#include "SerialConnection.h"
#include "../libutils/Exception.h"
#include "../libutils/strutils.h"


#ifdef WIN32
#else
	#include<unistd.h>
	#include<termios.h>
#endif


CSerialConnection::CSerialConnection(void)
{
#ifdef WIN32
	m_hCom = INVALID_HANDLE_VALUE;
#else
	m_hCom = -1;
#endif

   m_nBits = 8;
   m_nStop = 1;
   m_nEvent = 'N';
   m_baurate = 115200;

}

CSerialConnection::~CSerialConnection(void)
{
}

void CSerialConnection::Connect(string ConnectString)
{
	m_ConnectString = ConnectString;
	vector<string> params;
	SplitString(ConnectString, ':', params);
	if (params.size()!=2)
		throw CHaException(CHaException::ErrConnectStringError, "Bad connect string: %s", ConnectString.c_str());

#ifdef WIN32
	DCB dcb;
	BOOL fSuccess;

	m_hCom = CreateFileW( s2ws(params[0]).c_str(),
							GENERIC_READ | GENERIC_WRITE,
							0,    // must be opened with exclusive-access
							NULL, // default security attributes
							OPEN_EXISTING, // must use OPEN_EXISTING
							0,    // not overlapped I/O
							NULL  // hTemplate must be NULL for comm devices
						);

	if (m_hCom == INVALID_HANDLE_VALUE) 
		throw CHaException(CHaException::ErrConnectError, "Cannot open: %s. Err %d", params[0].c_str(), GetLastError());

	// Build on the current configuration, and skip setting the size
	// of the input and output buffers with SetupComm.

 	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	fSuccess = GetCommState(m_hCom, &dcb);

	if (!fSuccess) 
		throw CHaException(CHaException::ErrConnectError, "Failed GetCommState: %s", params[0].c_str());

	// Fill in DCB: 57,600 bps, 8 data bits, no parity, and 1 stop bit.
	dcb.BaudRate = atoi(params[1].c_str());     // set the baud rate
	dcb.ByteSize = 8;             // data size, xmit, and rcv
	dcb.Parity = NOPARITY;        // no parity bit
	dcb.StopBits = ONESTOPBIT;    // one stop bit?

	if (!SetCommState(m_hCom, &dcb)) 
		throw CHaException(CHaException::ErrConnectError, "Failed SetCommState: %s, %d", params[0].c_str(), GetLastError());

	if (!SetCommMask(m_hCom, EV_ERR|EV_RXCHAR|EV_BREAK)) 
		throw CHaException(CHaException::ErrConnectError, "Failed SetCommMask: %s, %d", params[0].c_str(), GetLastError());

#else 
	m_hCom = open(params[0].c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

	if (m_hCom == -1)
		throw CHaException(CHaException::ErrConnectError, "Cannot open: %s", params[0].c_str());
	
    termios my_termios;
    tcgetattr(m_hCom, &my_termios);
    tcflush(m_hCom, TCIFLUSH);
    my_termios.c_cflag = B9600 | CS8 |CREAD | CLOCAL | HUPCL;
    cfsetospeed(&my_termios, B9600);
    tcsetattr(m_hCom, TCSANOW, &my_termios);
#endif
	SetTimeout(1);

}

int CSerialConnection::setOpt(void)
{
#ifdef WIN32
#else 
	struct termios newtio,oldtio;
	if  ( tcgetattr( m_hCom,&oldtio)  !=  0) { 
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( m_nBits )
	{
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch( m_nEvent )
	{
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E': 
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':  
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch( m_baurate)
	{
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;/*
	case 460800:
		cfsetispeed(&newtio, B460800);
		cfsetospeed(&newtio, B460800);
		break;*/
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if( m_nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( m_nStop == 2 )
	newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 0;//ÖØÒª
	newtio.c_cc[VMIN] = 100;//·µ»ØµÄ×îÐ¡Öµ  ÖØÒª
	tcflush(m_hCom,TCIFLUSH);
	if((tcsetattr(m_hCom,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
#endif
	return 0;
}


void CSerialConnection::Disconnect()
{
#ifdef WIN32
	if (m_hCom != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
	}
#else
	if (m_hCom != -1)
	{
		close(m_hCom);
		m_hCom = -1;
	}
#endif
}


void CSerialConnection::Send(const char *Buffer, size_t Size)
{
#ifdef WIN32
	DWORD b;
	if (!WriteFile(m_hCom, Buffer, Size, &b, NULL))
		throw CHaException(CHaException::ErrSendError, "Failed WriteFile");

	if (b!=Size)
		throw CHaException(CHaException::ErrSendError, "bytes!=Size");

#else
	int b = write(m_hCom, Buffer, Size);
	if (b<0)
		throw CHaException(CHaException::ErrSendError, "Failed write to serial. Error %d", errno);

	if (b!=Size)
		throw CHaException(CHaException::ErrSendError, "bytes!=Size");
#endif
}


void CSerialConnection::Recv(char *Buffer, size_t &Size, bool bWaitAll )
{
	size_t recieved = 0;
	do
	{
#ifdef WIN32
		DWORD b = 0;
		if (!ReadFile(m_hCom, Buffer, Size - recieved, &b, NULL))
			throw CHaException(CHaException::ErrRecvError, "Failed ReadFile");

#else
		if (m_Timeout>0)
		{
			fd_set fd_recv;
			FD_ZERO(&fd_recv);
			FD_SET(m_hCom, &fd_recv);
			timeval t;
			t.tv_sec=m_Timeout;
			t.tv_usec=0;
			int sel = select(m_hCom+1, &fd_recv, NULL, NULL, &t);
			if (sel==SOCKET_ERROR)
				throw CHaException(CHaException::ErrRecvError, "Error at select(): %ld", errno);
			else if (sel==0)
			{
				Size=0;
				return;
			}
		}

		int b = read(m_hCom, Buffer, Size);

		if (b==SOCKET_ERROR)
				throw CHaException(CHaException::ErrRecvError, "Error at recv(): %ld", errno);

#endif
		recieved+=b;
		Buffer+=b;
	}
	while (bWaitAll && recieved<Size);

	Size=recieved;
}


void CSerialConnection::SetTimeout(int Timeout)
{
#ifdef WIN32
	if (Timeout==m_Timeout)
		return;

	CConnection::SetTimeout(Timeout);

	COMMTIMEOUTS timeouts;
	if (!GetCommTimeouts(m_hCom, &timeouts)) 
		throw CHaException(CHaException::ErrConnectError, "Failed GetCommTimeouts: %d", GetLastError());
	
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = m_Timeout*1000;

	if (!SetCommTimeouts(m_hCom, &timeouts)) 
		throw CHaException(CHaException::ErrConnectError, "Failed GetCommTimeouts: %d", GetLastError());
#else
	m_Timeout = Timeout;
#endif
}
