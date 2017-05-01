#include "RFProtocolHS24Bit.h"

#include <cassert>

#include "stdafx.h"
#include "../libutils/strutils.h"
#include "../libutils/DebugPrintf.h"

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



CRFProtocolHS24Bit::CRFProtocolHS24Bit()
    : CRFProtocol(g_timing_pause, g_timing_pulse, 24, 2, "Ac")
{
}


CRFProtocolHS24Bit::~CRFProtocolHS24Bit()
{
}

string CRFProtocolHS24Bit::DecodePacket(const string &packet)
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Enter\n");
    string bits;
    if (packet.length() != 48)
        return "";
    for (int i = 0; i < (int)packet.size(); i += 2) {
        if (packet[i] == 'A' && packet[i + 1] == 'b') {
            bits.push_back('0');
            continue;
        }
        if (packet[i] == 'B' && packet[i + 1] == 'a') {
            bits.push_back('1');
            continue;
        }
        return "";
    }
    dprintf("$P Bits are %\n", bits);
    return bits;
}


string CRFProtocolHS24Bit::DecodeData(const string &bits)
{
    
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Enter\n");
    if (bits.size() != 24) {
        dprintf("$P bits.size() != 24 - broken invariant for this protocol");
        return "";
    }
    const int base = 2;
    int msg = 0;
    for (int i = 0; i < 20; i++)
        msg = msg * base + bits[i] - '0';
    int ch = 0;
    for (int i = 20; i < 24; i++)
        ch = ch * base + bits[i] - '0';
        
    string data = String::ComposeFormat("msg_id=%d ch=%d", msg, ch);
    dprintf("$P Data are %\n", data);
    return data;
}
