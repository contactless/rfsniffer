#pragma once
#include "RFProtocol.h"

class RFLIB_API CRFProtocolHS24Bit :
    public CRFProtocol
{
  public:
    CRFProtocolHS24Bit();
    virtual ~CRFProtocolHS24Bit();

    virtual std::string getName()
    {
        return "HS24Bits";
    };
    virtual std::string DecodePacket(const std::string &);
    virtual std::string DecodeData(const std::string &);

};

