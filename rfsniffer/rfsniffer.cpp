#include <algorithm>
#include <cassert>
#include <fstream>
#include <stdexcept>

#include <../libs/libutils/DebugPrintf.h>
#include <../libs/libutils/strutils.h>

#include "rfsniffer.h"

typedef std::string string;
using namespace strutils;

RFSniffer::RFSnifferArgs::RFSnifferArgs():
    config(""),
    bDebug(false),
    bDumpAllRegs(false),
    bLircPedantic(true),

    spiDevice("/dev/spidev32766.0"),
    spiSpeed(500000),
    gpioInt(38),

    fixedThresh(0),
    rssi(0),
    bRfmEnable(true),

    bCoreTestMod(false),

    lircDevice("/dev/lirc0"),

    mqttHost("localhost"),

    scannerParams(""),

    bDumpAllLircStream(false),
    bSimultaneouslyDumpStreamAndWork(false),

    savePath("."),
    inverted(false),

    enabledProtocols({"All"}),
    enabledFeatures()
{

}

bool RFSniffer::waitForData(int fd, unsigned long maxusec)
{
    return waitForData({fd}, maxusec);
}

int RFSniffer::waitForData(std::initializer_list<int> fd, unsigned long maxusec)
{
    //DPRINTF_DECLARE(dprintf, false);

    fd_set fds;
    int ret;
    struct timeval tv;
    tv.tv_sec = maxusec / 1000000;
    tv.tv_usec = maxusec % 1000000;

    while (1) {
        FD_ZERO(&fds);
        for (int oneFD : fd) {
            //dprintf("$P set fd %\n", oneFD);
            if (oneFD >= 0) {
                FD_SET(oneFD, &fds);
            }
        }
        do {
            do {
                //dprintf("$P fd upper bound %\n", *std::max_element(fd.begin(), fd.end()) + 1);
                ret = select(*std::max_element(fd.begin(), fd.end()) + 1, &fds, NULL, NULL, (maxusec > 0 ? &tv : NULL));
                if (ret == 0)
                    return 0;
            } while (ret == -1 && errno == EINTR);
            if (ret == -1) {
                LOG(WARN) << "RF select() failed\n";
                continue;
            }
        } while (ret == -1);

        int ret = 0;
        for (int oneFD : fd) {
            //dprintf("$P check fd %\n", oneFD);
            ++ret;
            if (oneFD >= 0 && FD_ISSET(oneFD, &fds)) {
                //dprintf("$P Can read from % fd (% in arguments)\n", oneFD, ret);
                return ret;
            }
        }
    }

    return 0;
}


// usual call is showCandidates("/dev/", "spidev") - shows all files in path and beginning from filePrefix
void RFSniffer::showCandidates(const string &path, const string &filePrefix)
{
    const size_t len = filePrefix.length();
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            string name = ent->d_name;
            if (name.length() >= len && name.substr(0, len) == filePrefix)
                LOG(INFO) << "\tCandidate is: /dev/" << ent->d_name;
        }
        closedir(dir);
    } else {
        LOG(INFO) << "\tAnd couldn't see in /dev for " << path;
    }
}

void RFSniffer::readEnvironmentVariables()
{
    char *irq = getenv("WB_GPIO_RFM_IRQ");
    char *spiMajor = getenv("WB_RFM_SPI_MAJOR");
    char *spiMinor = getenv("WB_RFM_SPI_MINOR");

    if (spiMajor || spiMinor) {
        // plus 1 because of very strange setting of WB_RFM_SPI_MAJOR
        // it's somehow connected with python driver
        int spiMajorVal = spiMajor ? atoi(spiMajor) + 1 : 32766;
        int spiMinorVal = spiMinor ? atoi(spiMinor) : 0;
        args.spiDevice = String::ComposeFormat("/dev/spidev%d.%d", spiMajorVal, spiMinorVal);
    }

    if (irq && atoi(irq) > 0)
        args.gpioInt = atoi(irq);
}

