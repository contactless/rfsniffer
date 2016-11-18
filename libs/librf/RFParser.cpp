#include "stdafx.h"
#include <fcntl.h>

#include "RFParser.h"
#include "RFProtocolLivolo.h"
#include "RFProtocolX10.h"
#include "RFProtocolRST.h"
#include "RFProtocolRaex.h"
#include "RFProtocolOregon.h"
#include "RFProtocolNooLite.h"
#include "RFProtocolRubitek.h"
#include "RFProtocolMotionSensor.h"
#include "RFAnalyzer.h"

CRFParser::CRFParser(CLog *log, string SavePath)
	:b_RunAnalyzer(false), m_Analyzer(NULL), m_Log(log), m_SavePath(SavePath),
	m_maxPause(0)
{
}


CRFParser::~CRFParser()
{
	for_each(CRFProtocolList, m_Protocols, i)
	{
		delete *i;
	}

	delete m_Analyzer;
}

void CRFParser::AddProtocol(string protocol)
{
	if (protocol == "X10")
		AddProtocol(new CRFProtocolX10());
	else if (protocol == "RST")
		AddProtocol(new CRFProtocolRST());
	else if (protocol == "Raex")
		AddProtocol(new CRFProtocolRaex());
	else if (protocol == "Livolo")
		AddProtocol(new CRFProtocolLivolo());
	else if (protocol == "Oregon")
		AddProtocol(new CRFProtocolOregon());
	else if (protocol == "nooLite")
		AddProtocol(new CRFProtocolNooLite());
	else if (protocol == "Rubitek")
		AddProtocol(new CRFProtocolRubitek());
	else if (protocol == "MotionSensor")
		AddProtocol(new CRFProtocolMotionSensor());
	else if (protocol == "All")
	{
		AddProtocol(new CRFProtocolX10());
		AddProtocol(new CRFProtocolRST());
		AddProtocol(new CRFProtocolRaex());
		AddProtocol(new CRFProtocolLivolo());
		AddProtocol(new CRFProtocolOregon());
		AddProtocol(new CRFProtocolNooLite());
		AddProtocol(new CRFProtocolRubitek());
		//AddProtocol(new CRFProtocolMotionSensor());
	}
	else
		throw CHaException(CHaException::ErrBadParam, protocol);

	setMinMax();
}

void CRFParser::AddProtocol(CRFProtocol* p)
{
	p->setLog(m_Log);
	m_Protocols.push_back(p);
//	setMinMax();
}

// 1,2,3,1,100,1,1,2,3

string CRFParser::Parse(base_type** data, size_t* length)
{
	if (m_maxPause==0)
		setMinMax();

	base_type* saveStart = *data;
	base_type splitDelay = m_maxPause*10/8;
	base_type splitPulse = m_minPulse/2;

	for(base_type* ptr=*data; ptr-*data<*length; ptr++)
	{
		bool isPulse = CRFProtocol::isPulse(*ptr);
		base_type len = CRFProtocol::getLengh(*ptr);
		if ((!isPulse && len>splitDelay) || (isPulse && len<splitPulse))
		{
			size_t packetLen = ptr-*data;
			if (packetLen>50)
				m_Log->Printf(4, "Parse part of packet from %ld size %ld splitted by %ld", *data-saveStart, packetLen, CRFProtocol::getLengh(*ptr));

			string res = Parse(*data, packetLen);
			
			*data += packetLen+1;
			*length -= packetLen+1;
			
			if (res.length())
				return res;
		}
	}

	if (*length>MIN_PACKET_LEN)
	{
		string res = Parse(*data, *length);
		if (res.length())
		{
			*data += (*length);
			*length = 0;
			return res;
		}
	}

	return "";
}

string CRFParser::Parse(base_type* data, size_t len)
{
	if (len < MIN_PACKET_LEN)
		return "";

	// Пытаемся декодировать пакет каждым декодером по очереди
	for_each(CRFProtocolList, m_Protocols, i)
	{
		string retval = (*i)->Parse(data, len);
		if (retval.length())
			return retval;  // В случае успеха возвращаем результат
	}

	// В случае неуспеха пытаемся применить анализатор
	if (b_RunAnalyzer)
	{
		if (!m_Analyzer)
			m_Analyzer = new CRFAnalyzer(m_Log);

		m_Analyzer->Analyze(data, len);
	}

	// Если указан путь для сохранения - пишем в файл пакет, который не смогли декодировать
	if (m_SavePath.length())
	{ 
		for_each(CRFProtocolList, m_Protocols, i)
		{
			if ((*i)->needDumpPacket())
			{
				m_Log->Printf(3, "Dump packet for %s", (*i)->getName().c_str());
				SaveFile(data, len);
				return "";
			}
		}
	}

	return "";
}

void CRFParser::EnableAnalyzer()
{ 
	b_RunAnalyzer = true;
}

void CRFParser::SaveFile(base_type* data, size_t size)
{
#ifndef WIN32
	if (m_SavePath.length())
	{
		time_t Time = time(NULL);
		char DateStr[100], FileName[1024];
		strftime(DateStr, sizeof(DateStr), "%d%m-%H%M%S", localtime(&Time));
		snprintf(FileName, sizeof(FileName),  "%s/capture-%s.rcf", m_SavePath.c_str(), DateStr);
		m_Log->Printf(3, "Write to file %s %ld signals\n", FileName, size);
		int of = open(FileName, O_WRONLY | O_CREAT, S_IRUSR|S_IRGRP);
		
		if (of == -1) {
			m_Log->Printf(3, "error opening %s\n", FileName);
			return;
		};

		write(of, data, sizeof(data[0])*size);
		close(of);
	}
#endif
}

void CRFParser::SetSavePath(string SavePath)
{
	m_SavePath = SavePath;
}


void CRFParser::setMinMax()
{
	bool first = true;
	for_each(CRFProtocolList, m_Protocols, i)
	{
		if (first)
		{
			(*i)->getMinMax(&m_minPause, &m_maxPause, &m_minPulse, &m_maxPulse);
			first = false;
		}
		else
		{
			base_type minPause, maxPause, minPulse, maxPulse;
			(*i)->getMinMax(&minPause, &maxPause, &minPulse, &maxPulse);
			m_minPause = min(m_minPause, minPause);
			m_maxPause = max(m_maxPause, maxPause);
			m_minPulse = min(m_minPulse, minPulse);
			m_maxPulse = max(m_maxPulse, maxPulse);
		}
	}	
	m_Log->Printf(3, "CRFProtocol decoders use pauses %ld-%ld pulses %ld-%ld", m_minPause, m_maxPause, m_minPulse, m_maxPulse);
}
