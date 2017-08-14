#include "RFProtocol.h"

#include <algorithm>
#include <cstring>

#include "../libutils/DebugPrintf.h"
#include "../libutils/Exception.h"
#include "../libutils/logging.h"
#include "../libutils/strutils.h"

typedef std::string string;
using namespace strutils;

string c2s(char c)
{
    char tmp[2];
    tmp[0] = c;
    tmp[1] = 0;
    return tmp;
}

CRFProtocol::CRFProtocol(range_array_type zeroLengths, range_array_type pulseLengths, int bits,
                         int minRepeat, std::string PacketDelimeter)
    : m_ZeroLengths(zeroLengths), m_PulseLengths(pulseLengths), m_MinRepeat(minRepeat), m_Bits(bits),
      m_PacketDelimeter(PacketDelimeter), m_InvertPacket(true), m_CurrentRepeat(0)
{
    m_Debug = false;
    m_InvertPacket = false;
}


CRFProtocol::~CRFProtocol()
{
}

void CRFProtocol::SetTransmitTiming(const uint16_t *timings)
{
    m_SendTimingPauses = m_SendTimingPulses = timings;
    while (*m_SendTimingPulses++);
}

void CRFProtocol::ClearRetainedInputData() {
    m_CurrentRepeat = 0;
    m_InnerRepeat = 0;
    m_LastParsedTime = 0;
    m_LastParsed.clear();
}


string CRFProtocol::Parse(InputContainerIterator first, InputContainerIterator last, int64_t inputTime) {
    DPRINTF_DECLARE(dprintf, false);
    std::vector<base_type> input(first, last);
    dprintf("$P Got % data, try to parse TIME=%\n", input.size(), inputTime);
    m_InnerRepeat = 0;

    std::string parsed = Parse(first, last);

    if (parsed.empty())
        return "";
    if (parsed != m_LastParsed) {
        m_LastParsed = parsed;
        m_LastParsedTime = inputTime;
        m_CurrentRepeat = 0;
    }
    int64_t delayFromPreviousPacket = inputTime - m_LastParsedTime;
    m_LastParsedTime = inputTime;

    m_CurrentRepeat += m_InnerRepeat;

    dprintf("$P parsed % (% times of %)\n", parsed, m_CurrentRepeat, m_MinRepeat);
    if (delayFromPreviousPacket > MAX_DELAY_BETWEEN_PACKETS) {
        dprintf("$P parsed % (% times of %) - dropped to % repeat because of long delay (% > %)\n",
                parsed, m_CurrentRepeat, m_MinRepeat, m_InnerRepeat,
                delayFromPreviousPacket, MAX_DELAY_BETWEEN_PACKETS);
        m_CurrentRepeat = m_InnerRepeat;
    }

    if (m_CurrentRepeat >= m_MinRepeat) {
        m_CurrentRepeat = 0;
        return parsed;
    }
    return "";
}

/*
    Декодирование происходит в три этапа
*/
string CRFProtocol::Parse(InputContainerIterator first, InputContainerIterator last)
{
    DPRINTF_DECLARE(dprintf, false);

    Clean();
    //  Декодирование происходит в три этапа

    /*
        1 - разбиваем паузы и сигналы на группы в соответствии m_ZeroLengths и m_PulseLengths
            первое совпадение паузы с диапазоном из m_ZeroLengths декодируется как 'a', второе 'b' и т.д.
            первое совпадение сигнала с диапазоном из m_PulseLengths декодируется как 'A', второе 'A' и т.д.
            если не попали ни в один из диапазонов  - '?'
            результат работы этапа - строка вида "AaAbBaCc?d"
    */

    std::string decodedRaw = DecodeRaw(first, last);

    dprintf("$P - DecodeRaw returns: \'%\'\n", decodedRaw);

    if (!decodedRaw.length())
        return "";

    /*
        2 - разбиваем полученную строку на пакеты и пытаемся декодировать каждый пакет в набор бит
            результат работы функции - набор бит
    */
    string_vector rawPackets;
    SplitPackets(decodedRaw, rawPackets);
    //if ((int)rawPackets.size() < m_MinRepeat)
    //    return "";

    for (const std::string &packet : rawPackets)
        dprintf("$P \t\t rawPacket: \'%\'\n", packet);

    std::string bits = DecodeBits(rawPackets);
    dprintf("$P DecodeBits returns: \'%\'\n", bits);

    if (bits.length()) {
        //      3 - декодируем набор бит в осмысленные данные (команду, температуру etc)
        std::string data = DecodeData(bits);
        if (data.empty())
            return "";

        std::string res = getName() + ":" + data;
        return res;
    }

    if (needDump(decodedRaw))
        m_DumpPacket = true;

    return "";
}

