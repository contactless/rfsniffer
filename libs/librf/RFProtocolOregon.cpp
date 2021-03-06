#include "RFProtocolOregon.h"

#include "../libutils/DebugPrintf.h"
#include "../libutils/logging.h"

typedef std::string string;
using namespace strutils;

// Class that extract fields of specified device
// from sequence that is decoded Oregon Protocol
class OregonRFDevice
{
  public:
    enum SystemOfUnits {
        SI, // System International
        US  // Imperial or US system of units
    };
  private:
    std::vector<string> sensor_ids;

    // relative offsets from begin of sensor-specific data
    int temperature_offset;
    int humidity_offset;
    int ultraviolet_offset;
    int wind_offset;
    int rain_offset;
    int pressure_offset;
    int forecast_offset;
    int comfort_offset;

    SystemOfUnits rain_system_of_units;

    template <typename IteratorType, typename Type>
    static bool IsThere(const Type &val, IteratorType begin, IteratorType end)
    {
        for (IteratorType it = begin; it != end; it++)
            if (*it == val)
                return true;
        return false;
    }

    static int ExtractInt(const std::string &str, int offset, int length)
    {
        return atoi(CRFProtocol::reverse(str.substr(offset, length)));
    }

  public:

    OregonRFDevice &AddTemperatureField(int offset)
    {
        temperature_offset = offset;
        return *this;
    }

    OregonRFDevice &AddHumidityField(int offset)
    {
        humidity_offset = offset;
        return *this;
    }

    OregonRFDevice &AddUltravioletField(int offset)
    {
        ultraviolet_offset = offset;
        return *this;
    }

    OregonRFDevice &AddWindField(int offset)
    {
        wind_offset = offset;
        return *this;
    }

    OregonRFDevice &AddRainField(int offset, SystemOfUnits system_of_units)
    {
        rain_offset = offset;
        rain_system_of_units = system_of_units;
        return *this;
    }

    OregonRFDevice &AddPressureField(int offset)
    {
        pressure_offset = offset;
        return *this;
    }

    OregonRFDevice &AddForecastField(int offset)
    {
        forecast_offset = offset;
        return *this;
    }

    OregonRFDevice &AddComfortField(int offset)
    {
        comfort_offset = offset;
        return *this;
    }

