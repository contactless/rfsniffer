#pragma once
#include "RFProtocol.h"
class CRFProtocolMotionSensor :
    public CRFProtocol
{
  public:
    CRFProtocolMotionSensor();
    ~CRFProtocolMotionSensor();

    virtual std::string getName()
    {
        return "MotionSensor";
    };


};

