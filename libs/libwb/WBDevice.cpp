#include <cstdio>
#include "../libutils/strutils.h"
#include "../libutils/Exception.h"
#include "WBDevice.h"

using namespace strutils;

typedef CWBControl::ControlNames ControlNames;
typedef CWBControl::ControlType ControlType;

typedef const std::string &string_cref;

const std::vector<ControlNames> CWBControl::controlNamesList = {
    {Error, "error", "Error"},
    {Switch, "switch", "Switch"},
    {Alarm, "alarm", "Alarm"},
    {PushButton, "pushbutton", "Push button"},
    {Range, "range", "Range"},
    {Rgb, "rgb", "RGB"},
    {Text, "text", "Text"},
    {Generic, "value", "Value"},
    {Temperature, "temperature", "Temperature"},
    {RelativeHumidity, "rel_humidity", "Humidity"},
    {AtmosphericPressure, "pressure", "Atmospheric pressure"},
    {PrecipitationRate, "rainfall", "Precipitation rate"},
    {PrecipitationTotal, "raintotal", "Precipitation total"},
    {WindSpeed, "wind_speed", "Wind speed"},
    {WindAverageSpeed, "wind_avg_speed", "Wind average speed"},
    {WindDirection, "wind_direction", "Wind direction"},
    {Power, "power", "Power"},
    {PowerConsumption, "power_consumption", "Power consumption"},
    {Voltage, "voltage", "Voltage"},
    {WaterFlow, "water_flow", "Water flow"},
    {WaterTotal, "water_consumption", "Water consumption"},
    {Resistance, "resistance", "Resistance"},
    {GasConcentration, "concentration", "Gas concentration"},
    {BatteryLow, "alarm", "Battery low"},
    {UltravioletIndex, "ultraviolet", "Ultraviolet"},
    {Forecast, "forecast", "Forecast"},
    {Comfort, "comfort_level", "Comfort level"}
};


std::vector<std::string> CWBControl::getControlTypeToMetaType(
    const std::vector<ControlNames> &controlNamesList)
{
    std::vector<string> res(ControlType::ControlTypeCount);
    for (const ControlNames &names : controlNamesList)
        res[names.type] = names.metaType;
    return res;
}

std::vector<std::string> CWBControl::getControlTypeToDefaultName(
    const std::vector<ControlNames> &controlNamesList)
{
    std::vector<string> res(ControlType::ControlTypeCount);
    for (const ControlNames &names : controlNamesList)
        res[names.type] = names.defaultName;
    return res;
}

std::vector<std::string> CWBControl::controlTypeToMetaType =
    getControlTypeToMetaType(CWBControl::controlNamesList);
std::vector<std::string> CWBControl::controlTypeToDefaultName =
    getControlTypeToDefaultName(CWBControl::controlNamesList);


CWBControl::CWBControl(string_cref name_, ControlType type_, bool readonly_):
    name(name_), type(type_), readonly(readonly_)
{
    if (name.empty())
        name = controlTypeToDefaultName[type];
}

CWBControl::CWBControl(): name(""), type(ControlType::Error), readonly(false) {}

string_cref CWBControl::metaType() const
{
    return controlTypeToMetaType[type];
}

string_cref CWBControl::stringValue() const
{
    return value;
}

float CWBControl::floatValue() const
{
    return atof(value.c_str());
}


CWBControl &CWBControl::setSource(string_cref source_)
{
    source = source_;
    return *this;
}

CWBControl &CWBControl::setSourceType(string_cref sourceType_)
{
    sourceType = sourceType_;
    return *this;
}


ControlType CWBControl::getControlTypeByMetaType(string_cref metaType)
{
    for (const ControlNames &controlNames : controlNamesList)
        if (controlNames.metaType == metaType)
            return controlNames.type;
    return ControlType::Error;
}



CWBDevice::CWBDevice() { }

CWBDevice::CWBDevice(string_cref Name, string_cref Description)
    : deviceName(Name), deviceDescription(Description)
{ }


CWBDevice::~CWBDevice()
{ }

#ifdef USE_CONFIG
void CWBDevice::init(CConfigItem config)
{
    deviceName = config.getStr("Name");
    deviceDescription = config.getStr("Description");

    CConfigItemList controls;
    config.getList("Control", controls);

    for(CConfigItem *control : controls) {
        CWBControl Control = CWBControl(
                                 control->getStr("Name"),
                                 CWBControl::getControlTypeByMetaType(control->getStr("Type")),
                                 (control->getInt("Readonly", false, 1) != 0)
                             ).setSource(control->getStr("Source", false))
                             .setSourceType(control->getStr("SourceType", false));

        deviceControls[Control.name] = Control;
    }
}
#endif

void CWBDevice::addControl(const CWBControl &control)
{
    deviceControls[control.name] = control;
}

void CWBDevice::addControl(const string &name, ControlType type, bool readonly)
{
    addControl(CWBControl(name, type, readonly));
}


void CWBDevice::set(CWBControl::ControlType type, string_cref value)
{
    set(CWBControl::controlTypeToDefaultName[type], value);
}

void CWBDevice::set(string_cref name, string_cref value)
{
    CControlMap::iterator i = deviceControls.find(name);

    if (i == deviceControls.end())
        throw CHaException(CHaException::ErrBadParam, name);

    i->second.value = value;
    i->second.changed = true;
}

void CWBDevice::set(CWBControl::ControlType type, float value)
{
    set(CWBControl::controlTypeToDefaultName[type], value);
}

void CWBDevice::set(string_cref name, float value)
{
    CControlMap::iterator i = deviceControls.find(name);

    if (i == deviceControls.end())
        throw CHaException(CHaException::ErrBadParam, name);

    i->second.value = ftoa(value);
    i->second.changed = true;
}


float CWBDevice::getFloat(string_cref name)
{
    CControlMap::iterator i = deviceControls.find(name);

    if (i == deviceControls.end())
        throw CHaException(CHaException::ErrBadParam, name);

    return i->second.floatValue();
}

string_cref CWBDevice::getString(string_cref name)
{
    CControlMap::iterator i = deviceControls.find(name);

    if (i == deviceControls.end())
        throw CHaException(CHaException::ErrBadParam, name);


    return i->second.stringValue();
}

void CWBDevice::createDeviceValues(StringMap &v)
{
    const string base = "/devices/" + deviceName;

    v[base + "/meta/name"] = deviceDescription;

    for(const auto &i : deviceControls) {
        const string controlBase = base + "/controls/" + i.second.name;
        v[controlBase] = i.second.value;
        v[controlBase + "/meta/type"] = i.second.metaType();
        v[controlBase + "/meta/order"] = String::ValueOf((int)i.second.type);
        if (i.second.readonly)
            v[controlBase + "/meta/readonly"] = "1";
    }
}

void CWBDevice::updateValues(StringMap &v)
{
    const string base = "/devices/" + deviceName;

    for(auto &i : deviceControls) {
        if (i.second.changed) {
            v[base + "/controls/" + i.second.name] = i.second.value;
            i.second.changed = false;
        }
    }
}

std::string CWBDevice::getTopic(string_cref control)
{
    string base = "/devices/" + deviceName;
    return base + "/controls/" + control;
}

bool CWBDevice::sourceExists(string_cref source)
{
    for (const auto &i : deviceControls)
        if (i.second.source == source)
            return true;

    return false;
}

void CWBDevice::setBySource(string_cref source, string_cref sourceType, std::string value)
{
    if (sourceType == "X10")
        value = (value == ("ON" ? "1" : "0"));

    for (const auto &i : deviceControls) {
        if (i.second.source == source)
            set(i.first, value);
    }
}
