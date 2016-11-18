#pragma once

#include "libcomm.h"
#include "../libutils/thread.h"
#include "../libutils/locks.h"
#include "IPConnection.h"

class LIBCOMM_API CSupervisorClient
{
public:
	CSupervisorClient(void);
	virtual ~CSupervisorClient(void);

	virtual void OnRecieve(CConnection* Conn)=0;
	virtual void OnNewConnection(CConnection* Conn){};
	virtual void OnDeleteConnection(CConnection* Conn){};
	virtual void OnIdle();
};
