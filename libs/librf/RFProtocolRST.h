#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolRST :
	public CRFProtocol
{
public:
	CRFProtocolRST();
	virtual ~CRFProtocolRST();

	virtual string getName() { return "RST"; };
	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string&); // Преобразование бит в данные
};

