#ifndef __CONFIG_H
#define __CONFIG_H

#include "haconfig.h"
#include "ConfigItem.h"
#ifdef USE_XML
    #include "XmlDocument.h"
#endif

class LIBUTILS_API CConfig
{
#ifdef USE_XML
    CXmlDocument m_Document;
#elif defined(USE_JSON)
    CConfigItem m_Document;
#else
#endif

  public:
    CConfig(void);
    virtual ~CConfig(void);
    void Load(std::string ConfigFileName);
    void Close();

    std::string getStr(std::string path, bool bMandatory = true, std::string defaultValue = "");
    CConfigItem getNode(const char *path);
    void getList(const char *path, CConfigItemList &list);
    CConfigItem getRoot()
    {
        return m_Document;
    };

};

#endif
