#include "../libs/librf/DebugPrintf.h"

#include "stdafx.h"

#include "../libs/libutils/strutils.h"
//#include "../lua/liblua.h"

void LogTest();
void RfParserTest(string path);
void Rfm69Test();
void SnifferTest();
void MqttTest();

int main(int argc, char *argv[])
{
    // activate function-debug output
    DPrintf::globallyEnable();

    fprintf(stderr, "Begin tests\n");

    bool bTestLog = true, bTestParser = true, bTestRfm = false, bTestSniffer = false, bTestMqtt = false;
    string path;

    if (argc > 1)
        bTestLog = bTestParser = false;

    int c;
    while ( (c = getopt(argc, argv, "alprsmf:")) != -1) {
        switch (c) {
            case 'a':
                bTestLog = bTestParser = bTestRfm = bTestSniffer = bTestMqtt = true;
                break;
            case 'l':
                bTestLog = true;
                break;
            case 'p':
                bTestParser = true;
                break;
            case 'r':
                bTestRfm = true;
                break;
            case 's':
                bTestSniffer = true;
                break;
            case 'm':
                bTestMqtt = true;
                break;
            case 'f':
                path = optarg;
                break;
        }
    }

    srand(time(NULL));

    if (path.length()) {
        RfParserTest(path);
    }

    if (bTestLog)
        LogTest();

    if (bTestParser)
        RfParserTest("");

    if (bTestRfm)
        Rfm69Test();

    if (bTestSniffer)
        SnifferTest();

    if (bTestMqtt)
        MqttTest();
}
