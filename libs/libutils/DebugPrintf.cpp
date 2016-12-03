#include <cstdarg>
#include <cstdio>
#include "DebugPrintf.h"


int DPrintf::operator()(const char *format, ...)
{
    if (!isEnabled)
        return false;
    va_list args;
    va_start (args, format);
    int ret = vfprintf(outFile, format, args);
    va_end (args);
    return ret;
}

DPrintf &DPrintf::disabled()
{
    isEnabled = false;
    return *this;
}

DPrintf::DPrintf(FILE *outFile): outFile(outFile), isEnabled(true) {}

