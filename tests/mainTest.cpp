#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include "../libs/libutils/DebugPrintf.h"
#include "../libs/libutils/strutils.h"
#include "../libs/libutils/logging.h"
//#include "../lua/liblua.h"

using std::string;

void RfParserTest(string path);
void Rfm69Test();
void SnifferTest();
void MqttTest();

int main(int argc, char *argv[])
{
    log4cpp_AddOstream();

    // activate function-debug output
    DPrintf::globallyEnable(true);
    DPrintf::setPrefixLength(50);
    if (1) {
        DPRINTF_DECLARE(dprint, false);
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

    bool bTestParser = true, bTestRfm = false, bTestSniffer = false, bTestMqtt = false;
    std::string path;

    if (argc > 1)
        bTestParser = false;

    int c;
    while ( (c = getopt(argc, argv, "alprsmf:")) != -1) {
        switch (c) {
            case 'a':
                bTestParser = bTestRfm = bTestSniffer = bTestMqtt = true;
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

    DPRINTF_DECLARE(dprintf, false);

    if (path.length()) {
        dprintf("$P Before RfParserTest\n");
        RfParserTest(path);
        dprintf("$P After RfParserTest\n");
    }

    if (bTestParser) {
        dprintf("$P Before RfParserTest 2\n");
        RfParserTest("");
        dprintf("$P After RfParserTest 2\n");
    }

    if (bTestRfm) {
        dprintf("$P Before Rfm69Test\n");
        Rfm69Test();
        dprintf("$P After Rfm69Test\n");
    }

    if (bTestSniffer) {
        dprintf("$P Before SnifferTest\n");
        SnifferTest();
        dprintf("$P After SnifferTest\n");
    }

    if (bTestMqtt) {
        dprintf("$P Before MqttTest\n");
        MqttTest();
        dprintf("$P After MqttTest\n");
    }

    dprintf("$P end of tests\n");

    return 0;

}
