#include "stdafx.h"

#include "logging.h"
#include "strutils.h"
#ifdef USE_CONFIG
	#include "ConfigItem.h"
#endif
#include "Exception.h"

LogParam::LogParam()
{
	FileName = "";
	LogTime = true;
	ConsoleLevel = -1;
	FileLevel = 0;
}

#ifdef USE_CONFIG
LogParam::LogParam(CConfigItem node)
{
	FileName = node.getStr("FileName");
	LogTime = (node.getStr("LogTime", false, "yes")=="yes")?true:false;
	ConsoleLevel = node.getInt("ConsoleLevel", false, -1);
	FileLevel = node.getInt("FileLevel", false, 0);
}
#endif


CLogsMap CLog::m_Logs;
#ifdef USE_CONFIG
	CLogsParamMap CLog::m_LogsCfg;
#endif
string CLog::m_BasePath;
bool CLog::m_DisableConsole=false;

CLog* CLog::Default()
{
	return GetLog("Main");
}


CLog* CLog::GetLog(string Name)
{
	CLog *log  = m_Logs[Name];

	if (log)
		return log;
#ifdef USE_CONFIG
#else
#endif

	m_Logs[Name] = log  = new CLog();

	if (Name=="nul")
		return log;

#ifdef USE_CONFIG
	if (m_LogsCfg.size()==0)
#endif
	{
		#if defined (_WIN32_WCE)
				WCHAR Buffer[4096];
				GetModuleFileNameW(NULL, Buffer, sizeof(Buffer));
				m_BasePath = ws2s(Buffer);
				int pos = m_BasePath.find_last_of(FOLDER_DELIMETER);
				if (pos>0)
					m_BasePath  = m_BasePath.substr(0, pos+1);
		#elif defined(WIN32) 
				char Buffer[4096];
				GetModuleFileName(NULL, Buffer, sizeof(Buffer));
				m_BasePath = Buffer;
				int pos = m_BasePath.find_last_of("/\\");
				if (pos>0)
					m_BasePath  = m_BasePath.substr(0, pos+1);
		#else
				m_BasePath = ".";
//				m_BasePath = "/run/";
		#endif

		log->Open((m_BasePath+Name+".log").c_str());
	}
#ifdef USE_CONFIG
	else
	{
		LogParam node = m_LogsCfg[Name];

		if (node.FileName.length())
			log->Open(&node);
	}
#endif

	return log;
}

void CLog::CloseAll()
{
	for_each(CLogsMap, m_Logs, i)
	{
		delete i->second;
	}

	m_Logs.clear();
#ifdef USE_CONFIG
	m_LogsCfg.clear();
#endif

}


#ifdef USE_CONFIG
void CLog::Init(CConfigItem *Config)
{
	CConfigItemList nodes;
	Config->getList("Log", nodes);
	
	for(CConfigItemList::iterator i=nodes.begin();i!=nodes.end();i++)
	{
		m_LogsCfg[(*i)->getStr("Name")] = LogParam(**i);
	}
}
#endif

CLog::CLog()
{
	m_File = NULL;
	m_FileName = NULL;
	m_bTimeLog = false;
	m_iConsoleLogLevel = m_iLogLevel = -1;
}

CLog::~CLog()
{
	if (m_File)
		fclose(m_File);

	if (m_FileName)
		delete m_FileName;
}

#if defined(WIN32)
#else
uint64_t gettid() {
    pthread_t ptid = pthread_self();
    uint64_t threadId = 0;
    memcpy(&threadId, &ptid, std::min(sizeof(threadId), sizeof(ptid)));
    return threadId;
}
#endif


void CLog::Printf(int level, const char *Format, ...)
{
	va_list marker;
	va_start (marker,Format);
	VPrintf(level, Format, marker);
	va_end (marker);
}

void CLog::VPrintf(int level, const char *Format, va_list marker)
{
	va_list save_marker;
	va_copy(save_marker, marker);

	if (m_iConsoleLogLevel<level && m_iLogLevel<level)
		return;

	LOCK(lock);

	if (m_iLogLevel>=level)
	{
		if (!m_File)
		{
			if (m_FileName)
			{
#if defined(WIN32) && !defined (_WIN32_WCE)
				m_File = _fsopen(m_FileName, "a+", _SH_DENYWR);
#else
				m_File = fopen(m_FileName, "a+");
#endif
				if (!m_File)
					return;
			}
		}

		if (!m_File)
			return;
	}

	if (m_bTimeLog)
	{  
		char Buffer[255];

		#if defined(WIN32)
			SYSTEMTIME t;
			GetLocalTime(&t);
			snprintf(Buffer,sizeof(Buffer), "%02u/%02u %02u:%02u:%02u.%03lu [%03d] ",t.wDay,t.wMonth,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds, level);
		#else
			long Time=time(NULL);
			strftime(Buffer,sizeof(Buffer),"%d/%m %H:%M:%S ",localtime(&Time));
			sprintf(Buffer+strlen(Buffer), "[%d] ",  getpid()/*, gettid()*/);
		#endif

		if (m_iLogLevel>=level)
			fwrite(Buffer, strlen(Buffer), 1, m_File);

		if (m_iConsoleLogLevel>=level && !m_DisableConsole)
			printf("%s", Buffer);
	}

	if (m_iConsoleLogLevel>=level  && !m_DisableConsole)
	{
		vprintf(Format, marker);

		if (Format[strlen(Format)-1]!='\n')
			printf("\n");
	}

	if (m_iLogLevel>=level)
	{
		vfprintf(m_File, Format, save_marker);

		if (Format[strlen(Format)-1]!='\n')
			fprintf(m_File, "\n");

		fflush(m_File);
	}
}


