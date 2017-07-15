#include "RFProtocolVhome.h"

#include "../libutils/strutils.h"
#include "../libutils/DebugPrintf.h"

typedef std::string string;
using namespace strutils;


CRFProtocolVhome::CRFProtocolVhome()
	: CRFProtocolEV1527()
{
	m_Debug = true;
}


CRFProtocolVhome::~CRFProtocolVhome()
{
}



string CRFProtocolVhome::DecodeData(const string& bits)
{
	DPRINTF_DECLARE(dprintf, false);
	
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
