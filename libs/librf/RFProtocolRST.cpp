#include "stdafx.h"
#include "RFProtocolRST.h"

static range_type g_timing_pause[7] =
{
//	{ 70,310 },
//	{ 380,580 },
	{ 7800, 8300 },
	{ 800,1200 },
	{ 1800, 2200 },
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	//{ 7800, 8300 },
	//{ 800,1200 },
	//{ 1800, 2200 },
	{90, 210},
	{ 0,0 }
};


CRFProtocolRST::CRFProtocolRST()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 36, 2, "a")
{
}


CRFProtocolRST::~CRFProtocolRST()
{
}

string CRFProtocolRST::DecodePacket(const string&packet)
{
	string bits; 

	for_each_const(string, packet, s)
	{
		if (*s == 'b' )
		{
			bits += "0";
		}
		else if (*s == 'c')
		{
			bits += "1";
		}
		else if (*s == 'A')
		{
			continue;
		}
		else
			return "";
	}

	return bits;
}

string CRFProtocolRST::DecodeData(const string &raw)
{
	char buffer[100];
	float t = 0.1*bits2long(raw.substr(24));
	short h = (short)bits2long(raw.substr(16, 8));
	unsigned short id = (unsigned short)bits2long(raw.substr(0, 16));

	snprintf(buffer, sizeof(buffer), "id=%x h=%d t=%.1f", id, h, t);

	return buffer;
}