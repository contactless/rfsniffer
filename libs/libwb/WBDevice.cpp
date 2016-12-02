#include "stdafx.h"
#include "WBDevice.h"
#ifdef USE_CONFIG
	#include "../libutils/ConfigItem.h"
#endif
/*
const char *g_Topics[] =
{
	"error",
	"switch",
	"alarm",
	"pushbutton",
	"range",
	"rgb",
	"text",
	"value",
	"temperature",
	"rel_humidity",
	"pressure", // Change to more readable
    //"Temperature",
	//"Humidity",
	//"Pressure",
	"PrecipitationRate", //(rainfall rate)	rainfall	mm per hour	float
	"PrecipitationTotal", //(rainfall total) mm
	"WindSpeed", //	wind_speed	m / s	float
	"WindAverageSpeed", //	wind_avg_speed	m / s	float
	"WindDirection", // Wind direction in degrees float 0..360
	"PowerPower", //	watt	float
	"PowerConsumption", //	power_consumption	kWh	float
	"VoltageVoltage", //	volts	float
	"WaterFlow", //	water_flow	m ^ 3 / hour	float
	"WaterTotal", // consumption	water_consumption	m ^ 3	float
	"Resistance", //	resistance	Ohm	float
	"GasConcentration", //	concentration	ppm	float(unsigned)
	"Battery", // battery level - low or normal
	"UltravioletIndex", // integer
	"Forecast", // weather forecast - string
	"Comfort", // Level of comfort - string
	"",
};*/

struct TopicNames {
   CWBControl::ControlType type;
   string meta_type;
   string user_read_name;  
};

const std::vector<TopicNames> topic_names = {
    {CWBControl::ControlType::Error, "error", "Error"},
    {CWBControl::ControlType::Switch, "switch", "Switch"},
    {CWBControl::ControlType::Alarm, "alarm", "Alarm"},
    {CWBControl::ControlType::PushButton, "pushbutton", "Push button"},
    {CWBControl::ControlType::Range, "range", "Range"},
    {CWBControl::ControlType::Rgb, "rgb", "RGB"},
    {CWBControl::ControlType::Text, "text", "Text"},
    {CWBControl::ControlType::Generic, "value", "Value"},
    {CWBControl::ControlType::Temperature, "temperature", "Temperature"},
    {CWBControl::ControlType::RelativeHumidity, "rel_humidity", "Humidity"},
    {CWBControl::ControlType::AtmosphericPressure, "pressure", "Atmospheric pressure"}, 
    {CWBControl::ControlType::PrecipitationRate, "rainfall", "Precipitation rate"}, 
    {CWBControl::ControlType::PrecipitationTotal, "raintotal", "Precipitation total"}, 
    {CWBControl::ControlType::WindSpeed, "wind_speed", "Wind speed"}, 
    {CWBControl::ControlType::WindAverageSpeed, "wind_avg_speed", "Wind average speed"}, 
    {CWBControl::ControlType::WindDirection, "wind_direction", "Wind direction"},
    {CWBControl::ControlType::Power, "power", "Power"},
    {CWBControl::ControlType::PowerConsumption, "power_consumption", "Power consumption"}, 
    {CWBControl::ControlType::Voltage, "voltage", "Voltage"}, 
    {CWBControl::ControlType::WaterFlow, "water_flow", "Water flow"},
    {CWBControl::ControlType::WaterTotal, "water_consumption", "Water consumption"}, 
    {CWBControl::ControlType::Resistance, "resistance", "Resistance"},
    {CWBControl::ControlType::GasConcentration, "concentration", "Gas concentration"},
    {CWBControl::ControlType::BatteryLow, "battery", "Battery"},
    {CWBControl::ControlType::UltravioletIndex, "ultraviolet", "Ultraviolet"},
    {CWBControl::ControlType::Forecast, "forecast", "Forecast"},
    {CWBControl::ControlType::Comfort, "comfort_level", "Comfort level"}
};

string CWBDevice::GetControlMetaTypeByType(CWBControl::ControlType type) {
    for (const TopicNames &tn : topic_names)
        if (tn.type == type)
            return tn.meta_type;
	return GetControlMetaTypeByType(CWBControl::ControlType::Error);
}

CWBControl::ControlType CWBDevice::GetControlTypeByMetaType(string meta_type) {
    for (const TopicNames &tn : topic_names)
        if (tn.meta_type == meta_type)
            return tn.type;
	return CWBControl::ControlType::Error;
}

