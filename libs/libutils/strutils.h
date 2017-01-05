#ifndef __STR_UTILS_H
#define __STR_UTILS_H

#include <string>
#include <vector>
#include <map>
#include <cstring>

#include "libutils.h"



typedef LIBUTILS_API std::vector<std::string> string_vector;
typedef std::map<std::string, std::string> LIBUTILS_API string_map;
typedef std::map<int, std::string> map_i2s;
typedef std::map<std::string, int> map_s2i;

#ifdef WIN32
char inline *strnew(const char *src)
{
    size_t l = strlen(src);
    char *dest = new char[l + 1];
    strcpy_s(dest, l + 1, src);
    return dest;
};
#define FOLDER_DELIMETER '\\'
#define FOLDER_DELIMETER_STR "\\"

std::wstring LIBUTILS_API s2ws(const std::string &s);
std::string LIBUTILS_API ws2s(const std::wstring &s);

#ifdef _WIN32_WCE
    #define to_s(x) ws2s(x)
    #define to_ws(x) (x)
#else
#endif

#else
char inline *strnew(const char *src)
{
    size_t l = strlen(src);
    char *dest = new char[l + 1];
    strncpy(dest, src, l + 1);
    return dest;
};
char inline *strcpy_s(char *dest, size_t len, const char *src)
{
    return strncpy(dest, src, len);
};
#define FOLDER_DELIMETER '/'
#define FOLDER_DELIMETER_STR "/"
#endif

std::string LIBUTILS_API GetPath(std::string path);
void LIBUTILS_API SplitString(const std::string &s, char dlmt, string_vector &v);
void LIBUTILS_API SplitString(const std::string &s, std::string dlmt, string_vector &v);
void LIBUTILS_API SplitPair(const std::string &s, std::string dlmt, std::string &first,
                            std::string &second);
void LIBUTILS_API SplitPair(const std::string &s, char dlmt, std::string &first,
                            std::string &second);
void LIBUTILS_API SplitValues(const std::string &s, string_map &v, char groupDlmt = ' ',
                              char valueDlmt = '=');

#ifdef WIN32
    #define DLL_EXPORT __declspec(dllexport)
    #define DLL_IMPORT __declspec(dllimport)
#else
    #define DLL_EXPORT
    #define DLL_IMPORT
#endif

std::string LIBUTILS_API str_upper(const std::string &str);

enum BASE_PATH_TYPE {
    BASE_PATH_BIN = 34,
    BASE_PATH_CFG,
    BASE_PATH_LOG,
    BASE_PATH_LUA
};

std::string LIBUTILS_API GetBasePath(BASE_PATH_TYPE type);
std::string LIBUTILS_API ftoa(float f, int digits = 2);
std::string LIBUTILS_API itoa(int i);

inline int atoi(const std::string &s)
{
    return atoi(s.c_str());
};
inline double atof(const std::string &s)
{
    return atof(s.c_str());
};

#endif
