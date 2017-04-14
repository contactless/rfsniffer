#include "RFProtocolVhome.h"

// 
static range_type g_timing_pause[7] =
{
	{ 12000, 18000 }, // Разделитель
	{ 250, 550 }, // Короткий
	{ 1200, 1500 }, // Длинный
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 3500, 3500 },
	{ 250, 550 }, // Короткий
	{ 1200, 1500 }, // Длинный
	{ 0,0 }
};



CRFProtocolVhome::CRFProtocolVhome()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 25, 1, "a")
{
	m_Debug = true;
}


CRFProtocolVhome::~CRFProtocolVhome()
{
}


string CRFProtocolVhome::DecodePacket(const string&pkt)
{
	string packet = pkt, res;

	if (packet.length() == 48)
	{
		if (packet[0] == 'c')
			packet = "B" + packet;
		if (packet[0] == 'b')
			packet = "C" + packet;
	}

	if (packet.length() == 49)
	{
		if (packet[48] == 'B')
			packet += "c";
		if (packet[48] == 'C')
			packet += "b";
	}
	else
		return "";

	for (unsigned int i = 0; i < packet.length() - 1; i += 2)
	{
		string part = packet.substr(i, 2);
		if (part == "Bc")
			res += "0";
		else if (part == "Cb")
			res += "1";
		else
			return "";
	}

	return res;
}

string CRFProtocolVhome::DecodeData(const string& bits)
{
	if (bits.length()!=25 || bits2long(bits, 0, 4) != 7 || bits[24]!='0')
		return "";

	int addr = bits2long(bits, 4, 16);
	int cmd = bits2long(bits, 20, 4);

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "addr=%04x cmd=%d", addr, cmd);
	return buffer;
}