string CWBDevice::GetControlUserReadNameByType(CWBControl::ControlType type) {
    for (const TopicNames &tn : topic_names)
        if (tn.type == type)
            return tn.user_read_name;
	return GetControlUserReadNameByType(CWBControl::ControlType::Error);
}

CWBDevice::CWBDevice()
{

}

CWBDevice::CWBDevice(string Name, string Description)
:m_Name(Name), m_Description(Description)
{

}


CWBDevice::~CWBDevice()
{
	for_each(CControlMap, m_Controls, i)
		delete i->second;
}

#ifdef USE_CONFIG
void CWBDevice::Init(CConfigItem config)
{
	m_Name = config.getStr("Name");
	m_Description = config.getStr("Description");

	CConfigItemList controls;
	config.getList("Control", controls);
	for_each(CConfigItemList, controls, control)
	{
		CWBControl *Control = new CWBControl;
		Control->Name = (*control)->getStr("Name");
		Control->Source = (*control)->getStr("Source", false);
		Control->SourceType = (*control)->getStr("SourceType", false);
		Control->Readonly = (*control)->getInt("Readonly", false, 1) != 0;
	
    	//string type = (*control)->getStr("Type");
		
        Control->Type = GetControlTypeByMetaType((*control)->getStr("Type"));
        
        /*Control->Type = CWBControl::Error;
        
		for (int i = 0; g_Topics[i][0];i++)
		{
			if (type == g_Topics[i])
			{
				Control->Type = (CWBControl::ControlType)i;
				break;
			}
		}*/

		m_Controls[Control->Name] = Control;
	}
}
#endif

void CWBDevice::AddControl(string Name, CWBControl::ControlType Type, bool ReadOnly, string Source, string SourceType)
{
	CWBControl *Control = new CWBControl;
	Control->Name = (!Name.empty() ? Name : GetControlUserReadNameByType(Type));
	Control->Source = Source;
	Control->SourceType = SourceType;
	Control->Readonly = ReadOnly;
	Control->Type = Type;
	m_Controls[Control->Name] = Control;
}


void CWBDevice::set(CWBControl::ControlType Type, string Value)
{
    set(GetControlUserReadNameByType(Type), Value);
}

void CWBDevice::set(string Name, string Value)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	i->second->fValue = (float)atof(Value);
	i->second->sValue = Value;
	i->second->Changed = true;
}

void CWBDevice::set(CWBControl::ControlType Type, float Value)
{
    set(GetControlUserReadNameByType(Type), Value);
}

void CWBDevice::set(string Name, float Value)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	i->second->fValue = Value;
	i->second->sValue = ftoa(Value);
	i->second->Changed = true;
}


float CWBDevice::getF(string Name)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	return i->second->fValue;
}

string CWBDevice::getS(string Name)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	return i->second->sValue;
}

void CWBDevice::CreateDeviceValues(string_map &v)
{
	string base = "/devices/" + m_Name;
	v[base + "/meta/name"] = m_Description;

	for_each(CControlMap, m_Controls, i)
	{
		v[base + "/meta/name"] = m_Description;
		v[base + "/controls/" + i->first] = i->second->sValue;
		v[base + "/controls/" + i->first +"/meta/type"] = GetControlMetaTypeByType(i->second->Type);
		if (i->second->Readonly)
			v[base + "/controls/" + i->first + "/meta/readonly"] = "1";
	}
	//UpdateValues(v);
}

void CWBDevice::UpdateValues(string_map &v)
{
	string base = "/devices/" + m_Name;

	for_each(CControlMap, m_Controls, i)
	{
		if (i->second->Changed)
		{
			v[base + "/controls/" + i->first] = i->second->sValue;
			i->second->Changed = false;
		}
	}
}

string CWBDevice::getTopic(string Control)
{
	string base = "/devices/" + m_Name;
	return base + "/controls/" + Control;
}

bool CWBDevice::sourceExists(string source)
{
	for_each(CControlMap, m_Controls, i)
	{
		if (i->second->Source == source)
			return true;
	}

	return false;
}

void CWBDevice::setBySource(string source, string sourceType, string Value)
{
	if (sourceType=="X10")
		Value = (Value==("ON"?"1":"0"));

	for_each(CControlMap, m_Controls, i)
	{
		if (i->second->Source == source)
			set(i->first, Value);
	}
}
