// **********************************************************************************
// Driver definition for HopeRF RFM69W/RFM69HW/RFM69CW/RFM69HCW, Semtech SX1231/1231H
// **********************************************************************************
// Copyright Felix Rusu (2014), felix@lowpowerlab.com
// http://lowpowerlab.com/
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************


#include <cstring>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <memory>
#ifdef __MACH__
    #include <mach/clock.h>
    #include <mach/mach.h>
#endif
#include "spidev_lib++.h"
#include "RFM69OOK.h"
#include "RFM69OOKregisters.h"
#include "../libutils/Exception.h"
#include "../libutils/DebugPrintf.h"
#include "../libutils/strutils.h"
#include "../libutils/logging.h"

using namespace strutils;

volatile byte RFM69OOK::_mode;  // current transceiver state
volatile int RFM69OOK::RSSI;      // most accurate RSSI during reception (closest to the reception)
RFM69OOK *RFM69OOK::selfPointer;
volatile uint8_t RFM69OOK::PAYLOADLEN;

inline void Sleep(long ms)
{
    usleep((ms) * 1000);
};

RFM69OOK::RFM69OOK()
{
    init(NULL, -1);
}

RFM69OOK::RFM69OOK(SPI *spi, int gpioInt)
{
    init(spi, gpioInt);
}

void RFM69OOK::init(SPI *spi, int gpioInt)
{
    m_spi = spi;
    m_gpioInt = gpioInt;
}

