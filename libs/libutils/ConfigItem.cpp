#include "stdafx.h"
#include "ConfigItem.h"
#include "Exception.h"

#ifdef USE_CONFIG

const char *CConfigItem::CONFIG_EXTENSION = "json";

using std::string;

CConfigItemList::CConfigItemList()
    : vector<CConfigItem * >()
{
}

CConfigItemList::CConfigItemList(const CConfigItemList &cpy)
    : vector<CConfigItem * >()
{
    for (const_iterator i = cpy.begin(); i != cpy.end(); i++)
        push_back(new CConfigItem(**i));
}

CConfigItemList::~CConfigItemList()
{
    for (iterator i = begin(); i != end(); i++)
        delete (*i);
}

CConfigItem::CConfigItem(void)
{
}

CConfigItem::CConfigItem(configNode node)
{
    m_Node = node;
}

CConfigItem::CConfigItem(const CConfigItem &cpy)
{
    m_Node = Json::Value();
    SetNode(cpy);
}


CConfigItem::~CConfigItem(void)
{
}

void CConfigItem::ParseXPath(string Path, string &First, string &Other)
{
    int pos = Path.find_first_of('/');
    if (pos > 0) {
        First = Path.substr(0, pos);
        Other = Path.substr(pos + 1);
    } else {
        First = Path;
        Other = "";
    }
}

bool CConfigItem::isEmpty()
{
    return m_Node.isObject() && m_Node.empty() || m_Node.isArray() && m_Node.empty();
};


string CConfigItem::getStr(string path, bool bMandatory, string defaultValue)
{
    string First, Other;
    ParseXPath(path, First, Other);

    if (Other.length()) {
        return getNode(First).getStr(Other, bMandatory, defaultValue);
    } else {
        string sVal = m_Node[path].asCString();
        return sVal;
    }
}

int CConfigItem::getInt(string path, bool bMandatory, int defaultValue)
{
    string First, Other;
    ParseXPath(path, First, Other);

    if (Other.length()) {
        return getNode(First).getInt(Other, bMandatory, defaultValue);
    } else {
        int iVal = m_Node[path].asInt();
        return iVal;
    }
}

bool CConfigItem::getBool(string path, bool bMandatory, bool defaultValue)
{
    string First, Other;
    ParseXPath(path, First, Other);

    if (Other.length()) {
        return getNode(First).getInt(Other, bMandatory, defaultValue);
    } else {
        bool bVal = m_Node[path].asBool();
        return bVal;
    }
}

CConfigItem CConfigItem::getNode(string path, bool bMandatory)
{
    string First, Other;
    ParseXPath(path, First, Other);

    if (Other.length()) {
        CConfigItem parent = getNode(First);

        if (parent.isEmpty())
            return CConfigItem();

        return parent.getNode(Other);
    } else {
        configNode retVal = m_Node[First];
        return retVal;
    }
}

void CConfigItem::getList(string path, CConfigItemList &list)
{
    if (!m_Node)
        return;

    //  ;

    string First, Other;
    ParseXPath(path, First, Other);

    if (Other.length()) {
        getNode(First).getList(Other, list);
    } else {
        list.clear();
        configNode values = m_Node[First];
        for (Json::ArrayIndex i = 0; i < values.size(); i++) {
            list.push_back(new CConfigItem(values[i]));
        }
    }
}

void CConfigItem::SetNode(configNode node)
{
    m_Node = node;
    /*  never free single node
    if (m_Node)
        xmlFreeNode(m_Node);
    m_Node = xmlCopyNode(node, true);
    */
}

bool CConfigItem::isNode()
{
    return m_Node.isObject();
};



#endif
