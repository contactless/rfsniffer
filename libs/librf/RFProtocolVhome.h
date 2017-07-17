#pragma once
#include "RFProtocolEV1527.h"

class RFLIB_API CRFProtocolVhome :
	public CRFProtocolEV1527
{
public:
	CRFProtocolVhome();
	~CRFProtocolVhome();

	virtual std::string getName() { return "VHome"; };
	virtual std::string DecodeData(const std::string& bits);

};

