#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolX10 :
    public CRFProtocol
{
  public:
    CRFProtocolX10();
    virtual ~CRFProtocolX10();

    virtual std::string getName()
    {
        return "X10";
    };
    virtual std::string DecodePacket(const std::string &);
    virtual std::string DecodeData(const std::string &); // Преобразование бит в данные

    char parseHouseCode(uint8_t data);


};

