#include <cstdarg>
#include <cstdio>
#include "DebugPrintf.h"
#include "strutils.h"

using strutils::String;


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
    int formatOffset = 0;
    if (format[0] == '$' && format[1] == 'P') {
        fprintf(outFile, "%s", prefix.c_str());
        formatOffset = 2;
    }
    int ret = vfprintf(outFile, format + formatOffset, args);
    va_end (args);
    return ret;
}

DPrintf &DPrintf::disabled()
{
    isEnabled = false;
    return *this;
}

DPrintf &DPrintf::withPrefix(std::string prefix_)
{
    prefix = prefix_;
    return *this;
}


DPrintf &DPrintf::withPrefixFromDefine(const char *fileName_, const char *functionName,
                                       const int line)
{
    String fileName = String(fileName_).Split('/').back();
    return withPrefix(String::ComposeFormat("%s - %s (%d): ", fileName.c_str(), functionName, line));
}

DPrintf::DPrintf(): outFile(defaultOutput), isEnabled(true) {}

