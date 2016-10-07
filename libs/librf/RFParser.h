#pragma once
#include "stdafx.h"
#include "rflib.h"
#include "RFProtocol.h"

typedef vector<CRFProtocol*> CRFProtocolList;
class CRFAnalyzer;

class RFLIB_API CRFParser
{
	CLog *m_Log;
	CRFProtocolList m_Protocols;
	bool b_RunAnalyzer;
	CRFAnalyzer *m_Analyzer;
	string m_SavePath;
	base_type m_minPause, m_maxPause, m_minPulse, m_maxPulse;

public:
	static const int MIN_PACKET_LEN = 50; // Минимально возможная длина пакета

//  Конструктор. Принимает в качестве параметра логгер и путь для сохранения файлов. Если путь пустой, файлы не сохраняются	
	CRFParser(CLog *log, string savePath="");
	virtual ~CRFParser();

//	AddProtocol  добавляет декодер к общему пулу декодеров	
	void AddProtocol(CRFProtocol*);
	void AddProtocol(string protocol="all");

/*  Пытается декодировать пакет пулом декодеров. 
	Декодеры перебираются по очереди
	В случае успеха возвращает строку вида <Имя декодера>:<Результат декодирования>
*/	
	string Parse(base_type*, size_t len);
	string Parse(base_type**, size_t *len);

//  Включает анализатор для пакетов, которые не получилось декодировать. Пока не реализованно	
	void EnableAnalyzer();

//  Сохраняет пакет в файл	
	void SaveFile(base_type* data, size_t size);

// Устанавливает путь для сохранения пакетов	
	void SetSavePath(string savePath);
private:
	void setMinMax();
};
