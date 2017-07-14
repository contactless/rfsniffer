#include "RFProtocolVhome.h"

#include "../libutils/strutils.h"
#include "../libutils/DebugPrintf.h"

// 
static range_type g_timing_pause[7] =
{ 
	{ 1, 600 }, // lengths of signals are scattered very well 
	{ 601, 1400 },
	{ 2000, 20000 },
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 1, 700 },
	{ 1000, 1400 }, 
	{ 0, 0 }, 
	{ 0,0 }
};



CRFProtocolVhome::CRFProtocolVhome()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 24, 1, "Ac")
{
	m_Debug = true;
}


CRFProtocolVhome::~CRFProtocolVhome()
{
}


string CRFProtocolVhome::DecodePacket(const string &pkt)
{
	DPRINTF_DECLARE(dprintf, true);
	
	if (pkt.length() < 48)
		return "";
	
	dprintf("$P Started! got %\n", pkt);
	
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
		if (packet.length() > 0)
			dprintf("$P rejected %\n", packet);
		packet = "";
	}
	
	if (packet.length() == 24) {
		dprintf("$P ++++++++ Accepted ++++++++: %\n", packet);
		return packet;
	}

	dprintf("$P rejected %\n", packet);
	return "";
}

string CRFProtocolVhome::DecodeData(const string& bits)
{
	DPRINTF_DECLARE(dprintf, true);
	
	if (bits.length() != 24)
		return "";
	
	dprintf("$P Started! got %\n", bits);
	
	int cmd = bits2long(bits, 20, 4);
		
	
	dprintf("$P cmd = %\n", cmd);
		
	if (cmd > 4)
		return "";
	
	static const int map_1[] = {-1, -1, 1, -1, -1};
	static const int map_2[] = {-1,  1, -1, -1, 2};
	static const int map_3[] = {-1,  1,  2, -1, 3};
	
	bool is3btn = true;
	for (int i = 0; i < 9; i++)
		is3btn &= bits[i] == '1';
		
	
	dprintf("$P is3btn = %\n", is3btn);
		
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
		
	
		dprintf("$P btn in map_1 = %\n", btn);
		if (btn == -1) {
			btn = map_2[cmd];
			
			dprintf("$P btn in map_2 = %\n", btn);
			if (btn == -1)
				return "";
			return String::ComposeFormat("addr=%d type=2 btn=%d", addr, btn);
		}
		else {
			return String::ComposeFormat("addr=%d type=1 btn=1", addr);
		}
		
	}
		
}
