#pragma once
#include <deque>
#include <memory>

#include "stdafx.h"
#include "rflib.h"
#include "RFProtocol.h"


class CRFAnalyzer;

class RFLIB_API CRFParser
{
    typedef std::deque<base_type> InputContainer;
    typedef std::string string;
    
    CLog *m_Log;
    std::vector<std::unique_ptr<CRFProtocol>> m_Protocols;
    std::vector<int> m_ProtocolsBegins;

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

    
    // Пытается декодировать всю последовательность [data, data + len) каким-то одним декодером
    // используется для тестирования
    string Parse(base_type *data, size_t len);
    
    // Удалает все остаточную информацию о пришедших данных
    // В том числе входной буфер, указатели на позиции в нем для разных протоколов
    // Данные о повторах пакетов внутри парсеров протоколов и т. д.
    void ClearRetainedInputData();

    // add some data to parse
    // При добавлении пришедших данных 
    // происходит проверка на конец пакета и попытка декодировать 
    // для каждого протокола 
    // Результат можно проверить вызовом ExtractParsed
    void AddInputData(base_type dataElement);
    void AddInputData(base_type *data, size_t len);
    
    // Если данные заканчиваются раньше, чем наступает признак конца пакета
    // (приходит импульс/пауза не используемый протоколом)
    // то стоит вызвать эту функцию, она попробует декодировать то что есть
    void TryToParseExistingData();
    
    // get all results parsed from all data given by AddInputData
    std::vector<string> ExtractParsed();

    //  Сохраняет пакет в файл
    void SaveFile(base_type *data, size_t size, const char *prefix = "capture");

    static void SaveFile(base_type *data, size_t size, const char *prefix, string savePath, CLog *log);

    // Устанавливает путь для сохранения пакетов
    void SetSavePath(string savePath);
};
