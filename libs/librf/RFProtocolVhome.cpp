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
    int addr = bits2long(bits, 0, 16);
	
	dprintf("$P cmd = %\n", cmd);
		
	if (cmd > 8)
		return "";
	
	static const int map[] = {-1, 1, 2, -1, 3, -1, -1, -1, 4};
	
		
    int btn = map[cmd];
    
	if (btn == -1)
        return "";
    return String::ComposeFormat("addr=%d btn=%d", addr, btn);
}
