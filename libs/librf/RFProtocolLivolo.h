#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolLivolo :
    public CRFProtocol
{
  public:
    CRFProtocolLivolo();
    virtual ~CRFProtocolLivolo();

    virtual std::string getName()
    {
        return "Livolo";
    };
    virtual std::string DecodePacket(const std::string &);

};

