#include "stdafx.h"
#include "RFProtocolX10.h"

static range_type g_Pulse[3] =
{
	{ 300,500 },
	{ 501,800 },
	{ 0,0 }
};

static range_type g_Zero[4] =
{
	{ 4400, 4700 },
	{ 500,850 },
	{ 1500, 1950 },
	{ 0,0 }
};


CRFProtocolX10::CRFProtocolX10()
	:CRFProtocol(g_Zero, g_Pulse, 32, 2, "a")
{
}


CRFProtocolX10::~CRFProtocolX10()
{
}

string CRFProtocolX10::DecodePacket(const string&packet)
{
	string bits;

	for_each_const(string, packet, i)
	{
		if (*i == 'b')
			bits += "0";
		else if (*i == 'c')
			bits += "1";
		else if (*i == 'd')
			break;
		else if (*i == 'A')
			continue;
		else if (*i == 'B')
			continue;
		else
			return "";
	}

	if (bits.length() != 32)
		return "";

	for (int i = 0; i < 8; i++)
		if (bits[i] + bits[8 + i] != '0' + '1' || bits[16 + i] + bits[16 + 8 + i] != '0' + '1')
			return "";

	return bits;
}

static const char COMMAND[16][8] = { "?0?", "?1?", "ON", "OFF", "DIM", "BRIGHT", "?", "?", 
										"?","?","?","?","?","?","?","?"};


string CRFProtocolX10::DecodeData(const string&packet)
{
	if (packet.length() != 32)
		return "";

	unsigned char byte1 = (unsigned char)bits2long(reverse(packet.substr(0, 8)));
	unsigned char byte2 = (unsigned char)bits2long(reverse(packet.substr(16, 8)));
	unsigned char unit = 0, command = 0;

	char house = parseHouseCode(byte1 & 0xF);
	// Bright or Dim
	if (byte2 & 1)
	{
		unit = 0;
		// Bit magic to create X10 CMD_DIM (B0100) or CMD_BRIGHT (B0101) nibble
		command = byte2 >> 3 & 0x1 ^ 0x5;
	}
	// On or Off
	else
	{
		// Swap some bits to create unit integer from binary data
		unit = (byte2 >> 3 | byte2 << 1 & 0x4 | byte1 >> 2 & 0x8) + 1;
		// Bit magic to create X10 CMD_ON (B0010) or CMD_OFF (B0011) nibble
		command = byte2 >> 2 & 0x1 | 0x2;
	}

	char res[10];
	res[0] = house;
	res[1] = 0;

	return string(res)+itoa(unit)+COMMAND[command];
}

static const uint8_t HOUSE_CODE[16] =
{
	0B0110,0B1110,0B0010,0B1010,0B0001,0B1001,0B0101,0B1101,
	0B0111,0B1111,0B0011,0B1011,0B0000,0B1000,0B0100,0B1100,
};

char CRFProtocolX10::parseHouseCode(uint8_t data)
{
	for (uint8_t i = 0; i <= 0xF; i++)
	{
		if (HOUSE_CODE[i] == data) return i + 65;
	}

	return 'Z';
}