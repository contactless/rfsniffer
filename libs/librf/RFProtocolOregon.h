#pragma once
#include "RFProtocol.h"
class CRFProtocolOregon :
	public CRFProtocol
{
public:
	CRFProtocolOregon();
	~CRFProtocolOregon();

	virtual string getName() { return "Oregon"; };
	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string&); // Преобразование бит в данные
	virtual bool needDump(const string &rawData);
};

