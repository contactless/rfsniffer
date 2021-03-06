#include "RFProtocolOregonV3.h"

#include "../libutils/DebugPrintf.h"

typedef std::string string;
using namespace strutils;

static range_type g_timing_pause[7] = {
    { 40000, 47000 },
    { 380, 850 },
    { 851, 1400 },
    { 0, 0 }
};

static range_type g_timing_pulse[8] = {
    { 1101, 1101 },
    { 200, 615 },
    { 615, 1100 },
    { 0, 0 }
};


// originally preamble was "cCcCcCcCcCcCcCcCcCcCcCcCcCcCcCbBc" (plus "cCc" in the beginning)
// but porting tests from ism-radio it was shortened
CRFProtocolOregonV3::CRFProtocolOregonV3()
    : CRFProtocolOregon(g_timing_pause, g_timing_pulse, 0, 1, "BbBbBbBbBbBbBcCcC")
{
}


CRFProtocolOregonV3::~CRFProtocolOregonV3()
{
}


string CRFProtocolOregonV3::DecodePacket(const std::string &raw_)
{
    DPRINTF_DECLARE(dprintf, false);

    std::string raw = raw_;

    if (raw.length() < 10)
        return "";

    // truncate by 'a', '?' (it's the end of the packet)
    {
        size_t apos = raw.find('a');
        if (apos != string::npos)
            raw.resize(apos);
        apos = raw.find('?');
        if (apos != string::npos)
            raw.resize(apos);
    }
    dprintf("$P decodePacket: %\n", raw);

    std::vector <char> isPulse(raw.size());
    for (int i = 0; i < (int)raw.size(); i++)
        isPulse[i] = ('A' <= raw[i] && raw[i] <= 'Z');

    // check for alternating pulses and pauses
    for (int i = 1; i < (int)raw.size(); i++)
        if ((isPulse[i - 1] ^ isPulse[i]) == 0)
            return "";


    std::string packet = "0101";
    packet.reserve(raw.size());
    char demand_next_c = 0;

    for (char c : raw) {
        if (demand_next_c != 0) {
            if (c == demand_next_c) {
                demand_next_c = 0;
                continue;
            } else
                return "";
        }
        switch (c) {
            case 'b':
                packet.push_back('1');
                demand_next_c = 'B';
                break;
            case 'B':
                packet.push_back('0');
                demand_next_c = 'b';
                break;
            case 'c':
                packet.push_back('0');
                break;
            case 'C':
                packet.push_back('1');
                break;
            default:
                return "";
        }
    }


    //packet.resize(packet.length() - 8);
    dprintf("$P    decodedBits(%): %\n", packet.size(), packet);

    uint32_t crc = 0, originalCRC = -1;
    std::string hexPacket = "";

    if (packet.length() < 48) {
        dprintf("$P (only warning: it may be other protocol) "\
                "Too short packet %s\n", packet);
        return "";
    }

    int len = packet.length();
    while (len % 4)
        len--;

    dprintf("$P CRCs: ");
    for (int i = 0; i < len; i += 4) {
        std::string portion = reverse(packet.substr(i, 4));
        char buffer[20];
        uint32_t val = bits2long(portion);

        if (i > 0 && i < len - 16)
            crc += val;
        else if (i == len - 16)
            originalCRC = val;
        else if (i == len - 12)
            originalCRC += val << 4;

        dprintf("%(%) ", val, crc);
        snprintf(buffer, sizeof(buffer), "%X", val);
        hexPacket += buffer;
    }
    dprintf("\n");

    dprintf("$P    decodedData: %\n", hexPacket);

    if (crc != originalCRC) {
        dprintf("$P (only warning: it may be other protocol)"\
                " Bad CRC (calculated % != % told) for %", crc,
                originalCRC, packet);
        return "";
    }


    if (hexPacket[0] != 'A') {
        dprintf("$P First nibble is not 'A'. Data: %", hexPacket);
        return "";
    }

    return hexPacket;
}

