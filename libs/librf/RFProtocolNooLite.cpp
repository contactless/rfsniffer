#include "RFProtocolNooLite.h"

#include "../libutils/DebugPrintf.h"
#include "../libutils/Exception.h"
#include "../libutils/logging.h"

typedef std::string string;
using namespace strutils;

using std::ostream;

static const range_type g_timing_pause[7] = {
    { 380, 750 },
    { 851, 1400 },
    { 1800, 2300 },
    { 2500, 2700 },
    { 0, 0 }
};  // TODO FIXIT 500us, 1000us

static const range_type g_timing_pulse[8] = {
    { 200, 650 },
    { 700, 1100 },
    { 1800, 2300 },
    { 2500, 2700 },
    { 0, 0 }
};   // TODO FIXIT 500us, 1000us

static const uint16_t g_transmit_data[] = {
    500, 1000, 1500, 2000, 0,  // Pauses
    500, 1000, 1500, 2000, 0   // Pulses
};


static const char *g_nooLite_Commands[] = {
    "off",              //0 – выключить нагрузку
    "slowdown",         //1 – запускает плавное понижение яркости
    "on",               //2 – включить нагрузку
    "slowup",           //3 – запускает плавное повышение яркости
    "switch",           //4 – включает или выключает нагрузку
    "slowswitch",       //5 – запускает плавное изменение яркости в обратном
    "shadow_level",     //6 – установить заданную в «Данные к команде_0» яркость
    "callscene",        //7 – вызвать записанный сценарий
    "recordscene",      //8 – записать сценарий
    "unbind",           //9 – запускает процедуру стирания адреса управляющего устройства из памяти исполнительного
    "slowstop",         //10 – остановить регулировку
    "?11",              //значения 11, 12, 13, 14– зарезервированы, не используются
    "?12",              //
    "?13",              //
    "?14",              //
    "bind",             //15 – сообщает, что устройство хочет записать свой адрес в память
    "slowcolor",        //16 – включение плавного перебора цвета
    "switchcolor",      //17 – переключение цвета
    "switchmode",       //18 – переключение режима работы
    "switchspeed",      //19 – переключение скорости эффекта для режима работы
    "battery_low",      //20 – информирует о разряде батареи в устройстве
    "temperature",      //21 – передача информации о текущей температуре и
    //     влажности (Информация о температуре и влажности содержится в
    //     поле «Данные к команде_x».)
    "test_result",      //22
    "shadow_load_preset", //23
    "shadow_set_bright", //24
    "temporary_on",     //25
    "modes",            //26
    NULL
};
static int g_nooLite_Commands_len = 21;

const int CRFProtocolNooLite::fmt2length[] =
//       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
        {5, 6, 7, 9, 5, 6, 7, 9, 5, 6, 7, 9, 5, 6, 7, 9};
// I see cyclicity here...

CRFProtocolNooLite::CPacket::CPacket(bool correct): correct(correct) {}

ostream &operator<<(ostream &out, const CRFProtocolNooLite::CPacket &pack) {
    out << "nooLitePacket(";
    out << "flip=" << (int)pack.flip << ", ";
    out << "cmd=" << (int)pack.cmd << ", ";
    out << "addr=" << std::hex << (int)pack.addr << ", ";
    out << "res=" << (int)pack.res << ", ";
    out << "fmt=" << (int)pack.fmt << "";
    for (int i = 0; i < 4; i++)
        out << ", d" << i << "=" << std::hex << (int)pack.d[i];
    out << ")";
    return out;
}


CRFProtocolNooLite::CRFProtocolNooLite()
    : CRFProtocol(g_timing_pause, g_timing_pulse, 0, 1, "AaAaAaAaAaAaAaAaAaAc")
{
    m_Debug = false;
    SetTransmitTiming(g_transmit_data);
}


CRFProtocolNooLite::~CRFProtocolNooLite()
{
}