bool RFM69OOK::initialize()
{
    const byte CONFIG[][2] = {
        // perl -pe 's/Reg_(\w\w) = (\w\w)/{0x$1, 0x$2},/g' good
        // state after wb-homa-ism-radio (recover dump)
        //~ {0x00, 0x66}, {0x01, 0x10}, {0x02, 0x08}, {0x03, 0x3e},
        //~ {0x04, 0x80}, {0x05, 0x00}, {0x06, 0x52}, {0x07, 0x6c},
        //~ {0x08, 0x7a}, {0x09, 0xe1}, {0x0A, 0x41}, {0x0B, 0x40},
        //~ {0x0C, 0x02}, {0x0D, 0x92}, {0x0E, 0xf5}, {0x0F, 0x20},
        //~ {0x10, 0x24}, {0x11, 0xff}, {0x12, 0x09}, {0x13, 0x1a},
        //~ {0x14, 0x40}, {0x15, 0xb0}, {0x16, 0x7b}, {0x17, 0x9b},
        //~ {0x18, 0x10}, {0x19, 0x80}, {0x1A, 0x8a}, {0x1B, 0x40},
        //~ {0x1C, 0x80}, {0x1D, 0x06}, {0x1E, 0x10}, {0x1F, 0x00},
        //~ {0x20, 0x00}, {0x21, 0x00}, {0x22, 0x00}, {0x23, 0x00},
        //~ {0x24, 0x9d}, {0x25, 0x40}, {0x26, 0x90}, {0x27, 0xd9},
        //~ {0x28, 0x64}, {0x29, 0xaa}, {0x2A, 0x00}, {0x2B, 0x00},
        //~ {0x2C, 0x00}, {0x2D, 0x05}, {0x2E, 0x8d}, {0x2F, 0xaa},
        //~ {0x30, 0x66}, {0x31, 0x00}, {0x32, 0x00}, {0x33, 0x00},
        //~ {0x34, 0x00}, {0x35, 0x00}, {0x36, 0x00}, {0x37, 0x00},
        //~ {0x38, 0x3c}, {0x39, 0x00}, {0x3A, 0x00}, {0x3B, 0x00},
        //~ {0x3C, 0x8f}, {0x3D, 0xc2}, {0x3E, 0x00}, {0x3F, 0x00},
        //~ {0x40, 0x00}, {0x41, 0x00}, {0x42, 0x00}, {0x43, 0x00},
        //~ {0x44, 0x00}, {0x45, 0x00}, {0x46, 0x00}, {0x47, 0x00},
        //~ {0x48, 0x00}, {0x49, 0x00}, {0x4A, 0x00}, {0x4B, 0x00},
        //~ {0x4C, 0x00}, {0x4D, 0x00}, {0x4E, 0x01}, {0x4F, 0x00},
        //~ {0x6f, 0x30}, {0x71, 0x00},

        {0x00, 0x1e}, {0x01, 0x10}, {0x02, 0x68}, {0x03, 0x3e},
        {0x04, 0x80}, {0x05, 0x00}, {0x06, 0x52}, {0x07, 0x6c},
        {0x08, 0x7a}, {0x09, 0xe1}, {0x0A, 0x41}, {0x0B, 0x40},
        {0x0C, 0x02}, {0x0D, 0x92}, {0x0E, 0xf5}, {0x0F, 0x20},
        {0x10, 0x24}, {0x11, 0xff}, {0x12, 0x09}, {0x13, 0x1a},
        {0x14, 0x40}, {0x15, 0xb0}, {0x16, 0x7b}, {0x17, 0x9b},
        {0x18, 0x20}, {0x19, 0x80}, {0x1A, 0x8a}, {0x1B, 0x40},
        {0x1C, 0x80}, {0x1D, 0x14}, {0x1E, 0x10}, {0x1F, 0x00},
        {0x20, 0x00}, {0x21, 0x00}, {0x22, 0x00}, {0x23, 0x00},
        {0x24, 0xae}, {0x25, 0x00}, {0x26, 0x90}, {0x27, 0xd8},
        {0x28, 0x00}, {0x29, 0xb4}, {0x2A, 0x00}, {0x2B, 0x00},
        {0x2C, 0x00}, {0x2D, 0x05}, {0x2E, 0x00}, {0x2F, 0xaa},
        {0x30, 0x66}, {0x31, 0x00}, {0x32, 0x00}, {0x33, 0x00},
        {0x34, 0x00}, {0x35, 0x00}, {0x36, 0x00}, {0x37, 0x00},
        {0x38, 0x3c}, {0x39, 0x00}, {0x3A, 0x00}, {0x3B, 0x00},
        {0x3C, 0x8f}, {0x3D, 0xc2}, {0x3E, 0x00}, {0x3F, 0x00},
        {0x40, 0x00}, {0x41, 0x00}, {0x42, 0x00}, {0x43, 0x00},
        {0x44, 0x00}, {0x45, 0x00}, {0x46, 0x00}, {0x47, 0x00},
        {0x48, 0x00}, {0x49, 0x00}, {0x4A, 0x00}, {0x4B, 0x00},
        {0x4C, 0x00}, {0x4D, 0x00}, {0x4E, 0x01}, {0x4F, 0x00},
        {0x6f, 0x30}, {0x71, 0x00},

        // rfsniffer settings
        /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
        /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_CONTINUOUSNOBSYNC | RF_DATAMODUL_MODULATIONTYPE_OOK | RF_DATAMODUL_MODULATIONSHAPING_00 }, // no shaping
        /* 0x03 */ { REG_BITRATEMSB, 0x3E}, // bitrate: 32768 Hz
        /* 0x04 */ { REG_BITRATELSB, 0x80},
        /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_100 | RF_RXBW_MANT_16 | RF_RXBW_EXP_0},
        //    /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_4}, // BW: 10.4 kHz
        /* 0x1B */ { REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_PEAK | RF_OOKPEAK_PEAKTHRESHSTEP_000 | RF_OOKPEAK_PEAKTHRESHDEC_000 },
        /* 0x1D */ { REG_OOKFIX, 20 }, // Fixed threshold value (in dB) in the OOK demodulator
        /* 0x29 */ { REG_RSSITHRESH, 180 }, // RSSI threshold in dBm = -(REG_RSSITHRESH / 2)
        /* 0x6F */ { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode, recommended default for AfcLowBetaOn=0
        { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 | RF_DIOMAPPING1_DIO2_01},
        { REG_DIOMAPPING2, RF_DIOMAPPING2_DIO5_01 | RF_DIOMAPPING2_DIO4_10},
        { REG_PREAMBLELSB, 5},
        { REG_SYNCCONFIG, RF_SYNC_OFF},
        { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_FIXED | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_OFF | RF_PACKET1_CRCAUTOCLEAR_ON | RF_PACKET1_ADRSFILTERING_OFF},
        { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_NONE | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF },
        { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE},
        //               { REG_TESTAFC, 0},
        {255, 0}
    };

    for (byte i = 0; CONFIG[i][0] != 255; i++) {
        // byte cur = readReg(CONFIG[i][0]);
        //if (cur!=CONFIG[i][1] )
        //  printf ("SET %x from %x to %x\n", CONFIG[i][0], cur, CONFIG[i][1]);
        writeReg(CONFIG[i][0], CONFIG[i][1]);
    }

    setFrequencyMHz(433.92);
    //  setHighPower(_isRFM69HW); // called regardless if it's a RFM69W or RFM69HW
    setMode(RF69OOK_MODE_STANDBY);
    while ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // Wait for ModeReady

    selfPointer = this;
    return true;
}

