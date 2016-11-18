// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#if defined(_WIN32_WCE)
	// NOTE - this value is not strongly correlated to the Windows CE OS version being targeted
	#define WINVER _WIN32_WCE

	#include <ceconfig.h>
	#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
	#define SHELL_AYGSHELL
	#endif

	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
	#include <windows.h>
	#include <winsock2.h>
	#define snprintf _snprintf

/*
 * Internal send/recv flags
 */
#define MSG_OOB       0x1   /* process out-of-band data */
#define MSG_PEEK      0x2   /* peek at incoming message */
#define MSG_DONTROUTE 0x4   /* send without using routing tables */
#define MSG_EOR       0x8   /* data completes record */
#define MSG_TRUNC     0x10  /* data discarded before delivery */
#define MSG_CTRUNC    0x20  /* control data lost before delivery */
#define MSG_WAITALL   0x40  /* wait for full request or error */
#define MSG_DONTWAIT  0x80  /* this message should be nonblocking */

#elif defined(WIN32)
	#include "targetver.h"

	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
	#include <windows.h>

	#include <winsock2.h>
	#include <ws2tcpip.h>
//#	define snprintf sprintf_s
#else
	#include <pthread.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <errno.h>
	#include <netdb.h>
	#include <stdio.h>
//	#include <termio.h>
	#include <fcntl.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>



//	#include <error.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
//#include <fcntl.h>
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "../libutils/Exception.h"
#include "../libutils/logging.h"

