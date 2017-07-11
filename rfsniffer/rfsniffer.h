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
#include <memory>
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
        bool bRfmEnable;

        bool bCoreTestMod;

        string lircDevice;

        string mqttHost;

        string scannerParams;

        bool bDumpAllLircStream;
        bool bSimultaneouslyDumpStreamAndWork;

        string savePath;
        bool inverted;
        
        std::vector<std::string> enabledProtocols;
        std::vector<std::string> enabledFeatures;

        RFSnifferArgs();
    } args;

    std::unique_ptr<CLog> m_Log;

    Json::Value configJson;
    std::unique_ptr<SPI> mySPI;
    std::unique_ptr<RFM69OOK> rfm;

    const static unsigned long lircGetRecMode = _IOR('i', 0x00000002, uint32_t);

    // lirc
    int lircFD;
    // lirc_t buffer
    const static size_t maxMessageLength = (1 << 14);
    //const static size_t maxMessageLength = (1 << 25);
    const static size_t dataBuffSize = maxMessageLength * 2;

    lirc_t dataBuff[dataBuffSize];
    

    // utils
    bool waitForData(int fd, unsigned long maxusec);
    std::string composeString(const char *format, ...);
    void showCandidates(const string &path, const string &filePrefix);

    // read initial data
    void readEnvironmentVariables();
    void readCommandLineArguments(int argc, char **argv);
    void tryReadConfigFile();
    
    void logAllArguments();

    // initialize connections
    void initSPI();
    void initRFM();
    void openLirc() throw(CHaException);
    void tryJustScan() throw(CHaException);
    void tryFixThresh() throw(CHaException);

    // only dump all data that will be read from lirc device
    void tryDumpAllLircStream();

    // core work
    void receiveForever() throw(CHaException);

    // deinitialize connections
    void closeConnections();

  public:

    // this hunction should be called to start rfsniffer
    void run(int argc, char **argv);

    RFSniffer();
};
