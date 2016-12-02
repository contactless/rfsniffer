
#include "stdafx.h"
#include "../libs/libutils/logging.h"
#include "../libs/libutils/thread.h"
#include "../libs/libutils/Exception.h"
#include "../libs/librf/spidev_lib++.h"
#include "../libs/librf/RFM69OOKregisters.h"
#include "../libs/librf/RFM69OOK.h"
#include "../libs/librf/RFProtocolNooLite.h"

void dumpRegs(RFM69OOK *rfm)
{
	for (unsigned char reg=1;reg<0x4F;reg++)
	{
		unsigned char val = rfm->readReg(reg);
		//printf("Reg %x = %x (%d)\n", reg, val, val);

    printf("reg 0x%x = 0x%x = %d\n", reg, val, val);
	}
}

void Rfm69Test()
{

    printf("Rfm69Test\n");

	spi_config_t spi_config;
  spi_config.mode=0;
  spi_config.speed=500000;
  spi_config.delay=0;
  spi_config.bits_per_word=8;

	uint8_t tx_buffer[32];
	uint8_t rx_buffer[32];



  SPI mySPI("/dev/spidev32766.0",&spi_config);

  if (!mySPI.begin())
  {
    printf("mySPI->begin() failed\n");
    exit(1);
  }

  try
  {
    printf("mySPI->begin()\n");

  	/*
    memset(tx_buffer,0,32);
    memset(rx_buffer,0,32);


	//writeReg(REG_OPMODE, (self.readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER)
	//	self.spi.write_then_read([addr | 0x80, value], 0)

    sprintf((char*)tx_buffer,"hello world");
    printf("sending %s, to spidev2.0 in full duplex \n ",(char*)tx_buffer); 
    mySPI->xfer(tx_buffer,strlen((char*)tx_buffer),rx_buffer,strlen((char*)tx_buffer));
    printf("rx_buffer=%s\n",(char *)rx_buffer);
    //mySPI->end();
    */
    //writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);

    RFM69OOK rfm(&mySPI, 38);
    printf("Reg REG_OPMODE = %x\n", rfm.readReg(REG_OPMODE));
    printf("Reg REG_IRQFLAGS1 = %x\n", rfm.readReg(REG_IRQFLAGS1));
    rfm.writeReg(REG_OPMODE, (rfm.readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
    printf("Reg REG_OPMODE = %x\n", rfm.readReg(REG_OPMODE));
    printf("Bitrate = %ld\n", (long int)rfm.getBitrate());

    rfm.initialize();

    rfm.writeReg(REG_SYNCCONFIG, 0x8d);
//    rfm.writeReg(REG_DATAMODUL, RF_DATAMODUL_MODULATIONTYPE_OOK | RF_DATAMODUL_MODULATIONSHAPING_00);

    //rfm.writeReg(REG_OPMODE, 8);
    rfm.writeReg(0x25, 40);

    rfm.receiveBegin();

    CRFProtocolNooLite nooLite;

//11001011001000010110010000000001101110
//10001011001000010110010000000001010110
    while (1)
    {
      //*
      uint8_t buffer[100];
      size_t bufferSize=sizeof(buffer);
      printf("\nP1\n");
      nooLite.EncodeData("nooLite:flip=1 cmd=4 addr=9a13 fmt=00 crc=76", 2000, buffer, bufferSize);
      rfm.send(buffer, bufferSize);
      rfm.receiveBegin();

      Sleep(2000);

      printf("P0\n");
      nooLite.EncodeData("nooLite:flip=0 cmd=4 addr=9a13 fmt=00 crc=6a", 2000, buffer, bufferSize);
      rfm.send(buffer, bufferSize);
      rfm.receiveBegin();
      Sleep(2000);



      //*/
      
      //uint8_t buffer[100];
      //size_t bufferSize=sizeof(buffer);
      bufferSize=sizeof(buffer);
      printf("\nP1\n");
      nooLite.EncodeData("nooLite:flip=1 cmd=6 addr=9a13 level=30", 2000, buffer, bufferSize);
      rfm.send(buffer, bufferSize);
      rfm.receiveBegin();

      Sleep(2000);

      printf("P0\n");
      bufferSize=sizeof(buffer);
      nooLite.EncodeData("nooLite:flip=0 cmd=6 addr=9a13 level=99", 2000, buffer, bufferSize);
      rfm.send(buffer, bufferSize);
      rfm.receiveBegin();
      Sleep(2000);

      unsigned char data1[]={170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 133, 150, 86, 86, 106, 150, 101, 170, 106, 170, 169, 101, 11, 44, 172, 172, 213, 44, 203, 84, 213, 85, 82, 202};
  //    dumpRegs(&rfm);
      printf("\nW1\n");
      rfm.send(data1, sizeof(data1));
      rfm.receiveBegin();

      Sleep(1000);

      
      printf("W0\n");
      unsigned char data2[]={170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 134, 150, 154, 90, 106, 150, 101, 170, 106, 170, 86, 169, 13, 45, 52, 180, 213, 44, 203, 84, 213, 84, 173, 82};
    //  dumpRegs(&rfm);
      rfm.send(data2, sizeof(data2));
      rfm.receiveBegin();

      Sleep(5000);
//*/
    }

    rfm.writeReg(REG_OPMODE, (rfm.readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);

    //rfm.receiveBegin();

/*    printf("Reg REG_SYNCVALUE1 = %x\n", readReg(REG_SYNCVALUE1));
    writeReg(REG_SYNCVALUE1, 0xAA);
    printf("Reg REG_SYNCVALUE1 = %x\n", readReg(REG_SYNCVALUE1));
    writeReg(REG_SYNCVALUE1, 0xBB);
    printf("Reg REG_SYNCVALUE1 = %x\n", readReg(REG_SYNCVALUE1));
*/

  } 
  catch (CHaException ex)
  {
    printf ("failed with Exception %d/%s", ex.GetCode(), ex.GetMsg().c_str());
  }
}


void SendTest()
{



 /*
Reg 1 = c (12) /??



Reg 24 = ff (255)
+Reg 25 = 0 (0)    //????
Reg 27 = 80 (128)
Reg 2e = 87 (135)

Reg 24 = ff (255)  // RegRssiValue
Reg 25 = 44 (68)   // RegDioMapping1 |= Dio2Mapping
Reg 27 = 90 (144) // RegIrqFlags1
Reg 2e = 0 (0)  // RegSyncConfig  --- RF_SYNC_ON | RF_SYNC_SIZE_1 | RF_SYNC_TOL_7

reg 0x24 = 0xa1 = 161  //
reg 0x25 = 0x40 = 64
reg 0x27 = 0x90 = 144
reg 0x2e = 0x8d = 141






---------
Reg 1 = 10 (16)
Reg 2 = 68 (104)  ! 8 RegDataModul |=Continuous mode without bit synchronizer
Reg 3 = 3e (62)
Reg 4 = 80 (128)
Reg 5 = 0 (0)
Reg 6 = 52 (82)
Reg 7 = 6c (108)
Reg 8 = 7a (122)
Reg 9 = e1 (225)
Reg a = 41 (65)
Reg b = 0 (0)    // 0x40  |= AfcLowBetaOn Improved AFC routine
Reg c = 2 (2)
Reg d = 92 (146)
Reg e = f5 (245)
Reg f = 20 (32)

reg 0x1 = 0x10 = 16
reg 0x2 = 0x8 = 8
reg 0x3 = 0x3e = 62
reg 0x4 = 0x80 = 128
reg 0x5 = 0x0 = 0
reg 0x6 = 0x52 = 82
reg 0x7 = 0x6c = 108
reg 0x8 = 0x7a = 122
reg 0x9 = 0xe1 = 225
reg 0xa = 0x41 = 65
reg 0xb = 0x40 = 64
reg 0xc = 0x2 = 2
reg 0xd = 0x92 = 146
reg 0xe = 0xf5 = 245
reg 0xf = 0x20 = 32



Reg 10 = 24 (36)
Reg 11 = ff (255)
Reg 12 = 9 (9)
Reg 13 = 1a (26)
Reg 14 = 40 (64)
Reg 15 = b0 (176)
Reg 16 = 7b (123)
Reg 17 = 9b (155)
Reg 18 = 8 (8)
Reg 19 = 80 (128)
Reg 1a = 8a (138)
Reg 1b = 40 (64)
Reg 1c = 80 (128)
Reg 1d = 14 (20)
Reg 1e = 10 (16)
Reg 1f = 0 (0)

reg 0x10 = 0x24 = 36
reg 0x11 = 0xff = 255
reg 0x12 = 0x9 = 9
reg 0x13 = 0x1a = 26
reg 0x14 = 0x40 = 64
reg 0x15 = 0xb0 = 176
reg 0x16 = 0x7b = 123
reg 0x17 = 0x9b = 155
reg 0x18 = 0x8 = 8
reg 0x19 = 0x80 = 128
reg 0x1a = 0x8a = 138
reg 0x1b = 0x40 = 64
reg 0x1c = 0x80 = 128
reg 0x1d = 0x14 = 20
reg 0x1e = 0x10 = 16
reg 0x1f = 0x0 = 0



Reg 20 = 0 (0)
Reg 21 = 0 (0)
Reg 22 = 0 (0)
Reg 23 = 0 (0)
Reg 24 = ff (255)  // RegRssiValue
Reg 25 = 44 (68)   // RegDioMapping1 |= Dio2Mapping
Reg 26 = 90 (144)
Reg 27 = 90 (144)
Reg 28 = 60 (96)  //RegIrqFlags2 
Reg 29 = b4 (180)  //RssiThreshold = 90dbm
Reg 2a = 0 (0)
Reg 2b = 0 (0)
Reg 2c = 0 (0)
Reg 2d = 5 (5)
Reg 2e = 0 (0)  // RegSyncConfig  --- RF_SYNC_ON | RF_SYNC_SIZE_1 | RF_SYNC_TOL_7
Reg 2f = aa (170)

reg 0x20 = 0x0 = 0
reg 0x21 = 0x0 = 0
reg 0x22 = 0x0 = 0
reg 0x23 = 0x0 = 0
reg 0x24 = 0xa1 = 161  //
reg 0x25 = 0x40 = 64
reg 0x26 = 0x90 = 144
reg 0x27 = 0x90 = 144
reg 0x28 = 0x40 = 64
reg 0x29 = 0xaa = 170 //RssiThreshold = 85dbm
reg 0x2a = 0x0 = 0
reg 0x2b = 0x0 = 0
reg 0x2c = 0x0 = 0
reg 0x2d = 0x5 = 5
reg 0x2e = 0x8d = 141
reg 0x2f = 0xaa = 170


Reg 30 = 66 (102)
Reg 31 = 0 (0)
Reg 32 = 0 (0)
Reg 33 = 0 (0)
Reg 34 = 0 (0)
Reg 35 = 0 (0)
Reg 36 = 0 (0)
Reg 37 = 0 (0)
Reg 38 = 3c (60)
Reg 39 = 0 (0)
Reg 3a = 0 (0)
Reg 3b = 0 (0)
Reg 3c = 8f (143)
Reg 3d = c2 (194)
Reg 3e = 0 (0)

reg 0x30 = 0x66 = 102
reg 0x31 = 0x0 = 0
reg 0x32 = 0x0 = 0
reg 0x33 = 0x0 = 0
reg 0x34 = 0x0 = 0
reg 0x35 = 0x0 = 0
reg 0x36 = 0x0 = 0
reg 0x37 = 0x0 = 0
reg 0x38 = 0x3c = 60
reg 0x39 = 0x0 = 0
reg 0x3a = 0x0 = 0
reg 0x3b = 0x0 = 0
reg 0x3c = 0x8f = 143
reg 0x3d = 0xc2 = 194
reg 0x3e = 0x0 = 0
reg 0x3f = 0x0 = 0



Reg 3f = 0 (0)
Reg 40 = 0 (0)
Reg 41 = 0 (0)
Reg 42 = 0 (0)
Reg 43 = 0 (0)
Reg 44 = 0 (0)
Reg 45 = 0 (0)
Reg 46 = 0 (0)
Reg 47 = 0 (0)
Reg 48 = 0 (0)
Reg 49 = 0 (0)
Reg 4a = 0 (0)
Reg 4b = 0 (0)
Reg 4c = 0 (0)
Reg 4d = 0 (0)
Reg 4e = 1 (1)



radio send [170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 133, 166, 106, 150, 101, 170, 170, 170, 89, 86, 11, 76, 213, 44, 203, 85, 85, 84, 178, 172]
reg 0x40 = 0x0 = 0
reg 0x41 = 0x0 = 0
reg 0x42 = 0x0 = 0
reg 0x43 = 0x0 = 0
reg 0x44 = 0x0 = 0
reg 0x45 = 0x0 = 0
reg 0x46 = 0x0 = 0
reg 0x47 = 0x0 = 0
reg 0x48 = 0x0 = 0
reg 0x49 = 0x0 = 0
reg 0x4a = 0x0 = 0
reg 0x4b = 0x0 = 0
reg 0x4c = 0x0 = 0
reg 0x4d = 0x0 = 0
reg 0x4e = 0x1 = 1
reg 0x4f = 0x0 = 0 */ 
}
