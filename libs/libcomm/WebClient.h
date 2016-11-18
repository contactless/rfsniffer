#pragma once

#include "../libutils/strutils.h"
#include "IPSupervisor.h"
#include "SupervisorClient.h"

class LIBCOMM_API CWebClient
	:CSupervisorClient
{
public:
	enum ContentType
	{
		None = 0,
		Form
	};

	enum RequestType
	{
		Get,
		Post
	};

	string GetRequestType();
	string GetContentType();
	string GetHeaders();

private:
	CSupervisor *m_Supervisor;
	virtual void OnRecieve(CConnection* Conn);
	ContentType m_ContentType;
	RequestType m_RequestType;
	string m_Response;
	bool m_HaveResponse;
	string_map m_Headers;
	CIPConnection *m_Connection;
	int m_ResponseCode;
	time_t m_RequestTime;
	int m_Timeout;


public:
	CWebClient(CSupervisor *Supervisor, int Timeout=10);
	~CWebClient();

	void SetSupervisor(CSupervisor *Supervisor){ m_Supervisor = Supervisor; };
	void Send(string url, string host, string Content = "");
	void SetContentType(ContentType type){ m_ContentType = type; };
	void SetRequestType(RequestType type){ m_RequestType = type; };

	bool HaveResponse(){ return m_HaveResponse; };
	int GetResponseCode(){ return m_ResponseCode; };
	string GetResponse(){ return m_Response; };
	void ClearResponse();
	bool isTimeout();
};

