AM_CPPFLAGS = -I../include/
bin_PROGRAMS = rfsniffer
rfsniffer_SOURCES = rfsniffer.cpp MqttConnect.cpp
rfsniffer_LDADD = $(top_builddir)/libs/libutils/libutils.la \
	$(top_builddir)/libs/librf/librf.la $(top_builddir)/libs/libwb/libwb.la  \
	-lmosquitto  -lmosquittopp -lpthread
