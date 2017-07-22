#pragma once
#include <cstdio>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>



/*! The predestination of this class is to do debug output in functions (or just blocks {...})
 *  it may be enabled and disabled in every function by one modification
 *  in first line: DPrintf dprintf; or DPrintf dprintf = DPrintf().disabled();
 *  or for more complicated choose - DPrintf dprintf = DPrintf().enable(expression);
 *
 *  Class is not made for logging, but for debugging.
 *
 *  By default DPrintf is globally disabled, i. e. without DPrintf::globallyEnable();
 *  command in the beginning dprintf(...) does nothing
 */

class DPrintf
{
    typedef std::ostream &(*EndlType)(std::ostream &);

    FILE *outFile;
    static FILE *defaultOutput;
    bool isEnabled;
    static bool isGloballyEnabled;

    std::string prefix;
    static int prefixLength;

    std::unique_ptr<std::ostringstream> streamPtr;
    void prepareStream();
    void flushStream(bool printPrefix = true);


    void printPrefix(const char **format);

    bool printFormatStringToNextParameter(const char *&s);

    int formatPrintRecursive(const char *s);

    template<typename T, typename... Args>
    int formatPrintRecursive(const char *s, T value, Args... args)
    {
        if (printFormatStringToNextParameter(s)) {
            (*this) << value;
            return 1 + formatPrintRecursive(s, args...);
        }
        return formatPrintRecursive("<extra arguments provided to printf>");
    }

    bool checkIfCFormat(const char *format, ...);

  public:
    /*! You need to call this static method to use DPrintf
     *  it enables DPrintf
     */
    static void globallyEnable(bool isGloballyEnabled = true);
    static void globallyDisable();

    /*!
        Calls vfprintf inside
        \param [in] format string, like usual, but you can place $P in the very beginning to print specified (by withPrefix function) prefix
        \param [in] parameters for format like in printf
        \return vfprintf return value
     */
    int cFormatPrint(const char *format, ...);
    //! shortened name for previous function
    template<typename... Args>
    inline int c(const char *format, Args... args)
    {
        return cFormatPrint(format, args...);
    }

    template<typename... Args>
    inline int formatPrint(const char *format, Args... args)
    {
        // modifies format
        printPrefix(&format);
        return formatPrintRecursive(format, args...);
    }


    /*! Enables stream-like output
     * (Though why -like? It uses C++ streams)
     * When you first time use it, it automatically prints prefix,
     * then it doesn't print it until 'endl' is got. (Then it will be printed again)
     * Really printing happens only after 'endl' is got.
     * \param [in] some param like you can put to 'cout'.
     * \return [out] called object
     */
    template <typename T>
    DPrintf &operator<<(const T &value)
    {
        prepareStream();
        (*streamPtr) << value;
        return *this;
    }

    DPrintf &operator<<(EndlType pf);

    template <typename T>
    DPrintf &operator<<(const std::vector<T> &t)
    {
        (*this) << "vector=[";
        int iter = -1;
        for (auto i : t)
            (*this) << (++iter > 0 ? ", " : "") << i;
        (*this) << "]";
        return *this;
    }

    template <typename T>
    DPrintf &operator<<(const std::set<T> &t)
    {
        (*this) << "set=[";
        int iter = -1;
        for (auto i : t)
            (*this) << (++iter > 0 ? ", " : "") << i;
        (*this) << "]";
        return *this;
    }

    template <typename T1, typename T2>
    DPrintf &operator<<(const std::map<T1, T2> &t)
    {
        (*this) << "map=[";
        int iter = -1;
        for (auto i : t)
            (*this) << (++iter > 0 ? ", " : "") << i.first << "->" << i.second;
        (*this) << "]";
        return *this;
    }

    template <typename T1, typename T2>
    DPrintf &operator<<(const std::unordered_map<T1, T2> &t)
    {
        (*this) << "u_map=[";
        int iter = -1;
        for (auto i : t)
            (*this) << (++iter > 0 ? ", " : "") << i.first << "->" << i.second;
        (*this) << "]";
        return *this;
    }

    template <typename T1, typename T2>
    DPrintf &operator<<(const std::pair<T1, T2> &t)
    {
        (*this) << "(" << t.first << ", " << t.second << ")";
        return *this;
    }


    /*! Intelligent output function - just-'%'-style formatting
     *  Usage: dprintf("Here i %!", std::string("am"));
     *  Using C++ streams inside
     * \param [in] format, arguments
     * \return [out] amount of written
     */
    template<typename... Args>
    inline int operator()(const char *format, Args... args)
    {
        if (!isActive())
            return 0;
        return formatPrint(format, args...);
    }


    // tells if object of DPrintf is enabled to write or not
    bool isActive() const;
    inline operator bool() const
    {
        return isActive();
    }

    // Constructing is like DPrintf().setOutputStream(stderr).enabled();
    // Or with default settings: DPrintf dprintf;
    DPrintf &&disabled();
    DPrintf &&enabled(bool isEnabled = true);

    DPrintf &&setOutputStream(FILE *outFile);
    static void setDefaultOutputStream(FILE *defaultOutFile);

    /*!
     * These functions adds possibility to type specified prefix
     * by adding "$P" to the beginning of format string.
     * Notice that they are doing nothing if DPrintf is not active
     * (disabled by globallyDisable or just by disabled)
     * \param [in] prefix string
     * \return reference to called DPrintf object
     */
    //DPrintf &withPrefix(std::string prefix);
    DPrintf &&withPrefix(std::string &&prefix);

    /*!
     * Acts like previous function, but constructs prefix from specified
     * file name, function name and number of line.
     * Suitable for get __FILE__, __FUNCTION__ and __LINE__
     * \param [in] file, function names
     * \param [in] line number
     * \return reference to called DPrintf object
     */
    DPrintf &&withPrefixFromDefine(const char *fileName, const char *functionName, const int line);

    static void setPrefixLength(int prefixLength);

    /*!
     * Constructor does nothing special
     * if you want to specify something use
     * DPrintf dprintf = DPrintf().enabled(true).setOutputStream(outputFile).withPrefix("func1");
     * Also it will be better to use constructive methods only in initialization.
     */
    DPrintf();
    DPrintf(DPrintf &&) = default;
    ~DPrintf();
};


/*! So sad to make a define, but __FILE__ and similar preprocessor constants are so delicious...
 *  Declares DPrintf object with "local" (include file, function, line) prefix
 */
#define DPRINTF_DECLARE(name, isEnabled) DPrintf name = DPrintf().enabled(isEnabled).withPrefixFromDefine(__FILE__, __FUNCTION__, __LINE__)
