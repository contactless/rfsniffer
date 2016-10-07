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
	#include<string.h>
	#define snprintf _snprintf
#elif defined(WIN32)
	#include "targetver.h"

	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
	#include <windows.h>

	#include <winsock2.h>
	//#define snprintf sprintf_s
#else
	#include <pthread.h>
	#include <limits.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <errno.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
//#include <errno.h>
//#include <fcntl.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <map>
#include <queue>
using namespace std;

#include "haconfig.h"
