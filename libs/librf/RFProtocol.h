#pragma once
#include "stdafx.h"
#include "rflib.h"


//  Константы lirc
#define PULSE_BIT       0x01000000
#define PULSE_MASK      0x00FFFFFF

// В качестве базового типа для lirc используем ulong
typedef unsigned long base_type;
typedef  base_type range_type[2];
typedef  const range_type *range_array_type;

string c2s(char c);

class RFLIB_API CRFProtocol
{
protected:
	range_array_type m_ZeroLengths, m_PulseLengths;
	int m_MinRepeat, m_Bits;
	string m_PacketDelimeter;
	bool m_Debug, m_DumpPacket, m_InvertPacket;
	CLog *m_Log;
	const uint16_t *m_SendTimingPauses;
	const uint16_t *m_SendTimingPulses;

	string ManchesterDecode(const string&, bool expectPulse, char shortPause, char longPause, char shortPulse, char longPulse);
	string ManchesterEncode(const string&, bool invert, char shortPause, char longPause, char shortPulse, char longPulse);
	virtual void Clean() { m_DumpPacket = false; };
	virtual bool needDump(const string &rawData);
	void SetTransmitTiming(const uint16_t *timings);

public:
/*
	zeroLengths - Массив длинн пауз.
	pulseLengths - Массив длинн сигналов
	bits - размер пакета (0 - не проверять)
	minRepeat - минимальное число одинаковых повторений, чтобы считать сигнал корректным
	PacketDelimeter - разделитель пакетов в терминах буквенных констант
*/
	CRFProtocol(range_array_type zeroLengths, range_array_type pulseLengths, int bits, int minRepeat, string PacketDelimeter );
	virtual ~CRFProtocol();

	// Раскодируем пакет
	virtual string Parse(base_type*, size_t len);
	virtual string DecodeRaw(base_type* data, size_t dataLen);  // Декодироавние строки по длинам
	virtual bool SplitPackets(const string &rawData, string_vector& rawPackets); // Нарезка по пакетам
	virtual string DecodeBits(string_vector&rawPackets); // Сборка бит по массиву пакетов
	virtual string DecodePacket(const string&); // Преобразование строки длин в биты
	virtual string DecodeData(const string&); // Преобразование бит в данные

	// Кодируем пакет
	virtual void EncodeData(const string &data, uint16_t bitrate,  uint8_t *buffer, size_t &bufferSize);
	virtual void EncodePacket(const string &bits, uint16_t bitrate, uint8_t *buffer, size_t &bufferSize);
	virtual string bits2timings(const string &bits);
	virtual string data2bits(const string &data);

	//  Вспомогательные функции
	static inline bool isPulse(base_type v) { return (v&PULSE_BIT) != 0; };
	static inline base_type getLengh(base_type v) { return v&PULSE_MASK; };
	virtual string getName() = 0;
	bool needDumpPacket() {	return m_DumpPacket; };

	unsigned long bits2long(const string&); 
	unsigned long bits2long(const string&, size_t start, size_t len); 
	string reverse(const string&);
	void setLog(CLog *log) { m_Log = log; };
	void getMinMax(base_type* minPause, base_type* maxPause, base_type* minPulse, base_type* maxPulse);
};

