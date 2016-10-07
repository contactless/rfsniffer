#include "stdafx.h"
#include "ConfigItem.h"
#include "Exception.h"

#ifdef _LIBUTILS_USE_XML_LIBXML2
	#include "libxml/tree.h"
	#include "libxml/parser.h"
	#include <libxml/xpath.h>
	#include <libxml/xpathInternals.h>
#endif

#ifdef USE_CONFIG

#ifdef _LIBUTILS_USE_XML_LIBXML2
	const char* CConfigItem::CONFIG_EXTENSION = "xml";
#elif defined(USE_JSON)
	const char* CConfigItem::CONFIG_EXTENSION = "json";
#endif


CConfigItemList::CConfigItemList()
:vector<CConfigItem*>()
{
}

CConfigItemList::CConfigItemList(const CConfigItemList& cpy)
:vector<CConfigItem*>()
{
	for (const_iterator i=cpy.begin();i!=cpy.end();i++)
		push_back(new CConfigItem(**i));
}

CConfigItemList::~CConfigItemList()
{
	for (iterator i=begin();i!=end();i++)
		delete (*i);
}

CConfigItem::CConfigItem(void)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	m_Node = NULL;
#endif
}

CConfigItem::CConfigItem(configNode node)
{
	m_Node = node;
}

CConfigItem::CConfigItem(const CConfigItem& cpy)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	m_Node = NULL;
#elif defined(USE_JSON)
	m_Node = Json::Value();
#endif

	SetNode(cpy);
}


CConfigItem::~CConfigItem(void)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
//	if (m_Node)
//		xmlFreeNode(m_Node);
#endif
}

void CConfigItem::ParseXPath(string Path, string &First, string &Other)
{
	int pos = Path.find_first_of('/');
	if (pos>0)
	{
		First = Path.substr(0,pos);
		Other = Path.substr(pos+1);
	}
	else
	{
		First = Path;
		Other = "";
	}
}
/*
string CConfigItem::GetValue()
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	if (m_Node)
	{
		xmlNodePtr node = m_Node->children;

		if (!m_Node)
			return "";

		short type = node->type;
		const xmlChar *val = node->content;/*
		char* asciiVal = XMLString::transcode(val);
		string result = asciiVal;
		XMLString::release(&asciiVal);* /
		return (char*)val;
	}

	return "";
#endif
}


string CConfigItem::GetValue(string path)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	CConfigItem elem = GetElement(path);
	return elem.GetValue();
#endif
}
*/

bool CConfigItem::isEmpty() 
{ 
#ifdef _LIBUTILS_USE_XML_LIBXML2
	return m_Node == NULL;
#elif defined(USE_JSON)
	return m_Node.isObject() && m_Node.empty() || m_Node.isArray() && m_Node.empty();
#endif
};


string CConfigItem::getStr(string path, bool bMandatory, string defaultValue)
{
	string First, Other;
	ParseXPath(path, First, Other);

	if (Other.length())
	{
		return getNode(First).getStr(Other, bMandatory, defaultValue);
	}
	else
	{
#ifdef _LIBUTILS_USE_XML_LIBXML2
		char* val = (char*)xmlGetProp(m_Node, BAD_CAST path.c_str());

		if (!val)
		{
			if (bMandatory)
				throw CHaException(CHaException::ErrAttributeNotFound, path);
			else
				return defaultValue;
		}

		string sVal = val;
		xmlFree(val);
#elif defined(USE_JSON)
		string sVal = m_Node[path].asCString();
#endif
		return sVal;
	}
}

int CConfigItem::getInt(string path, bool bMandatory, int defaultValue)
{
	string First, Other;
	ParseXPath(path, First, Other);

	if (Other.length())
	{
		return getNode(First).getInt(Other, bMandatory, defaultValue);
	}
	else
	{
#ifdef _LIBUTILS_USE_XML_LIBXML2
		char* val = (char*)xmlGetProp(m_Node, BAD_CAST path.c_str());

		if (!val)
		{
			if (bMandatory)
				throw CHaException(CHaException::ErrAttributeNotFound, path);
			else
				return defaultValue;
		}

		int iVal = atoi(val);
		xmlFree(val);
#elif defined(USE_JSON)
		int iVal = m_Node[path].asInt();
#endif
		return iVal;
	}
}

CConfigItem CConfigItem::getNode(string path, bool bMandatory)
{
	string First, Other;
	ParseXPath(path, First, Other);

	if (Other.length())
	{
		CConfigItem parent = getNode(First);

		if (parent.isEmpty())
			return CConfigItem();

		return parent.getNode(Other);
	}
	else
	{
#ifdef _LIBUTILS_USE_XML_LIBXML2
		xmlNodePtr retVal = NULL, cur = m_Node->children;

		while(cur)
		{
			if (cur->type==XML_ELEMENT_NODE && strcmp((char*)cur->name, First.c_str())==0)
			{
				if (retVal)
				{
					return NULL;
				}
				retVal = cur;
			}

			cur = cur->next;
		}
#elif defined(USE_JSON)
		configNode retVal = m_Node[First];
#endif

		return retVal;
	}
}

void CConfigItem::getList(string path, CConfigItemList &list)
{
	if (!m_Node)
		return;

//	;

	string First, Other;
	ParseXPath(path, First, Other);

	if (Other.length())
	{
		getNode(First).getList(Other, list);
	}
	else
	{
		list.clear();
#ifdef _LIBUTILS_USE_XML_LIBXML2
		xmlNodePtr cur = m_Node->children;

		while(cur)
		{
			if (First.compare((char*)cur->name)==0)
			{
				list.push_back(new CConfigItem(cur));
			}
			cur = cur->next;
		}

		if (list.size() == 0)
		{
			CConfigItem node = getNode(path + "s");
			if (node.isNode())
			{
				node.getList(path, list);
			}
		}
#elif defined(USE_JSON)
		configNode values = m_Node[First];
		for (Json::ArrayIndex i=0;i<values.size();i++)
		{
			list.push_back(new CConfigItem(values[i]));
		}
#endif
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
#ifdef _LIBUTILS_USE_XML_LIBXML2
		return m_Node && m_Node->type==XML_ELEMENT_NODE;
#elif defined(USE_JSON)
	return m_Node.isObject();
#endif
};



#endif