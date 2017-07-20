#include "RFParser.h"

#include <algorithm>
#include <fcntl.h>
#include <unistd.h>

#include "RFProtocolLivolo.h"
#include "RFProtocolX10.h"
#include "RFProtocolRST.h"
#include "RFProtocolRaex.h"
#include "RFProtocolOregon.h"
#include "RFProtocolOregonV3.h"
#include "RFProtocolNooLite.h"
#include "RFProtocolRubitek.h"
#include "RFProtocolMotionSensor.h"
#include "RFProtocolHS24Bit.h"
#include "RFProtocolVhome.h"
#include "RFProtocolEV1527.h"

#include "../libutils/DebugPrintf.h"
#include "../libutils/Exception.h"
#include "../libutils/logging.h"

using std::string;

CRFParser::CRFParser()
{
    m_InputTime = 0;
}


CRFParser::~CRFParser()
{
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
    else if (protocol == "VHome")
        AddProtocol(new CRFProtocolVhome());
    else if (protocol == "EV1527")
        AddProtocol(new CRFProtocolEV1527());
    else if (protocol == "All") {
        AddProtocol(new CRFProtocolNooLite());
        AddProtocol(new CRFProtocolX10());
        AddProtocol(new CRFProtocolRST());
        AddProtocol(new CRFProtocolRaex());
        AddProtocol(new CRFProtocolLivolo());
        AddProtocol(new CRFProtocolOregon());
        AddProtocol(new CRFProtocolOregonV3());
        AddProtocol(new CRFProtocolRubitek());
        AddProtocol(new CRFProtocolHS24Bit());
        //AddProtocol(new CRFProtocolVhome());
        AddProtocol(new CRFProtocolEV1527());
        //AddProtocol(new CRFProtocolMotionSensor());
    } else
        throw CHaException(CHaException::ErrBadParam, "AddProtocol - no such protocol: " + protocol);
}

void CRFParser::AddProtocol(CRFProtocol *p)
{
    m_Protocols.push_back(std::unique_ptr<CRFProtocol>(p));
    m_ProtocolsBegins.push_back(0);
}


void CRFParser::ClearRetainedInputData() {
    m_ProtocolsBegins.assign(m_ProtocolsBegins.size(), 0);
    m_InputData.clear();
    m_InputTime = 0;
    m_ParsedResults.clear();
    for (auto &protocol : m_Protocols) {
        protocol->ClearRetainedInputData();
    }
}

string CRFParser::Parse(base_type *data, size_t len)
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Parse begin (len = %)\n", len);
    AddInputData(data, len);
    TryToParseExistingData();
    dprintf("$P Extracting parsed\n");
    auto ret = ExtractParsed();

    ClearRetainedInputData();

    dprintf("$P Extracted %\n", ret);
    if (ret.size() != 0) {
        std::string top = ret.front();
        dprintf("$P Just before return\n");
        return top;
    }
    return "";
}

void CRFParser::TryToParseExistingData() {
    DPRINTF_DECLARE(dprintf, false);

    if (m_InputData.size() < MIN_PACKET_LEN)
        return;

    bool lastWasPulse = CRFProtocol::isPulse(m_InputData.back());
    m_InputData.push_back((lastWasPulse ? 0 : PULSE_BIT) | PULSE_MASK);
    m_InputData.push_back((!lastWasPulse ? 0 : PULSE_BIT) | PULSE_MASK);

    bool beginWasShifted = false;

    for (int i = 0; i < (int)m_Protocols.size(); i++) {
        auto &protocol = m_Protocols[i];
        int &protocolBegin = m_ProtocolsBegins[i];

        int packetLen = m_InputData.size() - protocolBegin;

        if (packetLen < MIN_PACKET_LEN) {
            continue;
        }

        std::string parsed;

        try {
            parsed = protocol->Parse(
                    m_InputData.begin() + protocolBegin,
                    m_InputData.end(), m_InputTime);
        }
        catch (...) {
            LOG(CRIT) << "Error in parser. Protocol: " << protocol->getName();
            continue;
        }

        dprintf("$P Parsed = \'%\'\n", parsed);

        if (!parsed.empty()) {
            m_ParsedResults.push_back(parsed);
            protocolBegin = m_InputData.size();
        }
    }


    m_InputData.pop_back();
    m_InputData.pop_back();

    if (beginWasShifted) {
        int newBegin = *std::min_element(m_ProtocolsBegins.begin(), m_ProtocolsBegins.end());
        std::for_each(m_ProtocolsBegins.begin(), m_ProtocolsBegins.end(),
                [newBegin](int &beg) { beg -= newBegin; });
        m_InputData.erase(m_InputData.begin(), m_InputData.begin() + newBegin);
    }
}

