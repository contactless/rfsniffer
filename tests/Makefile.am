AM_CPPFLAGS = -I../include/
bin_PROGRAMS = wb-homa-rfsniffer-test test-core-cpp-wrapper
check_PROGRAMS = wb-homa-rfsniffer-test test-core-cpp-wrapper


test_core_cpp_wrapper_SOURCES = test_core_cpp_wrapper.cpp

wb_homa_rfsniffer_test_SOURCES = LogTest.cpp RfParserTest.cpp mainTest.cpp rfm69Test.cpp snifferTest.cpp mqttTest.cpp
wb_homa_rfsniffer_test_LDADD = $(top_builddir)/libs/libutils/libutils.la \
	$(top_builddir)/libs/librf/librf.la $(top_builddir)/libs/libwb/libwb.la  \
	-lmosquitto  -lmosquittopp -lpthread

AM_CFLAGS = -pthread
AM_CXXFLAGS = -pthread -std=c++0x

