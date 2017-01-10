#include <algorithm>
#include "stdafx.h"
#include <fcntl.h>

#include "RFParser.h"
#include "RFProtocolLivolo.h"
#include "RFProtocolX10.h"
#include "RFProtocolRST.h"
#include "RFProtocolRaex.h"
#include "RFProtocolOregon.h"
#include "RFProtocolOregonV3.h"
#include "RFProtocolNooLite.h"
#include "RFProtocolRubitek.h"
#include "RFProtocolMotionSensor.h"
#include "RFAnalyzer.h"

using std::string;

CRFParser::CRFParser(CLog *log, string SavePath)
    : b_RunAnalyzer(false), m_Analyzer(NULL), m_Log(log), m_SavePath(SavePath),
      m_maxPause(0)
{
}


CRFParser::~CRFParser()
{
    for_each(CRFProtocolList, m_Protocols, i) {
        delete *i;
    }

    delete m_Analyzer;
}

void CRFParser::AddProtocol(string protocol)
{
    if (protocol == "X10")
        AddProtocol(new CRFProtocolX10());
    else if (protocol == "RST")
        AddProtocol(new CRFProtocolRST());
    else if (protocol == "Raex")
        AddProtocol(new CRFProtocolRaex());
    else if (protocol == "Livolo")
        AddProtocol(new CRFProtocolLivolo());
    else if (protocol == "Oregon") {
        AddProtocol(new CRFProtocolOregon());
        AddProtocol(new CRFProtocolOregonV3());
    } else if (protocol == "nooLite")
        AddProtocol(new CRFProtocolNooLite());
    else if (protocol == "Rubitek")
        AddProtocol(new CRFProtocolRubitek());
    else if (protocol == "MotionSensor")
        AddProtocol(new CRFProtocolMotionSensor());
    else if (protocol == "All") {
        AddProtocol(new CRFProtocolX10());
        AddProtocol(new CRFProtocolRST());
        AddProtocol(new CRFProtocolRaex());
        AddProtocol(new CRFProtocolLivolo());
        AddProtocol(new CRFProtocolOregon());
        AddProtocol(new CRFProtocolOregonV3());
        AddProtocol(new CRFProtocolNooLite());
        AddProtocol(new CRFProtocolRubitek());
        //AddProtocol(new CRFProtocolMotionSensor());
    } else
        throw CHaException(CHaException::ErrBadParam, protocol);

    setMinMax();
}

void CRFParser::AddProtocol(CRFProtocol *p)
{
    p->setLog(m_Log);
    m_Protocols.push_back(p);
    //  setMinMax();
}




std::vector<string> CRFParser::ParseToTheEnd(base_type *data, size_t length,
        size_t *readLengthToReturn)
{
    const base_type *dataBegin = data;
    std::vector<string> results;
    string str;

    // While ParseRepetitive can recognize smth
    // recognize and add result to "results".
    // Add only unique strings.
    // Notice that used Parse changes data and length
    while (!(str = Parse(&data, &length)).empty())
        if (std::find(results.begin(), results.end(), str) == results.end())
            results.push_back(str);

    *readLengthToReturn = data - dataBegin;
    return results;
}


// Tries to recognize packet from begin of data.
// If data was recognised then returned string have non-zero length,
// otherwise returned string is empty.
// In every case read length (or just skipped) will be written to "readLength"
string CRFParser::ParseRepetitive(base_type *data, size_t length, size_t *readLength)
{
    base_type *data2 = data;
    size_t length2 = length;
    string ret = Parse(&data2, &length2);
    *readLength = data2 - data;
    if (*readLength > length) {
        m_Log->Printf(4, "Error! (*readLength > length)");
        *readLength = length;
    }
    return ret;
}

