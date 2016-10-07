#ifndef __HA_CONFIG_H
#define __HA_CONFIG_H

#ifdef HAVE_CONFIG_H
#	include "../../haconfig.inc"
#endif

#if defined(_WIN32_WCE)
#	define HA_WINCE
#	define snprintf _snprintf
#elif defined(WIN32)
#	define HA_WIN32
#	define HA2
#	define HAVE_JSONCPP_JSON_JSON_H
#	define JSONCPP_DISABLE_DLL_INTERFACE_WARNING
#	define HAVE_LIBXML_TREE_H
#	define HAVE_LUA_H
//#define snprintf sprintf_s
#else
#	define HA_LINUX
#endif

#if defined(HAVE_LUA_H) || defined(HAVE_LUA5_2_LUA_H)
#	define USE_LUA
#endif 

#ifdef HA2
#	ifdef HAVE_LIBXML_TREE_H
#		define USE_XML
#	endif
#	define USE_CONFIG
#endif

#if defined(HAVE_JSONCPP_JSON_JSON_H) || defined(HAVE_JSON_JSON_H)
#	define USE_JSON
#endif

#ifdef USE_XML
#	define _LIBUTILS_USE_XML
#	define _LIBUTILS_USE_XML_LIBXML2
#elif defined(USE_JSON) && defined(HAVE_JSON_JSON_H)
//#	define CConfigItem Jsoncpp::Value
#endif



#endif
