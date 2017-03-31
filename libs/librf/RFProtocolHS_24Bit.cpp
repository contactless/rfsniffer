#include "stdafx.h"
#include "RFProtocolLivolo.h"
#include "../libutils/strutils.h"

//
static range_type g_timing_pause[7] = {
    { 200, 600 }, 
    { 1100, 1400 }, 
    { 12000, 13000 }, // Длинный в преамбуле
    { 0, 0 }
};

static range_type g_timing_pulse[8] = {
    { 200, 600 },
    {  1000, 1400 },
    //  { 200000, 200001 },
    { 0, 0 }
};



CRFProtocolHS24Bits::CRFProtocolHS24Bits()
    : CRFProtocol(g_timing_pause, g_timing_pulse, 48, 2, "Ac")
{
}


CRFProtocolHS24Bits::~CRFProtocolHS24Bits()
{
}

string CRFProtocolHS24Bits::DecodePacket(const string &packet)
{
    string bits;
    if (packet.length() != 48)
        return "";
    if (int i = 0; i < (int)packet.size(); i += 2) {
        if (packet[i] == 'A' && packet[i + 1] == 'b') {
            bits.push_back('0');
            continue;
        }
        if (packet[i] == 'B' && packet[i + 1] == 'a') {
            bits.push_back('0');
            continue;
        }
        return "";
    }
    return bits;
}


string CRFProtocolHS24Bits::DecodeData(const string &bits)
{
    assert(packet.size() == 24);
    int msg = 0;
    for (int i = 19; i >= 0; i--)
        if (isdigit(bits[i]))
            msg = msg * base + bits[i] - 0;
        else
            return "";
    int ch = 0;
    for (int i = 23; i >= 20; i--)
        if (isdigit(bits[i]))
            ch = ch * base + bits[i] - 0;
        else
            return "";

    return String::ComposeFormat("msg_id=%d ch=%d", msg, ch);
}
