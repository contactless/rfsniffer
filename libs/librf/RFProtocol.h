#pragma once
#include <string>
#include <vector>
#include <deque>

#include "rflib.h"
#include "../libutils/strutils.h"


//  Константы lirc
#define PULSE_BIT       0x01000000
#define PULSE_MASK      0x00FFFFFF

// В качестве базового типа для lirc используем ulong
typedef uint32_t base_type;
typedef base_type range_type[2];
typedef const range_type *range_array_type;

std::string c2s(char c);

class RFLIB_API CRFProtocol
{
    typedef std::deque<base_type>::iterator InputContainerIterator;
    typedef std::vector<strutils::String> string_vector;
  protected:
    range_array_type m_ZeroLengths, m_PulseLengths;
    int m_MinRepeat, m_Bits;
    std::string m_PacketDelimeter;
    bool m_Debug, m_DumpPacket, m_InvertPacket;
    const uint16_t *m_SendTimingPauses;
    const uint16_t *m_SendTimingPulses;
    
    int m_CurrentRepeat, m_InnerRepeat;
    int64_t m_LastParsedTime;
    std::string m_LastParsed;
    
    static const int MAX_DELAY_BETWEEN_PACKETS = 500000;

    std::string ManchesterDecode(const std::string &, bool expectPulse, char shortPause, char longPause,
                            char shortPulse, char longPulse);
    std::string ManchesterEncode(const std::string &, bool invert, char shortPause, char longPause,
                            char shortPulse, char longPulse);
    virtual void Clean()
    {
        m_DumpPacket = false;
    };
    virtual bool needDump(const std::string &rawData);
    void SetTransmitTiming(const uint16_t *timings);

  public:
  
    void ClearRetainedInputData();
    
    /*
        zeroLengths - Массив длинн пауз.
        pulseLengths - Массив длинн сигналов
        bits - размер пакета (0 - не проверять)
        minRepeat - минимальное число одинаковых повторений, чтобы считать сигнал корректным
        PacketDelimeter - разделитель пакетов в терминах буквенных констант
    */
    CRFProtocol(range_array_type zeroLengths, range_array_type pulseLengths, int bits, int minRepeat,
                std::string PacketDelimeter );
                
    virtual ~CRFProtocol();
    
    void checkInverted(bool inverted)
    {
        m_InvertPacket = inverted;
    };

    // Раскодируем пакет
    std::string Parse(InputContainerIterator first, InputContainerIterator last, int64_t inputTime);
    
    virtual std::string Parse(InputContainerIterator first, InputContainerIterator last);
    virtual std::string DecodeRaw(InputContainerIterator first, 
                             InputContainerIterator last);  // Декодирование строки по длинам
    virtual bool SplitPackets(const std::string &rawData,
                              string_vector &rawPackets); // Нарезка по пакетам
    virtual std::string DecodeBits(string_vector
                              &rawPackets); // Сборка бит по массиву пакетов
    virtual std::string DecodePacket(const std::string
                                &); // Преобразование строки длин в биты
    virtual std::string DecodeData(const std::string &); // Преобразование бит в данные

    // Кодируем пакет
    virtual void EncodeData(const std::string &data, uint16_t bitrate,  uint8_t *buffer, size_t &bufferSize);
    virtual void EncodePacket(const std::string &bits, uint16_t bitrate, uint8_t *buffer,
                              size_t &bufferSize);
    virtual std::string bits2timings(const std::string &bits);
    virtual std::string data2bits(const std::string &data);

    //  Вспомогательные функции
    static inline bool isPulse(base_type v)
    {
        return (v & PULSE_BIT) != 0;
    };
    static inline base_type getLengh(base_type v)
    {
        return v & PULSE_MASK;
    };
    virtual std::string getName() = 0;
    bool needDumpPacket()
    {
        return m_DumpPacket;
    };

    unsigned long bits2long(const std::string &);
    unsigned long bits2long(const std::string &, size_t start, size_t len);
    static std::string reverse(const std::string &);

    //void getMinMax(base_type *minPause, base_type *maxPause, base_type *minPulse, base_type *maxPulse);
    bool IsGoodSignal(base_type signal);
    /// pulse: +pulse_len, pause: -pause_len
    static int SignedRepresentation(base_type signal);
};