string CRFProtocol::DecodeRaw(InputContainerIterator first, InputContainerIterator last)
{
    DPRINTF_DECLARE(dprintf, false);
    std::string decodedRaw;

    dprintf("$P signals to decode (first 50): ");
    for (auto i = first; i < first + 50 && dprintf.isActive(); i++)
        dprintf.c("%c%d (%d), ", (isPulse(*i) ? '+' : '-'), getLengh(*i), *i);
    dprintf("\n");

    for (auto i = first; i != last; i++) {
        base_type len = getLengh(*i);

        if (isPulse(*i) ^ m_InvertPacket) {
            int pos = 0;
            for (; m_PulseLengths[pos][0]; pos++) {
                if (len >= m_PulseLengths[pos][0] && len <= m_PulseLengths[pos][1]) {
                    //decodedRaw += c2s('A' + pos);
                    decodedRaw.push_back('A' + pos);
                    break;
                }
            }

            if (!m_PulseLengths[pos][0]) {
                if (m_Debug) // Если включена отладка - явно пишем длины плохих пауз
                    decodedRaw += string("[") + itoa(len) + "]";
                else {
                    decodedRaw.push_back('?');
                }
            }

        } else {
            int pos = 0;
            for (; m_ZeroLengths[pos][0]; pos++) {
                if (len >= m_ZeroLengths[pos][0] && len <= m_ZeroLengths[pos][1]) {
                    decodedRaw.push_back('a' + pos);
                    break;
                }
            }

            if (!m_ZeroLengths[pos][0]) {
                if (m_Debug)
                    decodedRaw += string("[") + itoa(len) + "]";
                else {
                    decodedRaw.push_back('?');
                }
            }
        }
    }
    dprintf("$P %\n", decodedRaw);
    return decodedRaw;
}

bool CRFProtocol::SplitPackets(const std::string &rawData, string_vector &rawPackets)
{
    String(rawData).Split(m_PacketDelimeter, rawPackets);
    return rawPackets.size() > 0;
}

string CRFProtocol::DecodeBits(string_vector &rawPackets)
{
    std::string res;
    int count = 0;

    for (const std::string &s : rawPackets) {
        std::string packet;
        size_t pos = s.find(m_Debug ? '[' : '?');
        if (pos != string::npos)
            packet = s.substr(0, pos);
        else
            packet = s;

        if (!packet.length())
            continue;

        std::string decoded = DecodePacket(packet);

        if (decoded == "")
            continue;

        if (m_Bits && (int)decoded.length() != m_Bits)
            continue;

        if (res.length()) {
            if (res == decoded) {
                if (++count >= m_MinRepeat)
                    break;
            } else {
                res = decoded;
                count = 1;
                if (m_MinRepeat == 1)
                    break;
            }
        } else {
            res = decoded;
            count = 1;
            if (m_MinRepeat == 1)
                break;
        }
    }
    m_InnerRepeat = count;
    if (res.length()) {
        return res;
        if (count >= m_MinRepeat)
            return res;
        else {
            m_DumpPacket = true;
            LOG(INFO) << "Decoded '" << res << "' but repeat " << count << " of " << m_MinRepeat;
        }
    }

    return "";
}

string CRFProtocol::DecodePacket(const std::string &raw)
{
    return raw;
}

string CRFProtocol::DecodeData(const std::string &raw)
{
    return raw;
}

unsigned long CRFProtocol::bits2long(const std::string &s, size_t start, size_t len)
{
    return bits2long(s.substr(start, len));
}

unsigned long CRFProtocol::bits2long(const std::string &raw)
{
    unsigned long res = 0;

    for (char c : raw) {
        res = res << 1;

        if (c == '1')
            res++;
    }

    return res;
}

string CRFProtocol::reverse(const std::string &s)
{
    std::string res = s;
    auto begin = res.begin(), end = res.end();
    while (begin + 1 < end) {
        --end;
        auto c = *begin;
        *begin = *end;
        *end = c;
        ++begin;
    }
    return res;
}

/*
    Декодер манчестера:
    raw - пакет
    expectPulse - ожидаем первым сигналом пульс
    shortPause - короткая пауза
    longPause - длинная пауза
    shortPulse - короткий сигнал
    longPulse - длинный сигнал
*/

string CRFProtocol::ManchesterDecode(const std::string &raw, bool expectPulse, char shortPause,
                                     char longPause, char shortPulse, char longPulse)
{
    enum t_state { expectStartPulse, expectStartPause, expectMiddlePulse, expectMiddlePause };

    t_state state = expectPulse ? expectStartPulse : expectStartPause;
    std::string res;
    res.reserve(75);

    char pulseBit = '1', pauseBit = '0';
    if (!expectPulse) {
        std::swap(pulseBit, pauseBit);
    }

    for (char c : raw) {
        switch (state) {
            case expectStartPulse:   // Ожидаем короткий пульс, всегда 1
                if (c == shortPulse) {
                    res.push_back(pulseBit);
                    state = expectMiddlePause;
                } else {
                    return "";
                }
                break;
            case expectStartPause:  // Ожидаем короткую паузу, всегда 0
                if (c == shortPause) {
                    res.push_back(pauseBit);
                    state = expectMiddlePulse;
                } else {
                    return "";
                }
                break;
            case expectMiddlePulse:  // Ожидаем пульс. Если короткий - пара закончилась, ждем короткую стартовую паузу. Если длинный, получили начало след пары и ждем среднюю паузу
                if (c == shortPulse) {
                    state = expectStartPause;
                } else if (c == longPulse) {
                    res.push_back(pulseBit);
                    state = expectMiddlePause;
                } else {
                    return "";
                }
                break;
            case expectMiddlePause:  // Ожидаем паузу. Если короткая - пара закончилась, ждем короткую стартовый пульс. Если длинная, получили начало след пары и ждем средний пульс
                if (c == shortPause) {
                    state = expectStartPulse;
                } else if (c == longPause) {
                    res.push_back(pauseBit);
                    state = expectMiddlePulse;
                } else {
                    return "";
                }
                break;
            default:
                return "";
        }
    }

    return res;
}

