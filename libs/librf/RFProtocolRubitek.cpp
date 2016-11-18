#include "RFProtocolRubitek.h"

// 
static range_type g_timing_pause[7] =
{
	{ 9400, 9700 }, // Разделитель
	{ 280, 480 }, // Короткий
	{ 920, 1100 }, // Длинный
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 3300, 3500 },
	{ 169, 340 },
	{ 820, 1000 },
	//	{ 200000, 200001 },
	{ 0,0 }
};



CRFProtocolRubitek::CRFProtocolRubitek()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 25, 2, "a")
{
}


CRFProtocolRubitek::~CRFProtocolRubitek()
{
}


string CRFProtocolRubitek::DecodePacket(const string&pkt)
{
	string packet = pkt, res;
	if (packet.length() == 49)
	{
		if (packet[48] == 'B')
			packet += "c";
		if (packet[48] == 'C')
			packet += "c";
	}
	else
		return "";

	for (unsigned int i = 0; i < packet.length() - 1; i+=2)
	{
		string part = packet.substr(i, 2);
		if (part == "Bc")
			res += "0";
		else if (part == "Cb")
			res += "1";
		else
			return 0;
	}

	return res;
}