CRFProtocolNooLite::nooLiteCommandType CRFProtocolNooLite::getCommand(const std::string &name)
{
    nooLiteCommandType res = nlcmd_off;
    for (const char **p = g_nooLite_Commands; *p; p++) {
        if (name == *p)
            return res;

        res = (nooLiteCommandType)(res + 1);
    }

    return nlcmd_error;
}

const char *CRFProtocolNooLite::getDescription(int cmd)
{
    if (cmd < 0 || cmd >= g_nooLite_Commands_len)
        return g_nooLite_Commands[cmd];
    else
        return "unknown";
}

// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
uint8_t CRFProtocolNooLite::crc8(uint8_t *addr, uint8_t len)
{
    uint8_t crc = 0;

    for (uint8_t i = 0; i < len; i++) {
        uint8_t inbyte = addr[i];
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix)
                crc ^= 0x8C;

            inbyte >>= 1;
        }
    }
    return crc;
}

unsigned char CRFProtocolNooLite::getByte(const std::string &bits, size_t first, size_t len)
{
    return (unsigned char)bits2long(reverse(bits.substr(first, len)));
}

bool CRFProtocolNooLite::bits2packet(const std::string &bits, uint8_t *packet, size_t *packetLen,
                                     uint8_t *CRC)
{
    size_t bytes = (bits.substr(1).length() + 7) / 8;
    //~ size_t extraBits = bits.substr(1).length() % 8;

    if (*packetLen < bytes)
        return false;

    std::string reverseBits = reverse(bits.substr(1));
    while (reverseBits.length() % 8)
        reverseBits.push_back('0');

    for (uint32_t i = 0; i < bytes; i++) {
        packet[bytes - 1 - i] = (uint8_t)bits2long(reverseBits.substr(i * 8,
                                8));// min(8, reverseBits.length() - i * 8)));
    }

    uint8_t packetCrc = crc8(packet, (uint8_t)bytes);
    *packetLen = bytes;

    if (CRC)
        *CRC = crc8(packet, (uint8_t)bytes - 1);

    return packetCrc == 0;
}


string CRFProtocolNooLite::DecodePacket(const std::string &raw_)
{
    DPRINTF_DECLARE(dprintf, false);
    String raw = String(raw_);
    dprintf("$P Noolite decode packet, raw = %\n", raw);

    // shortest nooLite message - 5 bytes - 40 bits - 40 signals at minimum
    // TODO move this logic to RFProtocol
    if (raw.length() < 40)
        return "";

    String::Vector v = raw.Split('d');
    //SplitString(raw, 'd', v);

    if (v.size() == 1)
        v = raw.Split('c');   // TODO CheckIT

    String res;

    for(const std::string &i : v) {
        res = ManchesterDecode('a' + i, false, 'a', 'b', 'A', 'B');
        dprintf("$P manch decoded: %\n", res);
        if (res.length() >= 37) {
            std::vector<uint8_t> packet(raw.size());
            size_t packetLen = packet.size() * sizeof(uint8_t);

            if (bits2packet(res, packet.data(), &packetLen))
                return res;

        }
    }

    return "";
}

