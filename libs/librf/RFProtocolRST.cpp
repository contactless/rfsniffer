#include "RFProtocolRST.h"

using namespace strutils;
typedef std::string string;

static range_type g_timing_pause[7] = {
    //  { 70,310 },
    //  { 380,580 },
    { 7800, 8300 },
    { 800, 1200 },
    { 1800, 2200 },
    { 0, 0 }
};

static range_type g_timing_pulse[8] = {
    //{ 7800, 8300 },
    //{ 800,1200 },
    //{ 1800, 2200 },
    {90, 210},
    { 0, 0 }
};


CRFProtocolRST::CRFProtocolRST()
    : CRFProtocol(g_timing_pause, g_timing_pulse, 36, 2, "a")
{
}


CRFProtocolRST::~CRFProtocolRST()
{
}

string CRFProtocolRST::DecodePacket(const std::string &packet)
{
    std::string bits;

    for (const char s : packet) {
        if (s == 'b' ) {
            bits.push_back('0');
        } else if (s == 'c') {
            bits.push_back('1');
        } else if (s == 'A') {
            continue;
        } else
            return "";
    }

    return bits;
}

string CRFProtocolRST::DecodeData(const std::string &raw)
{
    char buffer[100];
    short t0 = bits2long(raw.substr(24));
    if (t0 & 0x800)
        t0 |= 0xF000;

    float t = 0.1 * t0;
    short h = (short)bits2long(raw.substr(16, 8));
    unsigned short id = (unsigned short)bits2long(raw.substr(0, 16));

    snprintf(buffer, sizeof(buffer), "id=%x h=%d t=%.1f", id, h, t);

    return buffer;
}
