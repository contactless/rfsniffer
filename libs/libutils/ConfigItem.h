#pragma once
#pragma warning (disable: 4251)

#include "libutils.h"
#include "strutils.h"

#ifdef USE_CONFIG

#ifdef HAVE_JSON_JSON_H
#	include "json/json.h"
#elif defined(HAVE_JSONCPP_JSON_JSON_H)
#	include "jsoncpp/json/json.h"
#else 
#	error josn.h not found
#endif

#pragma warning (disable: 4275)
class CConfigItem;
class LIBUTILS_API CConfigItemList:
	public vector<CConfigItem*>
{
public:
	CConfigItemList();
	CConfigItemList(const CConfigItemList& cpy);

	~CConfigItemList();
};

#ifdef _LIBUTILS_USE_XML_LIBXML2
	typedef struct _xmlNode xmlNode;
	typedef xmlNode *xmlNodePtr;
	typedef xmlNodePtr configNode;
#elif defined(USE_JSON)
	typedef class Json::Value configNode;
#else
#	error usupported configuration
#endif


class LIBUTILS_API CConfigItem
{
	friend class CXmlDocument;
	configNode m_Node;

public:
	CConfigItem(void);
	CConfigItem(const CConfigItem& cpy);
	CConfigItem(configNode node);
	virtual ~CConfigItem(void);

	static const char* CONFIG_EXTENSION;
	
	static void ParseXPath(string Path, string &First, string &Other);


	string getStr(string path, bool bMandatory = true, string defaultValue = "");
	int getInt(string path, bool bMandatory = true, int defaultValue = 0);
	CConfigItem getNode(string path, bool bMandatory = true);
	void getList(string path, CConfigItemList &list);

	
	/*
	static string GetValue(const char* path);
	static string GetAttribute(const char* path);
	static CConfigItem GetElement(const char* path);
	static CConfigItemList GetElementList(const char* path);
*/
	//string GetValue();
	//string GetValue(string path);
	//string GetAttribute(string path, bool bMandatory=true, string defaultValue="");
	//int GetAttributeInt(string path, bool bMandatory=true, string defaultValue=""){return atoi(GetAttribute(path, bMandatory, defaultValue));};
	//CConfigItem GetElement(string path);
	//void GetElementsList(string path, CConfigItemList &list);

	bool isEmpty();
	bool isNode();
	configNode GetNode() { return m_Node; };
	const CConfigItem& operator =(const CConfigItem& node) { SetNode(node.m_Node); return *this; };

private:
	void SetNode(configNode node);
	void SetNode(const CConfigItem& node) { SetNode(node.m_Node); };

};

#endif
