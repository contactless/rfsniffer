#include <cstdarg>
#include <cstdio>

#include "strutils.h"
#include "DebugPrintf.h"

using strutils::String;


bool DPrintf::isGloballyEnabled = false;
FILE *DPrintf::defaultOutput = stderr;
int DPrintf::prefixLength = -1;


bool DPrintf::checkIfCFormat(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(nullptr, 0, format, args);
    va_end(args);
    return (ret >= 0);
}
bool DPrintf::printFormatStringToNextParameter(const char *&s)
{
    while (*s) {
        if (*s == '%' && *(++s) != '%')
            return true;
        (*this) << *s++;
    }
    return false;
}
int DPrintf::formatPrintRecursive(const char *s)
{
    while (printFormatStringToNextParameter(s)) {
        (*this) << "<missing argument>";
    }
    flushStream(false);
    return 0;
}

void DPrintf::globallyEnable(bool isGloballyEnabled_)
{
    isGloballyEnabled = isGloballyEnabled_;
}

void DPrintf::globallyDisable()
{
    isGloballyEnabled = false;
}

bool DPrintf::isActive() const
{
    return isEnabled && isGloballyEnabled && (defaultOutput != NULL);
}

DPrintf &&DPrintf::enabled(bool isEnabled_)
{
    isEnabled = isEnabled_;
    return std::move(*this);
}
DPrintf &&DPrintf::setOutputStream(FILE *outFile_)
{
    outFile = outFile_;
    return std::move(*this);
}

void DPrintf::setDefaultOutputStream(FILE *defaultOutFile)
{
    defaultOutput = defaultOutFile;
}


void DPrintf::printPrefix(const char **format_)
{
    const char *&format = *format_;
    if (format[0] == '$' && format[1] == 'P') {
        format += 2;
        if (prefix.empty())
            return;
        if (prefixLength < 0)
            fprintf(outFile, "%s", prefix.c_str());
        else {
            if ((int)prefix.length() <= prefixLength)
                fprintf(outFile, "%*s", prefixLength, prefix.c_str());
            else
                fprintf(outFile, "%s", prefix.substr(prefix.length() - prefixLength, prefixLength).c_str());

        }
    }
}

int DPrintf::cFormatPrint(const char *format, ...)
{
    if (!isActive())
        return 0;
    // modifies format
    printPrefix(&format);
    va_list args;
    va_start (args, format);
    int ret = vfprintf(outFile, format, args);
    va_end (args);
    return ret;
}



void DPrintf::prepareStream()
{
    if (!streamPtr) {
        streamPtr.reset(new std::ostringstream());
        // add prefix
    }
}

void DPrintf::flushStream(bool needPrintPrefix)
{
    if (streamPtr) {
        cFormatPrint((needPrintPrefix ? "$P%s" : "%s"), streamPtr->str().c_str());
        streamPtr->str("");
    }
}


DPrintf &DPrintf::operator<<(EndlType pf)
{
    prepareStream();
    (*streamPtr) << std::endl;
    flushStream();
    return *this;
}

DPrintf &&DPrintf::disabled()
{
    isEnabled = false;
    return std::move(*this);
}
/*
DPrintf &DPrintf::withPrefix(std::string prefix_)
{
    return withPrefix(prefix_);
}*/

DPrintf &&DPrintf::withPrefix(std::string &&prefix_)
{
    if (!isActive())
        return std::move(*this);
    prefix = prefix_;
    return std::move(*this);
}


DPrintf &&DPrintf::withPrefixFromDefine(const char *fileName_, const char *functionName,
                                        const int line)
{
    if (!isActive())
        return std::move(*this);
    String fileName = String(fileName_).Split('/').back();
    return withPrefix(std::move((std::string)String::ComposeFormat("%s - %s (%d): ", fileName.c_str(),
                                functionName, line)));
}

void DPrintf::setPrefixLength(int prefixLength_)
{
    prefixLength = prefixLength_;
}

DPrintf::DPrintf(): outFile(defaultOutput), isEnabled(true) {}

DPrintf::~DPrintf()
{
    flushStream(false);
}

