#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolHS24Bits :
    public CRFProtocol
{
  public:
    CRFProtocolHS24Bits();
    virtual ~CRFProtocolHS24Bits();

    virtual string getName()
    {
        return "HS24Bits";
    };
    virtual string DecodePacket(const string &);
    virtual string DecodeData(const string &);

};

