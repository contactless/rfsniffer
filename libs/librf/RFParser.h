#pragma once
#include "stdafx.h"
#include "rflib.h"
#include "RFProtocol.h"

typedef std::vector<CRFProtocol *> CRFProtocolList;
class CRFAnalyzer;

class RFLIB_API CRFParser
{
    typedef std::string string;
    CLog *m_Log;
    CRFProtocolList m_Protocols;
    bool b_RunAnalyzer;
    CRFAnalyzer *m_Analyzer;
    string m_SavePath;
    base_type m_minPause, m_maxPause, m_minPulse, m_maxPulse;

  public:
    static const int MIN_PACKET_LEN =
        50; // Минимально возможная длина пакета

    //  Конструктор. Принимает в качестве параметра логгер и путь для сохранения файлов. Если путь пустой, файлы не сохраняются
    CRFParser(CLog *log, string savePath = "");
    virtual ~CRFParser();

    //  AddProtocol  добавляет декодер к общему пулу декодеров
    void AddProtocol(CRFProtocol *);
    void AddProtocol(string protocol = "all");

    /*  Пытается декодировать пакет пулом декодеров.
        Декодеры перебираются по очереди
        В случае успеха возвращает строку вида <Имя декодера>:<Результат декодирования>
    */
    // Пытается декодировать всю последовательность [data, data + len) каким-то одним декодером
    string Parse(base_type *data, size_t len);
    // Режет последовательность на пакеты (по длинным паузам и коротким импульсам)
    // Делает это сначала и одновременно пытается распознать пакеты
    // Возвращает первый успешный результат, при этом "data_ptr" и "len_ptr" сдвигаются
    string Parse(base_type **data_ptr, size_t *len_ptr);

    // Tries to recognize packet from begin of data.
    // If data was recognised then returned string have non-zero length,
    // otherwise returned string is empty.
    // In every case read length (or just skipped) will be written to "readLength"
    string ParseRepetitive(base_type *data, size_t length, size_t *readLength);

    // Tries to recognize packet from begin of data.
    // It returns all recognized packets to the end
    // and parsed length
    std::vector<string> ParseToTheEnd(base_type *data, size_t length, size_t *readLength);

    //  Включает анализатор для пакетов, которые не получилось декодировать. Пока не реализованно
    void EnableAnalyzer();

    //  Сохраняет пакет в файл
    void SaveFile(base_type *data, size_t size);

    // Устанавливает путь для сохранения пакетов
    void SetSavePath(string savePath);
  private:
    void setMinMax();
};
