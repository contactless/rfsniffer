#include "stdafx.h"
#include <wchar.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "strutils.h"
#include "Exception.h"


string GetPath(string path)
{
	int pos = path.find_last_of(FOLDER_DELIMETER);
	if (pos>0)
		return path.substr(0, pos);
	else
		return "";
}


void SplitString(const string &str, char dlmt, string_vector &v)
{
	v.clear();
	string s = str;

	while (true)
	{
		int pos = s.find(dlmt);
		if (pos==string::npos)
			break;

		v.push_back(s.substr(0, pos));
		s = s.substr(pos+1);
	}

	if (s.length())
		v.push_back(s);
}

void SplitString(const string &str, string dlmt, string_vector &v)
{
	v.clear();
	string s = str;

	while (true)
	{
		int pos = s.find(dlmt);
		if (pos == string::npos)
			break;

		v.push_back(s.substr(0, pos));
		s = s.substr(pos + dlmt.length());
	}

	if (s.length())
		v.push_back(s);
}

void SplitPair(const string &str, string dlmt, string &first, string &second)
{
	string s = str;
	int part=0;

	while (true)
	{
		size_t pos = s.find(dlmt);
		if (pos==string::npos)
			break;

		string value = s.substr(0, pos);
		s = s.substr(pos+1);

		switch (part++)
		{
			case 0: first=value; break;
			case 1: second=value; break;
			default: throw CHaException(CHaException::ErrBadParam, str);
		}
	}

	if (part!=2)
		throw CHaException(CHaException::ErrBadParam, str);
}

void SplitPair(const string &s, char dlmt, string &first, string &second)
{
	int part=0;
	size_t pos = s.find(dlmt);

	if (pos == s.npos)
		throw CHaException(CHaException::ErrBadParam, s);

	first = s.substr(0, pos);
	second = s.substr(pos + 1);
}

void LIBUTILS_API SplitValues(const string &s, string_map &v, char groupDlmt, char valueDlmt)
{
	v.clear();
	string_vector tmp;
	SplitString(s, groupDlmt, tmp);

	for_each_const(string_vector, tmp, i)
	{
		string first, second;	
		SplitPair(*i, valueDlmt, first, second);
		v[first] = second;
	}
}


#ifdef WIN32

std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

std::string ws2s(const std::wstring& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, NULL, false); 
    char* buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, NULL, false);
    std::string r(buf);
    delete[] buf;
    return r;
}
#endif  //WIN32
/*
string str_upper(const string &str)
{
	char *Buffer=new char[str.length()+1];
	strcpy(Buffer, str.c_str());
	strupr(Buffer);
	string retval = Buffer;
	delete []Buffer;
	return retval;
} 

#else
*/
string str_upper(const string &str)
{
	string s = str;
	for (string::iterator i=s.begin();i!=s.end();i++)
		if (*i>='a' && *i<='z')
			*i = *i - 32;
	return s; 
}



string GetBasePath(BASE_PATH_TYPE type)
{
	string BasePath;

	#if defined (_WIN32_WCE)
		WCHAR Buffer[4096];
		GetModuleFileNameW(NULL, Buffer, sizeof(Buffer));
		BasePath = ws2s(Buffer);
		int pos = BasePath.find_last_of(FOLDER_DELIMETER);
		if (pos>0)
			BasePath  = BasePath.substr(0, pos+1);
		else
			throw CHaException(CHaException::ErrInvalidMessageFormat, "Bad path %s", BasePath.c_str());
	#elif defined(WIN32) 
		char Buffer[4096];
		GetModuleFileName(NULL, Buffer, sizeof(Buffer));
		BasePath = Buffer;
		int pos = BasePath.find_last_of(FOLDER_DELIMETER);
		if (pos>0)
			BasePath  = BasePath.substr(0, pos+1);
		else
			throw CHaException(CHaException::ErrInvalidMessageFormat, "Bad path %s", BasePath.c_str());
	#else
		switch(type)
		{
		case BASE_PATH_BIN:
			BasePath = "/opt/bin/";
			break;
		case BASE_PATH_CFG:
			BasePath = "/opt/etc/";
			break;
		case BASE_PATH_LOG:
			BasePath = "/run/";
			break;
		case BASE_PATH_LUA:
			BasePath = "/opt/share/ha/lua/";
			break;
}
	#endif

	return BasePath;
}

string ftoa(float f, int digits)
{
	char Buffer[20];
	snprintf(Buffer, sizeof(Buffer), "%0.*f",digits, f);
	return Buffer;
}

string itoa(int i)
{
	char Buffer[20];
	snprintf(Buffer, sizeof(Buffer), "%d",i);
	return Buffer;
}