void RFSniffer::readCommandLineArguments(int argc, char **argv)
{
    // I don't know why, but signed is essential (though it should be by default??)
    for (signed char c = ' '; c != -1; c = getopt(argc, argv, "Ds:m:l:TS:f:r:twWc:i")) {
        switch (c) {
            case ' ':
                // just nothing, for first iteration to avoid repetitive getopt
                break;

            case 'D':
                args.bDebug = true;
                break;

            case 's':
                args.spiDevice = optarg;
                break;

            case 'l':
                args.lircDevice = optarg;
                break;

            case 'T':
                args.bLircPedantic = false;
                args.bRfmEnable = false;
                args.bCoreTestMod = true;
                args.spiDevice = "/dev/null";
                break;

            case 'm':
                args.mqttHost = optarg;
                break;

            case 'i':
                args.inverted = true;
                break;

            case 'S':
                args.scannerParams = optarg;
                break;

            case 'f':
                args.fixedThresh = atoi(optarg);
                break;

            case 'r':
                args.rssi = atoi(optarg);
                break;

            case 'g':
                args.gpioInt = atoi(optarg);
                break;

            case 't':
                args.bDumpAllRegs = true;
                break;

            case 'W':
                args.bDumpAllLircStream = true;
                break;

            case 'w':
                args.bDumpAllLircStream = true;
                args.bSimultaneouslyDumpStreamAndWork = true;
                break;

            case 'c':
                args.config = optarg;
                break;

            case '?':
                printf("Usage: rfsniffer [params]\n");
                printf("-D - debug mode. Write good but not decoded packets to files\n");
                printf("-g <DIO0 gpio> - set custom DIO0 GPIO number. Default %d\n", args.gpioInt);
                printf("-s <spi device> - set custom SPI device. Default %s\n", args.spiDevice.c_str());
                printf("-l <lirc device> - set custom lirc device. Default %s\n"
                       "    If it is set with '/dev/null' driver will not read from it but still will not exit.", args.lircDevice.c_str());
                printf("-m <mqtt host> - set custom mqtt host. Default %s\n", args.mqttHost.c_str());
                printf("-W - write all data from lirc device to file until signal from keyboard\n");
                printf("-w - like -W but simultaneously do normal work of driver\n");
                printf("-S -<low level>..-<high level>/<seconds for step> - scan for noise. \n");
                printf("-r <RSSI> - reset RSSI Threshold after each packet. 0 - Disabled. Default %d\n",
                       (int)args.rssi);
                printf("-f <fixed Threshold> - Use OokFixedThresh with fixed level. 0 - Disabled. Default %d\n",
                       args.fixedThresh);

                printf("-T - disable pedantic check of lirc character device (may use pipe instead)\n" \
                       "    disable using SPI and RFM, do specific test output\n");
                printf("-c configfile or config itself - specify config file (parameters in config file are priority) \n");
                //          printf("-f <sampling freq> - set custom sampling freq. Default %d\n", samplingFreq);
                exit(0);
            default:
                printf ("?? getopt returned character code 0%o ??\n", c);
                exit(-1);
        }
    }
}

void RFSniffer::tryReadConfig()
{
    if (args.config.empty())
        return;
    try {
        Json::Value config;
        // reading
        if (args.config[0] != '{') {
            // reading from file
            std::fstream fs(args.config.c_str(), std::fstream::in);
            fs >> config;
            fs.close();
        }
        else {
            // reading json config from string
            std::istringstream(args.config) >> config;
        }

        auto radio = config["radio"];
        if (!!radio) {
            auto lircDevice = radio["lirc_device"];
            if (!!lircDevice)
                args.lircDevice = lircDevice.asString();
            auto spiDevice = radio["spi_device"];
            if (!!spiDevice)
                args.spiDevice = spiDevice.asString();
            auto rfmIrq = radio["rfm_irq"];
            if (!!rfmIrq)
                args.gpioInt = rfmIrq.asInt();
            auto rssi = radio["rssi"];
            if (!!rssi)
                args.rssi = rssi.asInt();
        }

        auto mqttHost = config["mqtt"]["host"];
        if (!!mqttHost)
            args.mqttHost = mqttHost.asString();

        auto debug = config["debug"];
        if (!!debug) {
            auto savePath = debug["save_path"];
            if (!!savePath)
                args.savePath = savePath.asString();
            auto dumpStream = debug["dump_stream"];
            if (!!dumpStream && dumpStream.asBool()) {
                args.bDumpAllLircStream = true;
                args.bSimultaneouslyDumpStreamAndWork = true;
            }
            auto log = debug["log"];
            if (!!log) {
                if (!log.isArray())
                    throw std::runtime_error("log must be array");
                for (int i = 0; i < (int)log.size(); ++i) {
                    auto logItem = log[i];
                    auto fileName = logItem["file_name"];
                    auto name = logItem["name"];
                    if (!name || !fileName)
                        throw std::runtime_error("incomplete log item");
                    log4cpp_AddOutput(name.asString(), fileName.asString());
                }
            }
        }

        auto enabledProtocols = config["enabled_protocols"];
        if (!!enabledProtocols) {
            if (!enabledProtocols.isArray())
                throw std::runtime_error("enabled_protocols must be array");
            args.enabledProtocols.clear();
            for (int i = 0; i < (int)enabledProtocols.size(); ++i) {
                args.enabledProtocols.push_back(enabledProtocols[i].asString());
                //fprintf(stderr, "Enabled protocol: %s\n", args.enabledProtocols.back().c_str());
            }
        }

        args.enabledFeatures = config["enabled_features"];

        this->configJson = config;
    }
    catch (CHaException ex) {
        fprintf(stderr, "Failed load config. Error: %s", ex.GetExplanation().c_str());
        exit(-1);
    }
    //catch (Json::Exception ex) {
    //    fprintf(stderr, "Failed load config. Error: %s", ex.what());
    //    exit(-1);
    //}
    catch (std::exception ex) {
        fprintf(stderr, "Failed load config. Error: %s", ex.what());
        exit(-1);
    }
}