    // returns either decoded data or "" if failed
    std::string DecodeData(string packet) const
    {

        //fprintf(stderr, "Decode_data BEGIN\n");
        packet = packet.substr(1, packet.size() - 1); // Shift to conform this article
        // http://www.altelectronics.co.uk/wp-content/uploads/2011/05/OregonScientific-RF-Protocols.pdf

        std::string sensor_id = packet.substr(0, 4); // Sensor ID
        //fprintf(stderr, "Sensor_id: %s\n", sensor_id.c_str());
        if (!IsThere(sensor_id, sensor_ids.begin(), sensor_ids.end()))
            return "";

        int channel = packet[4] - '0';  // Channel
        // Value, changing randomly every time when reset (its name in original code - "id")
        int rolling_code = ((packet[5] - '0') << 4) + (packet[6] - '0');
        int flags = packet[7] - '0';
        const int data_offset = 8;

        BufferWriter buffered_writer;
        buffered_writer.printf("type=%s id=%02X ch=%d low_bat=%s",
                               sensor_id.c_str(), rolling_code, channel, ((flags & 0x4) ? "1" : "0"));

        if (temperature_offset != -1) {
            int offset = data_offset + temperature_offset;
            if (offset + 4 > (int)packet.size())
                return "";
            float temp = (float)0.1 * ExtractInt(packet, offset, 3) *
                         (packet[offset + 3] == '0' ? 1 : -1);
            buffered_writer.printf(" t=%.1f", temp);
        }
        if (humidity_offset != -1) {
            int offset = data_offset + humidity_offset;
            if (offset + 2 > (int)packet.size())
                return "";
            int humidity = ExtractInt(packet, offset, 2);
            buffered_writer.printf(" h=%d", humidity);
        }
        if (ultraviolet_offset != -1) {
            int offset = data_offset + ultraviolet_offset;
            if (offset + 2 > (int)packet.size())
                return "";
            int ultraviolet = ExtractInt(packet, offset, 2);
            buffered_writer.printf(" uv=%d", ultraviolet);
        }
        if (rain_offset != -1) {
            int offset = data_offset + rain_offset;
            // Length of blocks, its different for different systems of units
            // For SI rain rate given as XY.Z, for US system as XY.ZT
            // For SI rain total given as XYZT.U, for US system as XYZT.UV

            int rain_rate_block_length = (rain_system_of_units == SystemOfUnits::SI ? 3 : 4);
            int rain_total_block_length = (rain_system_of_units == SystemOfUnits::SI ? 5 : 6);
            // Constant for shifting decimal point
            float precision = (rain_system_of_units == SystemOfUnits::SI ? 0.1 : 0.01);
            // We return answer in mm/h, so we should convert value from inch/h if US system used
            float rain_factor = (rain_system_of_units == SystemOfUnits::SI ? 1.0 : 25.4) * precision;
            if (offset + rain_rate_block_length + rain_total_block_length > (int)packet.size())
                return "";
            float rain_rate = (float)ExtractInt(packet, offset, rain_rate_block_length) * rain_factor;
            float rain_total = (float)ExtractInt(packet, offset + rain_rate_block_length,
                                                 rain_total_block_length) * rain_factor;

            buffered_writer.printf(" rain_rate=%0.2f rain_total=%0.2f", rain_rate, rain_total);
        }
        if (wind_offset != -1) {
            int offset = data_offset + wind_offset;
            if (offset + 9 > (int)packet.size())
                return "";
            float direction = 22.5 + float(packet[offset] - '0');
            float speed = (float)ExtractInt(packet, offset + 3, 3) * 0.1;
            float average_speed = (float)ExtractInt(packet, offset + 6, 3) * 0.1;
            buffered_writer.printf(" wind_dir=%0.2f wind_speed=%0.2f wind_avg_speed=%0.2f",
                                   direction, speed, average_speed);
        }
        if (pressure_offset != -1) {
            int offset = data_offset + pressure_offset;
            if (offset + 2 > (int)packet.size())
                return "";
            int pressure = 856 + ExtractInt(packet, offset, 2);
            buffered_writer.printf(" pressure=%d", pressure);
        }
        if (forecast_offset != -1) {
            int offset = data_offset + forecast_offset;
            if (offset + 1 > (int)packet.size())
                return "";
            std::string forecast = "???";
            switch (packet[offset]) {
                case '2':
                    forecast = "cloudy";
                    break;
                case '3':
                    forecast = "rainy";
                    break;
                case '6':
                    forecast = "partly_cloudy";
                    break;
                case 'C':
                    forecast = "sunny";
                    break;
            }
            buffered_writer.printf(" forecast=%s", forecast.c_str());
        }
        if (comfort_offset != -1) {
            int offset = data_offset + comfort_offset;
            if (offset + 1 > (int)packet.size())
                return "";
            std::string comfort = "???";
            switch (packet[offset]) {
                case '0':
                    comfort = "normal";
                    break;
                case '4':
                    comfort = "comfortable";
                    break;
                case '8':
                    comfort = "dry";
                    break;
                case 'C':
                    comfort = "wet";
                    break;
            }
            buffered_writer.printf(" comfort=%s", comfort.c_str());
        }

        return buffered_writer.getString();
    }

    OregonRFDevice(std::vector<string> sensor_ids_):
        sensor_ids(sensor_ids_),
        temperature_offset(-1), humidity_offset(-1), ultraviolet_offset(-1),
        wind_offset(-1), rain_offset(-1), pressure_offset(-1),
        forecast_offset(-1), comfort_offset(-1)
    {

    }
};


/* OregonRFDevice({"1D20", "1D30", "F824", "F8B4"}) - creates device with names "1D20", ...
 * of course they are different but they are acting in the same way
 * .addTemperatureField(0).addHumidityField(4) - means that we should extract temperature
 * that begins from 0-th nibble of data block
 * and extract humidity that begins from 4-th nibble of data block
 * */
std::vector<OregonRFDevice> devices = {
    OregonRFDevice({"1D20", "1D30", "F824", "F8B4"}).AddTemperatureField(0).AddHumidityField(4),
    OregonRFDevice({"EC40", "C844"}).AddTemperatureField(0),
    OregonRFDevice({"EC70"}).AddUltravioletField(0),
    OregonRFDevice({"D874"}).AddUltravioletField(2),
    OregonRFDevice({"1994", "1984"}).AddWindField(0),
    OregonRFDevice({"2914"}).AddRainField(0, OregonRFDevice::SystemOfUnits::SI),
    OregonRFDevice({"2D10"}).AddRainField(0, OregonRFDevice::SystemOfUnits::US),
    OregonRFDevice({"5D60"}).AddTemperatureField(0).AddHumidityField(4).AddComfortField(6).AddPressureField(7).AddForecastField(9)
};



static range_type g_timing_pause[7] = {
    { 40000, 47000 },
    { 380, 850 },
    { 851, 1400 },
    { 0, 0 }
};