unsigned long millis(void)
{
    struct timespec ts;

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts.tv_sec = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif

    return ( ts.tv_sec * 1000 + ts.tv_nsec / 1000000L );
}

bool RFM69OOK::canSend()
{
    if (_mode == RF69OOK_MODE_RX && readRSSI() < CSMA_LIMIT) { // if signal stronger than -100dBm is detected assume channel activity
        setMode(RF69OOK_MODE_STANDBY);
        return true;
    }

    return false;
}

// checks if a packet was received and/or puts transceiver in receive (ie RX or listen) mode
bool RFM69OOK::receiveDone()
{
    if (_mode == RF69OOK_MODE_RX) {
        setMode(RF69OOK_MODE_STANDBY); // enables interrupts
        return true;
    } else if (_mode == RF69OOK_MODE_RX) { // already in RX no payload yet
        return false;
    }

    receiveBegin();
    return false;
}


void RFM69OOK::send(const void *buffer, uint8_t size)
{
    writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFB) |
             RF_PACKET2_RXRESTART); // avoid RX deadlocks

    uint8_t dataModul = readReg(REG_DATAMODUL);
    writeReg(REG_DATAMODUL, RF_DATAMODUL_MODULATIONTYPE_OOK | RF_DATAMODUL_MODULATIONSHAPING_00);

    //while (!canSend())
    //    receiveDone();

    sendFrame(buffer, size);
    writeReg(REG_DATAMODUL, dataModul);
}

// internal function
void RFM69OOK::sendFrame(const void *buffer, uint8_t bufferSize)
{
    DPRINTF_DECLARE(dprintf, false);

    dprintf("$P Start\n");

    setMode(RF69OOK_MODE_STANDBY); // turn off receiver to prevent reception while filling fifo
    while ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00)
        usleep(5); // wait for ModeReady

    writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00); // DIO0 is "Packet Sent"

    unsigned char data[RF69OOK_MAX_DATA_LEN + 2];
    unsigned char tmp[RF69OOK_MAX_DATA_LEN + 2];
    if (bufferSize > RF69OOK_MAX_DATA_LEN) {
        bufferSize = RF69OOK_MAX_DATA_LEN;
    }
    data[0] = REG_FIFO | 0x80;
    data[1] = bufferSize;
    memcpy(data + 2, buffer, bufferSize);

    //~ if (m_Log)
        //~ m_Log->PrintBuffer(1, data, bufferSize + 2);

    dprintf("$P Before xfer2\n");
    m_spi->xfer2(data, bufferSize + 2, tmp, 0);
    dprintf("$P After xfer2\n");

    // no need to wait for transmit mode to be ready since its handled by the radio
    setMode(RF69OOK_MODE_TX);


    uint32_t txStart = millis();
    if (!waitGpio(m_gpioInt, RF69_TX_LIMIT_MS)) {
        LOG(WARN) << "Wait gpio failed. Sleep.\n";
        Sleep(RF69_TX_LIMIT_MS);
    }
    dprintf("$P waited for %\n", millis() - txStart);

    setMode(RF69OOK_MODE_STANDBY);
    dprintf("$P Finish\n");
}




