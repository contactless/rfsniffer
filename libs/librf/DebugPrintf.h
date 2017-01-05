#pragma once
#include <cstdio>

/** The predestination of this class is to do debug output in functions (or just blocks {...})
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
  public:
    static void globallyEnable(bool isGloballyEnabled = true);
    static void globallyDisable();

    int operator()(const char *format, ...);

    // tells if object of DPrintf is enabled to write or not
    bool isActive();
    // Constructing is like DPrintf().setOutputStream(stderr).enabled();
    // Or DPrintf dprintf;
    DPrintf &disabled();
    DPrintf &enabled(bool isEnabled = true);
    static void setDefaultOutputStream(FILE *defaultOutFile);
    DPrintf &setOutputStream(FILE *outFile);
    DPrintf();
};
