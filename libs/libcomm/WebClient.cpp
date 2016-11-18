#include "stdafx.h"
#include "WebClient.h"
#include "../libutils/strutils.h"


CWebClient::CWebClient(CSupervisor *Supervisor, int Timeout)
	:m_Supervisor(Supervisor), m_ContentType(None), m_RequestType(Get), m_HaveResponse(false), m_ResponseCode(0), m_Connection(NULL), m_Timeout(Timeout)
{
	m_RequestTime = 0;
}


CWebClient::~CWebClient()
{
	if (m_Connection)
		m_Connection->Disconnect();

	m_Supervisor->RemoveClient(this, true);
}


void CWebClient::Send(string url, string host, string Content)
{
	if (Content.length() > 0)
	{
		SetRequestType(Post);
		SetContentType(Form);
	}
	string Method;

	string req;
	req = GetRequestType()+" "+url+" HTTP/1.1\n";
	req += "Host: "+host+"\n";
	req += GetHeaders();
	req += "Content-Length: " + itoa(Content.length()) + "\n\n";
	req += Content;

	m_Headers.clear();
	m_HaveResponse = false;
	m_Response.clear();

	m_Connection = new CIPConnection();
	m_Connection->SetAutoDelete(true);
	m_Connection->Connect(host + ":80");
	m_Supervisor->AddConnection(this, m_Connection);
	m_Connection->Send(req.c_str(), req.length());
	time(&m_RequestTime);

}


string CWebClient::GetRequestType()
{
	switch (m_RequestType)
	{
	case Get:
		return "GET";
	case Post:
		return "POST";
	}

	return "";
}
string CWebClient::GetContentType()
{
	switch (m_ContentType)
	{
	case None:
		return "";
	case Form:
		return "application/x-www-form-urlencoded";
	}

	return "";
}

string CWebClient::GetHeaders()
{
	string header = "Connection: close\n";

	if (m_ContentType!=None)
		header += "Content-Type: "+GetContentType()+"\n";

	return header;
}

void CWebClient::OnRecieve(CConnection* Conn)
{
	char Buffer[4096];
	size_t size = sizeof(Buffer) - 1;


	try
	{
		Conn->Recv(Buffer, size, false);
		Buffer[size] = 0;
	}
	catch (CHaException ex)
	{
		ex.Dump(CLog::GetLog("Main"));
		Conn->Disconnect();
		m_Connection = NULL;
		m_HaveResponse = true;
		m_Response = "<FAIL>:" + ex.GetMessage();
		return;
	}

	bool bFirstLine = true;
	char *start_line = Buffer;
	while(true)
	{
		char *end_line = strchr(start_line, '\r');

		if (!end_line)
			break;

		if (end_line == start_line) // empty line - start of body
		{
			while (*end_line == '\n' || *end_line == '\r')
				end_line++;

			m_Response = end_line;
			m_HaveResponse = true;
			Conn->Disconnect();
			m_Connection = NULL;
			return;
		}

		string line(start_line, end_line - start_line);
		start_line = end_line + 1;
		if (start_line[0] == '\n')
			start_line++;

		if (bFirstLine)
		{
			string_vector v;
			SplitString(line, ' ', v);
			if (v.size() < 2)
			{ 
				m_ResponseCode = -1;
			}
			else
			{
				m_ResponseCode = atoi(v[1].c_str());
			}

			bFirstLine = false;
		}
		else
		{
			string_vector v;
			SplitString(line, ':', v);
			if (v.size() != 2)
				continue; // bad line?

			string key = v[0];
			string value = v[1];
			if (value.length()>0 && value[0] == ' ')
				value = value.substr(1);

			m_Headers[key] = value;
		}
	}
}

void CWebClient::ClearResponse(){
	m_HaveResponse = false;
	m_Response.clear();
	m_Headers.clear();
	m_ResponseCode = 0;
	m_ContentType = None;
}


bool CWebClient::isTimeout(){
	return m_RequestTime>0 && m_RequestTime+m_Timeout<time(NULL);
}
