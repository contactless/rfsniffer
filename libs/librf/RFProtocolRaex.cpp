#include "stdafx.h"
#include "RFProtocolRaex.h"

static range_type g_timing_pause[7] =
{
	{ 2500, 2700 },
	{ 550, 750 },
	{ 1200, 1400 },
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 2400, 2600 },
	{ 450, 650 },
	{ 110, 1300 },
	{ 0,0 }
};


CRFProtocolRaex::CRFProtocolRaex()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 57, 2, "A")
{
}


CRFProtocolRaex::~CRFProtocolRaex()
{
}


string CRFProtocolRaex::DecodePacket(const string&raw)
{
	if (raw.length() < 10 || raw[0] != 'a')
		return "";

	return ManchesterDecode(raw.substr(1), true, 'b', 'c', 'B', 'C');
}

/*
1O 1 11110 0000111101111110000 10 00000010111111 1111111111111111
1S 1 11110 0000111101111110000 01 00000001111111 1111111111111111
1C 1 11110 0000111101111110000 11 00000011111111 1111111111111111

2O 1 00001 0000111101111110000 10 00000001111111 1111111111111111
2S 1 00001 0000111101111110000 01 00000011111111 1111111111111111
2C 1 00001 0000111101111110000 11 00000000000000 1111111111111111

3O 1 10001 0000111101111110000 10 00000011111111 1111111111111111
3S 1 10001 0000111101111110000 01 00000000000000 1111111111111111
3C 1 10001 0000111101111110000 11 00000010000000 1111111111111111

4O 1 01001 0000111101111110000 10 00000000000000 1111111111111111

5O 1 11001 0000111101111110000 10 00000010000000 1111111111111111

6O 1 00101 0000111101111110000 10 00000001000000 1111111111111111
00101000011110111111000010 00000001000000 1111111111111111

2    8    7    B    F    0    8    0    4    0
60 1 0010 1000 0111 1011 1111 0000 1000 0000 0100 0000 1111 1111 1111 1111
F    0    7    B    F    0    4    0    7    F
1S 1 1111 0000 0111 1011 1111 0000 0100 0000 0111 1111 1111 1111 1111 1111

28 7B F0 08 40 FF FF
F0 7B F0 40 7F FF FF


1 = F0
2 = 80
3 = 88
4 = 48
5 = C8
6 = 24
*/

string CRFProtocolRaex::DecodeData(const string& packet)
{
	if (packet.length() != 57)
		return "";

	if (packet.substr(57-8)!="11111111")
		return "";

	string raw;

	for (int i = 0; i < 7; i++)
	{
		unsigned char b = (unsigned char)bits2long(packet, 1 + i * 8, 8);
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "%02X", b);
		raw += buffer;
	}

	string res = "raw=" + raw;
	unsigned char ch = (unsigned char)bits2long(packet, 1, 8);
	unsigned char cmd = (unsigned char)bits2long(packet, 25, 2);
	char buffer[10];
	snprintf(buffer, sizeof(buffer), "%02X", ch);
	res += " ch=" + (string)buffer + " btn=" + itoa(cmd);

	return res;
}
