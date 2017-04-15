#pragma once
#include <deque>

#include "stdafx.h"
#include "rflib.h"
#include "RFProtocol.h"


class CRFAnalyzer;

class RFLIB_API CRFParser
{
    typedef std::deque<base_type> InputContainer;
    typedef std::string string;
    
    CLog *m_Log;
    std::vector<CRFProtocol*> m_Protocols;
    std::vector<InputContainer::iterator> m_ProtocolsBegins;
    
    bool b_RunAnalyzer;
    CRFAnalyzer *m_Analyzer;
    string m_SavePath;

    std::vector<string> m_ParsedResults;
    InputContainer m_InputData;
    /// Time calculated as sum of lengths of all pauses and impulses
    int64_t m_InputTime;

  public:
    static const int MIN_PACKET_LEN = 50; // Минимально возможная длина пакета
    static const int MAX_PACKET_LEN = 40000;

    //  Конструктор. Принимает в качестве параметра логгер и путь для сохранения файлов. Если путь пустой, файлы не сохраняются
    CRFParser(CLog *log, string savePath = "");
    virtual ~CRFParser();

    //  AddProtocol  добавляет декодер к общему пулу декодеров
    void AddProtocol(CRFProtocol *);
    void AddProtocol(string protocol = "all");

    // Определяет, может ли сигнал (импульс или пауза) быть корректным для какого-либо протокола
    bool IsGoodSignal(base_type signal);
    bool IsGoodSignal(bool isPulse, base_type length);

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
    // string ParseRepetitive(base_type *data, size_t length, size_t *readLength);

    // Tries to recognize packet from begin of data.
    // It returns all recognized packets to the end
    // and parsed length
    // std::vector<string> ParseToTheEnd(base_type *data, size_t length, size_t *readLength);

    // add some data to parse
    void AddInputData(base_type dataElement);
    void AddInputData(base_type *data, size_t len);
    // get all results parsed from all data given by AddInputData
    std::vector<string> ExtractParsed();

    //  Включает анализатор для пакетов, которые не получилось декодировать. Пока не реализованно
    void EnableAnalyzer();

    //  Сохраняет пакет в файл
    void SaveFile(base_type *data, size_t size, const char *prefix = "capture");

    static void SaveFile(base_type *data, size_t size, const char *prefix, string savePath, CLog *log);

    // Устанавливает путь для сохранения пакетов
    void SetSavePath(string savePath);
  private:
    void setMinMax();
};
