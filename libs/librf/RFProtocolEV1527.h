#pragma once
#include "RFProtocol.h"

class RFLIB_API CRFProtocolEV1527 : public CRFProtocol
{
public:
	CRFProtocolEV1527();
	~CRFProtocolEV1527();

	virtual std::string getName() { return "EV1527"; };
	virtual std::string DecodePacket(const std::string&);
	virtual std::string DecodeData(const std::string& bits);

};