bool RFM69OOK::waitGpio(int num, int timeLimitMs)
{
    struct FilePtr : std::unique_ptr<FILE, int(*)(FILE *)> {
        FilePtr(std::string fileName, const char *mode)
            : std::unique_ptr<FILE, int(*)(FILE *)>(fopen(fileName.c_str(), mode), fclose)
        {}
        operator FILE*() {
            return get();
        }
    };

    struct FdPtr {
        int fd;
        FdPtr(std::string fileName, int mode) {
            fd = open(fileName.c_str(), mode);
        }
        ~FdPtr() {
            if (*this) {
                close(fd);
            }
        }
        void operator=(FdPtr &ptr) = delete;

        void operator=(FdPtr &&ptr) {
            fd = ptr.fd;
            ptr.fd = -1;
        }

        operator int() {
            return fd;
        }
        operator bool() {
            return fd >= 0;
        }
    };

    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Start\n");

    const int iters = 10;

    String gpioFileName = String::ComposeFormat("/sys/class/gpio/gpio%d/value", num);
    String gpioEdgeFileName = String::ComposeFormat("/sys/class/gpio/gpio%d/edge", num);
    String gpioDirectionFileName = String::ComposeFormat("/sys/class/gpio/gpio%d/direction", num);

    auto gpioFd = FdPtr(gpioFileName, O_RDONLY);
    dprintf("$P after opening attempt (%)\n", bool(gpioFd));

    if (!gpioFd) {
        LOG(INFO) << "Gpio is not exported. Try to export\n";
        auto exportFile = FilePtr("/sys/class/gpio/export", "w");

        if (!exportFile) {
            throw CHaException(CHaException::ErrBadParam, "Can open neither gpio file nor export file =(");
        }
        fprintf(exportFile, "%s", String::ValueOf(num).c_str());
    }

    dprintf("$P (after exporting)\n");

    for (int i = 0; i < iters; i++) {
        auto file = FilePtr(gpioDirectionFileName, "w");
        if (file) {
            fprintf(file, "%s", "in");
            break;
        }
        LOG(WARN) << "Can't open " << gpioDirectionFileName << " attempt " << i << " of " << iters << "\n";
        if (i + 1 == iters) {
            throw CHaException(CHaException::ErrBadParam, "Can't open " + gpioDirectionFileName);
        }
        Sleep(20);
    }

    for (int i = 0; i < iters; i++) {
        auto gpioEdgeFile = FilePtr(gpioEdgeFileName, "w");
        if (gpioEdgeFile) {
            fprintf(gpioEdgeFile, "%s", "rising");
            break;
        }
        LOG(WARN) << "Can't open " << gpioEdgeFileName << " attempt " << i << " of " << iters << "\n";
        if (i + 1 == iters) {
            throw CHaException(CHaException::ErrBadParam, "Can't open " + gpioEdgeFileName);
        }
        Sleep(20);
    }

    dprintf("$P (after setting edge)\n");

    for (int i = 1; !gpioFd && i < iters; i++) {
        gpioFd = FdPtr(gpioFileName, O_RDONLY);
        if (gpioFd) {
            break;
        }
        LOG(WARN) << "Can't open " << gpioFileName << " attempt " << i << " of " << iters << "\n";
        Sleep(20);
    }

    dprintf("$P after attemtps of opening\n");

    if (!gpioFd) {
        throw CHaException(CHaException::ErrBadParam, "Can't open " + gpioFileName);
    }

    dprintf("$P before trying to read fd=% (max %)\n", (int)gpioFd, FD_SETSIZE);

    for (int i = 0; i < 2; i++) {
        dprintf("$P Iter %\n", i);

        char buffer[3] = "0";

        if (i == 1) {
            fd_set fds;
            struct timeval tv;
            tv.tv_sec = timeLimitMs / 1000;
            tv.tv_usec = (timeLimitMs % 1000) * 1000;

            FD_ZERO(&fds);
            FD_SET(gpioFd, &fds);

            int ret = ::select(gpioFd + 1, &fds, NULL, NULL, &tv);

            dprintf("$P after select call ret=% fd_isset=%\n", ret, FD_ISSET(gpioFd, &fds));

            if (ret == -1) {
                LOG(WARN) << "select call of gpio38/value failed"
                    << (errno == EBADF) << (errno == EINTR) << (errno == EINVAL) << (errno == ENOMEM) << "\n";
                Sleep(20);
                --i;
                continue;
            }
            if (!FD_ISSET(gpioFd, &fds)) {
                return false;
            }
        }

        dprintf("$P before lseek\n");

        lseek(gpioFd, 0, SEEK_SET);

        dprintf("$P before read\n");

        read(gpioFd, buffer, sizeof(char));

        dprintf("$P after read (%)\n", buffer);

        if (i == 1) {
            if (buffer[0] != '1') {
                --i;
                continue;
            }
            else {
                return true;
            }
        }
    }
    return false;
}

