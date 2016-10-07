#include "stdafx.h"
#include "RFProtocolOregon.h"

static range_type g_timing_pause[7] =
{
	{ 40000, 47000 },
	{ 380, 850 },
	{ 851, 1400 },
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 1101, 1101 },
	{ 200, 615 },
	{ 615, 1100 },
	{ 0,0 }
};


CRFProtocolOregon::CRFProtocolOregon()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 0, 1, "cCcCcCcCcCcCcCcCcCcCcCcCcCcCcCbBc")
{
}


CRFProtocolOregon::~CRFProtocolOregon()
{
}


string CRFProtocolOregon::DecodePacket(const string& raw)
{
	if (raw.length() < 10)
		return "";

	string packet="0";
	char next=0;
	bool second = false;
	bool expectPause = false;

	for_each_const(string, raw, c)
	{
		if (*c == 'a')
			break;

		if (expectPause)
		{
			if (*c!='b' && *c!='c')
				return "";
		}
		else
		{
			if (*c != 'B' && *c != 'C')
				return "";
		}

		expectPause ^= 1;

		if (next)
		{
			if (*c != next)
				return "";
			else
			{
				next = 0;
				continue;
			}
		}

		switch (*c)
		{
		case 'b':
			if (second)
				return "";

			packet += '0';
			next = 'B';
			second = true;
			break;
		case 'B':
			if (second)
				return "";

			packet += '1';
			next = 'b';
			second = true;
			break;
		case 'c':
			if (second)
				second = false;
			else
			{
				packet += '1';
				second = true;
			}
			break;
		case 'C':
			if (second)
				second = false;
			else
			{
				packet += '0';
				second = true;
			}
			break;
		default:
			return "";
		}
	}

	unsigned int crc = 0, originalCRC = -1;
	string hexPacket="";

	if (packet.length()<48)
	{
		m_Log->Printf(3, "Too short packet %s", packet.c_str());
		return "";
	}

	int len = packet.length();
	while (len%4)
		len--;

	for (int i = 0; i < len; i += 4)
	{
		string portion = reverse(packet.substr(i, 4));
		char buffer[20];
		unsigned int val = bits2long(portion);

		if (i > 0 && i < len - 16)
			crc += val;
		else if (i == len - 16)
			originalCRC = val;
		else if (i == len - 12)
			originalCRC += val << 4;

		snprintf(buffer, sizeof(buffer), "%X", val);
		hexPacket += buffer;
	}

	if (crc != originalCRC)
	{
		m_Log->Printf(3, "Bad CRC for %s", packet.c_str());
		return "";
	}

	if (hexPacket[0] != 'A')
	{
		m_Log->Printf(4, "First nibble is not 'A'. Data: %s", hexPacket.c_str());
		return "";
	}

	return hexPacket;
}


string CRFProtocolOregon::DecodeData(const string& packet) // Преобразование бит в данные
{
	string type = packet.substr(1, 4);
	if (type=="1D20" || type=="F824" || type=="F8B4")
	{
		/*
		  ID   C RC F TTT - H  ? CRC
		a 1d20 1 51 8 742 0 64 4 a3 0a
		a 1d20 1 51 0 222 0 83 8 03 fb
		*/
		int channel = packet[5]-'0';
		int id = ((packet[7]-'0')<<4)+ (packet[6] - '0');
		float temp = (float)0.1*atoi(reverse(packet.substr(9, 3)));
		if (packet[12] != '0')
			temp *= -1;
		int hum = atoi(reverse(packet.substr(13, 2)));
		char buffer[40];
		snprintf(buffer, sizeof(buffer), "type=%s id=%02X ch=%d t=%.1f h=%d", type.c_str(), id, channel, temp, hum);
		return buffer;
	}
	else if (type == "EC40" || type == "C844" )
	{
		int channel = packet[5]-'0';
		int id = ((packet[6]-'0')<<4)+ (packet[7] - '0');
		float temp = (float)0.1*atoi(reverse(packet.substr(9, 3)));
		if (packet[12] != '0')
			temp *= -1;
		char buffer[40];
		snprintf(buffer, sizeof(buffer), "type=%s id=%02X ch=%d t=%.1f", type.c_str(), id, channel, temp);
		return buffer;
	}
	else
	{
		m_Log->Printf(4, "Unknown sensor type %s. Data: %s", type.c_str(), packet.c_str());
		return "raw:" + packet;
	}

	return "";
	/*	unsigned long l1 = bits2long(packet.substr(0, 32));
	unsigned long l2 = bits2long(packet.substr(32, 32));
	unsigned long l3 = bits2long(packet.substr(64, 32));

	unsigned long l4 = bits2long(packet2.substr(0, 32));
	unsigned long l5 = bits2long(packet2.substr(32, 32));
	unsigned long l6 = bits2long(packet2.substr(64, 32));

	char Buffer[200];
	snprintf(Buffer, sizeof(Buffer), "%08X%08X%04X", l1, l2, l3);
	string res = Buffer;
	snprintf(Buffer, sizeof(Buffer), "%08X%08X%04X", l4, l5, l6);
	return res+"_"+reverse(Buffer);*/

}


bool CRFProtocolOregon::needDump(const string &rawData)
{
	return rawData.find(m_PacketDelimeter) != rawData.npos;
//	return rawData.find("cCcCcCcC") != rawData.npos;
}