void RFSniffer::logAllArguments() {

    LOG(INFO) << "RFSniffer parameters";

    #define print_bool(a) LOG(INFO) << ("  ||  " #a " = ") << (args.a ? "true" : "false");
    #define print_str(a) LOG(INFO) << ("  ||  " #a " = '") << args.a << "'";
    #define print_int(a) LOG(INFO) << ("  ||  " #a " = ") << args.a;
    #define print_gap() LOG(INFO) << "  ||  ";

    print_str(config);
    print_bool(bDebug);
    print_bool(bDumpAllRegs);
    print_bool(bLircPedantic);
    print_gap();
    print_str(spiDevice);
    print_int(spiSpeed);
    print_int(gpioInt);
    print_gap();
    print_int(fixedThresh);
    print_int(rssi);
    print_bool(bRfmEnable);
    print_gap();
    print_bool(bCoreTestMod);
    print_gap();
    print_str(lircDevice);
    print_gap();
    print_str(mqttHost);
    print_gap();
    print_str(scannerParams);
    print_gap();
    print_bool(bDumpAllLircStream);
    print_bool(bSimultaneouslyDumpStreamAndWork);
    print_gap();
    print_str(savePath);
    print_bool(inverted);

    #undef print_bool
    #undef print_str
    #undef print_int
    #undef print_gap
}

void RFSniffer::initSPI()
{
    if (!args.bRfmEnable)
        return;
    spi_config_t spi_config;
    spi_config.mode = 0;
    spi_config.speed = args.spiSpeed;
    spi_config.delay = 0;
    spi_config.bits_per_word = 8;
    // do not use "=" because destructor breaks all
    mySPI.reset(new SPI(args.spiDevice.c_str(), &spi_config));
    if (!mySPI->begin()) {
        LOG(ERROR) << "SPI init failed (probably no such device: " << args.spiDevice << ")";
        showCandidates("/dev/", "spidev");
        LOG(INFO) << "Please contact developers";
        exit(-1);
    }
}

void RFSniffer::initRFM()
{
    if (!args.bRfmEnable)
        return;

    rfm.reset(new RFM69OOK(mySPI.get(), args.gpioInt));

    if (args.bDumpAllRegs) {
        char *Buffer = (char *)dataBuff;
        char *BufferPtr = Buffer;
        size_t BufferSize = dataBuffSize;
        for (int i = 0; i <= 0x4F; i++) {
            byte cur = rfm->readReg(i);
            BufferPtr += snprintf(BufferPtr, BufferSize - (BufferPtr - Buffer), "Reg_%02X = %02x ", i, cur);

            if (i % 4 == 3) {
                LOG(INFO) << Buffer;
                BufferPtr = Buffer;
            }
        }

        if (BufferPtr != Buffer) {
            LOG(INFO) << Buffer;
        }

        LOG(INFO) << String::ComposeFormat("Reg_%02x = %02x Reg_%02x = %02x", 0x6F, rfm->readReg(0x6F), 0x71,
                      rfm->readReg(0x71));
        exit(0);
    }

    rfm->initialize();
}

