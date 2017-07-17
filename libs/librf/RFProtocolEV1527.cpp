#include "RFProtocolEV1527.h"

#include "../libutils/strutils.h"
#include "../libutils/DebugPrintf.h"

typedef std::string string;
using namespace strutils;

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
	{ 1, 600 },
	{ 601, 1400 }, 
	{ 0, 0 }, 
	{ 0,0 }
};



CRFProtocolEV1527::CRFProtocolEV1527()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 24, 1, "Ac")
{
	m_Debug = true;
}


CRFProtocolEV1527::~CRFProtocolEV1527()
{
}


string CRFProtocolEV1527::DecodePacket(const std::string &pkt)
{
	DPRINTF_DECLARE(dprintf, false);
	
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

string CRFProtocolEV1527::DecodeData(const string& bits)
{

	if (bits.length() != 24)
		return "";
	
	int cmd = bits2long(bits, 20, 4);
	int addr = bits2long(bits, 0, 16);
	
	return String::ComposeFormat("addr=%d cmd=%d", addr, cmd);
}
