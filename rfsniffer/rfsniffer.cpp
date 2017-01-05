#include "rfsniffer.h"


RFSniffer::RFSnifferArgs::RFSnifferArgs():
    configName(""),
    bDebug(false),
    bDumpAllRegs(false),
    bLircPedantic(false),

    spiDevice("/dev/spidev32766.0"),
    spiSpeed(500000),
    gpioInt(38),

    fixedThresh(0),
    rssi(0),

    lircDevice("/dev/lirc0"),

    mqttHost("localhost"),

    scannerParams(""),
    writePackets(0),
    savePath("."),
    inverted(false)
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
        char buffer[256];
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
    for (signed char c = ' '; c != -1; c = getopt(argc, argv, "Ds:m:l:LS:f:r:tw:c:i")) {
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

            case 'L':
                args.bLircPedantic = false;
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

            case 'w':
                args.writePackets = atoi(optarg);
                break;

            case 'c':
                args.configName = optarg;
                break;

            case '?':
                printf("Usage: rfsniffer [params]\n");
                printf("-D - debug mode. Write good but not decoded packets to files\n");
                printf("-g <DIO0 gpio> - set custom DIO0 GPIO number. Default %d\n", args.gpioInt);
                printf("-s <spi device> - set custom SPI device. Default %s\n", args.spiDevice.c_str());
                printf("-l <lirc device> - set custom lirc device. Default %s\n", args.lircDevice.c_str());
                printf("-m <mqtt host> - set custom mqtt host. Default %s\n", args.mqttHost.c_str());
                printf("-w <seconds> - write to file all packets for <secods> second and exit\n");

                printf("-S -<low level>..-<high level>/<seconds for step> - scan for noise. \n");
                printf("-r <RSSI> - reset RSSI Threshold after each packet. 0 - Disabled. Default %d\n",
                       (int)args.rssi);
                printf("-f <fixed Threshold> - Use OokFixedThresh with fixed level. 0 - Disabled. Default %d\n",
                       args.fixedThresh);

                printf("-L - disable pedantic check of lirc character device (may use pipe instead)\n");
                printf("-c configfile - specify config file\n");
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
        CConfig config;
        config.Load(args.configName);

        CConfigItem radio = config.getNode("radio");
        if (radio.isNode()) {
            args.lircDevice = radio.getStr("lirc_device", false, args.lircDevice);
            args.spiDevice = radio.getStr("spi_device", false, args.spiDevice);
            args.gpioInt = radio.getInt("rfm_irq", false, args.gpioInt);
            args.rssi = radio.getInt("rssi", false, args.rssi);
        }

        args.mqttHost = config.getStr("mqtt/host", false, args.mqttHost);

        CConfigItem debug = config.getNode("debug");
        if (debug.isNode()) {
            args.savePath = debug.getStr("save_path", false, args.savePath);
            args.writePackets = debug.getInt("write_packets", false, args.writePackets);
            CLog::Init(&debug);
        }

    } catch (CHaException ex) {
        fprintf(stderr, "Failed load config. Error: %s (%d)", ex.GetMsg().c_str(), ex.GetCode());
        exit(-1);
    }
}



void RFSniffer::initSPI()
{
    spi_config_t spi_config;
    spi_config.mode = 0;
    spi_config.speed = args.spiSpeed;
    spi_config.delay = 0;
    spi_config.bits_per_word = 8;
    // do not use "=" because destructor breaks all
    mySPI.init(args.spiDevice.c_str(), &spi_config);
    if (!mySPI.begin()) {
        m_Log->Printf(0, "SPI init failed (probably no such device: %s)", args.spiDevice.c_str());
        showCandidates("/dev/", "spidev");
        m_Log->Printf(0, "Please contact developers");
        exit(-1);
    }
}

