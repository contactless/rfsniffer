#include "stdafx.h"
#include "libutils.h"
#include "Exception.h"
#include "XmlDocument.h"

#ifdef HAVE_LIBXML2_LIBXML_TREE_H
	#include "libxml2/libxml/tree.h"
	#include "libxml2/libxml/parser.h"
	#include "libxml2/libxml/xpath.h"
	#include "libxml2/libxml/xpathInternals.h"
#elif defined(_LIBUTILS_USE_XML_LIBXML2)
	#include "libxml/tree.h"
	#include "libxml/parser.h"
	#include "libxml/xpath.h"
	#include "libxml/xpathInternals.h"
#endif

#ifdef _LIBUTILS_USE_XML

CXmlDocument::CXmlDocument(void)
{
	m_Document = NULL;
}

CXmlDocument::~CXmlDocument(void)
{
	xmlFreeDoc(m_Document);
}

void CXmlDocument::Load(string ConfigFileName)
{
	m_Document = xmlParseFile(ConfigFileName.c_str());

	if (m_Document == NULL) 
		throw CHaException(CHaException::ErrInvalidConfig, "Config file (%s) not found",ConfigFileName.c_str());

	SetNode(GetRootElement());
}

void CXmlDocument::Parse(string Xml)
{
	m_Document = xmlParseMemory(Xml.c_str(), Xml.length());

	if (m_Document == NULL) 
		throw CHaException(CHaException::ErrInvalidConfig, "Config file (%s) not found",Xml.c_str());

	SetNode(GetRootElement());
}



CConfigItem CXmlDocument::GetRootElement()
{
	return xmlDocGetRootElement(m_Document);
};

void CXmlDocument::Close()
{
}

#endif