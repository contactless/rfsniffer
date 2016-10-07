#ifndef __LOGGING_H
#define __LOGGING_H

#pragma warning (disable: 4251)

#include "libutils.h"
#include "locks.h"

class CLog;
typedef map<string, CLog*> LIBUTILS_API CLogsMap;

#ifdef USE_CONFIG
	class CConfigItem;
#endif

struct LogParam
{
	string FileName;
	bool LogTime;
	int ConsoleLevel;
	int FileLevel;
	LogParam();
#ifdef USE_CONFIG
	LogParam(CConfigItem node);
#endif
};

typedef map<string, LogParam> LIBUTILS_API CLogsParamMap;


class LIBUTILS_API CLog
{
	CLock lock;
	char* m_FileName;
	FILE *m_File;
	bool m_bTimeLog;
	int m_iConsoleLogLevel, m_iLogLevel;
	static CLogsMap m_Logs;
	static string m_BasePath;
	static bool m_DisableConsole;
#ifdef USE_CONFIG
	static CLogsParamMap m_LogsCfg;
#endif

public:
	CLog();
	~CLog();	
	void Open(const char *FileName);
	void Open(LogParam *Config);

#ifdef USE_CONFIG
	static void Init(CConfigItem *Config);
#endif

	static CLog* GetLog(string Name);
	static CLog* Default();
	static void CloseAll();
	static void DisableConsole(bool bDisable){m_DisableConsole = bDisable;};
	void Printf(int level, const char *Format, ...);
	void VPrintf(int level, const char *Format, va_list argptr);
	void PrintBufferEx(int level, char* Preffix, const char *Buffer, int BufferSize){PrintBufferEx(level, Preffix, (unsigned char*)Buffer, BufferSize);};
	void PrintBufferEx(int level, char* Preffix, const unsigned char *Buffer, int BufferSize);
	void PrintBuffer(int level, const char *Buffer, int BufferSize){PrintBuffer(level, (unsigned char*)Buffer, BufferSize);};
	void PrintBuffer(int level, const unsigned char *Buffer, int BufferSize){PrintBufferEx(level, NULL, Buffer, BufferSize);};
	
	void SetLogTime(bool bTimeLog = true){m_bTimeLog = bTimeLog;};
	void SetLogLevel(int level){m_iLogLevel = level;};
	void SetConsoleLogLevel(int level){m_iConsoleLogLevel = level;};

	bool isOpen();
};

#endif