// Turn the radio into transmission mode
void RFM69OOK::transmitBegin()
{
    setMode(RF69OOK_MODE_TX);
}

// Turn the radio back to standby
void RFM69OOK::transmitEnd()
{
    setMode(RF69OOK_MODE_STANDBY);
}

// Turn the radio into OOK listening mode
void RFM69OOK::receiveBegin()
{
    setMode(RF69OOK_MODE_RX);
}

// Turn the radio back to standby
void RFM69OOK::receiveEnd()
{
    setMode(RF69OOK_MODE_STANDBY);
}

// Handle pin change interrupts in OOK mode
void RFM69OOK::interruptHandler()
{
    if (userInterrupt != null) (*userInterrupt)();
}

// Set a user interrupt for all transfer methods in receive mode
// call with NULL to disable the user interrupt handler
void RFM69OOK::attachUserInterrupt(void (*function)())
{
    userInterrupt = function;
}

// return the frequency (in Hz)
uint32_t RFM69OOK::getFrequency()
{
    return RF69OOK_FSTEP * (((uint32_t)readReg(REG_FRFMSB) << 16) + ((uint16_t)readReg(
                                REG_FRFMID) << 8) + readReg(REG_FRFLSB));
}

// Set literal frequency using floating point MHz value
void RFM69OOK::setFrequencyMHz(float f)
{
    setFrequency(f * 1000000);
}

// set the frequency (in Hz)
void RFM69OOK::setFrequency(uint32_t freqHz)
{
    // TODO: p38 hopping sequence may need to be followed in some cases
    freqHz /= RF69OOK_FSTEP; // divide down by FSTEP to get FRF
    writeReg(REG_FRFMSB, freqHz >> 16);
    writeReg(REG_FRFMID, freqHz >> 8);
    writeReg(REG_FRFLSB, freqHz);
}

// set OOK bandwidth
void RFM69OOK::setBandwidth(uint8_t bw)
{
    writeReg(REG_RXBW, (readReg(REG_RXBW) & 0xE0) | bw);
}

// set RSSI threshold
void RFM69OOK::setRSSIThreshold(int8_t rssi)
{
    writeReg(REG_RSSITHRESH, -(rssi << 1));
}

// set OOK fixed threshold
void RFM69OOK::setFixedThreshold(uint8_t threshold)
{
    writeReg(REG_OOKFIX, threshold);
}

// set sensitivity boost in REG_TESTLNA
// see: http://www.sevenwatt.com/main/rfm69-ook-dagc-sensitivity-boost-and-modulation-index
void RFM69OOK::setSensitivityBoost(uint8_t value)
{
    writeReg(REG_TESTLNA, value);
}

