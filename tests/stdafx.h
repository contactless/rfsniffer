// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#ifdef WIN32
	#include "targetver.h"
	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
	#include <windows.h>
	#include <winsock2.h>
	#include <io.h>
//	#define snprintf sprintf_s
#else
	#include <pthread.h>
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
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <queue>
#include <map>
using namespace std;

#include "../libs/libutils/Exception.h"
#include "../libs/libutils/logging.h"