void RFSniffer::openLirc() throw(CHaException)
{
    lircFD = open(args.lircDevice.c_str(), O_RDONLY);
    if (lircFD == -1) {
        LOG(ERROR) << "Error opening device " << args.lircDevice;
        showCandidates("/dev/", "lirc");
        exit(EXIT_FAILURE);
    }

    if (args.bLircPedantic) {
        struct stat s;
        if (fstat(lircFD, &s) == -1) {
            LOG(ERROR) << "fstat: Can't read file status! : " << strerror(errno);
            throw CHaException(CHaException::ErrBadParam, "fstat: Can't read file status! : %s\n",
                               strerror(errno));
        }
        if (!S_ISCHR(s.st_mode)) {
            LOG(ERROR) << "Lirc device is not character device! st_mode = " << (int)s.st_mode;
            throw CHaException(CHaException::ErrBadParam, "%s is not a character device\n",
                               args.lircDevice.c_str());
        }

        uint32_t mode = 2;
        if (ioctl(lircFD, lircGetRecMode, &mode) == -1) {
            LOG(ERROR) << "This program is only intended for receivers supporting the pulse/space layer.\n";
            throw CHaException(CHaException::ErrBadParam,
                               "This program is only intended for receivers supporting the pulse/space layer.");
        }
    }
}

void RFSniffer::tryJustScan() throw(CHaException)
{
    string scannerParams = args.scannerParams;

    if (scannerParams.length() == 0)
        return;

    LOG(INFO) << "Scanner params are: \"" << scannerParams.c_str() << '"';
    int minLevel = 30;
    int maxLevel = 60;
    int scanTime = 15;

    int pos = scannerParams.find("..");
    if (pos != (int)scannerParams.npos) {
        minLevel = atoi(scannerParams.substr(0, pos));
        scannerParams = scannerParams.substr(pos + 2);

        pos = scannerParams.find("/");
        if (pos != (int)scannerParams.npos) {
            maxLevel = atoi(scannerParams.substr(0, pos));
            scanTime = atoi(scannerParams.substr(pos + 1));
        } else {
            maxLevel = atoi(scannerParams);
        }
    } else {
        LOG(ERROR) << "Error in parsing scanner params." \
                "Use -S <low level>..<high level>/<seconds for step>\n";
        exit(-1);
    }


    int curLevel = minLevel;

    while (curLevel < maxLevel) {
        rfm->receiveEnd();
        rfm->writeReg(REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_FIXED);
        rfm->writeReg(REG_OOKFIX, curLevel);
        rfm->receiveBegin();
        int pulses = 0;
        time_t startTime = time(NULL);

        while (difftime(time(NULL), startTime) < scanTime) {
            if (!waitForData(lircFD, 100))
                continue;
            int result = read(lircFD, (void *)dataBuff, dataBuffSize);
            if (result == -1 && errno == EAGAIN)
                result = 0;
            if (result == 0 && args.bLircPedantic) {
                LOG(ERROR) << "read() failed [during opening lirc device part]\n";
                break;
            }
            for (int i = 0; i < result; i++) {
                if (CRFProtocol::isPulse(dataBuff[i]))
                    pulses++;
            }
        }

        LOG(INFO) << "Recv fixed level=" << curLevel << " pulses=" << pulses;
        curLevel++;
    }
    close(lircFD);
    exit(0);
}

void RFSniffer::tryFixThresh() throw(CHaException)
{
    if (rfm && args.fixedThresh) {
        rfm->writeReg(REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_FIXED);
        rfm->writeReg(REG_OOKFIX, args.fixedThresh);
    }
}

