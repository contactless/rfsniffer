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

#include "stdafx.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#ifdef __MACH__
# include <mach/clock.h>
# include <mach/mach.h>
#endif
#include "../libutils/thread.h"
#include "spidev_lib++.h"
#include "RFM69OOK.h"
#include "RFM69OOKregisters.h"

volatile byte RFM69OOK::_mode;  // current transceiver state
volatile int RFM69OOK::RSSI; 	  // most accurate RSSI during reception (closest to the reception)
RFM69OOK* RFM69OOK::selfPointer;
volatile uint8_t RFM69OOK::PAYLOADLEN;

RFM69OOK::RFM69OOK(SPI* spi, int gpioInt, CLog *log)
  :m_spi(spi), m_gpioInt(gpioInt), m_Log(log)
{

}

bool RFM69OOK::initialize()
{
  const byte CONFIG[][2] =
  {
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

  for (byte i = 0; CONFIG[i][0] != 255; i++)
  {
    byte cur = readReg(CONFIG[i][0]);
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

void RFM69OOK::send(const void* buffer, uint8_t size)
{
  writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
  
  uint8_t dataModul = readReg(REG_DATAMODUL);
  writeReg(REG_DATAMODUL, RF_DATAMODUL_MODULATIONTYPE_OOK | RF_DATAMODUL_MODULATIONSHAPING_00);
  
  while (!canSend()) 
    receiveDone();

  sendFrame(buffer, size);
  writeReg(REG_DATAMODUL, dataModul);
}

// internal function
void RFM69OOK::sendFrame(const void* buffer, uint8_t bufferSize)
{
  setMode(RF69OOK_MODE_STANDBY); // turn off receiver to prevent reception while filling fifo
  while ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00)
    usleep(5); // wait for ModeReady
 
  writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00); // DIO0 is "Packet Sent"

  unsigned char data[RF69OOK_MAX_DATA_LEN+2], tmp[RF69OOK_MAX_DATA_LEN+2];
  if (bufferSize > RF69OOK_MAX_DATA_LEN) bufferSize = RF69OOK_MAX_DATA_LEN;
  data[0] = REG_FIFO | 0x80;
  data[1] = bufferSize;
  memcpy(data+2, buffer, bufferSize);

  if (m_Log)
    m_Log->PrintBuffer(1, data, bufferSize+2);

  m_spi->xfer2(data, bufferSize+2, tmp, 0);

  // no need to wait for transmit mode to be ready since its handled by the radio
  setMode(RF69OOK_MODE_TX);
  uint32_t txStart = millis();
  while (!getGPIO(m_gpioInt) && millis() - txStart < RF69_TX_LIMIT_MS)
    Sleep(20);

  setMode(RF69OOK_MODE_STANDBY);
}

bool RFM69OOK::canSend()
{
  if (_mode == RF69OOK_MODE_RX && PAYLOADLEN == 0 && readRSSI() < CSMA_LIMIT) // if signal stronger than -100dBm is detected assume channel activity
  {
    setMode(RF69OOK_MODE_STANDBY);
    return true;
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
  return RF69OOK_FSTEP * (((uint32_t)readReg(REG_FRFMSB)<<16) + ((uint16_t)readReg(REG_FRFMID)<<8) + readReg(REG_FRFLSB));
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
        default: return;
    }

    // waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
    while (_mode == RF69OOK_MODE_SLEEP && (readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // Wait for ModeReady

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

void RFM69OOK::isr0() { selfPointer->interruptHandler(); }

int RFM69OOK::readRSSI(bool forceTrigger) 
{
  int rssi = 0;
  
  if (forceTrigger)
  {
    // RSSI trigger not needed if DAGC is in continuous mode
    writeReg(REG_RSSICONFIG, RF_RSSI_START);
    while ((readReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00); // Wait for RSSI_Ready
  }
  
  rssi = -readReg(REG_RSSIVALUE);
  rssi >>= 1;
  return rssi;
}

byte RFM69OOK::readReg(byte addr)
{
  unsigned char val, tmp;
  tmp = addr & 0x7F;
  int res = m_spi->xfer2(&tmp, 1, &val, 1);
  return val;

}

void RFM69OOK::writeReg(byte addr, byte value)
{
  unsigned char data[2], tmp;
  data[0] = addr | 0x80;
  data[1] = value;
  int res = m_spi->xfer2(data, 2, &tmp, 0);
}

void RFM69OOK::setHighPower(bool onOff) 
{
  _isRFM69HW = onOff;
  writeReg(REG_OCP, _isRFM69HW ? RF_OCP_OFF : RF_OCP_ON);
  if (_isRFM69HW) // turning ON
    writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
  else
    writeReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | _powerLevel); // enable P0 only
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
  while ((readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
  return ~readReg(REG_TEMP2) + COURSE_TEMP_COEF + calFactor; // 'complement' corrects the slope, rising temp = rising val
}                                                            // COURSE_TEMP_COEF puts reading in the ballpark, user can add additional correction

void RFM69OOK::rcCalibration()
{
  writeReg(REG_OSC1, RF_OSC1_RCCAL_START);
  while ((readReg(REG_OSC1) & RF_OSC1_RCCAL_DONE) == 0x00);
}

// checks if a packet was received and/or puts transceiver in receive (ie RX or listen) mode
bool RFM69OOK::receiveDone() 
{
  if (_mode == RF69OOK_MODE_RX && PAYLOADLEN > 0)
  {
    setMode(RF69OOK_MODE_STANDBY); // enables interrupts
    return true;
  }
  else if (_mode == RF69OOK_MODE_RX) // already in RX no payload yet
  {
    return false;
  }

  receiveBegin();
  return false;
}

bool RFM69OOK::getGPIO(int num)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", num);
  FILE *f = fopen(buffer, "r");
  if (!f)
    throw CHaException(CHaException::ErrBadParam, buffer);

  if (fread(buffer, 1, 1, f)!=1)
    throw CHaException(CHaException::ErrBadParam, "Read GPIO file failed");

  fclose(f);

  return buffer[0]=='1';
}

uint32_t RFM69OOK::getBitrate()
{
  return FXOSC/((readReg(REG_BITRATEMSB)<<8)+readReg(REG_BITRATELSB)); 
}

void RFM69OOK::setBitrate(uint32_t freqHz)
{
  uint32_t val = FXOSC/freqHz;

  if (val>0x10000)
    throw CHaException(CHaException::ErrBadParam, "setBitrate failed");

  writeReg(REG_BITRATEMSB, val>>8);
  writeReg(REG_BITRATELSB, val&0xFF);
}


