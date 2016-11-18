#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolRubitek :
	public CRFProtocol
{
public:
	CRFProtocolRubitek();
	~CRFProtocolRubitek();

	virtual string getName() { return "Rubitek"; };
	virtual string DecodePacket(const string&);
};

