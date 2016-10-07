#pragma once
//#include "../engine/StateJob.h"
#include "libwb.h"
#include "../libutils/strutils.h"

struct LIBWB_API CWBControl
{
	enum ControlType
	{
		Error=0,
		Switch,  //0 or 1
		Alarm, // 
		PushButton, // 1
		Range, // 0..255 [TODO] - max value
		Rgb, 
		Text,
		Generic,
		Temperature, //	temperature	°C	float
		RelativeHumidity, //	rel_humidity	%, RH	float, 0 - 100
		AtmosphericPressure, //	atmospheric_pressure	millibar(100 Pa)	float
		PrecipitationRate, //(rainfall rate)	rainfall	mm per hour	float
		WindSpeed, //	wind_speed	m / s	float
		PowerPower, //	watt	float
		PowerConsumption, //	power_consumption	kWh	float
		VoltageVoltage, //	volts	float
		WaterFlow, //	water_flow	m ^ 3 / hour	float
		WaterTotal, // consumption	water_consumption	m ^ 3	float
		Resistance, //	resistance	Ohm	float
		GasConcentration //	concentration	ppm	float(unsigned)

	};

	ControlType Type;
	string Name, Source, SourceType;
	bool Readonly, Changed;
	string sValue;
	float fValue;
};

typedef map<string, CWBControl*> CControlMap;

class LIBWB_API CWBDevice
{
	string m_Name;
	string m_Description;
	CControlMap m_Controls;


public:
	CWBDevice(string Name, string Description);
	CWBDevice();
	~CWBDevice();

	string getName(){return m_Name;};
	string getDescription(){return m_Description;};
#ifdef USE_CONFIG
	void Init(CConfigItem config);
#endif	
	void AddControl(string Name, CWBControl::ControlType Type, bool ReadOnly, string Source="", string SourceType="");
	bool sourceExists(string source);
	void setBySource(string source, string sourceType, string Value);
	void set(string Name, string Value);
	void set(string Name, float Value);
	float getF(string Name);
	string getS(string Name);
	void CreateDeviceValues(string_map &);
	void UpdateValues(string_map &);
	const CControlMap *getControls(){return &m_Controls;};
	string getTopic(string Control);

};

typedef map<string, CWBDevice*> CWBDeviceMap;