void RFSniffer::tryDumpAllLircStream()
{
    if (!args.bDumpAllLircStream || args.bSimultaneouslyDumpStreamAndWork)
        return;

    DPRINTF_DECLARE(dprintf, false);

    LOG(INFO) << "Saving all data from lirs device started.\n"\
                  "Print any button to finish\n" \
                  "Lirc fd is " << lircFD;

    if (rfm)
        rfm->receiveBegin();

    std::vector<lirc_t> lircData;
    while (true) {
        if (waitForData(lircFD, 100000)) {
            int resultBytes = read(lircFD, (void *)dataBuff, sizeof(dataBuff));
            LOG(INFO) << "Read" << resultBytes << " bytes\n";

            // I hope this never happen
            while (resultBytes % sizeof(lirc_t) != 0) {
                LOG(WARN) << "Bad amount (amount % 4 != 0) of bytes read from lirc";
                usleep(1000);
                int remainBytes = sizeof(lirc_t) - resultBytes % sizeof(lirc_t);
                int readTailBytes = read(lircFD, (void *)((char *)dataBuff + resultBytes), remainBytes);
                if (readTailBytes != -1)
                    resultBytes += readTailBytes;
            }

            int result = resultBytes / sizeof(lirc_t);
            lircData.insert(lircData.end(), dataBuff, dataBuff + result);
        }
        if (waitForData(0, 100000))
            break;
    }

    CRFParser::SaveFile(lircData.data(), lircData.size(), "dump-all", args.savePath);

    closeConnections();
    exit(0);
}

void RFSniffer::receiveForever() throw(CHaException)
{
    DPRINTF_DECLARE(dprintf, false);

    dprintf("$P Start!\n");

    LOG(INFO) << "RF Receiver begins";

    if (rfm)
        rfm->receiveBegin();

    auto devicesConfig = configJson["devices"];

    CMqttConnection conn(args.mqttHost, rfm.get(), devicesConfig, args.enabledFeatures);

    int connFD = conn.socket();

    CRFParser parser;

    for (auto protocol : args.enabledProtocols)
        parser.AddProtocol(protocol);

    string dumpFileName;
    std::unique_ptr<FILE, int(*)(FILE *)> dumpFile(nullptr, fclose);

    if (args.bDumpAllLircStream) {
        assert(args.bSimultaneouslyDumpStreamAndWork);
        dumpFileName = CRFParser::GenerateFileName("dump-all", args.savePath);
        // make unique_ptr for automatic close of file
        dumpFile = std::unique_ptr<FILE, int(*)(FILE *)>(fopen(dumpFileName.c_str(), "w"), fclose);

        LOG(INFO) << "Saving stream dump to '" << dumpFileName << "'. Press Ctrl-C to stop driver";
    }

    int lastRSSI = -1000, minGoodRSSI = 0;
    time_t lastReport = 0, lastRegularWorkTime = time(NULL);

    dprintf("$P Start cycle lircFD = %, connFD = %\n", lircFD, connFD);


    while (true) {
        DPRINTF_DECLARE(dprintf, false);
        // try is placed here to handle exceptions and just restart, not to crush
        try {
            dprintf("$P process all parsed messages\n");
            for (const string &parsedResult : parser.ExtractParsed()) {
                LOG(INFO) << "RF Received: " << parsedResult
                          << ". RSSI=" << lastRSSI << " (" << minGoodRSSI << ")";
                if (minGoodRSSI > lastRSSI)
                    minGoodRSSI = lastRSSI;
                if (args.bCoreTestMod)
                    fprintf(stderr, "TEST_RF_RECEIVED %s\n", parsedResult.c_str());
                try {
                    conn.NewMessage(parsedResult);
                    conn.loop(0);
                } catch (CHaException ex) {
                    LOG(ERROR) << "conn.NewMessage failed: Exception " << ex.GetExplanation();
                    if (args.bCoreTestMod)
                        throw;
                }
            }

            // do not read very often
            // and process incoming messages
            if (conn.loop(10) != MOSQ_ERR_SUCCESS) {
                conn.reconnect();
            }
            // try get more data and sleep if fail
            const int waitDataReadUsec = 1000000;
            switch (waitForData({lircFD, connFD}, waitDataReadUsec)) {
                case 1: {
                    dprintf("$P got data from lirc\n");
                    /*
                     * This strange thing is needed to lessen received trash
                     */
                    if (0 && rfm) {
                        rfm->receiveEnd();
                        usleep(10000);
                        rfm->receiveBegin();
                    }
                    // do not try to read much, because it easier to process it by small parts
                    //size_t tryToReadCount = std::min(normalMessageLength, remainingDataCount());
                    dprintf("$P before read() call (can read % bytes) \n", sizeof(dataBuff));
                    int resultBytes = read(lircFD, (void *)dataBuff, sizeof(dataBuff));
                    dprintf("$P after read() call\n");

                    if (resultBytes == 0) {
                        if (args.bLircPedantic) {
                            LOG(INFO) << "read() failed [during endless cycle]\n";
                        }
                        if (args.bCoreTestMod) {
                            dprintf("$P No more input data. Exiting!\n");
                            // if lirc is a fictive device then break
                            // if lirc is dumb device (/dev/null) consider situation as just lack of data
                            if (args.lircDevice != "/dev/null")
                                return;
                        }
                    } else {
                        dprintf("$P % bytes were read from lirc device\n", resultBytes);

                        // I hope this never happen
                        while (resultBytes % sizeof(lirc_t) != 0) {
                            LOG(INFO) << "Bad amount (amount % 4 != 0) of bytes read from lirc";
                            usleep(waitDataReadUsec);
                            int remainBytes = sizeof(lirc_t) - resultBytes % sizeof(lirc_t);
                            int readTailBytes = read(lircFD, (void *)((char *)dataBuff + resultBytes), remainBytes);
                            if (readTailBytes != -1)
                                resultBytes += readTailBytes;
                        }

                        int result = resultBytes / sizeof(lirc_t);

                        if (dumpFile) {
                            fwrite(dataBuff, sizeof(lirc_t), result, dumpFile.get());
                            fflush(dumpFile.get());
                        }

                        /// pass data to parser
                        dprintf("$P before AddInputData() call\n");
                        parser.AddInputData(dataBuff, result);
                        dprintf("$P after AddInputData() call\n");

                        if (lastReport != time(NULL) && result >= 32) {
                            LOG(INFO) << "RF got data " << result << " signals. RSSI=" << lastRSSI;
                            lastReport = time(NULL);
                        }

                        if (rfm)
                            lastRSSI = rfm->readRSSI();
                    }
                    break;
                };
                case 2: {
                    dprintf("$P got data from mqtt\n");
                    conn.loop();
                    break;
                };
                default: {};
            }
            dprintf("$P after read more\n");


            if (rfm && args.rssi < 0)
                rfm->setRSSIThreshold(args.rssi);

            // do regular work, but not very often
            if (lastRegularWorkTime != time(NULL)) {
                lastRegularWorkTime = time(NULL);
                conn.SendAliveness();
                conn.SendScheduledChanges();
            }

        } catch (CHaException ex) {
            LOG(ERROR) << "Exception " << ex.GetExplanation();
            if (args.bCoreTestMod)
                throw;
        }
    }
}

