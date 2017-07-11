#pragma once
#pragma warning (disable: 4251)

#include <vector>
#include <string>


#include <jsoncpp/json/json.h>


#include "strutils.h"
#include "libutils.h"

#pragma warning (disable: 4275)
class CConfigItem;
class LIBUTILS_API CConfigItemList:
    public std::vector<CConfigItem *>
{
  public:
    CConfigItemList();
    CConfigItemList(const CConfigItemList &cpy);

    ~CConfigItemList();
};


typedef class Json::Value configNode;


class LIBUTILS_API CConfigItem
{
    typedef std::string string;

    configNode m_Node;

  public:
    CConfigItem(void);
    CConfigItem(const CConfigItem &cpy);
    CConfigItem(configNode node);
    virtual ~CConfigItem(void);

    static const char *CONFIG_EXTENSION;

    static void ParseXPath(string Path, string &First, string &Other);


    string getStr(string path, bool bMandatory = true, string defaultValue = "");
    int getInt(string path, bool bMandatory = true, int defaultValue = 0);
    bool getBool(string path, bool bMandatory = true, bool defaultValue = false);
    CConfigItem getNode(string path, bool bMandatory = true);
    void getList(string path, CConfigItemList &list);


    bool isEmpty();
    bool isNode();
    configNode GetNode()
    {
        return m_Node;
    };
    const CConfigItem &operator =(const CConfigItem &node)
    {
        SetNode(node.m_Node);
        return *this;
    };

  private:
    void SetNode(configNode node);
    void SetNode(const CConfigItem &node)
    {
        SetNode(node.m_Node);
    };

};