string CRFParser::Parse(base_type **data_ptr, size_t *length_ptr)
{
    if (m_maxPause == 0)
        setMinMax();

    // I think references is more understandable
    base_type *&data = *data_ptr;
    size_t &length = *length_ptr;

    base_type *saveStart = data;
    base_type splitDelay = m_maxPause * 10 / 8;
    base_type splitPulse = m_minPulse / 2;

    for(base_type *ptr = data; ptr - data < length; ptr++) {
        bool isPulse = CRFProtocol::isPulse(*ptr);
        base_type len = CRFProtocol::getLengh(*ptr);
        bool badInterval1 = ((!isPulse && len > splitDelay) || (isPulse && len < splitPulse));
        bool badInterval2 =
            (( isPulse && (len < m_minPulse || len > m_maxPulse)) ||
             (!isPulse && (len < m_minPause || len > m_maxPause)));
        if (badInterval2) {
            size_t packetLen = ptr - data;
            if (packetLen > 50)
                m_Log->Printf(4, "Parse part of packet from %ld size %ld splitted by %c%ld", data - saveStart,
                              packetLen, (isPulse ? '+' : '-'), len);

            //string res = Parse(data, packetLen);
            // Important feature with: packetLen * 2 + 20
            // it's enable tests to work, because some protocols repeats message twice
            // TODO (?) change to maximum repeat count
            string res = Parse(data, std::min(packetLen * 2 + 20, length));

            data += packetLen + 1;
            length -= packetLen + 1;

            if (res.length())
                return res;
        }
    }

    if (length > MIN_PACKET_LEN) {
        string res = Parse(data, length);
        if (res.length()) {
            data += length;
            length = 0;
            return res;
        }
    }

    return "";
}

string CRFParser::Parse(base_type *data, size_t len)
{
    if (len < MIN_PACKET_LEN)
        return "";

    // Пытаемся декодировать пакет каждым декодером по очереди
    for (CRFProtocol *protocol : m_Protocols) {
        string retval = protocol->Parse(data, len);
        if (retval.length())
            return retval;  // В случае успеха возвращаем результат
    }

    // В случае неуспеха пытаемся применить анализатор
    if (b_RunAnalyzer) {
        if (!m_Analyzer)
            m_Analyzer = new CRFAnalyzer(m_Log);

        m_Analyzer->Analyze(data, len);
    }

    // Если указан путь для сохранения - пишем в файл пакет, который не смогли декодировать
    if (m_SavePath.length()) {
        for (CRFProtocol *protocol : m_Protocols) {
            if (protocol->needDumpPacket()) {
                m_Log->Printf(3, "Dump packet for %s", protocol->getName().c_str());
                SaveFile(data, len);
                return "";
            }
        }
    }

    return "";
}





void CRFParser::EnableAnalyzer()
{
    b_RunAnalyzer = true;
}

void CRFParser::SaveFile(base_type *data, size_t size)
{
#ifndef WIN32
    if (m_SavePath.length()) {
        time_t Time = time(NULL);
        char DateStr[100], FileName[1024];
        strftime(DateStr, sizeof(DateStr), "%d%m-%H%M%S", localtime(&Time));
        snprintf(FileName, sizeof(FileName),  "%s/capture-%s.rcf", m_SavePath.c_str(), DateStr);
        m_Log->Printf(3, "Write to file %s %ld signals\n", FileName, size);
        int of = open(FileName, O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP);

        if (of == -1) {
            m_Log->Printf(3, "error opening %s\n", FileName);
            return;
        };

        size_t ignore_ = write(of, data, sizeof(data[0]) * size);
        close(of);
    }
#endif
}

void CRFParser::SetSavePath(string SavePath)
{
    m_SavePath = SavePath;
}


void CRFParser::setMinMax()
{
    bool first = true;
    for (CRFProtocol *protocol : m_Protocols) {
        if (first) {
            protocol->getMinMax(&m_minPause, &m_maxPause, &m_minPulse, &m_maxPulse);
            first = false;
        } else {
            base_type minPause, maxPause, minPulse, maxPulse;
            protocol->getMinMax(&minPause, &maxPause, &minPulse, &maxPulse);
            m_minPause = std::min(m_minPause, minPause);
            m_maxPause = std::max(m_maxPause, maxPause);
            m_minPulse = std::min(m_minPulse, minPulse);
            m_maxPulse = std::max(m_maxPulse, maxPulse);
        }
    }
    m_Log->Printf(3, "CRFProtocol decoders use pauses %ld-%ld pulses %ld-%ld", m_minPause, m_maxPause,
                  m_minPulse, m_maxPulse);
}
