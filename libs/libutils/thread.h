#ifndef __THREAD_H
#define __THREAD_H

#include "libutils.h"
#include "locks.h"

class LIBUTILS_API CThread
{
#ifdef WIN32
	HANDLE m_hThreadHandle;
#else
	pthread_t m_hThreadHandle;
#endif

public:
	CThread();
	virtual ~CThread();
	void Start();
	bool Wait(DWORD timeOut);
	virtual int ThreadProc() = 0;
};

#ifndef WIN32
	inline void Sleep(long ms){usleep((ms)*1000);};
#endif

#endif
