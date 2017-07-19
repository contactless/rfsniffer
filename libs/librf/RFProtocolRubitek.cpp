#include "RFProtocolRubitek.h"

using namespace strutils;
typedef std::string string;

//
static range_type g_timing_pause[7] = {
    { 9400, 9700 }, // Разделитель
    { 280, 480 }, // Короткий
    { 920, 1100 }, // Длинный
    { 0, 0 }
};

static range_type g_timing_pulse[8] = {
    { 3300, 3500 },
    { 169, 340 },
    { 820, 1000 },
    //  { 200000, 200001 },
    { 0, 0 }
};



CRFProtocolRubitek::CRFProtocolRubitek()
    : CRFProtocol(g_timing_pause, g_timing_pulse, 25, 2, "a")
{
}


CRFProtocolRubitek::~CRFProtocolRubitek()
{
}


string CRFProtocolRubitek::DecodePacket(const std::string &pkt)
{
    std::string packet = pkt, res;
    if (packet.length() == 49) {
        if (packet[48] == 'B')
            packet.push_back('c');
        if (packet[48] == 'C')
            packet.push_back('c');
    } else
        return "";

    for (int i = 0; i < (int)packet.length() - 1; i += 2) {
        std::string part = packet.substr(i, 2);
        if (packet[i] == 'B' && packet[i + 1] == 'c')
            res.push_back('0');
        else if (packet[i] == 'C' && packet[i + 1] == 'b')
            res.push_back('1');
        else
            return "";
    }

    return res;
}
