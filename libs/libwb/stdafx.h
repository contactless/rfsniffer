// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#	include "targetver.h"
// Windows Header Files:
#	include <windows.h>
#	include <winsock2.h>
#else
#	include <pthread.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <map>
#include <queue>
using namespace std;

#include "../libutils/haconfig.h"
#include "../libutils/Exception.h"
#include "../libutils/logging.h"
#include "../libutils/strutils.h"


