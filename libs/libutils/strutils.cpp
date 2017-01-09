#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "strutils.h"
#include <cstring>
#include <cstdarg>


namespace strutils
{

typedef String::Vector Vector;
typedef String::Map Map;
typedef std::string string;


int String::SplitByExactlyOneDelimiter(string delimiter, string &first, string &second) const
{
    size_t pos = find(delimiter);

    if (pos != npos) {
        first = substr(0, pos);
        second = substr(pos + delimiter.length());
        if (second.find(delimiter) == second.npos)
            return 0;
    }

    first = "";
    second = "";
    return -1;
}

int String::SplitByExactlyOneDelimiter(char delimiter, string &first, string &second) const
{
    size_t pos = find(delimiter);

    if (pos != npos) {
        first = substr(0, pos);
        second = substr(pos + 1);
        if (second.find(delimiter) == second.npos)
            return 0;
    }

    first = "";
    second = "";
    return -1;
}

void String::SplitByFirstOccurenceDelimiter(string delimiter, string &first, string &second) const
{
    size_t pos = find(delimiter);

    if (pos != npos) {
        first = substr(0, pos);
        second = substr(pos + delimiter.length());
        return;
    }
	else {
		first = *this;
		second = "";
	}    
}

void String::SplitByFirstOccurenceDelimiter(char delimiter, string &first, string &second) const
{
    size_t pos = find(delimiter);

    if (pos != npos) {
        first = substr(0, pos);
        second = substr(pos + 1);
        return;
    }
	else {
		first = *this;
		second = "";
	}
}

void String::Split(char dlmt, std::vector<String> &splitted) const
{
    splitted.clear();
    size_t begin_pos = 0, pos;

    while ((pos = find(dlmt, begin_pos)) != npos) {
        if (pos != begin_pos)
            splitted.push_back(SubstrFromTo(begin_pos, pos));
        begin_pos = pos + 1;
    }

    if (begin_pos != length())
        splitted.push_back(SubstrFromTo(begin_pos, length()));
}

void String::Split(const string &dlmt, std::vector<String> &splitted) const
{
    splitted.clear();
    size_t begin_pos = 0, pos;

    while ((pos = find(dlmt, begin_pos)) != npos) {
        if (pos != begin_pos)
            splitted.push_back(SubstrFromTo(begin_pos, pos));
        begin_pos = pos + dlmt.length();
    }

    if (begin_pos != length())
        splitted.push_back(SubstrFromTo(begin_pos, length()));
}

Vector String::Split(char delimiter) const
{
    Vector splitted;
    Split(delimiter, splitted);
    return splitted;
}

Vector String::Split(const string &delimiter) const
{
    Vector splitted;
    Split(delimiter, splitted);
    return splitted;
}

Map String::SplitToPairs(char out_pair_delimiter, char in_pair_delimiter) const
{
    Map pairs;
    std::pair<String, String> one_pair;

    for (const String &pair_str : Split(out_pair_delimiter)) {
        int return_code = pair_str.SplitByExactlyOneDelimiter(
                              in_pair_delimiter, one_pair.first, one_pair.second);
        if (return_code == 0)
            pairs.insert(std::move(one_pair));
        else
            pairs[pair_str] = String("");
    }
    return pairs;
}


String String::ComposeFormat(const char *format, ...)
{
	std::vector<char> data;
	for (int loop = 0; loop < 2; loop++) {
		va_list args;
		va_start (args, format);
		int written_length = vsnprintf(
								 data.data(), data.size(), format, args
							 );
		va_end (args);
		// plus 1 for terminal 0 symbol in the end of the string
		data.resize(written_length + 1);
	}
	return data.data();
}

String String::ValueOf(float f, int digits)
{
    return ftoa(f, digits);
}

String String::ValueOf(int i)
{
    return itoa(i);
}

int BufferWriter::printf(const char *format, ...)
{
	while (true) {
		va_list args;
		va_start (args, format);
		int written_length = vsnprintf(
								 buff.data() + offset, buff.size() - offset - 1, format, args
							 );
		va_end (args);
		if (written_length < 0)
			return written_length;
		if (offset + written_length + 1 >= buff.size()) {
			buff.resize((offset + written_length + 1) * 2);
			continue;
		}
		offset += written_length;
		return written_length;
	}
}

std::string ftoa(float f, int digits)
{
    char Buffer[12];
    snprintf(Buffer, sizeof(Buffer), "%0.*f", digits, f);
    return Buffer;
}

std::string itoa(int i)
{
    char Buffer[12];
    snprintf(Buffer, sizeof(Buffer), "%d", i);
    return Buffer;
}

int atoi(const std::string &s)
{
    int i;
    sscanf(s.c_str(), "%d", &i);
    return i;
};

inline double atof(const std::string &s)
{
    float f;
    sscanf(s.c_str(), "%f", &f);
    return f;
};



string GetPath(string path)
{
    int pos = path.find_last_of(FOLDER_DELIMETER);
    if (pos > 0)
        return path.substr(0, pos);
    else
        return "";
}

#ifdef WIN32

std::wstring s2ws(const std::string &s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t *buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

std::string ws2s(const std::wstring &s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, NULL, false);
    char *buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, NULL, false);
    std::string r(buf);
    delete[] buf;
    return r;
}
#endif  //WIN32