CRFProtocolNooLite::CPacket CRFProtocolNooLite::DecodeBitsToStruct(const string &bits) {
    DPRINTF_DECLARE(dprintf, false);
    uint8_t packet[20];
    size_t packetLen = sizeof(packet);

    if (!bits2packet(bits, packet, &packetLen))
        return CPacket(false);

    if (packetLen < 5)
        return CPacket(false);

    /*
    #                        [ (FLIP) CMD  ] [           RGB          ] [   ?  ] [      ADDR     ] [ FMT  ] [ CRC  ]
    #                        1FCCCC
    #ch:2 r:1 g:1 b:1        110110          10000000 10000000 10000000 00000000 10011111 10100100 11000000 11001011  fmt=3, cmd=6
    #ch:2 r:1 g:1 b:2        100110          10000000 10000000 01000000 00000000 10011111 10100100 11000000 11101101  fmt=3
    #ch:2 r:255 g:255 b:255  110110          11111111 11111111 11111111 00000000 10011111 10100100 11000000 10110001  fmt=3
    #ch:14 r:1 g:1 b:2       110110          10000000 10000000 01000000 00000000 11111111 10100100 11000000 00110010  fmt=3
    #ch:14 r:1 g:1 b:2       100110          10000000 10000000 01000000 00000000 11111111 10100100 11000000 01100110  fmt=3
    #ch:15 r:1 g:1 b:2       110110          10000000 10000000 01000000 00000000 11111111 10100100 11000000 00110010  fmt=3
    #
    #ch:2 switch mode        11     01001000                                     10011111 10100100 00100000 00010101  fmt=4, cmd=18
    #ch:2 switch color       11     10001000                                     10011111 10100100 00100000 00000100  fmt=4, cmd=17
    #                                                                   [LEVEL ]
    #ch:2 lvl=46             110110                                     01110100 10011111 10100100 10000000 10010100  fmt=1
    #
    #ch:2 cmd=10             110101                                              10011111 10100100 00000000 00010001  fmt=0
    #ch:2 off_ch             110000                                              10011111 10100100 00000000 10000100  fmt=0
    #                                                                   [ TIME ]
    #                        11     00011000                            00100110 10100000 01000100 10100000 00110110  fmt=5, cmd=24
    #                                                          [      TIME     ]
    #                        11     10011000                   10000000 10011000 10100000 01000100 01100000 00100001  fmt=6 ch:5 cmd=25 timeout=(25*256+1)*5
    */

    dprintf("$P bits: (%), packet: (", bits);
    for (size_t i = 0; i < packetLen && dprintf.isActive(); i++)
        dprintf.c("%02X ", (int)packet[i]);
    dprintf(")\n");

    // DEBUG
    int calculated_crc = crc8(packet, packetLen - 1);
    int received_crc = packet[packetLen - 1];
    if (calculated_crc != received_crc) {
        LOG(INFO) << String::ComposeFormat(
                "CRFProtocolNooLite::DecodeData - Incorrect packet - wrong CRC (received %02x != %02x calculated)",
                received_crc, calculated_crc);
        return CPacket(false);
    }
    int crc = received_crc;

    int fmt = packet[packetLen - 2];

    if (fmt < 0 || fmt >= int(sizeof(fmt2length) / sizeof(int)) ||
        !(fmt2length[fmt] == (int)packetLen ||
            (fmt2length[fmt] + 1 == (int)packetLen && fmt > 3))) {
        LOG(INFO) << "CRFProtocolNooLite::DecodeData - Incorrect packet - strange "\
                     "fmt=" << fmt << ", received_len=" << packetLen;
        return CPacket(false);
    }

    bool isLongCmd = (fmt2length[fmt] + 1 == (int)packetLen);

    CPacket ret;
    if (isLongCmd) {
        ret.flip = (packet[0] & 0x80) != 0;
        ret.cmd = packet[1];

    } else {
        ret.flip = (packet[0] & 0x08) != 0;
        ret.cmd = packet[0] >> 4;
    }
    ret.fmt = fmt;
    ret.addr = (uint16_t(packet[packetLen - 3]) << 8) | uint16_t(packet[packetLen - 4]);
    ret.crc = crc;

    for (int i = 0; i < 4 && 1 + isLongCmd + i < (int)packetLen - 4; i++)
        ret.d[i] = packet[1 + isLongCmd + i];
    dprintf("$P ret = %\n", ret);
    return ret;
}

