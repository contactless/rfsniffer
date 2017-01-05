#pragma once

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <dirent.h>
#include <unistd.h>
#include <functional>
#include "../libs/libutils/logging.h"
#include "../libs/libutils/Exception.h"
#include "../libs/libutils/Config.h"
#include "../libs/libutils/ConfigItem.h"
#include "../libs/librf/spidev_lib++.h"
#include "../libs/librf/RFM69OOKregisters.h"
#include "../libs/librf/RFM69OOK.h"
#include "../libs/librf/RFParser.h"
#include "../libs/librf/RFProtocolLivolo.h"
#include "../libs/librf/RFProtocolX10.h"
#include "../libs/librf/RFProtocolRST.h"
#include "../libs/librf/RFProtocolRaex.h"
#include "MqttConnect.h"


class RFSniffer
{
  protected:
    typedef base_type lirc_t;

    struct RFSnifferArgs {
        string configName;

        bool bDebug;
        bool bDumpAllRegs;
        bool bLircPedantic;

        string spiDevice;
        long spiSpeed;
        int gpioInt;

        int fixedThresh;
        int rssi;

        string lircDevice;

        string mqttHost;

        string scannerParams;
        int writePackets;
        string savePath;
        bool inverted;

        RFSnifferArgs();
    } args;

    CLog *m_Log;

    SPI mySPI;
    RFM69OOK rfm;

    const static unsigned long lircGetRecMode = _IOR('i', 0x00000002, uint32_t);

    // lirc
    int lircFD;
    // lirc_t buffer
    const static size_t maxMessageLength = (1 << 17);
    const static size_t dataSize = maxMessageLength * 2;
    lirc_t data[dataSize];
    lirc_t *const dataBegin;
    lirc_t *const dataEnd;
    lirc_t *dataPtr;
    inline size_t remainingDataCount()
    {
        return dataEnd - dataPtr;
    }
    inline size_t readDataCount()
    {
        return dataPtr - dataBegin;
    }


    // utils
    bool waitForData(int fd, unsigned long maxusec);
    std::string composeString(const char *format, ...);
    void showCandidates(const string &path, const string &filePrefix);

    // read initial data
    void readEnvironmentVariables();
    void readCommandLineArguments(int argc, char **argv);
    void tryReadConfigFile();

    // initialize connections
    void initSPI();
    void initRFM();
    void openLirc() throw(CHaException);
    void tryJustScan() throw(CHaException);
    void tryFixThresh() throw(CHaException);

    // core work
    void receiveForever() throw(CHaException);

    // deinitialize connections
    void closeConnections();

  public:

    // this hunction should be called to start rfsniffer
    void run(int argc, char **argv);

    RFSniffer();
};
