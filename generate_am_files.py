#! /usr/bin/python3


def generate(file_name, target=None, sources=None, compiler_flags=None, ldflags=None, ldadd=None, first_strings=None):
    with open(file_name, "w") as f:
        f.write("# Automatically generated, see generate_am_files.py\n")
        if first_strings is not None:
            f.write(first_strings + "\n")
        if sources is not None:
            f.write(target + "_SOURCES = " + " ".join(sources) + "\n")
        if compiler_flags is not None:
            f.write(target + "_CXXFLAGS = " + " ".join(compiler_flags) + "\n")
        if ldflags is not None:
            f.write(target + "_LDFLAGS = " + " ".join(ldflags) + "\n")
        if ldadd is not None:
            f.write(target + "_LDADD = " + " ".join(ldadd) + "\n")
        
        

"""
# AM_CPPFLAGS=$(XML2_CFLAGS)

lib_LTLIBRARIES = libutils.la
libutils_la_SOURCES = Buffer.cpp Buffer.h Exception.cpp Exception.h \
	Serializable.cpp Serializable.h XmlDocument.cpp Config.cpp Config.h \
	XmlDocument.h ConfigItem.cpp ConfigItem.h libutils.cpp libutils.h locks.cpp locks.h \
	logging.cpp logging.h md.h sha1c.cpp sha_locl.h stdafx.cpp stdafx.h strutils.cpp  \
	strutils.h targetver.h thread.cpp thread.h DebugPrintf.cpp DebugPrintf.h

# AM_CFLAGS = -pthread
AM_CXXFLAGS = -pthread --std=c++0x -pg -Wall

libutils_la_LDFLAGS = -all-static
"""


common_compiler_flags = ["-pthread", "-std=c++0x"]

only_release_compiler_flags = ["-O2"]
only_debug_compiler_flags = ["-g", "-pg", "-Wall"]

release_compiler_flags = common_compiler_flags + only_release_compiler_flags
debug_compiler_flags = common_compiler_flags + only_debug_compiler_flags

debug_mode = False

if debug_mode:
    compiler_flags = debug_compiler_flags
else:
    compiler_flags = release_compiler_flags
    
    

generate(file_name="libs/libutils/Makefile.am", target="libutils_la", 
         sources=["Buffer.cpp", "Buffer.h", "Exception.cpp", "Exception.h",
                  "Serializable.cpp", "Serializable.h", "XmlDocument.cpp", "Config.cpp", "Config.h", 
                  "XmlDocument.h", "ConfigItem.cpp", "ConfigItem.h", "libutils.cpp", "libutils.h", "locks.cpp", "locks.h",
                  "logging.cpp", "logging.h", "md.h", "sha1c.cpp", "sha_locl.h", "stdafx.cpp", "stdafx.h", "strutils.cpp",
                  "strutils.h", "targetver.h", "thread.cpp", "thread.h", "DebugPrintf.cpp", "DebugPrintf.h"],
         compiler_flags=compiler_flags,
         ldflags=["-all-static"],
         first_strings=
"""
lib_LTLIBRARIES = libutils.la
"""
        )
   
"""
lib_LTLIBRARIES = librf.la
librf_la_SOURCES = RFParser.cpp RFProtocol.cpp RFProtocolLivolo.cpp RFProtocolRST.cpp RFProtocolRaex.cpp RFProtocolX10.cpp spidev_lib++.cpp \
		RFProtocolOregon.cpp RFProtocolOregonV3.cpp RFProtocolNooLite.cpp RFProtocolMotionSensor.cpp RFProtocolHS24Bit.cpp RFAnalyzer.cpp RFProtocolVhome.cpp \
		RFParser.h RFProtocol.h RFProtocolLivolo.h RFProtocolRST.h RFProtocolRaex.h RFProtocolX10.h spidev_lib++.h RFM69OOK.cpp RFM69OOK.h RFProtocolRubitek.cpp 

AM_CXXFLAGS = -pthread -std=c++0x -pg -Wall
librf_la_LDFLAGS = -all-static
"""
        
