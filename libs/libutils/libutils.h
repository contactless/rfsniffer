// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBUTILS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBUTILS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WIN32
	
	#ifdef LIBUTILS_EXPORTS
		#define LIBUTILS_API __declspec(dllexport)
		//#define JSON_DLL_BUILD   //Defined in project
	#else
		#define LIBUTILS_API __declspec(dllimport)
		#define JSON_DLL
	#endif

	//#define _LIBUTILS_USE_XML_MSXML

#else
	#define LIBUTILS_API 
#endif

#define for_each(_type, _object, _iterator) for (_type::iterator _iterator=_object.begin(); _iterator!=_object.end();_iterator++)
#define for_each_const(_type, _object, _iterator) for (_type::const_iterator _iterator=_object.begin(); _iterator!=_object.end();_iterator++)

#include "haconfig.h"