// add some data to parse
void CRFParser::AddInputData(base_type signal)
{
    DPRINTF_DECLARE(dprintf, false);
    m_InputData.push_back(signal);
    m_InputTime += CRFProtocol::getLengh(signal);

    //dprintf("$P Start\n");

    bool beginWasShifted = false;

    for (int i = 0; i < (int)m_Protocols.size(); i++) {
        auto &protocol = m_Protocols[i];
        int &protocolBegin = m_ProtocolsBegins[i];

        int packetLen = m_InputData.size() - protocolBegin;
        //dprintf("$P protocol=%, sig=%, good=%\n", protocol->getName(), CRFProtocol::SignedRepresentation(signal), (protocol->IsGoodSignal(signal) ? "YES" : "NO"));
        if (!protocol->IsGoodSignal(signal)) {

            beginWasShifted = true;
            if (packetLen < MIN_PACKET_LEN) {
                protocolBegin = m_InputData.size();
                continue;
            }

            dprintf("$P Add input data, try decode packet (protocol = %, len = %)\n",
                    protocol->getName(), packetLen);

            std::string parsed = protocol->Parse(m_InputData.begin() + protocolBegin,
                                            m_InputData.end(), m_InputTime);
            dprintf("$P Parsed = \'%\'\n", parsed);

            if (!parsed.empty()) {
                m_ParsedResults.push_back(parsed);
            }

            protocolBegin = m_InputData.size();
        }
        else {
            if (packetLen > MAX_PACKET_LEN) {
                protocolBegin = m_InputData.size();
                beginWasShifted = true;
            }
        }
    }

    if (beginWasShifted) {
        int newBegin = *std::min_element(m_ProtocolsBegins.begin(), m_ProtocolsBegins.end());
        std::for_each(m_ProtocolsBegins.begin(), m_ProtocolsBegins.end(),
                [newBegin](int &beg) { beg -= newBegin; });
        m_InputData.erase(m_InputData.begin(), m_InputData.begin() + newBegin);
    }

    //dprintf("$P Finished\n");

}

// add some data to parse
void CRFParser::AddInputData(base_type *data, size_t len)
{
    for (base_type *i = data; i != data + len; i++)
        AddInputData(*i);
}
// get all results parsed from all data given by AddInputData
std::vector<string> CRFParser::ExtractParsed()
{
    std::vector<string> parsed;
    std::swap(parsed, m_ParsedResults);
    return parsed;
}

string CRFParser::GenerateFileName(string prefix, std::string savePath) {

#ifndef WIN32
    static int internalNumber = 0;
    time_t Time = time(NULL);
    char DateStr[100], FileName[1024];
    strftime(DateStr, sizeof(DateStr), "%d%m-%H%M%S", localtime(&Time));
    snprintf(FileName, sizeof(FileName),  "%s/%s-%s-%03d.rcf", savePath.c_str(),
             prefix.c_str(), DateStr, (++internalNumber) % 1000);
    return FileName;
#endif
}

void CRFParser::SaveFile(base_type *data, size_t size, const char *prefix, std::string savePath)
{
#ifndef WIN32
    if (savePath.length()) {
        std::string FileName = GenerateFileName(prefix, savePath);
        LOG(INFO) << "Write to file " << FileName << " " << size << " signals";
        int of = open(FileName.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP);

        if (of == -1) {
            LOG(INFO) << "error opening " << FileName;
            return;
        };

        size_t ret = write(of, data, sizeof(data[0]) * size);

        close(of);

        if (ret == -1) {
            throw CHaException(CHaException::ErrSystemAPIError, "Can't save file");
        }

    }
#endif
}

