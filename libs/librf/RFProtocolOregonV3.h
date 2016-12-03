#pragma once
#include "RFProtocol.h"
#include "RFProtocolOregon.h"

class CRFProtocolOregonV3 :
    public CRFProtocolOregon
{
  public:
    CRFProtocolOregonV3();
    ~CRFProtocolOregonV3();

    virtual string getName()
    {
        return "Oregon";
    };
    virtual string DecodePacket(const string &);
};

