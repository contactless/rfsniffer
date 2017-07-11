#pragma once
#include "libwb.h"
#include <vector>
#include <unordered_map>
#include <string>

#ifdef USE_CONFIG
    #include "../libutils/ConfigItem.h"
#endif

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
        string metaType;
        string defaultName;
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

    static ControlType getControlTypeByMetaType(const string &metaType);

    // "type" - type of control
    // "name" is a name of control in mqtt tree and also description in web interface
    // "metaType" is the type that will be written into .../meta/type
    // upd: "metaType" will be generated from "type"
    ControlType type;
    string name;
    // "readonly" is a flag answering if item is readonly in web-interface
    // "changed" is a flag that is used when we update mqtt
    bool readonly, changed;
    // "value" - value on control
    string value;
    string order, max;
    string source, sourceType;

    // when timeWhen comes, then valueThen will be written to value
    bool isValueChangeSheduled;
    time_t timeWhen;
    string valueThen;

    const string &metaType() const;

    const string &stringValue() const;
    float floatValue() const;

    CWBControl &setSource(const string &source_);
    CWBControl &setSourceType(const string &sourceType_);

    CWBControl(const string &name, ControlType type, bool readonly = true);
    CWBControl(const string &name_, ControlType type_, 
            const string &initialValue, const string &order_, bool readonly_);
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
    string deviceName;
    string deviceDescription;
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
    CWBDevice(const string &deviceName, const string &deviceDescription);
    ~CWBDevice();


    void findAndSetConfigs(Json::Value &devices);

    const string &getName()
    {
        return deviceName;
    };
    const string &getDescription()
    {
        return deviceDescription;
    };
#ifdef USE_CONFIG
    void init(CConfigItem config);
#endif
    void addControl(const CWBControl &device);
    void addControl(const string &name, CWBControl::ControlType type, bool readonly = true);
    void addControl(const string &name, CWBControl::ControlType type, 
            const string &initialValue, const string &order, bool readonly);
    bool controlExists(const string &name);
    bool sourceExists(const string &source);
    void setBySource(const string &source, const string &sourceType, string value);
    void set(CWBControl::ControlType type, const string &value);
    void set(CWBControl::ControlType type, float value);
    void set(const string &name, const string &value);
    void set(const string &name, float value);
    void setMax(const string &name, const string &max);

    // Sensor PM112 sends signal to enable smth for given time
    // So such function is needed...
    void setForAndThen(const string &name, const string &value, int timeFor, const string &valueThen);
    void updateScheduled(StringMap &);

    float getFloat(const string &name);
    const string &getString(const string &name);

    // output functions, they are returning changes that are to be done in mqtt
    void createDeviceValues(StringMap &);
    void updateValues(StringMap &);
    void updateAliveness(StringMap &);
    bool isAlive();

    const CControlMap *getControls()
    {
        return &deviceControls;
    };
    string getTopic(const string &control);

};

