#include "RFProtocolLivolo.h"
#include "../libutils/strutils.h"

typedef std::string string;
using namespace strutils;

//
static range_type g_timing_pause[7] = {
    { 420, 570 }, // Разделитель
    { 140, 280 }, // Короткий
    { 290, 440 }, // Длинный
    { 0, 0 }
};

static range_type g_timing_pulse[8] = {
    { 380, 500 },
    {  20, 170 },
    { 190, 300 },
    //  { 200000, 200001 },
    { 0, 0 }
};


/*
    00110011110100010001000 - A
    00110011110100010010000 - B
    00110011110100010111000 - C
    00110011110100010101010 - D
*/


CRFProtocolLivolo::CRFProtocolLivolo()
    : CRFProtocol(g_timing_pause, g_timing_pulse, 23, 2, "A")
{
}


CRFProtocolLivolo::~CRFProtocolLivolo()
{
}

string CRFProtocolLivolo::DecodePacket(const std::string &packet)
{
    std::string bits;
    bool waitSecondShort = false;

    for (const char s : packet) {
        if (waitSecondShort) {
            waitSecondShort = false;
            if (s == 'b' || s == 'B')
                continue;
            else
                return "";

        }

        if (s == 'b' || s == 'B') {
            bits.push_back('0');
            waitSecondShort = true;
        } else if (s == 'c' || s == 'C') {
            bits.push_back('1');
        }
    }

    return bits;
}
