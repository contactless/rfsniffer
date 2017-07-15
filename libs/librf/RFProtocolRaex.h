#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolRaex :
    public CRFProtocol
{
  public:
    CRFProtocolRaex();
    virtual ~CRFProtocolRaex();

    virtual std::string getName()
    {
        return "Raex";
    };
    virtual std::string DecodePacket(const std::string &);
    virtual std::string DecodeData(const std::string &); // Преобразование бит в данные

};

