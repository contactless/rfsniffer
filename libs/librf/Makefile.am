lib_LTLIBRARIES = librf.la
librf_la_SOURCES = RFParser.cpp RFProtocol.cpp RFProtocolLivolo.cpp RFProtocolRST.cpp RFProtocolRaex.cpp RFProtocolX10.cpp spidev_lib++.cpp \
		RFProtocolOregon.cpp RFProtocolOregonV3.cpp RFProtocolNooLite.cpp RFProtocolMotionSensor.cpp RFProtocolHS24Bit.cpp RFAnalyzer.cpp RFProtocolVhome.cpp \
		RFParser.h RFProtocol.h RFProtocolLivolo.h RFProtocolRST.h RFProtocolRaex.h RFProtocolX10.h spidev_lib++.h RFM69OOK.cpp RFM69OOK.h RFProtocolRubitek.cpp 

AM_CFLAGS = -pthread
AM_CXXFLAGS = -pthread -std=c++0x -pg
librf_la_LDFLAGS = -all-static

