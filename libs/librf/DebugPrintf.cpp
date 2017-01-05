#include <cstdarg>
#include <cstdio>
#include "DebugPrintf.h"


bool DPrintf::isGloballyEnabled = false;
FILE *DPrintf::defaultOutput = stderr;



void DPrintf::globallyEnable(bool isGloballyEnabled_)
{
    isGloballyEnabled = isGloballyEnabled_;
}

void DPrintf::globallyDisable()
{
    isGloballyEnabled = false;
}

bool DPrintf::isActive()
{
    return isEnabled && isGloballyEnabled && (defaultOutput != NULL);
}

DPrintf &DPrintf::enabled(bool isEnabled_)
{
    isEnabled = isEnabled_;
    return *this;
}
DPrintf &DPrintf::setOutputStream(FILE *outFile_)
{
    outFile = outFile_;
    return *this;
}

void DPrintf::setDefaultOutputStream(FILE *defaultOutFile)
{
    defaultOutput = defaultOutFile;
}

int DPrintf::operator()(const char *format, ...)
{
    if (!isActive())
        return 0;
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

DPrintf::DPrintf(): outFile(defaultOutput), isEnabled(true) {}