void RFSniffer::closeConnections()
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P called\n");
    if (rfm)
        rfm->receiveEnd();
    if (lircFD >= 0)
        close(lircFD);

}

void RFSniffer::run(int argc, char **argv)
{
    DPrintf::globallyEnable(false);
    //DPrintf::setDefaultOutputStream(fopen("/root/rfs.log", "wt"));
    DPrintf::setPrefixLength(40);

    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Driver has been started.\n");

    readEnvironmentVariables();
    dprintf("$P Environment variables have been read.\n");
    readCommandLineArguments(argc, argv);
    dprintf("$P Command line arguments have been read.\n");
    tryReadConfig();
    dprintf("$P Config file has been read.\n");

    log4cpp_AddOstream();

    logAllArguments();

    try {
        dprintf("$P before SPI has been inited.\n");
        initSPI();
        dprintf("$P SPI has been inited.\n");
        initRFM();
        dprintf("$P RFM has been inited.\n");
        openLirc();
        dprintf("$P LIRC has been opened.\n");
        tryJustScan(); // something test feature written by https://github.com/avp-avp, it may be broken
        tryFixThresh();
        tryDumpAllLircStream();
        receiveForever();
    } catch (CHaException ex) {
        LOG(ERROR) << "Exception " << ex.GetExplanation();
        if (args.bCoreTestMod)
            throw;
    }

    closeConnections();
    LOG(INFO) << "RFSNIFFER FINISHED\n";
}

RFSniffer::RFSniffer():
    mySPI(nullptr),
    rfm(nullptr),
    lircFD(-1)
{

}

RFSniffer rfsniffer;

int main(int argc, char *argv[])
{
    rfsniffer.run(argc, argv);
    return 0;
}