string CRFProtocolNooLite::DecodeData(const string
                                      &bits) // Ïðåîáðàçîâàíèå áèò â äàííûå
{
    DPRINTF_DECLARE(dprintf, false);

    CPacket pack = DecodeBitsToStruct(bits);

    int easyFmt = pack.fmt % 4;

    BufferWriter buffer;
    buffer.printf("flip=%d cmd=%d ", (int)pack.flip, (int)pack.cmd);

    uint32_t data = 0;
    switch (easyFmt) {
        case 1: {
            data = pack.d[0];
            break;
        };
        case 2: {
            data = ((uint32_t)pack.d[0] << 8) | (uint32_t)pack.d[1];
            break;
        };
        case 3: {
            data = ((uint32_t)pack.d[0] << 24) | ((uint32_t)pack.d[1] << 16) | ((uint32_t)pack.d[2] << 8) | (uint32_t)pack.d[3];
            break;
        };
        default: {};
    }

    bool successSwitch = true;
    switch (pack.cmd) {
        case nlcmd_temporary_on: {
            buffer.printf("time=%d ", data * 5);
            break;
        };
        case nlcmd_shadow_set_bright:
        case nlcmd_shadow_level: {
            if (easyFmt == 1) {
                // pc118 it's magic
                int level = (data - 0x23) * 10 / 12;
                dprintf("$P data = %, level = %\n", data, level);
                // (level % 10 == 0) must be true
                buffer.printf("%s=%d ", (pack.cmd == nlcmd_shadow_level ? "level" : "shadow_level"), level);
                break;
            }
            if (easyFmt == 3) {
                buffer.printf("r=%d g=%d b=%d unknown=%d ", (int)pack.d[0], (int)pack.d[1], (int)pack.d[2], (int)pack.d[3]);
                break;
            }
            successSwitch = false;
            break;
        };
        case nlcmd_temp_humi: {
            if (easyFmt != 3) {
                successSwitch = false;
                break;
            }

            int type = (pack.d[1] >> 4) & 7;
            int t_raw = ((uint32_t)(pack.d[1] & 0xF) << 8) | pack.d[0];
            const int t_signum_bit = 11, t_signum_bit_mask = (1 << t_signum_bit);

            float t = 0.1 * (
                (t_raw & t_signum_bit_mask) ?
                -((t_signum_bit_mask << 1) - t_raw) :
                +t_raw
            );

            int h = pack.d[2];
            int s3 = pack.d[3];
            int bat = ((pack.d[1] & 0x80) != 0);

            if (type == 2) {
                buffer.printf("type=%d t=%.1f h=%d s3=%02x low_bat=%d ", type, t, h, s3, bat);
            } else if (type == 3) {
                buffer.printf("type=%d t=%.1f s3=%02x low_bat=%d ", type, t, s3, bat);
            } else {
                successSwitch = false;
            }
            break;
        };
        default: {
            successSwitch = false;
        };
    }

    if (!successSwitch && easyFmt != 0) {
        buffer.printf("data=%08x ", data);
    }

    buffer.printf("addr=%04x fmt=%02x crc=%02x",
            (int)pack.addr, (int)pack.fmt, (int)pack.crc);

    dprintf("$P ret = %\n", buffer.getString());
    return buffer.getString();
}

bool CRFProtocolNooLite::needDump(const std::string &rawData)
{
    return rawData.find(m_PacketDelimeter) != rawData.npos;
}


string CRFProtocolNooLite::bits2timings(const std::string &bits)
{
    DPRINTF_DECLARE(dprintf, false);
    std::string start;
    for (int i = 0; i < 39; i++) {
        start.push_back('1');
    }
    std::string encodedBits = ManchesterEncode(bits, true, 'a', 'b', 'A', 'B');
    dprintf("$P manch encode gives: % -> %\n", bits, encodedBits);
    // it is not obvious but consequent pauses 'b' and 'a' form pause 'c'
    return 'A' + ManchesterEncode(start, true, 'a', 'b', 'A', 'B')
           + 'b' + encodedBits + 'b' + encodedBits;
}

string l2bits(uint16_t val, int bits)
{
    std::string res;
    for (int i = 0; i < bits; i++) {
        res = res + ((val & 1) ? '1' : '0');
        val >>= 1;
    }

    return res;
}