void RFSniffer::initRFM()
{
    rfm.init(&mySPI, args.gpioInt);
    rfm.initialize();

    if (args.bDumpAllRegs) {
        char *Buffer = (char *)data;
        char *BufferPtr = Buffer;
        size_t BufferSize = dataSize;
        for (int i = 0; i <= 0x4F; i++) {
            byte cur = rfm.readReg(i);
            BufferPtr += snprintf(BufferPtr, BufferSize - (BufferPtr - Buffer), "Reg_%02X = %02x ", i, cur);

            if (i % 4 == 3) {
                m_Log->Printf(3, "%s", Buffer);
                BufferPtr = Buffer;
            }
        }

        if (BufferPtr != Buffer) {
            m_Log->Printf(3, "%s", Buffer);
        }

        m_Log->Printf(0, "Reg_%02x = %02x Reg_%02x = %02x", 0x6F, rfm.readReg(0x6F), 0x71,
                      rfm.readReg(0x71));
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
    if (pos != scannerParams.npos) {
        minLevel = atoi(scannerParams.substr(0, pos));
        scannerParams = scannerParams.substr(pos + 2);

        pos = scannerParams.find("/");
        if (pos != scannerParams.npos) {
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
        rfm.receiveEnd();
        rfm.writeReg(REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_FIXED);
        rfm.writeReg(REG_OOKFIX, curLevel);
        rfm.receiveBegin();
        int pulses = 0;
        time_t startTime = time(NULL);

        while (time(NULL) - startTime < scanTime) {
            if (!waitForData(lircFD, 100))
                continue;
            int result = read(lircFD, (void *)data, dataSize);
            if (result == -1 && errno == EAGAIN)
                result = 0;
            if (result == 0 && args.bLircPedantic) {
                m_Log->Printf(0, "read() failed [during opening lirc device part]\n");
                break;
            }
            for (size_t i = 0; i < result; i++) {
                if (CRFProtocol::isPulse(data[i]))
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
        rfm.writeReg(REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_FIXED);
        rfm.writeReg(REG_OOKFIX, args.fixedThresh);
    }
}


void RFSniffer::receiveForever() throw(CHaException)
{
    m_Log->Printf(3, "RF Reciever begins");

    rfm.receiveBegin();
    CMqttConnection conn(args.mqttHost, m_Log, &rfm);
    CRFParser m_parser(m_Log, (args.bDebug || args.writePackets > 0) ? args.savePath : "");
    m_parser.AddProtocol("All");

    time_t lastReport = 0, packetStartTime = time(NULL), startTime = time(NULL);
    int lastRSSI = -1000, minGoodRSSI = 0;

    bool readSmthNew = false;

    while (true) {
        // notice that writePackets is 0 if corresponding command line argument is not set
        if (args.writePackets > 0 && time(NULL) - startTime > args.writePackets)
            break;

        // try recognize packets
        while (readSmthNew && readDataCount() > 0) {
            size_t parsedLength;
            string parsedResult = m_parser.ParseRepetitive(dataBegin, readDataCount(), &parsedLength);
            if (parsedResult.length() > 0) {
                m_Log->Printf(3, "RF Recieved: %s (parsed from %u lirc_t). RSSI=%d (%d)",
                              parsedResult.c_str(), parsedLength, lastRSSI, minGoodRSSI);
                conn.NewMessage(parsedResult);
                if (minGoodRSSI > lastRSSI)
                    minGoodRSSI = lastRSSI;

                // copy [dataBegin + parsedLength, dataPtr) to [dataBegin, dataPtr - parsedLength)
                for (lirc_t *p = data; p < dataPtr - parsedLength; p++)
                    *p = *(p + parsedLength);
                // and shift pointer
                dataPtr -= parsedLength;
            } else {
                if (readDataCount() > maxMessageLength) {
                    dataPtr = dataBegin; // clean buffer
                    m_Log->Printf(3, "RF Recieved too long message or just a lot of trash RSSI=%d (%d)", lastRSSI,
                                  minGoodRSSI);
                }
                break;
            }

        }
        readSmthNew = false;

        // try get more data and sleep if fail
        if (waitForData(lircFD, 300000)) {
            int resultBytes = read(lircFD, (void *)dataPtr, remainingDataCount() * sizeof(lirc_t));
            // I hope this never happen
            while (resultBytes % sizeof(lirc_t) != 0) {
                m_Log->Printf(3, "Bad amount (amount % 4 != 0) of bytes read from lirc");
                usleep(1000000);
                int remainBytes = 4 - resultBytes % sizeof(lirc_t);
                int readTailBytes = read(lircFD, (void *)((char *)dataPtr + resultBytes), remainBytes);
                if (readTailBytes != -1)
                    resultBytes += readTailBytes;
            }

            int result = resultBytes / sizeof(lirc_t);
            dataPtr += result;

            if (result != 0) {
                readSmthNew = true;
                packetStartTime = time(NULL);
                if (lastReport != time(NULL) && result >= 32) {
                    m_Log->Printf(args.writePackets ? 3 : 4, "RF got data %ld bytes. RSSI=%d", (int)result, lastRSSI);
                    lastReport = time(NULL);
                }
            }

            if (result == 0 && args.bLircPedantic) {
                m_Log->Printf(0, "read() failed [during endless cycle]\n");
                break;
            }
            lastRSSI = rfm.readRSSI();
        }



        if (args.rssi < 0)
            rfm.setRSSIThreshold(args.rssi);

        /*
                if (readCount >= 32 &&
                        (!waitForData(fd, 300000)
                         || remainedCount < 10
                         || time(NULL) - packetStartTime > 2)) {
                    if (writePackets > 0) {
                        m_parser.SaveFile(data, data_ptr - data);
                        m_Log->Printf(3, "Saved file RSSI=%d (%d)", lastRSSI, minGoodRSSI);
                    }

                    // What is it? Why does he make it .reseiveEnd() and .receiveBegin?
                    // TODO. Erase it and test.
                    // Upd: It works without it, but it's safer to keep it as is.
                    // It's in not connected with kernel error:
                    // [  384.785198] lirc_pwm lirc-rfm69: wtf? value=0, last=351115718, now=384643156, delta=33527437
                    rfm.receiveEnd();
                    // How many lirc_t were read
                    size_t parsedLength;
                    string parsedResult = m_parser.ParseRepetitive(data, data_ptr - data, &parsedLength);
                    rfm.receiveBegin();
                    if (parsedResult.length()) {
                        m_Log->Printf(3, "RF Recieved: %s (parsed from %u lirc_t). RSSI=%d (%d)",
                            parsedResult.c_str(), parsedLength, lastRSSI, minGoodRSSI);
                        conn.NewMessage(parsedResult);
                        if (minGoodRSSI > lastRSSI)
                            minGoodRSSI = lastRSSI;
                    } else {
                        m_Log->Printf(4, "Recieved %ld signals. Not decoded");
                        if (writePackets > 0) {
                            vector <char> buff_data(12000);
                            char *buff = buff_data.data();
                            char *buff_ptr = buff;
                            for (long unsigned int *c = data; c < data_ptr && buff_ptr + 20 < buff + buff_data.size(); c++)
                                buff_ptr += sprintf(buff_ptr, "%u ", (unsigned int)*c);
                            *buff_ptr = 0;
                            m_Log->Printf(4, "(\n%s\n)\n", data_ptr - data, buff);
                        }
                    }


                    if (parsedLength != 0) {

                    }
                    else
                        data_ptr = data;

                    packetStartTime = time(NULL);

                    if (rssi < 0)
                        rfm.setRSSIThreshold(rssi);
                }

           */
    }
}

void RFSniffer::closeConnections()
{
    rfm.receiveEnd();
    if (lircFD > 0)
        close(lircFD);

}

void RFSniffer::run(int argc, char **argv)
{
    readEnvironmentVariables();
    readCommandLineArguments(argc, argv);
    tryReadConfigFile();


    // important to initialize m_Log after reading config file
    m_Log = CLog::Default();
    m_Log->Printf(0, "Using SPI device %s, lirc device %s, mqtt on %s", args.spiDevice.c_str(),
                  args.lircDevice.c_str(), args.mqttHost.c_str());
    if (args.configName.length() == 0)
        m_Log->SetLogLevel(3);

    try {
        initSPI();
        initRFM();
        openLirc();
        tryJustScan(); // something test feature written by https://github.com/avp-avp, it may be broken
        tryFixThresh();
        receiveForever();
    } catch (CHaException ex) {
        m_Log->Printf(0, "Exception %s (%d)", ex.GetMsg().c_str(), ex.GetCode());
    }

    closeConnections();
}

RFSniffer::RFSniffer():
    lircFD(-1),
    dataBegin(data),
    dataEnd(data + dataSize),
    dataPtr(data)
{

}

RFSniffer rfsniffer;

int main(int argc, char *argv[])
{
    rfsniffer.run(argc, argv);
    return 0;
}
