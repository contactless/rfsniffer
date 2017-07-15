#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolRubitek :
    public CRFProtocol
{
  public:
    CRFProtocolRubitek();
    ~CRFProtocolRubitek();

    virtual std::string getName()
    {
        return "Rubitek";
    };
    virtual std::string DecodePacket(const std::string &);
};