void CLog::PrintBufferEx(int level, char *Preffix, const unsigned char *Buffer, int BufferSize)
{
	if (m_iConsoleLogLevel<level && m_iLogLevel<level)
		return;

	LOCK(lock);

	char HexBuffer[200], CharBuffer[200];
	char *HexBufferPtr = HexBuffer, *CharBufferPtr = CharBuffer;
	HexBuffer[0] = CharBuffer[0]=0;
	int i;

	for (i=0;i<BufferSize;i++)
	{
		HexBufferPtr+=snprintf(HexBufferPtr, sizeof(HexBuffer) - (HexBufferPtr-HexBuffer), "%02x ", Buffer[i]);

		if (Buffer[i]>=' ')
			CharBufferPtr+=snprintf(CharBufferPtr, sizeof(CharBuffer) - (CharBufferPtr-CharBuffer), "%c", Buffer[i]);
		else
			CharBufferPtr+=snprintf(CharBufferPtr, sizeof(CharBuffer) - (CharBufferPtr-CharBuffer), "%c", '.');

		if ((i%16)==15)
		{
			if (Preffix)
				Printf(level, "%s %s: %s", Preffix, HexBuffer, CharBuffer);
			else
				Printf(level, "%s: %s", HexBuffer, CharBuffer);

			HexBufferPtr = HexBuffer;
			CharBufferPtr = CharBuffer;
		}
	}

	if (BufferSize%16)
	{
		for (i=0;i<16-(BufferSize%16);i++)
			HexBufferPtr+=snprintf(HexBufferPtr, sizeof(HexBuffer) - (HexBufferPtr-HexBuffer), "   ");

		if (Preffix)
			Printf(level, "%s %s: %s", Preffix, HexBuffer, CharBuffer);
		else
			Printf(level, "%s: %s", HexBuffer, CharBuffer);
	}
}


void CLog::Open(LogParam *Config)
{
	if (m_BasePath.length()==0)
	{
#if defined (_WIN32_WCE)
		WCHAR Buffer[4096];
		GetModuleFileNameW(NULL, Buffer, sizeof(Buffer));
		m_BasePath = ws2s(Buffer);
		int pos = m_BasePath.find_last_of(FOLDER_DELIMETER);
		if (pos>0)
			m_BasePath  = m_BasePath.substr(0, pos);
#elif defined(WIN32) 
		char Buffer[4096];
		GetModuleFileName(NULL, Buffer, sizeof(Buffer));
		m_BasePath = Buffer;
		int pos = m_BasePath.find_last_of(FOLDER_DELIMETER);
		if (pos>0)
			m_BasePath  = m_BasePath.substr(0, pos);
#endif
	}

	if (!Config)
	{
		throw CHaException(CHaException::ErrLogInitError, "Invalid log config");
	}

	if (m_File)
	{
		throw CHaException(CHaException::ErrLogInitError, "Log already open");
	}

	string FileName = Config->FileName;
	if (FileName.length()==0)
	{
		throw CHaException(CHaException::ErrLogInitError, "Log file name not set");
	}

	if (GetPath(FileName).length()==0)
		FileName = m_BasePath + string(FOLDER_DELIMETER_STR) + FileName;

	m_FileName = strnew(FileName.c_str());

	m_bTimeLog = Config->LogTime;
	m_iConsoleLogLevel = Config->ConsoleLevel;
	m_iLogLevel = Config->FileLevel;

	printf("Open log file %s\n", m_FileName);
	Printf(m_iLogLevel, "************************************** started **********************************************\n");
}

void CLog::Open(const char *FileName)
{
	m_FileName = strnew(FileName);
	m_bTimeLog = true;
	m_iConsoleLogLevel = m_iLogLevel = 100;
}

bool CLog::isOpen()
{
	return this==NULL?false:m_FileName!=NULL;
}

