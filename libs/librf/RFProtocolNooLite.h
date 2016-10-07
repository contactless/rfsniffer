#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolNooLite :
	public CRFProtocol
{ 
	map<uint16_t,bool> m_lastFlip;

public:
	enum nooLiteCommandType
	{
		nlcmd_off,              //0 – выключить нагрузку
		nlcmd_slowdown,			//1 – запускает плавное понижение яркости
		nlcmd_on,				//2 – включить нагрузку
		nlcmd_slowup,			//3 – запускает плавное повышение яркости
		nlcmd_switch,			//4 – включает или выключает нагрузку
		nlcmd_slowswitch,		//5 – запускает плавное изменение яркости в обратном
		nlcmd_shadow_level,		//6 – установить заданную в «Данные к команде_0» яркость
		nlcmd_callscene,		//7 – вызвать записанный сценарий
		nlcmd_recordscene,		//8 – записать сценарий
		nlcmd_unbind,			//9 – запускает процедуру стирания адреса управляющего устройства из памяти исполнительного
		nlcmd_slowstop,			//10 – остановить регулировку
		nlcmd_bind=15,			//15 – сообщает, что устройство хочет записать свой адрес в память
		nlcmd_slowcolor,		//16 – включение плавного перебора цвета
		nlcmd_switchcolor,		//17 – переключение цвета
		nlcmd_switchmode,		//18 – переключение режима работы
		nlcmd_switchspeed,		//19 – переключение скорости эффекта для режима работы
		nlcmd_battery_low,		//20 – информирует о разряде батареи в устройстве
		nlcmd_temperature,		//21 – передача информации о текущей температуре и
								//     влажности (Информация о температуре и влажности содержится в
								//     поле «Данные к команде_x».)

		nlcmd_error=-1
	};

	CRFProtocolNooLite();
	~CRFProtocolNooLite();

	virtual string getName() { return "nooLite"; };
	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string&); // Преобразование бит в данные
	virtual bool needDump(const string &rawData);
	
	// Кодирование
	virtual string bits2timings(const string &bits);
	virtual string data2bits(const string &data);
	nooLiteCommandType getCommand(const string &name);

private:
	unsigned char getByte(const string &bits, size_t first, size_t len=8);
	bool bits2packet(const string& bits, uint8_t *packet, size_t *packetLen, uint8_t *CRC=NULL);
	uint8_t crc8(uint8_t *addr, uint8_t len);
};