generate(file_name="libs/librf/Makefile.am", target="librf_la", 
         sources=["RFParser.cpp", "RFProtocol.cpp", "spidev_lib++.cpp", "RFAnalyzer.cpp", "RFParser.h", 
                  "RFProtocol.h", "spidev_lib++.h ", "RFM69OOK.cpp", "RFM69OOK.h"] \
                 + ["RFProtocol" + name + ".cpp" for name in ["Livolo", "RST", "Raex", "X10", "Oregon", "OregonV3", 
                                                              "NooLite", "MotionSensor", "HS24Bit", "Vhome", "Rubitek"]],
         compiler_flags=compiler_flags,
         ldflags=["-all-static"],
         first_strings="lib_LTLIBRARIES = librf.la")
     
     
"""
lib_LTLIBRARIES = libwb.la
libwb_la_SOURCES = WBDevice.cpp WBDevice.h

AM_CXXFLAGS = -pthread -std=c++0x -pg -Wall

libwb_la_LDFLAGS = -all-static
"""

generate(file_name="libs/libwb/Makefile.am", target="libwb_la", 
         sources=["WBDevice.cpp", "WBDevice.h"],
         compiler_flags=compiler_flags,
         ldflags=["-all-static"],
         first_strings="lib_LTLIBRARIES = libwb.la")
        
        
"""
bin_PROGRAMS = wb-homa-rfsniffer-test test-core-cpp-wrapper
check_PROGRAMS = wb-homa-rfsniffer-test test-core-cpp-wrapper

test_core_cpp_wrapper_SOURCES = test_core_cpp_wrapper.cpp

wb_homa_rfsniffer_test_SOURCES = LogTest.cpp RfParserTest.cpp mainTest.cpp rfm69Test.cpp snifferTest.cpp mqttTest.cpp backward.cpp
wb_homa_rfsniffer_test_LDADD = $(top_builddir)/libs/libutils/libutils.la \
	$(top_builddir)/libs/librf/librf.la $(top_builddir)/libs/libwb/libwb.la  \
	-lmosquitto  -lmosquittopp -lpthread -ldw

AM_CXXFLAGS = -pthread -std=c++0x -g -pg -DBACKWARD_HAS_DW=1
"""


generate(file_name="tests/Makefile.am", target="wb_homa_rfsniffer_test", 
         sources=["LogTest.cpp", "RfParserTest.cpp", "mainTest.cpp", "rfm69Test.cpp", "snifferTest.cpp", "mqttTest.cpp", "backward.cpp"],
         compiler_flags=compiler_flags,
         ldadd=["$(top_builddir)/libs/" + lib + ".la" for lib in ["libutils/libutils", "librf/librf", "libwb/libwb"]]
               + ["-lmosquitto", "-lmosquittopp", "-lpthread"] + (["-ldw"] if debug_mode else []),
         first_strings=
"""
bin_PROGRAMS = wb-homa-rfsniffer-test test-core-cpp-wrapper
check_PROGRAMS = wb-homa-rfsniffer-test test-core-cpp-wrapper

AM_CXXFLAGS = -std=c++0x

test_core_cpp_wrapper_SOURCES = test_core_cpp_wrapper.cpp
""" 
        )
        
        
"""
AM_CXXFLAGS = -I../include/ -std=c++0x -Wall

bin_PROGRAMS = wb-homa-rfsniffer wb-homa-rfsniffer-dbg

wb_homa_rfsniffer_SOURCES = rfsniffer.cpp MqttConnect.cpp backward.cpp 
wb_homa_rfsniffer_LDADD = $(top_builddir)/libs/libutils/libutils.la \
	$(top_builddir)/libs/librf/librf.la $(top_builddir)/libs/libwb/libwb.la  \
	-lmosquitto -lmosquittopp -lpthread 

"""

        
generate(file_name="rfsniffer/Makefile.am", target="wb_homa_rfsniffer", 
         sources=["rfsniffer.cpp", "MqttConnect.cpp", "backward.cpp"],
         compiler_flags=compiler_flags,
         ldadd=["$(top_builddir)/libs/" + lib + ".la" for lib in ["libutils/libutils", "librf/librf", "libwb/libwb"]]
               + ["-lmosquitto", "-lmosquittopp", "-lpthread"] + (["-ldw"] if debug_mode else []),
               
               
         first_strings=
"""
bin_PROGRAMS = wb-homa-rfsniffer
""" 
        )
    
    
generate(file_name="Makefile.am",
         first_strings=
"""
AM_CXXFLAGS = -DRFSNIFFER -std=c++0x

SUBDIRS = libs/libutils libs/librf libs/libwb tests rfsniffer
dist_doc_DATA = README.md
TESTS = tests/wb-homa-rfsniffer-test tests/test-core-cpp-wrapper
"""
        )
