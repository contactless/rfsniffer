#pragma once
#include "libutils.h"
#include "ConfigItem.h"

#ifdef _LIBUTILS_USE_XML

#ifdef _LIBUTILS_USE_XML_LIBXML2
	typedef struct _xmlDoc xmlDoc;
	typedef xmlDoc *xmlDocPtr;
#elif defined(_LIBUTILS_USE_XML_MSXML)
	using namespace MSXML2;
#endif

class LIBUTILS_API CXmlDocument :
	public CConfigItem
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	xmlDocPtr m_Document;
#elif defined(_LIBUTILS_USE_XML_MSXML)
	IXMLDOMDocument m_Document;
#endif

public:
	CXmlDocument(void);
	virtual ~CXmlDocument(void);

	void Load(string ConfigFileName);
	void Parse(string Xml);
	CConfigItem GetRootElement();
	void Close();

};

#endif
