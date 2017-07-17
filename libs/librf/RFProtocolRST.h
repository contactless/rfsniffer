#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolRST :
    public CRFProtocol
{
  public:
    CRFProtocolRST();
    virtual ~CRFProtocolRST();

    virtual std::string getName()
    {
        return "RST";
    };
    virtual std::string DecodePacket(const std::string &);
    virtual std::string DecodeData(const std::string &); // Преобразование бит в данные
};

