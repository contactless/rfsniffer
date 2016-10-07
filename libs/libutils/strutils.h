#ifndef __STR_UTILS_H
#define __STR_UTILS_H

#include "libutils.h"

typedef LIBUTILS_API vector<string> string_vector;
typedef map<string, string> LIBUTILS_API string_map;
typedef map<int, string> map_i2s;
typedef map<string, int> map_s2i;

#ifdef WIN32
	char inline *strnew(const char *src){size_t l =strlen(src); char *dest = new char[l+1]; strcpy_s(dest, l+1, src);return dest;};
	#define FOLDER_DELIMETER '\\'
	#define FOLDER_DELIMETER_STR "\\"

	std::wstring LIBUTILS_API s2ws(const std::string& s);
	std::string LIBUTILS_API ws2s(const std::wstring& s);

#ifdef _WIN32_WCE
	#define to_s(x) ws2s(x)
	#define to_ws(x) (x)
#else
#endif

#else
	char inline *strnew(const char *src){size_t l =strlen(src); char *dest = new char[l+1]; strncpy(dest, src, l+1);return dest;};
	char inline *strcpy_s(char *dest, size_t len, const char *src){return strncpy(dest, src, len);};
	#define FOLDER_DELIMETER '/'
	#define FOLDER_DELIMETER_STR "/"
#endif

string LIBUTILS_API GetPath(string path);
void LIBUTILS_API SplitString(const string &s, char dlmt, string_vector &v);
void LIBUTILS_API SplitString(const string &s, string dlmt, string_vector &v);
void LIBUTILS_API SplitPair(const string &s, string dlmt, string &first, string &second);
void LIBUTILS_API SplitPair(const string &s, char dlmt, string &first, string &second);
void LIBUTILS_API SplitValues(const string &s, string_map &v, char groupDlmt=' ', char valueDlmt='=');

#ifdef WIN32
	#define DLL_EXPORT __declspec(dllexport)
	#define DLL_IMPORT __declspec(dllimport)
#else
	#define DLL_EXPORT
	#define DLL_IMPORT
#endif

string LIBUTILS_API str_upper(const string &str);

enum BASE_PATH_TYPE
{
	BASE_PATH_BIN=34,
	BASE_PATH_CFG,
	BASE_PATH_LOG,
	BASE_PATH_LUA
};

string LIBUTILS_API GetBasePath(BASE_PATH_TYPE type);
string LIBUTILS_API ftoa(float f, int digits=2);
string LIBUTILS_API itoa(int i);

inline int atoi(const string &s){return atoi(s.c_str());};
inline double atof(const string &s){return atof(s.c_str());};

#endif
