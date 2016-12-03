#pragma once
#include <cstdio>


class DPrintf
{
    FILE *outFile;
    bool isEnabled;
  public:
    int operator()(const char *format, ...);

    DPrintf &disabled();
    DPrintf(FILE *outFile = stderr);
};
