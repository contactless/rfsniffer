#ifndef __STR_UTILS_H
#define __STR_UTILS_H

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <array>
#include <unordered_map>
#include <string.h>

namespace strutils
{
/*!
    Class expanding possibilities of std::string
*/
class String : public std::string
{
    typedef std::string string;
  public:
    typedef std::vector<String> Vector;
    typedef std::unordered_map<String, String, std::hash<string> > Map;

    inline const string &StdString() const
    {
        return *this;
    }
    inline string &StdString()
    {
        return *this;
    }

    inline String Substr(size_t begin_pos, size_t length) const
    {
        return String(substr(begin_pos, length));
    }

    inline String SubstrFromTo(size_t begin_pos, int end_pos) const
    {
        return String(substr(begin_pos, end_pos - begin_pos));
    }


    /*!
    Splits String by delimiter to two ones
    \param [in] delimiter Delimiter
    \param [out] first, second Strings before delimiter and after
    \return 0 if success, -1 if there is not exactly one delimiter in input string
    */
    int SplitByExactlyOneDelimiter(string delimiter, string &first, string &second) const;
    int SplitByExactlyOneDelimiter(char delimiter, string &first, string &second) const;

    /*!
    Splits String by delimiter to two ones
    \param [in] delimiter Delimiter
    \param [out] first, second Strings before delimiter and after
    If input string doesn't contain delimiter that first string will be equal to input string and second to empty one
    */
    void SplitByFirstOccurenceDelimiter(string delimiter, string &first, string &second) const;
    void SplitByFirstOccurenceDelimiter(char delimiter, string &first, string &second) const;

    /*!
    Splits String by delimiter to some ones
    \param [in] delimiter Delimiter
    \param [out] splitted Strings between delimiter occurences
    */
    void Split(char delimiter, std::vector<String> &splitted) const;
    void Split(const string &delimiter, std::vector<String> &splitted) const;


    template <typename T, size_t N, typename ...TTypes>
    struct TupleTypeGenerator : TupleTypeGenerator<T, N - 1, T, TTypes...> {};

    template <typename T, typename ...TTypes>
    struct TupleTypeGenerator<T, 0, TTypes...> {
        typedef std::tuple<TTypes...> type;
    };

    template <size_t N>
    typename TupleTypeGenerator<String, N>::type Split(char delimeter, size_t beginPos = 0) const {
        String part = "";
        if (beginPos < length()) {
            int pos = find(delimeter, beginPos);
            if (pos == npos) {
                pos = length();
            }
            part = SubstrFromTo(beginPos, pos);
            beginPos = pos + 1;
        }
        return std::tuple_cat(
            std::tuple<String>(part),
            Split<N - 1>(delimeter, beginPos)
        );
    }
    /*!
    Splits String by delimiter to some ones
    \param [in] delimiter Delimiter
    \return splitted Strings between delimiter occurences
    */
    Vector Split(char delimiter) const;
    Vector Split(const string &delimiter) const;
    /*!
    Splits String by out_pair_delimiter to some ones
    and splits that ones as a pairs by in_pair_delimiter
    \param [in] out_pair_delimiter Delimiter between pairs
    \param [in] in_pair_delimiter Delimiter in pairs
    \return a map from first components of pairs to second ones \
    if a component is not splittable than there will be mapping from all "pair" to empty string
    */
    Map SplitToPairs(char out_pair_delimiter = ' ', char in_pair_delimiter = '=') const;


    String &operator=(const String &str) = default;
    String &operator=(String &&str) = default;

    String(): string() {}

    String(String &&str): string(std::move(str)) {}

    String(const String &str): string(str) {}

    String(const string &data): string(data) {}

    String(string &&data): string(std::move(data)) {}

    String(const char *data_array): std::string(data_array) {}

    //

    static String ComposeFormat(const char *format, ...);

    // converters

    static String ValueOf(float f, int digits = 2);

    static String ValueOf(int i);

    inline int IntValue() const
    {
        return atoi(c_str());
    };


    inline float FloatValue() const
    {
        return atof(c_str());
    };
};



template <>
std::tuple<> String::Split<0>(char delimeter, size_t beginPos) const;


class BufferWriter
{
    std::vector <char> buff;
    int offset;
  public:
    inline void clear()
    {
        offset = 0;
    }

    inline String getString()
    {
        return String(buff.data());
    }

    // consequently prints to buffer
    int printf(const char *format, ...);

    BufferWriter(): buff(2), offset(0) { }
};

// converters
std::string ftoa(float f, int digits = 2);
std::string itoa(int i);
int atoi(const std::string &s);
double atof(const std::string &s);



typedef std::vector<String> string_vector;
typedef std::map<String, String> string_map;
typedef std::map<int, String> map_i2s;
typedef std::map<String, int> map_s2i;

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

std::wstring s2ws(const std::string &s);
std::string ws2s(const std::wstring &s);

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

std::string GetPath(std::string path);

#ifdef WIN32
    #define DLL_EXPORT __declspec(dllexport)
    #define DLL_IMPORT __declspec(dllimport)
#else
    #define DLL_EXPORT
    #define DLL_IMPORT
#endif

std::string str_upper(const std::string &str);

enum BASE_PATH_TYPE {
    BASE_PATH_BIN = 34,
    BASE_PATH_CFG,
    BASE_PATH_LOG,
    BASE_PATH_LUA
};

std::string GetBasePath(BASE_PATH_TYPE type);

};




/*

typedef std::vector<std::string> string_vector;
typedef std::map<std::string, std::string> string_map;
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

std::wstring s2ws(const std::string &s);
std::string ws2s(const std::wstring &s);

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

std::string GetPath(std::string path);
void SplitString(const std::string &s, char dlmt, string_vector &v);
void SplitString(const std::string &s, std::string dlmt, string_vector &v);
int SplitPair(const std::string &s, std::string dlmt, std::string &first,
                            std::string &second);
void SplitPair(const std::string &s, char dlmt, std::string &first,
                            std::string &second);
void SplitValues(const std::string &s, string_map &v,
                              char groupDlmt = ' ',
                              char valueDlmt = '=');

#ifdef WIN32
    #define DLL_EXPORT __declspec(dllexport)
    #define DLL_IMPORT __declspec(dllimport)
#else
    #define DLL_EXPORT
    #define DLL_IMPORT
#endif

std::string str_upper(const std::string &str);

enum BASE_PATH_TYPE {
    BASE_PATH_BIN = 34,
    BASE_PATH_CFG,
    BASE_PATH_LOG,
    BASE_PATH_LUA
};

std::string GetBasePath(BASE_PATH_TYPE type);
std::string ftoa(float f, int digits = 2);
std::string itoa(int i);

inline int atoi(const std::string &s)
{
    return atoi(s.c_str());
};
inline double atof(const std::string &s)
{
    return atof(s.c_str());
};*/

#endif
