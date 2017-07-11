#include "WBDevice.h"

#include <cstdio>
#include <ctime>
#include <stdexcept>

#include "../libutils/strutils.h"
#include "../libutils/Exception.h"
#include "../libutils/DebugPrintf.h"

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
    type(type_), name(name_), readonly(readonly_), isValueChangeSheduled(false)
{
    if (name.empty())
        name = controlTypeToDefaultName[type];
}

CWBControl::CWBControl(string_cref name_, ControlType type_, 
            string_cref initialValue, string_cref order_, bool readonly_):
    type(type_), name(name_), readonly(readonly_), value(initialValue), 
    order(order_), max(""), isValueChangeSheduled(false)
{
    if (name.empty())
        name = controlTypeToDefaultName[type];
}

CWBControl::CWBControl(): type(ControlType::Error), name(""), readonly(false),
    isValueChangeSheduled(false) {}

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
    : deviceName(Name), deviceDescription(Description), deviceIsActive(true),
      heartbeat(-1), deviceIsAlive(false), lastMessageReceiveTime(time(NULL))
{
    // device is firstly marked as non-alive because
    // it's a good idea to drop all /meta/error when creating a device
    // but they drop only when alive-state changes
}


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



void CWBDevice::findAndSetConfigs(Json::Value &devices)
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P findAndSetConfigs for %\n", deviceName);
    if (!devices)
        return;
    // check common settings
    if (devices["unknown_devices_politics"].asString() == "ignore")
        deviceIsActive = false;

    // check known devices settings
    if (!devices["use_devices_list"].asBool())
        return;

    auto knownDevices = devices["known_devices"];
    
	if (!!knownDevices) {
		if (!knownDevices.isArray())
			throw std::runtime_error("known_devices must be array");
		for (int i = 0; i < (int)knownDevices.size(); ++i) {
			auto dev = knownDevices[i];
			dprintf("$P There is a variant : %\n",
                    dev["name"].asString());
            if (dev["name"].asString() == deviceName) {
				deviceIsActive = !(dev["politics"].asString() == "ignore");
				heartbeat = dev["heartbeat"].asInt();
				dprintf.c("$P found device in the list! "\
						  "politics=%s   heartbeat=%d\n",
						  (deviceIsActive ? "show" : "ignore"), heartbeat);
			}  
		}		
	}
}


void CWBDevice::doHeartbeat()
{
    lastMessageReceiveTime = time(NULL);
}



void CWBDevice::addControl(const CWBControl &control)
{
    deviceControls[control.name] = control;
}

void CWBDevice::addControl(const string &name, ControlType type, bool readonly)
{
    addControl(CWBControl(name, type, readonly));
}

void CWBDevice::addControl(const string &name, CWBControl::ControlType type, 
        const string &initialValue, const string &order, bool readonly) 
{
    addControl(CWBControl(name, type, initialValue, order, readonly));       
}

bool CWBDevice::controlExists(const string &name)
{
	return deviceControls.count(name);
}

void CWBDevice::set(CWBControl::ControlType type, string_cref value)
{
    set(CWBControl::controlTypeToDefaultName[type], value);
}

void CWBDevice::set(string_cref name, string_cref value)
{
    doHeartbeat();

    CControlMap::iterator i = deviceControls.find(name);

    if (i == deviceControls.end())
        throw CHaException(CHaException::ErrBadParam, name);

    i->second.value = value;
    i->second.changed = true;
    //i->second.isValueChangeSheduled = false;
}

void CWBDevice::set(CWBControl::ControlType type, float value)
{
    set(CWBControl::controlTypeToDefaultName[type], value);
}

void CWBDevice::set(string_cref name, float value)
{
    set(name, ftoa(value));
}

void CWBDevice::setMax(string_cref name, string_cref max)
{
    CControlMap::iterator i = deviceControls.find(name);

    if (i == deviceControls.end())
        throw CHaException(CHaException::ErrBadParam, name);

    i->second.max = max;
    i->second.changed = true;
}


void CWBDevice::setForAndThen(const string &name, const string &value, int timeFor,
                              const string &valueThen)
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P (%, %, %, %) called\n", name, value, timeFor, valueThen);

    doHeartbeat();

    CControlMap::iterator i = deviceControls.find(name);

    if (i == deviceControls.end())
        throw CHaException(CHaException::ErrBadParam, name);

    i->second.value = value;
    i->second.changed = true;

    i->second.isValueChangeSheduled = true;
    i->second.timeWhen = time(nullptr) + timeFor;
    dprintf("$P current_time %, time when %\n", time(nullptr), i->second.timeWhen);
    i->second.valueThen = valueThen;
    dprintf("$P value is % will be %\n", i->second.value, i->second.valueThen);
}
void CWBDevice::updateScheduled(StringMap &v)
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P start\n");
    if (!deviceIsActive) // do nothing if device is disabled
        return;

    const string base = "/devices/" + deviceName;

    for(auto &i : deviceControls) {
        dprintf("$P       % % %\n", i.second.name, i.second.value, i.second.isValueChangeSheduled);
        if (i.second.isValueChangeSheduled &&
                difftime(time(nullptr), i.second.timeWhen) >= 0) {
            dprintf("$P change % from % to %\n", base + "/controls/" + i.second.name, i.second.value,
                    i.second.valueThen);
            i.second.isValueChangeSheduled = false;
            i.second.value = i.second.valueThen;
            v[base + "/controls/" + i.second.name] = i.second.value;
            i.second.changed = false;
        }
    }
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
    if (!deviceIsActive) // do nothing if device is disabled
        return;

    const string base = "/devices/" + deviceName;

    v[base + "/meta/name"] = deviceDescription;

    for(const auto &i : deviceControls) {
        const CWBControl &control = i.second;
        const string controlBase = base + "/controls/" + control.name;
        v[controlBase] = control.value;
        v[controlBase + "/meta/type"] = i.second.metaType();
        if (control.order.empty())
            v[controlBase + "/meta/order"] = String::ValueOf((int)i.second.type);
        else
            v[controlBase + "/meta/order"] = control.order;
        if (!control.max.empty())
            v[controlBase + "/meta/max"] = control.max;
        if (i.second.readonly)
            v[controlBase + "/meta/readonly"] = "1";
    }
}

bool CWBDevice::isAlive()
{
    if (!deviceIsActive || heartbeat < 0)
        return true;
    return (difftime(time(NULL), lastMessageReceiveTime) <= heartbeat);
}


void CWBDevice::updateAliveness(StringMap &v)
{
    // notice that deviceIsAlive mirrors the state that was already sent
    // and isAlive() mirrors the realtime state
    bool deviceIsReallyAlive = isAlive();
    if (deviceIsAlive != deviceIsReallyAlive) {
        const string base = "/devices/" + deviceName;
        const string error = (deviceIsReallyAlive ? "" : "DEVICE_LEVEL_ERROR: "\
                              "No heartbeat was received (device is probably discharged or just off)");
        v[base + "/meta/error"] = error;
        for(auto &i : deviceControls)
            v[base + "/controls/" + i.second.name + "/meta/error"] = error;

        deviceIsAlive = deviceIsReallyAlive;
    }
}

void CWBDevice::updateValues(StringMap &v)
{
    if (!deviceIsActive) // do nothing if device is disabled
        return;

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
