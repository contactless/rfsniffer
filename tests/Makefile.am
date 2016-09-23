AM_CPPFLAGS = -I../include/
bin_PROGRAMS = rftest
check_PROGRAMS = rftest
rftest_SOURCES = LogTest.cpp RfParser.cpp main.cpp rfm69Test.cpp snifferTest.cpp mqttTest.cpp
rftest_LDADD = $(top_builddir)/libs/libutils/libutils.la \
	$(top_builddir)/libs/librf/librf.la $(top_builddir)/libs/libwb/libwb.la  \
	-lmosquitto  -lmosquittopp -lpthread

AM_CFLAGS = -pthread
AM_CXXFLAGS = -pthread