string CRFProtocolNooLite::data2bits(const std::string &data)
{
    String proto, dataDetail;
    String(data).SplitByExactlyOneDelimiter(':', proto, dataDetail);
    if (proto != "nooLite")
        throw CHaException(CHaException::ErrBadParam, "Bad protocol in '" + data + "'");

    String::Map values = dataDetail.SplitToPairs();

    std::string sAddr = values["addr"];
    std::string sCmd = values["cmd"];
    std::string sFmt = values["fmt"];
    std::string sFlip = values["flip"];
    //int level = (data - 0x23) * 10 / 12;
    std::string sLevel = values["level"];
    std::string sShadowLevel = values["shadow_level"];
    std::string sR = values["r"];
    uint8_t r = sR.length() ? atoi(sR) : 255;
    std::string sG = values["g"];
    uint8_t g = sG.length() ? atoi(sG) : 255;
    std::string sB = values["b"];
    uint8_t b = sB.length() ? atoi(sB) : 255;

    uint16_t addr = (uint16_t)strtol(sAddr.c_str(), NULL, 16);
    uint8_t cmd = atoi(sCmd);
    uint8_t fmt = sFmt.length() ? atoi(sFmt) : 0xFF;
    bool flip = sFlip.length() ? (bool)atoi(sFlip) : !m_lastFlip[addr];
    m_lastFlip[addr] = flip;
    uint8_t level = String(!sLevel.empty() ? sLevel : sShadowLevel).IntValue() * 12 / 10 + 0x23;

    std::string res = "1" + l2bits(flip, 1) + l2bits(cmd, (cmd <= 0xF ? 4 : 8));

    switch (cmd) {
        case nlcmd_off:             //0 – выключить нагрузку
        case nlcmd_slowdown:        //1 – запускает плавное понижение яркости
        case nlcmd_on:              //2 – включить нагрузку
        case nlcmd_slowup:          //3 – запускает плавное повышение яркости
        case nlcmd_switch:          //4 – включает или выключает нагрузку
        case nlcmd_slowswitch:      //5 – запускает плавное изменение яркости в обратном
        case nlcmd_slowstop:        //10 – остановить регулировку
        case nlcmd_bind:            //15 – сообщает, что устройство хочет записать свой адрес в память
        case nlcmd_unbind:          //9 – запускает процедуру стирания адреса управляющего устройства из памяти исполнительного
        case nlcmd_switchcolor:     //17 – переключение цвета
            if (fmt == 0xff)    fmt = 0;
            else if (fmt != 0) throw CHaException(CHaException::ErrBadParam, "bad format: " + data);
            break;

        case nlcmd_shadow_level:        //6 – установить заданную в «Данные к команде_0» яркость
        case nlcmd_shadow_set_bright:
            if (fmt == 0xff) {
                if (sR.length() == 0)
                    fmt = 1;
                else
                    fmt = 3;
            }

            if (fmt == 1)
                res += l2bits(level, 8);
            else if (fmt == 3)
                res += l2bits(r, 8) + l2bits(g, 8) + l2bits(b, 8) + l2bits(0, 8);
            else
                throw CHaException(CHaException::ErrBadParam, "bad format: " + data);
            break;

        /*
                nlcmd_callscene,        //7 – вызвать записанный сценарий
                nlcmd_recordscene,      //8 – записать сценарий
                nlcmd_slowcolor,        //16 – включение плавного перебора цвета
                nlcmd_switchcolor,      //17 – переключение цвета
                nlcmd_switchmode,       //18 – переключение режима работы
                nlcmd_switchspeed,      //19 – переключение скорости эффекта для режима работы
                nlcmd_battery_low,      //20 – информирует о разряде батареи в устройстве
                nlcmd_temperature,      //21 – передача информации о текущей температуре и
        */
        default:
            throw CHaException(CHaException::ErrBadParam, "unsupported cmd: " + data);
    }

    res += l2bits(addr, 16) + l2bits(((cmd <= 0xF || fmt >= 4) ? fmt : (fmt + 4)), 8) + l2bits(0/*crc*/, 8);
    uint8_t packet[100];
    size_t packetLen = sizeof(packet);
    uint8_t crc;
    bits2packet(res, packet, &packetLen, &crc);
    res = res.substr(0, res.length() - 8) + l2bits(crc, 8);
    LOG(INFO) << "res=" << res;

    return res;
}

