#pragma once
#include "RFProtocol.h"
#include <map>
#include <ostream>


class RFLIB_API CRFProtocolNooLite :
    public CRFProtocol
{
    std::map<uint16_t, bool> m_lastFlip;
    static const int fmt2length[];
  public:
    enum nooLiteCommandType {
        nlcmd_off,              //0 – выключить нагрузку
        nlcmd_slowdown,         //1 – запускает плавное понижение яркости
        nlcmd_on,               //2 – включить нагрузку
        nlcmd_slowup,           //3 – запускает плавное повышение яркости
        nlcmd_switch,           //4 – включает или выключает нагрузку
        nlcmd_slowswitch,       //5 – запускает плавное изменение яркости в обратном
        nlcmd_shadow_level,     //6 – установить заданную в «Данные к команде_0» яркость
        nlcmd_callscene,        //7 – вызвать записанный сценарий
        nlcmd_recordscene,      //8 – записать сценарий
        nlcmd_unbind,           //9 – запускает процедуру стирания адреса управляющего устройства из памяти исполнительного
        nlcmd_slowstop,         //10 – остановить регулировку
        nlcmd_bind = 15,        //15 – сообщает, что устройство хочет записать свой адрес в память
        nlcmd_slowcolor,        //16 – включение плавного перебора цвета
        nlcmd_switchcolor,      //17 – переключение цвета
        nlcmd_switchmode,       //18 – переключение режима работы
        nlcmd_switchspeed,      //19 – переключение скорости эффекта для режима работы
        nlcmd_battery_low,      //20 – информирует о разряде батареи в устройстве
        nlcmd_temp_humi,        //21 – передача информации о текущей температуре и
        nlcmd_test_result,      //22
        nlcmd_shadow_callscene, //23
        nlcmd_shadow_set_bright,//24
        nlcmd_temporary_on,     //25
        nlcmd_modes,            //26

        //     влажности (Информация о температуре и влажности содержится в
        //     поле «Данные к команде_x».)

        nlcmd_error = -1
    };

    struct CPacket {
        bool correct, flip;
        uint8_t cmd;
        uint16_t addr;
        uint8_t res;
        uint8_t fmt;
        uint8_t d[4];
        uint8_t crc;

        CPacket(bool correct = true);

        friend std::ostream &operator<<(std::ostream &out, const CPacket &pack);
    };

    CRFProtocolNooLite();
    ~CRFProtocolNooLite();

    virtual std::string getName()
    {
        return "nooLite";
    };
    virtual std::string DecodePacket(const std::string &);
    CPacket DecodeBitsToStruct(const std::string &);
    virtual std::string DecodeData(const std::string &); // Преобразование бит в данные
    virtual bool needDump(const std::string &rawData);

    // Кодирование
    virtual std::string bits2timings(const std::string &bits);
    virtual std::string data2bits(const std::string &data);
    static nooLiteCommandType getCommand(const std::string &name);
    static const char *getDescription(int cmd);

  private:
    uint8_t getByte(const std::string &bits, size_t first, size_t len = 8);
    bool bits2packet(const std::string &bits, uint8_t *packet, size_t *packetLen, uint8_t *CRC = NULL);
    uint8_t crc8(uint8_t *addr, uint8_t len);
};

