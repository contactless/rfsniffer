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
		PrecipitationTotal, // (rainfall total) mm
		WindSpeed, //	wind_speed	m / s	float
		WindAverageSpeed, //	wind_avg_speed	m / s	float
		WindDirection, // Wind direction in degrees float 0..360
		Power, //	watt	float
		PowerConsumption, //	power_consumption	kWh	float
		Voltage, //	volts	float
		WaterFlow, //	water_flow	m ^ 3 / hour	float
		WaterTotal, // consumption	water_consumption	m ^ 3	float
		Resistance, //	resistance	Ohm	float
		GasConcentration, //	concentration	ppm	float(unsigned)
		BatteryLow, // Low battery level - 0 or 1
		UltravioletIndex, // integer
		Forecast, // weather forecast - string
		Comfort // Level of comfort - string
		

	};

	ControlType Type;
	// Name is a name of control in mqtt tree and also description in web interface
	// MetaType is the type that will be written into .../meta/type
	string Name, MetaType, Source, SourceType;
	bool Readonly, Changed;
	string sValue;
	float fValue;
};

typedef map<string, CWBControl*> CControlMap;

class LIBWB_API CWBDevice
{
	string deviceName;
	string deviceDescription;
	CControlMap deviceControls;

    string GetControlMetaTypeByType(CWBControl::ControlType type);
    CWBControl::ControlType GetControlTypeByMetaType(string meta_type);
    string GetControlUserReadNameByType(CWBControl::ControlType type);
    
public:
	// Name is the name in mqtt tree
	// Description will be written into .../meta/name
	CWBDevice(string DeviceName, string DeviceDescription);
	CWBDevice();
	~CWBDevice();

	string getName(){return deviceName;};
	string getDescription(){return deviceDescription;};
#ifdef USE_CONFIG
	void Init(CConfigItem config);
#endif	
	void AddControl(string Name, CWBControl::ControlType Type, bool ReadOnly, string Source="", string SourceType="");
	bool sourceExists(string source);
	void setBySource(string source, string sourceType, string Value);
    void set(CWBControl::ControlType Type, string Value);
    void set(CWBControl::ControlType Type, float Value);
	void set(string Name, string Value);
	void set(string Name, float Value);
	float getF(string Name);
	string getS(string Name);
	void CreateDeviceValues(string_map &);
	void UpdateValues(string_map &);
	const CControlMap *getControls(){return &deviceControls;};
	string getTopic(string Control);

};

typedef map<string, CWBDevice*> CWBDeviceMap;
