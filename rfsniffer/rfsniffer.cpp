#include <algorithm>
#include <cassert>
#include <fstream>

#include <../libs/libutils/DebugPrintf.h>

#include "rfsniffer.h"



RFSniffer::RFSnifferArgs::RFSnifferArgs():
    configName(""),
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
    enabledFeatures({})
{

}

bool RFSniffer::waitForData(int fd, unsigned long maxusec)
{
    fd_set fds;
    int ret;
    struct timeval tv;
    tv.tv_sec = maxusec / 1000000;
    tv.tv_usec = maxusec % 1000000;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        do {
            do {
                ret = select(fd + 1, &fds, NULL, NULL, (maxusec > 0 ? &tv : NULL));
                if (ret == 0)
                    return false;
            } while (ret == -1 && errno == EINTR);
            if (ret == -1) {
                CLog::Default()->Printf(0, "RF select() failed\n");
                continue;
            }
        } while (ret == -1);

        if (FD_ISSET(fd, &fds)) {
            /* we will read later */
            return true;
        }
    }

    return false;
}


std::string RFSniffer::composeString(const char *format, ...)
{
    std::vector<char> data;
    for (int loop = 0; loop < 2; loop++) {
        va_list args;
        va_start (args, format);
        int written_length = vsnprintf(
                                 data.data(), data.size(), format, args
                             );
        va_end (args);
        // plus 1 for terminal 0 symbol in the end of the string
        data.resize(written_length + 1);
    }
    return data.data();
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
                m_Log->Printf(0, "\tCandidate is: /dev/%s", ent->d_name);
        }
        closedir(dir);
    } else {
        m_Log->Printf(0, "\tAnd couldn't see in /dev for %s", path.c_str());
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
        args.spiDevice = composeString("/dev/spidev%d.%d", spiMajorVal, spiMinorVal);
    }

    if (irq && atoi(irq) > 0)
        args.gpioInt = atoi(irq);
}

void RFSniffer::readCommandLineArguments(int argc, char **argv)
{
    // I don't know why, but signed is essential (though it should by by default??)
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
                if (args.spiDevice == "do_not_use") {
                    args.spiDevice = "/dev/null";
                    args.bRfmEnable = false;
                }
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
                args.configName = optarg;
                break;

            case '?':
                printf("Usage: rfsniffer [params]\n");
                printf("-D - debug mode. Write good but not decoded packets to files\n");
                printf("-g <DIO0 gpio> - set custom DIO0 GPIO number. Default %d\n", args.gpioInt);
                printf("-s <spi device> - set custom SPI device. Default %s\n" \
                       "    to disable SPI and RFM put \'do_not_use\' ", args.spiDevice.c_str());
                printf("-l <lirc device> - set custom lirc device. Default %s\n"
                       "    Set it with '/dev/null' for do not exit when can't read.", args.lircDevice.c_str());
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
                printf("-c configfile - specify config file (parameters in config file are priority) \n");
                //          printf("-f <sampling freq> - set custom sampling freq. Default %d\n", samplingFreq);
                exit(0);
            default:
                printf ("?? getopt returned character code 0%o ??\n", c);
                exit(-1);
        }
    }
}

void RFSniffer::tryReadConfigFile()
{
    if (args.configName.empty())
        return;
    try {
        Json::Value config;
        // reading
        {
            std::fstream fs(args.configName.c_str(), std::fstream::in);
            fs >> config;
            fs.close();
        }
        // DEBUG
        // std::cout << "CONFIG:\n" << config << std::endl;
        
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
            
            // I don't entrust it
            //CLog::Init(debug);
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
        
        auto enabledFeatures = config["enabled_features"];
        if (!!enabledFeatures) {
            if (!enabledFeatures.isArray())
                throw std::runtime_error("enabled_features must be array");
            args.enabledFeatures.clear();
            for (int i = 0; i < (int)enabledFeatures.size(); ++i) {
                args.enabledFeatures.push_back(enabledFeatures[i].asString());
                //fprintf(stderr, "Enabled protocol: %s\n", args.enabledFeatures.back().c_str());
            }       
        }
        
        this->configJson = config;
    } catch (CHaException ex) {
        fprintf(stderr, "Failed load config. Error: %s (%d)", ex.GetMsg().c_str(), ex.GetCode());
        exit(-1);
    } catch (Json::RuntimeError ex) {
        fprintf(stderr, "Failed load config. Error: %s", ex.what());
        exit(-1);
    } catch (std::exception ex) {
        fprintf(stderr, "Failed load config. Error: %s", ex.what());
        exit(-1);
    }
}

