#include "stdafx.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
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


typedef uint32_t __u32;
#define LIRC_GET_REC_MODE              _IOR('i', 0x00000002, __u32)
#define lirc_t base_type
#define PULSE_BIT       0x01000000
#define PULSE_MASK      0x00FFFFFF


int waitfordata(int fd, unsigned long maxusec)
{
    fd_set fds;
    int ret;
    struct timeval tv;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        do { 
            do {
                if (maxusec > 0) {
                    tv.tv_sec = maxusec / 1000000;
                    tv.tv_usec = maxusec % 1000000;
                    ret = select(fd + 1, &fds, NULL, NULL, &tv);
                    if (ret == 0)
                        return (0);
                } else {
                    ret = select(fd + 1, &fds, NULL, NULL, NULL);
                }
            }
            while (ret == -1 && errno == EINTR);
            if (ret == -1) {
                CLog::Default()->Printf(0, "RF select() failed\n");
                continue;
            }
        }
        while (ret == -1);

        if (FD_ISSET(fd, &fds)) {
            /* we will read later */
            return (1);
        }
    }

    return false;
}


int main(int argc, char* argv[])
{
    bool bDebug = false, bDaemon = false, bDumpAllRegs = false;
    string spiDevice = "/dev/spidev32766.0";
    string lircDevice="/dev/lirc0";
    string mqttHost="localhost";
    string scannerParams;
    string configName;
    long spiSpeed = 500000;
    int gpioInt = 38;
    int fixedThresh = 0;
    int rssi = 0;
    int writePackets = 0;
    string savePath = ".";


    char* irq = getenv("WB_GPIO_RFM_IRQ");
    char* spiMajor = getenv("WB_RFM_SPI_MAJOR");
    char* spiMinor = getenv("WB_RFM_SPI_MINOR");
//  m_Log->Printf(0, "WB_GPIO_RFM_IRQ=%s", irq);
//  m_Log->Printf(0, "WB_RFM_SPI_MAJOR=%s", spiMajor);
//  m_Log->Printf(0, "WB_RFM_SPI_MINOR=%s", spiMinor);

    if (spiMajor || spiMinor)
    {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/dev/spidev%s.%s", spiMajor ? spiMajor : "32766", spiMinor ? spiMinor : "0");
        spiDevice = buffer;
    }

    if (irq && atoi(irq) > 0)
        gpioInt = atoi(irq);

    int c;
    //~ int digit_optind = 0;
    //~ int aopt = 0, bopt = 0;
    //~ char *copt = 0, *dopt = 0;
    while ( (c = getopt(argc, argv, "Dds:m:l:S:f:r:tw:c:")) != -1) {
        //~ int this_option_optind = optind ? optind : 1;
        switch (c) {
         case 'D':
            bDebug = true;
            break;

         case 'd':
            bDaemon = true;
            break;

         case 's':
             spiDevice = optarg;
            break;

         case 'l':
            lircDevice = optarg;
            break;

         case 'm':
            mqttHost = optarg;
            break;

         case 'S':
            scannerParams = optarg;
            break;

         case 'f':
            fixedThresh = atoi(optarg);
            break;

         case 'r':
            rssi = atoi(optarg);
            break;

         case 'g':
            gpioInt = atoi(optarg);
            break;

         case 't':
            bDumpAllRegs = true;
            break;

         case 'w':
            writePackets = atoi(optarg);
            break;

        case 'c':
            configName = optarg;
            break;

        case '?':
            printf("Ussage: rfsniffer [params]\n");
            printf("-D - debug mode. Write good but not decoded packets to files\n");
            printf("-d - start daemon\n");
            printf("-g <DIO0 gpio> - set custom DIO0 GPIO number. Default %d\n", gpioInt);
            printf("-s <spi device> - set custom SPI device. Default %s\n", spiDevice.c_str());
            printf("-l <lirc device> - set custom lirc device. Default %s\n", lircDevice.c_str());
            printf("-m <mqtt host> - set custom mqtt host. Default %s\n", mqttHost.c_str());
            printf("-w <seconds> - write to file all packets for <secods> second and exit\n");

            printf("-S -<low level>..-<high level>/<seconds for step> - scan for noise. \n");
            printf("-r <RSSI> - reset RSSI Threshold after each packet. 0 - Disabled. Default %d\n", (int)rssi);
            printf("-f <fixed Threshold> - Use OokFixedThresh with fixed level. 0 - Disabled. Default %d\n", fixedThresh);
//          printf("-f <sampling freq> - set custom sampling freq. Default %d\n", samplingFreq);
            return 0;
        default:
            printf ("?? getopt returned character code 0%o ??\n", c);
            return 0;
        }
    }

    try
    {
        if (configName.length())
        {
            CConfig config;
            config.Load(configName);

            CConfigItem radio = config.getNode("radio");
            if (radio.isNode())
            {               
                lircDevice = radio.getStr("lirc_device", false, lircDevice);    
                spiDevice = radio.getStr("spi_device", false, spiDevice);           
                gpioInt = radio.getInt("rfm_irq", false, gpioInt);
                rssi = radio.getInt("rssi", false, rssi);
            }
        
            mqttHost = config.getStr("mqtt/host", false, mqttHost);
        
            CConfigItem debug = config.getNode("debug");
            if (debug.isNode())
            {               
                savePath = debug.getStr("save_path", false, savePath);  
                writePackets = debug.getInt("write_packets", false, writePackets);  
                CLog::Init(&debug);
            }
        }
    }
    catch (CHaException ex)
    {
        CLog *m_Log = CLog::Default();
        m_Log->Printf(0, "Failed load config. Error: %s (%d)", ex.GetMsg().c_str(), ex.GetCode());
        return -1;
    }

    CLog *m_Log = CLog::Default();
    m_Log->Printf(0, "Using SPI device %s, lirc device %s, mqtt on %s", spiDevice.c_str(), lircDevice.c_str(), mqttHost.c_str());

    if (bDaemon)
    {
        printf ("Start daemon mode\n");
        pid_t pid = fork();
        if (pid < 0)
        {
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            exit(EXIT_SUCCESS);
        }
        fclose (stdout);
    }
    else if (configName.length() == 0)
    {
        m_Log->SetConsoleLogLevel(4);
    }

    if (configName.length() == 0)
        m_Log->SetLogLevel(3);

    const size_t BUFFER_SIZE = 1024 * 128;
    lirc_t *data = new lirc_t[BUFFER_SIZE];
    int fd = -1;

    spi_config_t spi_config;
    spi_config.mode = 0;
    spi_config.speed = spiSpeed;
    spi_config.delay = 0;
    spi_config.bits_per_word = 8;
    SPI mySPI(spiDevice.c_str(), &spi_config);
    if (!mySPI.begin())
    {
        m_Log->Printf(0, "SPI init failed");
        return 1;
    }

    RFM69OOK rfm(&mySPI, gpioInt);
    rfm.initialize();

    try
    {
        if (bDumpAllRegs)
        {
            char *Buffer = (char*)data;
            char *BufferPtr = Buffer;
            size_t BufferSize = BUFFER_SIZE;
            for (int i=0;i<=0x4F;i++)
            {
                byte cur = rfm.readReg(i);
                BufferPtr+=snprintf(BufferPtr, BufferSize-(BufferPtr-Buffer), "Reg_%02X = %02x ", i, cur);

                if (i % 4 == 3)
                {
                    m_Log->Printf(3, "%s", Buffer);
                    BufferPtr = Buffer;
                }
            }

            if (BufferPtr!=Buffer)
            {
                m_Log->Printf(3, "%s", Buffer);
            }

            m_Log->Printf(0, "Reg_%02x = %02x Reg_%02x = %02x", 0x6F, rfm.readReg(0x6F), 0x71, rfm.readReg(0x71));

            return 0;
        }

        // Opening lirc device
        {
            fd = open(lircDevice.c_str(), O_RDONLY);
            if (fd == -1) 
            {
                m_Log->Printf(0, "Error opening device %s\n", lircDevice.c_str());
                exit(EXIT_FAILURE);
            }

            struct stat s;
            if (fstat(fd, &s) == -1) 
            {
                m_Log->Printf(0, "fstat: Can't read file status! : %s\n", strerror(errno));
                throw CHaException(CHaException::ErrBadParam, "fstat: Can't read file status! : %s\n", strerror(errno));
            }
            if (!S_ISCHR(s.st_mode)) 
            {
                m_Log->Printf(0, "Lirc device is not character device! st_mode = %d\n", (int)s.st_mode);
                throw CHaException(CHaException::ErrBadParam,"%s is not a character device\n", lircDevice.c_str());
            }
            
            __u32 mode = 2;
            if (ioctl(fd, LIRC_GET_REC_MODE, &mode) == -1) 
            {
                m_Log->Printf(0, "This program is only intended for receivers supporting the pulse/space layer.\n");
                throw CHaException(CHaException::ErrBadParam,"This program is only intended for receivers supporting the pulse/space layer.");
            }
        }

        if (scannerParams.length() > 0)
        {
            m_Log->Printf(0, "Scanner params are: \"%s\"\n", scannerParams.c_str());
            int minLevel = 30;
            int maxLevel = 60;
            int scanTime = 15;

            int pos = scannerParams.find("..");
            if (pos != scannerParams.npos)
            {
                minLevel = atoi(scannerParams.substr(0, pos));
                scannerParams = scannerParams.substr(pos+2);

                pos = scannerParams.find("/");
                if (pos != scannerParams.npos)
                {
                    maxLevel = atoi(scannerParams.substr(0, pos));
                    scanTime = atoi(scannerParams.substr(pos+1));
                }
                else
                {
                    maxLevel = atoi(scannerParams);
                }
            }
            else
            {
                m_Log->Printf(0, "Error in parsing scanner params." \
                            "Use -S <low level>..<high level>/<seconds for step>\n");
                return -1;
            }
            
            int curLevel = minLevel;

            while (curLevel < maxLevel)
            {
                rfm.receiveEnd();
                rfm.writeReg(REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_FIXED);
                rfm.writeReg(REG_OOKFIX, curLevel);
                rfm.receiveBegin();
                int pulses=0;
                time_t startTime=time(NULL);

                while (time(NULL) - startTime < scanTime)
                {
                    if (!waitfordata(fd, 100)) 
                        continue;
                    int result = read(fd, (void *)data, BUFFER_SIZE);
                    if (result == -1 && errno == EAGAIN)
                        result = 0;
                    if (result == 0) {
                        m_Log->Printf(0, "read() failed\n");
                        break;
                    }
                    for (size_t i = 0; i < result; i++)
                    {
                        if (CRFProtocol::isPulse(data[i]))
                            pulses++;
                    }
                }

                m_Log->Printf(3, "Recv fixed level=%d pulses=%d", curLevel, pulses);
                curLevel++;
            }
            return 0;
        }

        if (fixedThresh)
        {
            rfm.writeReg(REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_FIXED);
            rfm.writeReg(REG_OOKFIX, fixedThresh);
        }

        rfm.receiveBegin();
        CMqttConnection conn(mqttHost, m_Log, &rfm);
        CRFParser m_parser(m_Log, (bDebug || writePackets > 0) ? savePath : "");
        m_parser.AddProtocol("All");

        lirc_t *data_ptr = data;
        time_t lastReport = 0, packetStart = time(NULL), startTime = time(NULL);
        int lastRSSI = -1000, minGoodRSSI = 0;

        while (true) {
            if (writePackets > 0 && time(NULL) - startTime > writePackets)
                break;
            
            int result;
            usleep(10);

            size_t count = sizeof(lirc_t) * BUFFER_SIZE - (data_ptr - data) * sizeof(lirc_t);

            if (count == 0)
            {
                m_Log->Printf(0, "RF buffer full");
                data_ptr = data;
                continue;
            }

            if (lastReport != time(NULL) && data_ptr - data >= 32)
            {
                m_Log->Printf(writePackets ? 3 : 4,"RF got data %ld bytes. RSSI=%d", data_ptr - data, lastRSSI);
                lastReport = time(NULL);
            }

            if (data_ptr == data)
            {
                packetStart = time(NULL);
            }
            
            if (data_ptr - data >= 32 && (!waitfordata(fd, 300000) 
                    || data_ptr - data > BUFFER_SIZE - 10 || time(NULL) - packetStart > 2))
            {
                if (writePackets > 0)
                {
                    m_parser.SaveFile(data, data_ptr - data);
                    m_Log->Printf(3, "Saved file RSSI=%d (%d)", lastRSSI, minGoodRSSI);
                }

                // What is it? Why does he make it .reseiveEnd() and .receiveBegin?
                // TODO. Erase it and test.
                // Upd: It works without it, but it's safer to keep it as is.
                // It's in not connected with kernel error:
                // [  384.785198] lirc_pwm lirc-rfm69: wtf? value=0, last=351115718, now=384643156, delta=33527437
                rfm.receiveEnd();
                string parsedResult = m_parser.Parse(data, data_ptr - data);
                rfm.receiveBegin();
                if (parsedResult.length())
                {
                    m_Log->Printf(3, "RF Recieved: %s. RSSI=%d (%d)", parsedResult.c_str(), lastRSSI, minGoodRSSI);
                    conn.NewMessage(parsedResult);
                    if (minGoodRSSI > lastRSSI)
                        minGoodRSSI = lastRSSI;
                }
                else
                {
                    m_Log->Printf(4, "Recieved %ld signals. Not decoded\n", data_ptr - data);
                }
                data_ptr = data;
                packetStart = time(NULL);

                if (rssi<0)
                    rfm.setRSSIThreshold(rssi);
            }

            result = read(fd, (void *)data_ptr, count);
            if (result == 0) {
                m_Log->Printf(0, "read() failed\n");
                break;
            }
            lastRSSI = rfm.readRSSI();

            data_ptr += result / sizeof(lirc_t);
        }
    } 
    catch (CHaException ex)
    {
        m_Log->Printf(0, "Exception %s (%d)", ex.GetMsg().c_str(), ex.GetCode());
    }

    rfm.receiveEnd();
    delete []data;
    
    if (fd > 0)
        close(fd);
}
