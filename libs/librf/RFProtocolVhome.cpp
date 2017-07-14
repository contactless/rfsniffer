#include "RFProtocolVhome.h"

// 
static range_type g_timing_pause[7] =
{ 
	{ 1, 800 }, // lengths of signals are scattered very well 
	{ 801, 1400 },
	{ 20000, 2000 },
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 1, 700 },
	{ 1000, 1400 }, // Короткий
	{ 0, 0 }, // Длинный
	{ 0,0 }
};



CRFProtocolVhome::CRFProtocolVhome()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 24, 1, "a")
{
	m_Debug = true;
}


CRFProtocolVhome::~CRFProtocolVhome()
{
}


string CRFProtocolVhome::DecodePacket(const string &pkt)
{
	if (pkt.length() < 50)
		return "";
	
	string packet;
	
	for (int i = 0; i + 1 < (int)pkt.length(); i++) {
		if (pkt[i] == 'A' && pkt[i + 1] == 'b') {
			packet.push_back('0');
			i++;
			continue;
		}
		if (pkt[i] == 'B' && pkt[i + 1] == 'a') {
			packet.push_back('1');
			i++;
			continue;
		}
		if (pkt[i] == 'A' && pkt[i + 1] == 'c' && packet.length() == 24) {
			return packet;
		}
		return "";
	}
	return "";
}

string CRFProtocolVhome::DecodeData(const string& bits)
{
	if (bits.length() != 24)
		return "";
	
	int cmd = bits2long(bits, 20, 4);
		
	if (cmd > 4)
		return "";
	
	static const int map_1[] = {-1, -1, 1, -1, -1};
	static const int map_2[] = {-1,  2, -1, -1, 1};
	static const int map_3[] = {-1,  1,  2, -1, 3};
	
	bool is3btn = true;
	for (int i = 0; i < 9; i++)
		is3btn &= bits[i] == 1;
		
	if (is3btn) {
		int addr = bits2long(bits, 9, 7);
		int btn = map_3[cmd];
		if (btn == -1)
			return "";
		return String::ComposeFormat("addr=%d type=3 btn=%d", addr, btn);
	}
	else {
		int addr = bits2long(bits, 0, 16);
		int btn = map_1[cmd];
		if (btn != -1) {
			btn = map_2[cmd];
			if (btn == -1)
				return "";
			return String::ComposeFormat("addr=%d type=2 btn=%d", addr, btn);
		}
		else {
			return String::ComposeFormat("addr=%d type=1 btn=1", addr);
		}
		
	}
		
}
