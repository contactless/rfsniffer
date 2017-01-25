#include <cstdio>
#include <cstdlib>
#include <string>

#include "../libs/libutils/DebugPrintf.h"
#include "../libs/libutils/strutils.h"
//#include "../lua/liblua.h"

using std::string;

void LogTest();
void RfParserTest(string path);
void Rfm69Test();
void SnifferTest();
void MqttTest();

int main(int argc, char *argv[])
{
    // activate function-debug output
    DPrintf::globallyEnable();
    DPrintf::setPrefixLength(30);
    if (0) {
        DPRINTF_DECLARE(dprint, true);
        dprint("$P Hello world!\n");
        dprint << "hehe" << "12133" << 13242 << (int *)nullptr << std::endl;
        dprint << "mamasmdamsd" << std::endl;
        dprint << "1" << std::endl;
        dprint << "2" << std::endl;
        dprint << "3" << std::endl;
        dprint("$P %   mimi %", 1, "42\n");
        dprint("$P %d   mimi %s", 1, "42\n");
        dprint("% d\n", 45);
        dprint("$P %  d\n", 45);
        dprint.c("$P %   d\n", 45);
        dprint("$P %   d\n", string(""));
        exit(0);

    }
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
