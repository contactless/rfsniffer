#pragma once
#include "libwb.h"
#include <vector>
#include <unordered_map>
#include <string>

#include "../libutils/Config.h"

class LIBWB_API CWBControl
{
    // Using std::string without namespace
    typedef std::string string;

  public:

    enum ControlType {
        Error = 0,
        Switch,  //0 or 1
        Alarm, //
        PushButton, // 1
        Range, // 0..255 [TODO] - max value
        Rgb,
        Text,
        Generic,
        Temperature, // temperature °C float
        RelativeHumidity, //    rel_humidity %, RH float, 0 - 100
        AtmosphericPressure, // atmospheric_pressure    millibar(100 Pa)    float
        PrecipitationRate, //(rainfall rate)    rainfall    mm per hour float
        PrecipitationTotal, // (rainfall total) mm
        WindSpeed, //   wind_speed  m / s   float
        WindAverageSpeed, //    wind_avg_speed  m / s   float
        WindDirection, // Wind direction in degrees float 0..360
        Power, //   watt    float
        PowerConsumption, //    power_consumption   kWh float
        Voltage, // volts   float
        WaterFlow, //   water_flow  m ^ 3 / hour    float
        WaterTotal, // consumption  water_consumption   m ^ 3   float
        Resistance, //  resistance  Ohm float
        GasConcentration, //    concentration   ppm float(unsigned)
        BatteryLow, // Low battery level - 0 or 1
        UltravioletIndex, // integer
        Forecast, // weather forecast - string
        Comfort, // Level of comfort - string
        ControlTypeCount // amount of above-mentioned types, terminal item in enum
    };

    struct ControlNames {
        ControlType type;
        std::string metaType;
        std::string defaultName;
    };

  protected:

    static std::vector<string> getControlTypeToMetaType(
        const std::vector<ControlNames> &controlNamesList);

    static std::vector<string> getControlTypeToDefaultName(
        const std::vector<ControlNames> &controlNamesList);

  public:

    const static std::vector<ControlNames> controlNamesList;
    // failed to do them const =(
    static std::vector<string> controlTypeToMetaType;
    static std::vector<string> controlTypeToDefaultName;

    static ControlType getControlTypeByMetaType(const std::string &metaType);

    // "type" - type of control
    // "name" is a name of control in mqtt tree and also description in web interface
    // "metaType" is the type that will be written into .../meta/type
    // upd: "metaType" will be generated from "type"
    ControlType type;
    std::string name;
    // "readonly" is a flag answering if item is readonly in web-interface
    // "changed" is a flag that is used when we update mqtt
    bool readonly, changed, initialized;
    // "value" - value on control
    std::string value;
    std::string order, max;
    std::string source, sourceType;

    // when timeWhen comes, then valueThen will be written to value
    bool isValueChangeSheduled;
    time_t timeWhen;
    std::string valueThen;

    const std::string &metaType() const;

    const std::string &stringValue() const;
    float floatValue() const;

    CWBControl &setSource(const std::string &source_);
    CWBControl &setSourceType(const std::string &sourceType_);

    CWBControl(const std::string &name, ControlType type, bool readonly = true);
    CWBControl(const std::string &name_, ControlType type_,
            const std::string &initialValue, const std::string &order_, bool readonly_);
    CWBControl();
};


class LIBWB_API CWBDevice
{
    // Using std::vector, std::string and std::unordered_map without namespace
    typedef std::string string;
  public:
    typedef std::unordered_map<string, CWBControl> CControlMap;
    typedef std::unordered_map<string, CWBDevice *> CWBDeviceMap;
    typedef std::unordered_map<string, string> StringMap;
  private:
    std::string deviceName;
    std::string deviceDescription;
    CControlMap deviceControls;

    // if this flag is NOT set then output functions won't write anything
    bool deviceIsActive;
    // it's a time in seconds that defines how long may be max pause between messages
    // it helps determine that device is dead
    long long heartbeat;
    // it's a flag of aliveness of device by heartbeat (it's changing when updates to mqtt are sent)
    bool deviceIsAlive;
    time_t lastMessageReceiveTime;
    void doHeartbeat();

  public:
    // deviceName is the name in mqtt tree
    // deviceDescription will be written into .../meta/name
    CWBDevice();
    CWBDevice(const std::string &deviceName, const std::string &deviceDescription);
    ~CWBDevice();


    void findAndSetConfigs(Json::Value &devices);

    const std::string &getName()
    {
        return deviceName;
    };
    const std::string &getDescription()
    {
        return deviceDescription;
    };
//~ #ifdef USE_CONFIG
    //~ void init(CConfigItem config);
//~ #endif
    void addControl(const CWBControl &device);
    void addControl(const std::string &name, CWBControl::ControlType type, bool readonly = true);
    void addControl(const std::string &name, CWBControl::ControlType type,
            const std::string &initialValue, const std::string &order, bool readonly);
    bool controlExists(const std::string &name);
    bool sourceExists(const std::string &source);
    void setBySource(const std::string &source, const std::string &sourceType, std::string value);
    void set(CWBControl::ControlType type, const std::string &value);
    void set(CWBControl::ControlType type, float value);
    void set(const std::string &name, const std::string &value);
    void set(const std::string &name, float value);
    void setMax(const std::string &name, const std::string &max);

    // Sensor PM112 sends signal to enable smth for given time
    // So such function is needed...
    void setForAndThen(const std::string &name, const std::string &value, int timeFor, const std::string &valueThen);
    void updateScheduled(StringMap &);

    float getFloat(const std::string &name);
    const std::string &getString(const std::string &name);

    // output functions, they are returning changes that are to be done in mqtt
    void createDeviceValues(StringMap &);
    void updateValues(StringMap &);
    void updateAliveness(StringMap &);
    bool isAlive();

    const CControlMap *getControls()
    {
        return &deviceControls;
    };
    std::string getTopic(const std::string &control);

};