void RFSniffer::logAllArguments() {
    
    m_Log->Printf(3, "RFSniffer parameters");
    
    #define print_bool(a) m_Log->Printf(3, "  ||  " #a " = %s", args.a ? "true" : "false");
    #define print_str(a) m_Log->Printf(3, "  ||  " #a " = '%s'", args.a.c_str());
    #define print_int(a) m_Log->Printf(3, "  ||  " #a " = %d", args.a);
    #define print_gap() m_Log->Printf(3, "  ||  ");
    
    print_str(configName);
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
        m_Log->Printf(0, "SPI init failed (probably no such device: %s)", args.spiDevice.c_str());
        showCandidates("/dev/", "spidev");
        m_Log->Printf(0, "Please contact developers");
        exit(-1);
    }
}

void RFSniffer::initRFM()
{
    if (!args.bRfmEnable)
        return;

    rfm.reset(new RFM69OOK(mySPI.get(), args.gpioInt));
    rfm->initialize();

    if (args.bDumpAllRegs) {
        char *Buffer = (char *)dataBuff;
        char *BufferPtr = Buffer;
        size_t BufferSize = dataBuffSize;
        for (int i = 0; i <= 0x4F; i++) {
            byte cur = rfm->readReg(i);
            BufferPtr += snprintf(BufferPtr, BufferSize - (BufferPtr - Buffer), "Reg_%02X = %02x ", i, cur);

            if (i % 4 == 3) {
                m_Log->Printf(3, "%s", Buffer);
                BufferPtr = Buffer;
            }
        }

        if (BufferPtr != Buffer) {
            m_Log->Printf(3, "%s", Buffer);
        }

        m_Log->Printf(0, "Reg_%02x = %02x Reg_%02x = %02x", 0x6F, rfm->readReg(0x6F), 0x71,
                      rfm->readReg(0x71));
        exit(0);
    }
}

void RFSniffer::openLirc() throw(CHaException)
{
    lircFD = open(args.lircDevice.c_str(), O_RDONLY);
    if (lircFD == -1) {
        m_Log->Printf(0, "Error opening device %s\n", args.lircDevice.c_str());
        showCandidates("/dev/", "lirc");
        exit(EXIT_FAILURE);
    }

    if (args.bLircPedantic) {
        struct stat s;
        if (fstat(lircFD, &s) == -1) {
            m_Log->Printf(0, "fstat: Can't read file status! : %s\n", strerror(errno));
            throw CHaException(CHaException::ErrBadParam, "fstat: Can't read file status! : %s\n",
                               strerror(errno));
        }
        if (!S_ISCHR(s.st_mode)) {
            m_Log->Printf(0, "Lirc device is not character device! st_mode = %d\n", (int)s.st_mode);
            throw CHaException(CHaException::ErrBadParam, "%s is not a character device\n",
                               args.lircDevice.c_str());
        }

        uint32_t mode = 2;
        if (ioctl(lircFD, lircGetRecMode, &mode) == -1) {
            m_Log->Printf(0, "This program is only intended for receivers supporting the pulse/space layer.\n");
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

    m_Log->Printf(0, "Scanner params are: \"%s\"\n", scannerParams.c_str());
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
        m_Log->Printf(0, "Error in parsing scanner params." \
                      "Use -S <low level>..<high level>/<seconds for step>\n");
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
                m_Log->Printf(0, "read() failed [during opening lirc device part]\n");
                break;
            }
            for (int i = 0; i < result; i++) {
                if (CRFProtocol::isPulse(dataBuff[i]))
                    pulses++;
            }
        }

        m_Log->Printf(3, "Recv fixed level=%d pulses=%d", curLevel, pulses);
        curLevel++;
    }
    close(lircFD);
    exit(0);
}

void RFSniffer::tryFixThresh() throw(CHaException)
{
    if (args.fixedThresh) {
        rfm->writeReg(REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_FIXED);
        rfm->writeReg(REG_OOKFIX, args.fixedThresh);
    }
}

