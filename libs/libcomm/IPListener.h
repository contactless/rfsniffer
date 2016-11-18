#pragma once
#include "IPConnection.h"

class LIBCOMM_API CIPListener :
	public CIPConnection
{
	string m_Name;
public:
	CIPListener(void);
	virtual ~CIPListener(void);
	void Listen(int Port);
	virtual void OnSelect(CSupervisor *superviser);

};
