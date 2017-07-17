#pragma once
#include "RFProtocol.h"
#include "RFProtocolOregon.h"

class CRFProtocolOregonV3 :
    public CRFProtocolOregon
{
  public:
    CRFProtocolOregonV3();
    ~CRFProtocolOregonV3();

    virtual std::string getName()
    {
        return "Oregon";
    };
    virtual std::string DecodePacket(const std::string &);
};