void RFSniffer::tryDumpAllLircStream()
{
    if (!args.bDumpAllLircStream || args.bSimultaneouslyDumpStreamAndWork)
        return;

    DPRINTF_DECLARE(dprintf, false);

    m_Log->Printf(3, "Saving all data from lirs device started.\n"\
                  "Print any button to finish\n" \
                  "Lirc fd is %d\n", lircFD);

    if (rfm)
        rfm->receiveBegin();

    std::vector<lirc_t> lircData;
    while (true) {
        if (waitForData(lircFD, 100000)) {
            int resultBytes = read(lircFD, (void *)dataBuff, sizeof(dataBuff));
            m_Log->Printf(3, "Read %d bytes\n", resultBytes);

            // I hope this never happen
            while (resultBytes % sizeof(lirc_t) != 0) {
                m_Log->Printf(3, "Bad amount (amount % 4 != 0) of bytes read from lirc");
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

    CRFParser::SaveFile(lircData.data(), lircData.size(), "dump-all", args.savePath, m_Log.get());

    closeConnections();
    exit(0);
}

void RFSniffer::receiveForever() throw(CHaException)
{
    DPRINTF_DECLARE(dprintf, false);

    dprintf("$P DPrintf initiaized\n");

    m_Log->Printf(3, "RF Receiver begins");

    if (rfm)
        rfm->receiveBegin();

    auto devicesConfig = configJson["devices"];

    CMqttConnection conn(args.mqttHost, m_Log.get(), rfm.get(), devicesConfig, args.enabledFeatures);

    CRFParser m_parser(m_Log.get(), args.bDebug ? args.savePath : "");

    for (auto protocol : args.enabledProtocols)
        m_parser.AddProtocol(protocol);

    string dumpFileName;
    std::unique_ptr<FILE, int(*)(FILE *)> dumpFile(nullptr, fclose);
    
    if (args.bDumpAllLircStream) {
        assert(args.bSimultaneouslyDumpStreamAndWork);
        dumpFileName = CRFParser::GenerateFileName("dump-all", args.savePath);
        // make unique_ptr for automatic close of file
        dumpFile = std::unique_ptr<FILE, int(*)(FILE *)>(fopen(dumpFileName.c_str(), "w"), fclose);
        
        m_Log->Printf(3, "Saving stream dump to '%s'. Press Ctrl-C to stop driver", dumpFileName.c_str());
    }

    int lastRSSI = -1000, minGoodRSSI = 0;
    time_t lastReport = 0, lastRegularWorkTime = time(NULL);

    dprintf("$P Start cycle\n");


    while (true) {
        DPRINTF_DECLARE(dprintf, false);
        // try is placed here to handle exceptions and just restart, not to crush
        try {
            dprintf("$P process all parsed messages\n");
            for (const string &parsedResult : m_parser.ExtractParsed()) {
                m_Log->Printf(3, "RF Received: %s. RSSI=%d (%d)",
                              parsedResult.c_str(), lastRSSI, minGoodRSSI);
                if (minGoodRSSI > lastRSSI)
                        minGoodRSSI = lastRSSI;
                if (args.bCoreTestMod)
                    fprintf(stderr, "TEST_RF_RECEIVED %s\n", parsedResult.c_str());
                try {
                    conn.NewMessage(parsedResult);
                } catch (CHaException ex) {
                    m_Log->Printf(0, "conn.NewMessage failed: Exception", ex.GetExplanation().c_str());
                    if (args.bCoreTestMod)
                        throw;
                }
            }

            // do not read very often
            usleep(100000);
            // process incoming messages
            conn.loop(500);
            // try get more data and sleep if fail
            const int waitDataReadUsec = 1000000;
            if (waitForData(lircFD, waitDataReadUsec)) {
                /*
                 * This strange thing is needed to lessen received trash
                 */
                if (rfm) {
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
                        m_Log->Printf(0, "read() failed [during endless cycle]\n");
                    }
                    if (args.bCoreTestMod) {
                        dprintf("$P No more input data. Exiting!\n");
                        // if lirc is a fictive device then break
                        // if lirc is dumb device (/dev/null) consider situation as just lack of data
                        if (args.lircDevice != "/dev/null") 
                            break;
                    }
                } else {
                    dprintf("$P % bytes were read from lirc device\n", resultBytes);
                    
                    // I hope this never happen
                    while (resultBytes % sizeof(lirc_t) != 0) {
                        m_Log->Printf(3, "Bad amount (amount % 4 != 0) of bytes read from lirc");
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
                    m_parser.AddInputData(dataBuff, result);
                    dprintf("$P after AddInputData() call\n");

                    if (lastReport != time(NULL) && result >= 32) {
                        m_Log->Printf(4, "RF got data %ld signals. RSSI=%d", (int)result, lastRSSI);
                        lastReport = time(NULL);
                    }

                    if (rfm)
                        lastRSSI = rfm->readRSSI();
                }
            }
            //~ it doesn't work somehow
            //~ if (args.bDumpAllLircStream && waitForData(0, 100)) {
                //~ m_Log->Printf(3, "You can find stream dump in '%s'", dumpFileName.c_str());
                //~ break;
            //~ }
            
            
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
            m_Log->Printf(0, "Exception %s", ex.GetExplanation().c_str());
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
    //DPrintf::setDefaultOutputStream(fopen("rfs.log", "wt"));
    DPrintf::setPrefixLength(40);

    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Driver has been started.\n");

    readEnvironmentVariables();
    dprintf("$P Environment variables have been read.\n");
    readCommandLineArguments(argc, argv);
    dprintf("$P Command line arguments have been read.\n");
    tryReadConfigFile();
    dprintf("$P Config file has been read.\n");
    
    // important to initialize m_Log after reading config file
    m_Log.reset(CLog::Default());
    
    logAllArguments();
    
    if (args.configName.length() == 0)
        m_Log->SetLogLevel(3);
    try {
        dprintf("$P before $P SPI has been inited.\n");
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
        m_Log->Printf(0, "Exception %s", ex.GetExplanation().c_str());
        if (args.bCoreTestMod)
            throw;
    }

    closeConnections();
    m_Log->Printf(0, "RFSNIFFER FINISHED\n");
}

RFSniffer::RFSniffer():
    m_Log(nullptr),
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