static range_type g_timing_pulse[8] = {
    { 1101, 1101 },
    { 200, 615 },
    { 615, 1100 },
    { 0, 0 }
};


// originally preamble was "cCcCcCcCcCcCcCcCcCcCcCcCcCcCcCbBc" (plus "cCc" in the beginning)
// but porting tests from ism-radio it was shortened
CRFProtocolOregon::CRFProtocolOregon()
    : CRFProtocol(g_timing_pause, g_timing_pulse, 0, 1, "CcCcCcCcCcCcCcCcCcCcCcCcCcCbBc")
{
}

CRFProtocolOregon::CRFProtocolOregon(range_array_type zeroLengths, range_array_type pulseLengths,
                                     int bits, int minRepeat, std::string PacketDelimeter)
    : CRFProtocol(zeroLengths, pulseLengths, bits, minRepeat, PacketDelimeter)
{
}

CRFProtocolOregon::~CRFProtocolOregon()
{
}


string CRFProtocolOregon::DecodePacket(const std::string &raw_)
{
    DPRINTF_DECLARE(dprintf, false);

    std::string raw = raw_;

    // 48 chars in minimal binary packet -> 48 signals at minimum
    if (raw.length() < 96)
        return "";


    // truncate by 'a', '?' (it's the end of the packet)
    {
        size_t apos = raw.find('a');
        if (apos != string::npos)
            raw.resize(apos);
        apos = raw.find('?');
        if (apos != string::npos)
            raw.resize(apos);
    }
    dprintf("$P decodePacket: %\n", raw);

    std::vector <char> isPulse(raw.size());
    for (int i = 0; i < (int)raw.size(); i++)
        isPulse[i] = ('A' <= raw[i] && raw[i] <= 'Z');

    // check for alternating pulses and pauses
    for (int i = 1; i < (int)raw.size(); i++)
        if ((isPulse[i - 1] ^ isPulse[i]) == 0)
            return "";


    std::string packet = "0";
    bool second = false;
    char demand_next_c = 0;

    for (char c : raw) {
        if (demand_next_c != 0) {
            if (c == demand_next_c) {
                demand_next_c = 0;
                continue;
            } else
                return "";
        }
        switch (c) {
            case 'b':
                if (second)
                    return "";

                packet.push_back('0');
                demand_next_c = 'B';
                second = true;
                break;
            case 'B':
                if (second)
                    return "";

                packet.push_back('1');
                demand_next_c = 'b';
                second = true;
                break;
            case 'c':
                if (second)
                    second = false;
                else {
                    packet.push_back('1');
                    second = true;
                }
                break;
            case 'C':
                if (second)
                    second = false;
                else {
                    packet.push_back('0');
                    second = true;
                }
                break;
            default:
                return "";
        }
    }

    uint32_t crc = 0, originalCRC = -1;
    std::string hexPacket = "";

    if (packet.length() < 48) {
        dprintf("$P (only warning: it may be other protocol) Too short packet %s\n",
                packet.c_str());
        return "";
    }
    dprintf("$P decoded bits = %\n", packet);
    int len = packet.length();
    while (len % 4)
        len--;

    for (int i = 0; i < len; i += 4) {
        std::string portion = reverse(packet.substr(i, 4));
        char buffer[20];
        uint32_t val = bits2long(portion);

        if (i > 0 && i < len - 16)
            crc += val;
        else if (i == len - 16)
            originalCRC = val;
        else if (i == len - 12)
            originalCRC += val << 4;

        snprintf(buffer, sizeof(buffer), "%X", val);
        hexPacket += buffer;
    }

    dprintf("$P decoded data = %\n", hexPacket);

    if (crc != originalCRC) {
        dprintf("$P (only warning: it may be other protocol) Bad CRC for %s", packet.c_str());
        return "";
    }

    if (hexPacket[0] != 'A') {
        dprintf("$P First nibble is not 'A'. Data: %s", hexPacket.c_str());
        return "";
    }

    return hexPacket;
}


string CRFProtocolOregon::DecodeData(const std::string &packet) // �������������� ��� � ������
{
    std::string parsed;
    for (auto device = devices.begin(); device != devices.end() && parsed.length() == 0; device++)
        parsed = device->DecodeData(packet);
    if (parsed.size() == 0) {
        LOG(INFO) << "OregonV2: Unknown sensor type " << packet.substr(1, 4)
                  << ". Data: " << packet;
        return "raw:" + packet;
    }
    return parsed;
}


bool CRFProtocolOregon::needDump(const std::string &rawData)
{
    return rawData.find(m_PacketDelimeter) != rawData.npos;
    //  return rawData.find("cCcCcCcC") != rawData.npos;
}
