#pragma once
#include "RFProtocol.h"

class CRFProtocolOregon :
    public CRFProtocol
{
  public:
    CRFProtocolOregon();
    CRFProtocolOregon(range_array_type zeroLengths, range_array_type pulseLengths, int bits,
                      int minRepeat, std::string PacketDelimeter);
    ~CRFProtocolOregon();

    virtual std::string getName()
    {
        return "Oregon";
    };
    virtual std::string DecodePacket(const std::string &);
    virtual std::string DecodeData(const std::string &); // Преобразование бит в данные
    virtual bool needDump(const std::string &rawData);
};

