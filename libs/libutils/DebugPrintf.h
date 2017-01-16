#pragma once
#include <cstdio>
#include <string>

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
    FILE *outFile;
    static FILE *defaultOutput;
    bool isEnabled;
    static bool isGloballyEnabled;

    std::string prefix;

  public:
    static void globallyEnable(bool isGloballyEnabled = true);
    static void globallyDisable();

    /*!
        Calls vfprintf inside
        \param [in] format string, like usual, but you can place $P in the very beginning to print specified (by withPrefix function) prefix
        \param [in] parameters for format like in printf
        \return vfprintf return value
     */
    int operator()(const char *format, ...);

    // tells if object of DPrintf is enabled to write or not
    bool isActive();
    // Constructing is like DPrintf().setOutputStream(stderr).enabled();
    // Or DPrintf dprintf;
    DPrintf &disabled();
    DPrintf &enabled(bool isEnabled = true);
    static void setDefaultOutputStream(FILE *defaultOutFile);
    DPrintf &setOutputStream(FILE *outFile);
    DPrintf &withPrefix(std::string prefix);
    DPrintf &withPrefixFromDefine(const char *fileName, const char *functionName, const int line);
    DPrintf();
};


/*! So sad to make a define, but __FILE__ and similar preprocessor constants are so delicious...
 *  Declares DPrintf object with "local" (include file, function, line) prefix
 */
#define DPRINTF_DECLARE(name, isEnabled) DPrintf name = DPrintf().enabled(isEnabled).withPrefixFromDefine(__FILE__, __FUNCTION__, __LINE__)