string str_upper(const string &str)
{
    string s = str;
    for (string::iterator i = s.begin(); i != s.end(); i++)
        if (*i >= 'a' && *i <= 'z')
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
    if (pos > 0)
        BasePath  = BasePath.substr(0, pos + 1);
    else
        throw CHaException(CHaException::ErrInvalidMessageFormat, "Bad path %s", BasePath.c_str());
#elif defined(WIN32)
    char Buffer[4096];
    GetModuleFileName(NULL, Buffer, sizeof(Buffer));
    BasePath = Buffer;
    int pos = BasePath.find_last_of(FOLDER_DELIMETER);
    if (pos > 0)
        BasePath  = BasePath.substr(0, pos + 1);
    else
        throw CHaException(CHaException::ErrInvalidMessageFormat, "Bad path %s", BasePath.c_str());
#else
    switch(type) {
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


};


/*
string GetPath(string path)
{
    int pos = path.find_last_of(FOLDER_DELIMETER);
    if (pos > 0)
        return path.substr(0, pos);
    else
        return "";
}


void SplitString(const string &str, char dlmt, string_vector &v)
{
    v.clear();
    string s = str;

    while (true) {
        int pos = s.find(dlmt);
        if (pos == string::npos)
            break;

        v.push_back(s.substr(0, pos));
        s = s.substr(pos + 1);
    }

    if (s.length())
        v.push_back(s);
}

void SplitString(const string &str, string dlmt, string_vector &v)
{
    v.clear();
    string s = str;

    while (true) {
        int pos = s.find(dlmt);
        if (pos == string::npos)
            break;

        v.push_back(s.substr(0, pos));
        s = s.substr(pos + dlmt.length());
    }

    if (s.length())
        v.push_back(s);
}



int SplitPair(const string &str, string dlmt, string &first, string &second)
{
    size_t pos = s.find(dlmt);

    if (pos != s.npos) {
        first = s.substr(0, pos);
        second = s.substr(pos + dlmt.length());
        if (second.find(dlmt) == second.npos)
            return 0;
    }

    first = "";
    second = "";
    return -1;
}



int SplitPair(const string &str, char dlmt, string &first, string &second)
{
    size_t pos = s.find(dlmt);

    if (pos != s.npos) {
        first = s.substr(0, pos);
        second = s.substr(pos + 1);
    }

    first = "";
    second = "";
    return -1;
}



void LIBUTILS_API SplitValues(const string &s, string_map &v, char groupDlmt, char valueDlmt)
{
    v.clear();
    string_vector tmp;
    SplitString(s, groupDlmt, tmp);

    for_each_const(string_vector, tmp, i) {
        string first, second;
        if (SplitPair(*i, valueDlmt, first, second) == 0)
            v[first] = second;
    }
}


#ifdef WIN32

std::wstring s2ws(const std::string &s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t *buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

std::string ws2s(const std::wstring &s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, NULL, false);
    char *buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, NULL, false);
    std::string r(buf);
    delete[] buf;
    return r;
}
#endif  //WIN32

string str_upper(const string &str)
{
    string s = str;
    for (string::iterator i = s.begin(); i != s.end(); i++)
        if (*i >= 'a' && *i <= 'z')
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
    if (pos > 0)
        BasePath  = BasePath.substr(0, pos + 1);
    else
        throw CHaException(CHaException::ErrInvalidMessageFormat, "Bad path %s", BasePath.c_str());
#elif defined(WIN32)
    char Buffer[4096];
    GetModuleFileName(NULL, Buffer, sizeof(Buffer));
    BasePath = Buffer;
    int pos = BasePath.find_last_of(FOLDER_DELIMETER);
    if (pos > 0)
        BasePath  = BasePath.substr(0, pos + 1);
    else
        throw CHaException(CHaException::ErrInvalidMessageFormat, "Bad path %s", BasePath.c_str());
#else
    switch(type) {
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
*/

