AM_CPPFLAGS = -I../include/ -std=c++0x
bin_PROGRAMS = wb-homa-rfsniffer
wb_homa_rfsniffer_SOURCES = rfsniffer.cpp MqttConnect.cpp
wb_homa_rfsniffer_LDADD = $(top_builddir)/libs/libutils/libutils.la \
	$(top_builddir)/libs/librf/librf.la $(top_builddir)/libs/libwb/libwb.la  \
	-lmosquitto  -lmosquittopp -lpthread
