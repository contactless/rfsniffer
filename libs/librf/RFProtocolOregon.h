#pragma once
#include "RFProtocol.h"

class CRFProtocolOregon :
    public CRFProtocol
{
  public:
    CRFProtocolOregon();
    CRFProtocolOregon(range_array_type zeroLengths, range_array_type pulseLengths, int bits,
                      int minRepeat, string PacketDelimeter);
    ~CRFProtocolOregon();

    virtual string getName()
    {
        return "Oregon";
    };
    virtual string DecodePacket(const string &);
    virtual string DecodeData(const string &); // Преобразование бит в данные
    virtual bool needDump(const string &rawData);
};

