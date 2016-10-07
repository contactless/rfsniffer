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
#ifndef RFM69OOK_h
#define RFM69OOK_h

typedef unsigned char byte;
typedef unsigned char uint8_t;
typedef signed char int8_t;

typedef unsigned int uint32_t;


#define RF69OOK_MODE_SLEEP       0 // XTAL OFF
#define RF69OOK_MODE_STANDBY     1 // XTAL ON
#define RF69OOK_MODE_SYNTH       2 // PLL ON
#define RF69OOK_MODE_RX          3 // RX MODE
#define RF69OOK_MODE_TX          4 // TX MODE
#define RF69OOK_CSMA_LIMIT_MS 1000
#define RF69OOK_MAX_DATA_LEN       61 // to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)

#define null                  0
#define COURSE_TEMP_COEF    -90 // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69OOK_FSTEP 61.03515625 // == FXOSC/2^19 = 32mhz/2^19 (p13 in DS)
#define CSMA_LIMIT              -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
#define RF69_TX_LIMIT_MS 500
#define FXOSC 32000000L

class SPI;
class CLog;

class RFM69OOK {
  public:
    static volatile int RSSI; //most accurate RSSI during reception (closest to the reception)
    static volatile byte _mode; //should be protected?
    static volatile uint8_t PAYLOADLEN;

/*
    RFM69OOK(byte slaveSelectPin=RF69OOK_SPI_CS, byte interruptPin=RF69OOK_IRQ_PIN, bool isRFM69HW=false, byte interruptNum=RF69OOK_IRQ_NUM) {
      _slaveSelectPin = slaveSelectPin;
      _interruptPin = interruptPin;
      _interruptNum = interruptNum;
      _mode = RF69OOK_MODE_STANDBY;
      _powerLevel = 31;
      _isRFM69HW = isRFM69HW;
    }
*/
    RFM69OOK(SPI* spi, int gpioInt, CLog *log=NULL);

    bool initialize();
    uint32_t getFrequency();
    void setFrequency(uint32_t freqHz);
    void setFrequencyMHz(float f);
    uint32_t getBitrate();
    void setBitrate(uint32_t freqHz);
    int readRSSI(bool forceTrigger=false);
    void setHighPower(bool onOFF=true); //have to call it after initialize for RFM69HW
    void setPowerLevel(byte level); //reduce/increase transmit power level
    void sleep();
    byte readTemperature(byte calFactor=0); //get CMOS temperature (8bit)
    void rcCalibration(); //calibrate the internal RC oscillator for use in wide temperature variations - see datasheet section [4.3.5. RC Timer Accuracy]

    // allow hacking registers by making these public
    byte readReg(byte addr);
    void writeReg(byte addr, byte val);
    void readAllRegs();
    uint32_t getAFC(){return 0;};

    // functions related to OOK mode
    void receiveBegin();
    void receiveEnd();
    void transmitBegin();
    void transmitEnd();
    bool poll();
    void send(const void* buffer, uint8_t size);
    void attachUserInterrupt(void (*function)());
	void setBandwidth(uint8_t bw);
	void setRSSIThreshold(int8_t rssi);
	void setFixedThreshold(uint8_t threshold);
	void setSensitivityBoost(uint8_t value);

  protected:
    static void isr0();
    void virtual interruptHandler();

    static RFM69OOK* selfPointer;
    byte _slaveSelectPin;
    byte _interruptPin;
    byte _interruptNum;
    byte _powerLevel;
    bool _isRFM69HW;
    byte _SPCR;
    byte _SPSR;
    CLog *m_Log;

    void setMode(byte mode);
    void setHighPowerRegs(bool onOff);
    void select();
    void unselect();
    bool canSend();
    void sendFrame(const void* buffer, uint8_t size);
    bool receiveDone();

    // functions related to OOK mode
    void (*userInterrupt)();
    void ookInterruptHandler();
    SPI* m_spi;
    int m_gpioInt;

    bool getGPIO(int num);
};

unsigned long millis(void);


#endif
