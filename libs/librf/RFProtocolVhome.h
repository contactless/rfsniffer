#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolVhome :
	public CRFProtocol
{
public:
	CRFProtocolVhome();
	~CRFProtocolVhome();

	virtual std::string getName() { return "VHome"; };
	virtual std::string DecodePacket(const std::string&);
	virtual std::string DecodeData(const std::string& bits);

};