void RFM69OOK::setMode(byte newMode)
{
    if (newMode == _mode) return;

    switch (newMode) {
        case RF69OOK_MODE_TX:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
            if (_isRFM69HW) setHighPowerRegs(true);
            break;
        case RF69OOK_MODE_RX:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
            if (_isRFM69HW) setHighPowerRegs(false);
            break;
        case RF69OOK_MODE_SYNTH:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
            break;
        case RF69OOK_MODE_STANDBY:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
            break;
        case RF69OOK_MODE_SLEEP:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
            break;
        default:
            return;
    }

    // waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
    while (_mode == RF69OOK_MODE_SLEEP
            && (readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) {
        Sleep(5);
    }

    _mode = newMode;
}

void RFM69OOK::sleep()
{
    setMode(RF69OOK_MODE_SLEEP);
}

// set output power: 0=min, 31=max
// this results in a "weaker" transmitted signal, and directly results in a lower RSSI at the receiver
void RFM69OOK::setPowerLevel(byte powerLevel)
{
    _powerLevel = powerLevel;
    writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0xE0) | (_powerLevel > 31 ? 31 : _powerLevel));
}

void RFM69OOK::isr0()
{
    selfPointer->interruptHandler();
}

int RFM69OOK::readRSSI(bool forceTrigger)
{
    int rssi = 0;

    if (forceTrigger) {
        // RSSI trigger not needed if DAGC is in continuous mode
        writeReg(REG_RSSICONFIG, RF_RSSI_START);
        while ((readReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00) {
            Sleep(5);
        }
    }

    rssi = -readReg(REG_RSSIVALUE);
    rssi >>= 1;
    return rssi;
}

byte RFM69OOK::readReg(byte addr)
{
    unsigned char val, tmp;
    tmp = addr & 0x7F;
    m_spi->xfer2(&tmp, 1, &val, 1);
    return val;

}

void RFM69OOK::writeReg(byte addr, byte value)
{
    unsigned char data[2], tmp;
    data[0] = addr | 0x80;
    data[1] = value;
    m_spi->xfer2(data, 2, &tmp, 0);
}

void RFM69OOK::setHighPower(bool onOff)
{
    _isRFM69HW = onOff;
    writeReg(REG_OCP, _isRFM69HW ? RF_OCP_OFF : RF_OCP_ON);
    if (_isRFM69HW) // turning ON
        writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON |
                 RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
    else
        writeReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF |
                 _powerLevel); // enable P0 only
}

void RFM69OOK::setHighPowerRegs(bool onOff)
{
    writeReg(REG_TESTPA1, onOff ? 0x5D : 0x55);
    writeReg(REG_TESTPA2, onOff ? 0x7C : 0x70);
}

// for debugging
void RFM69OOK::readAllRegs()
{
    byte regVal;
    for (byte regAddr = 1; regAddr <= 0x4F; regAddr++) {
        regVal = readReg(regAddr);
        printf("%X = %X", regAddr, regVal);
    }
}

byte RFM69OOK::readTemperature(byte calFactor)  // returns centigrade
{
    setMode(RF69OOK_MODE_STANDBY);
    writeReg(REG_TEMP1, RF_TEMP1_MEAS_START);
    while ((readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING)) {
        Sleep(5);
    }
    return ~readReg(REG_TEMP2) + COURSE_TEMP_COEF +
           calFactor; // 'complement' corrects the slope, rising temp = rising val
}                                                            // COURSE_TEMP_COEF puts reading in the ballpark, user can add additional correction

void RFM69OOK::rcCalibration()
{
    writeReg(REG_OSC1, RF_OSC1_RCCAL_START);
    while ((readReg(REG_OSC1) & RF_OSC1_RCCAL_DONE) == 0x00) {
        Sleep(5);
    }
}


uint32_t RFM69OOK::getBitrate()
{
    return FXOSC / ((readReg(REG_BITRATEMSB) << 8) + readReg(REG_BITRATELSB));
}

void RFM69OOK::setBitrate(uint32_t freqHz)
{
    uint32_t val = FXOSC / freqHz;

    if (val > 0x10000)
        throw CHaException(CHaException::ErrBadParam, "setBitrate failed");

    writeReg(REG_BITRATEMSB, val >> 8);
    writeReg(REG_BITRATELSB, val & 0xFF);
}


