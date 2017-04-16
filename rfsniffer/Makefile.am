AM_CXXFLAGS = -I../include/ -std=c++0x -Wall

bin_PROGRAMS = wb-homa-rfsniffer wb-homa-rfsniffer-dbg

wb_homa_rfsniffer_SOURCES = rfsniffer.cpp MqttConnect.cpp backward.cpp 
wb_homa_rfsniffer_LDADD = $(top_builddir)/libs/libutils/libutils.la \
	$(top_builddir)/libs/librf/librf.la $(top_builddir)/libs/libwb/libwb.la  \
	-lmosquitto  -lmosquittopp -lpthread -ldw

wb_homa_rfsniffer_dbg_SOURCES = rfsniffer.cpp MqttConnect.cpp
wb_homa_rfsniffer_dbg_CXXFLAGS = -I../include/ -std=c++0x -pg
wb_homa_rfsniffer_dbg_LDADD = $(top_builddir)/libs/libutils/libutils.la \
	$(top_builddir)/libs/librf/librf.la $(top_builddir)/libs/libwb/libwb.la  \
	-lmosquitto  -lmosquittopp -lpthread
