#include "stdafx.h"

#include "locks.h"
#include "Exception.h"

#ifndef WIN32
	#include <sys/time.h>
#endif

CLock::CLock()
{
#ifdef WIN32
	InitializeCriticalSection(&cs);
#else
	pthread_mutexattr_init(&cs_attr);
	pthread_mutexattr_settype(&cs_attr, PTHREAD_MUTEX_RECURSIVE);
	if (pthread_mutex_init(&cs, &cs_attr))
	{
		throw CHaException(CHaException::ErrSyncFailed, "Cannot initialize mutex");
	}
#endif
}


CLock::~CLock()
{
#ifdef WIN32
	DeleteCriticalSection(&cs);
#else
	if (pthread_mutex_destroy(&cs))
	{
		throw CHaException(CHaException::ErrSyncFailed, "Cannot destroy mutex");
	}
	pthread_mutexattr_destroy(&cs_attr);
	
#endif
}

void CLock::Lock()
{
#ifdef WIN32
	EnterCriticalSection(&cs);
#else
	if(pthread_mutex_lock(&cs))
	{
		throw CHaException(CHaException::ErrSyncFailed, "Cannot lock mutex");	
	}
#endif
}

void CLock::Unlock()
{
#ifdef WIN32
	LeaveCriticalSection(&cs);
#else
	if (pthread_mutex_unlock(&cs))
	{
		throw CHaException(CHaException::ErrSyncFailed, "Cannot unlock mutex");	
	}
#endif
}

CLockHelper::CLockHelper(CLock* lock)
:m_lock(lock)
{
	m_lock->Lock();
}

CLockHelper::~CLockHelper()
{
	m_lock->Unlock();
}


CEvent::CEvent(bool InitState)
{
#ifdef WIN32
	m_Event = CreateEvent(NULL, true, InitState, NULL);
#else
   m_bSignaled = false;
	
	if (pthread_cond_init(&m_Cond, NULL))
	{
		throw CHaException(CHaException::ErrSyncFailed, "Cannot init cond");	
	}
	
	if(pthread_mutex_init(&m_Mutex, NULL))
	{
		throw CHaException(CHaException::ErrSyncFailed, "Cannot init mutex");	
	}
#endif
}

CEvent::~CEvent()
{
#ifdef WIN32
	CloseHandle(m_Event);
#else
	if (pthread_cond_destroy(&m_Cond))
	{
		throw CHaException(CHaException::ErrSyncFailed, "Cannot destroy cond");	
	}
	
	if(pthread_mutex_destroy(&m_Mutex))
	{
		throw CHaException(CHaException::ErrSyncFailed, "Cannot destroy mutex");	
	}
#endif
}

bool CEvent::Wait(DWORD ms)
{
#ifdef WIN32
	return WaitForSingleObject(m_Event, ms)==WAIT_OBJECT_0;
#else
	if (ms == WAIT_INFINITE)
	{
		if (pthread_mutex_lock(&m_Mutex))
		{
			throw CHaException(CHaException::ErrSyncFailed, "Cannot lock mutex");	
		}
		
		while (!m_bSignaled)
		{
			pthread_cond_wait(&m_Cond, &m_Mutex);
		}
		
		if (pthread_mutex_unlock(&m_Mutex))
		{
			throw CHaException(CHaException::ErrSyncFailed, "Cannot unlock mutex");	
		}
	}
	else
	{
		struct timeval now;
        struct timespec timeout;
        int retcode;

		if(pthread_mutex_lock(&m_Mutex))
		{
			throw CHaException(CHaException::ErrSyncFailed, "Cannot lock mutex");	
		}

        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec + 5;
        timeout.tv_nsec = now.tv_usec * 1000;
        retcode = 0;

		while (!m_bSignaled && retcode != ETIMEDOUT)
		{
			retcode = pthread_cond_timedwait(&m_Cond, &m_Mutex, &timeout);
		}

        if (retcode == ETIMEDOUT) 
		{
            /* timeout occurred */
        } 
		else 
		{
			/* OK */
		}

		if (pthread_mutex_unlock(&m_Mutex))
		{
			throw CHaException(CHaException::ErrSyncFailed, "Cannot unlock mutex");	
		}
	}

	return m_bSignaled;
#endif
}

bool CEvent::Signal()
{
#ifdef WIN32
	return SetEvent(m_Event)!=0;	
#else
	pthread_mutex_lock(&m_Mutex);
	m_bSignaled = true;
	pthread_cond_broadcast(&m_Cond);
	pthread_mutex_unlock(&m_Mutex);
	return true;
#endif
}

bool CEvent::Reset()
{
#ifdef WIN32
	return ResetEvent(m_Event)!=0;	
#else
	pthread_mutex_lock(&m_Mutex);
	m_bSignaled = false;
	pthread_mutex_unlock(&m_Mutex);
	return true;
#endif
}
