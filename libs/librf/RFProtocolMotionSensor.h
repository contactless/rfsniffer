#pragma once
#include "RFProtocol.h"
class CRFProtocolMotionSensor :
	public CRFProtocol
{
public:
	CRFProtocolMotionSensor();
	~CRFProtocolMotionSensor();

	virtual string getName() { return "MotionSensor"; };


};

