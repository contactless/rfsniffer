/* Test of Logging system
 * (mlog->Printf(4, ....);)
 * */


#include "stdafx.h"
#include "../libs/libutils/logging.h"

void LogTest()
{
    CLog log;
    log.Open("Test.log");
    printf("test 01 %d \n", __LINE__);
    log.Printf(0, "test 1 %d ", __LINE__);

    CLog *pLog = CLog::GetLog("Main");
    log.Printf(0, "test 2 %d ", __LINE__);
    pLog->Printf(0, "test");
    log.Printf(0, "test 3 %d ", __LINE__);

    char Buffer[] = "qwertyuiopasdfghjkl;'ZXCVBNM<>?\0\1\5\t\n\rÿקסלטעüביצףךו";
    log.PrintBuffer(0, Buffer, 0);
    log.PrintBuffer(0, Buffer, 1);
    log.PrintBuffer(0, Buffer, 15);
    log.PrintBuffer(0, Buffer, 16);
    log.PrintBuffer(0, Buffer, 17);
    log.PrintBuffer(0, Buffer, sizeof(Buffer));
}
