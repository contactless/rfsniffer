#include "stdafx.h"
#ifdef WIN32
//#include <process.h>
#endif
#include "thread.h"
#include "Exception.h"
#include "logging.h"

CThread::CThread()
{
	m_hThreadHandle = 0;
}

CThread::~CThread()
{
}

#ifdef WIN32
	DWORD WINAPI ThreadProc(void *param)
	{
		try
		{
			CThread* thread = (CThread*)param;
			thread->ThreadProc();
		}
		catch (CHaException ex)
		{
			CLog *log=CLog::GetLog("Main");
			if (log)
				log->Printf(0, "ThreadProc got unhandled exception, close thread. Error code %d, Message '%s'.\n", ex.GetCode(), ex.GetMessage().c_str());
			else
				printf("ThreadProc got unhandled exception, close thread. Error code %d, Message '%s'.\n", ex.GetCode(), ex.GetMessage().c_str());
		}
		return 0;
	}
#else
	void *ThreadProc(void *param)
	{
		{
			pthread_attr_t attr;
			void * stackaddr;
			int * plocal_var;
			size_t stacksize;

			//pthread_getattr_np(pthread_self(), &attr);
			//pthread_attr_getstack( &attr, &stackaddr, &stacksize );

			//CLog *log=CLog::GetLog("Main");
			//log->Printf(0, "stackaddr = %p, stacksize = %d\n", stackaddr, stacksize );
		}
		
		try
		{
			CThread* thread = (CThread*)param;
			thread->ThreadProc();
		}
		catch (CHaException ex)
		{
			CLog *log=CLog::GetLog("Main");
			if (log)
				log->Printf(0, "ThreadProc got unhandled exception, close thread. Error code %d, Message '%s'.\n", ex.GetCode(), ex.GetMessage().c_str());
			else
				printf("ThreadProc got unhandled exception, close thread. Error code %d, Message '%s'.\n", ex.GetCode(), ex.GetMessage().c_str());
		}
		return NULL;
	}
#endif

void CThread::Start()
{
#ifdef WIN32
	DWORD ThreadID;
	m_hThreadHandle = CreateThread(NULL, 0, ::ThreadProc, this, 0, &ThreadID);
#else
	pthread_attr_t tattr;
	pthread_t tid;
	int ret;

	size_t size = PTHREAD_STACK_MIN*4 + 0x4000;

	/* initialized with default attributes */
	ret = pthread_attr_init(&tattr);

	/* setting the size of the stack also */
	ret = pthread_attr_setstacksize(&tattr, size);

	/* only size specified in tattr*/
  	if (pthread_create(&m_hThreadHandle, &tattr, ::ThreadProc, this))
	{	
	}
#endif

}

bool CThread::Wait(DWORD timeOut)
{
#ifdef WIN32
	return WaitForSingleObject(m_hThreadHandle, timeOut) == WAIT_OBJECT_0;
#else
	pthread_join(m_hThreadHandle, NULL);
//	#error PLATFORM NOT SUPPORTED
#endif
	return false;
}

