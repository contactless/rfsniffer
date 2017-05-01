#pragma once
#include "RFProtocol.h"

class RFLIB_API CRFProtocolHS24Bit :
    public CRFProtocol
{
  public:
    CRFProtocolHS24Bit();
    virtual ~CRFProtocolHS24Bit();

    virtual string getName()
    {
        return "HS24Bits";
    };
    virtual string DecodePacket(const string &);
    virtual string DecodeData(const string &);

};