// заменяем <search><search> на <replace>
string replaceDouble(const std::string &src, char search, char replace)
{
    std::string res = src;
    char tmp[3];
    tmp[0] = tmp[1] = search;
    tmp[2] = 0;
    char tmp2[2];
    tmp2[0] = replace;
    tmp2[1] = 0;

    size_t pos;
    while ((pos = res.find(tmp)) != string::npos) {
        res = res.replace(pos, 2, tmp2);
    }

    return res;
}

/*
    Енкодер манчестера:
    raw - пакет
    invert - инвертированный манчестер
    shortPause - короткая пауза
    longPause - длинная пауза
    shortPulse - короткий сигнал
    longPulse - длинный сигнал
*/
string CRFProtocol::ManchesterEncode(const std::string &bits, bool invert, char shortPause,
                                     char longPause, char shortPulse, char longPulse)
{
    std::string res;
    res.reserve(bits.size() * 2);
    for (char c : bits) {
        if ((c == '1') ^ invert) {
            res.push_back(shortPulse);
            res.push_back(shortPause);
        } else {
            res.push_back(shortPause);
            res.push_back(shortPulse);
        }

    }

    size_t curPos = 1;
    for (size_t i = 1; i < res.size(); i++) {
        if (res[i] == res[curPos - 1]) {
            res[curPos - 1] = (res[i] == shortPause) ? longPause : longPulse;
        }
        else {
            res[curPos++] = res[i];
        }
    }
    res.resize(curPos);
    return res;
}

bool CRFProtocol::needDump(const std::string &)
{
    return false;
}

void CRFProtocol::EncodeData(const std::string &data, uint16_t bitrate, uint8_t *buffer,
                             size_t &bufferSize)
{
    EncodePacket(data2bits(data), bitrate, buffer, bufferSize);
}

// Кодируем пакет
// TODO: Синхронизировать bitrate с драйвером радио
void CRFProtocol::EncodePacket(const std::string &bits, uint16_t bitrate, uint8_t *buffer,
                               size_t &bufferSize)
{

    DPRINTF_DECLARE(dprintf, false);

    std::string timings = bits2timings(bits);
    dprintf("$P % -> %\n", bits, timings);

    uint16_t bitLen = 1000000L / bitrate;
    memset(buffer, 0, bufferSize);

    size_t bitNum = 0;
    for (const char i : timings) {
        bool pulse = i < 'a';
        uint16_t len = pulse ? m_SendTimingPulses[i - 'A'] : m_SendTimingPulses[i - 'a'];
        uint16_t bits = len /
                        bitLen; // TODO Округление для некратного битрейта?

        for (int j = 0; j < bits; j++) {
            if (pulse)
                buffer[bitNum >> 3] |= (1 << (7 - (bitNum & 7)));

            bitNum++;
        }
    }

    bufferSize = (bitNum + 7) >> 3;
}


string CRFProtocol::bits2timings(const std::string &)
{
    throw CHaException(CHaException::ErrNotImplemented, "CRFProtocol::bits2timings");
}

string CRFProtocol::data2bits(const std::string &)
{
    throw CHaException(CHaException::ErrNotImplemented, "CRFProtocol::data2bits");
}
/*
void CRFProtocol::getMinMax(base_type *minPause, base_type *maxPause, base_type *minPulse,
                            base_type *maxPulse)
{
    *minPause = *maxPause = m_ZeroLengths[0][0];
    *minPulse = *maxPulse = m_PulseLengths[0][1];

    int pos = 0;
    for (; m_PulseLengths[pos][0]; pos++) {
        *minPulse = std::min (*minPulse, m_PulseLengths[pos][0]);
        *maxPulse = std::max (*maxPulse, m_PulseLengths[pos][1]);
    }

    pos = 0;
    for (; m_ZeroLengths[pos][0]; pos++) {
        *minPause = std::min (*minPause, m_ZeroLengths[pos][0]);
        *maxPause = std::max (*maxPause, m_ZeroLengths[pos][1]);
    }
}*/

bool CRFProtocol::IsGoodSignal(base_type signal) {
    range_array_type lengths = isPulse(signal) ? m_PulseLengths : m_ZeroLengths;
    size_t len = getLengh(signal);
    for (int pos = 0; lengths[pos][0]; pos++) {
        if (lengths[pos][0] <= len && len <= lengths[pos][1])
            return true;
    }
    return false;
}

int CRFProtocol::SignedRepresentation(base_type signal) {
    return getLengh(signal) * (isPulse(signal) ? +1 : -1);
}
