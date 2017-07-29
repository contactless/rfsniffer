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


debug_mode = False

common_compiler_flags = ["-pthread", "-std=c++0x"]

common_linker_flags = ["-lmosquittopp", "-lpthread", "-llog4cpp"] + (["-ldw"] if debug_mode else [])

only_release_compiler_flags = ["-O2"]
# notice: -pg flag conflicts with valgrind!
only_debug_compiler_flags = ["-g", "-O0", "-Wall"]
#only_debug_compiler_flags = ["-g", "-pg", "-Wall"]

release_compiler_flags = common_compiler_flags + only_release_compiler_flags
debug_compiler_flags = common_compiler_flags + only_debug_compiler_flags

if debug_mode:
    compiler_flags = debug_compiler_flags
else:
    compiler_flags = release_compiler_flags



generate(file_name="libs/libutils/Makefile.am", target="libutils_la",
         sources=["Exception.cpp", "Exception.h", "Config.h",
                  "logging.cpp", "logging.h", "strutils.cpp",
                  "strutils.h", "DebugPrintf.cpp", "DebugPrintf.h"],
         compiler_flags=compiler_flags,
         ldflags=["-all-static"],
         first_strings=
"""
lib_LTLIBRARIES = libutils.la
"""
        )


generate(file_name="libs/librf/Makefile.am", target="librf_la",
         sources=["RFParser.cpp", "RFProtocol.cpp", "spidev_lib++.cpp", "RFParser.h",
                  "RFProtocol.h", "spidev_lib++.h ", "RFM69OOK.cpp", "RFM69OOK.h"] \
                 + ["RFProtocol" + name + ".cpp" for name in ["Livolo", "RST", "Raex", "X10", "Oregon", "OregonV3",
                                                              "NooLite", "MotionSensor", "HS24Bit", "Vhome", "EV1527", "Rubitek"]],
         compiler_flags=compiler_flags,
         ldflags=["-all-static"],
         first_strings="lib_LTLIBRARIES = librf.la")


generate(file_name="libs/libwb/Makefile.am", target="libwb_la",
         sources=["WBDevice.cpp", "WBDevice.h"],
         compiler_flags=compiler_flags,
         ldflags=["-all-static"],
         first_strings="lib_LTLIBRARIES = libwb.la")


generate(file_name="tests/Makefile.am", target="wb_homa_rfsniffer_test",
         sources=["RfParserTest.cpp", "mainTest.cpp", "rfm69Test.cpp", "snifferTest.cpp", "mqttTest.cpp", "backward.cpp"],
         compiler_flags=compiler_flags,
         ldadd=["$(top_builddir)/libs/" + lib + ".la" for lib in ["libutils/libutils", "librf/librf", "libwb/libwb"]]
               + common_linker_flags,
         first_strings=
"""
bin_PROGRAMS = wb-homa-rfsniffer-test test-core-cpp-wrapper
check_PROGRAMS = wb-homa-rfsniffer-test test-core-cpp-wrapper

AM_CXXFLAGS = -std=c++0x

test_core_cpp_wrapper_SOURCES = test_core_cpp_wrapper.cpp
"""
        )


generate(file_name="rfsniffer/Makefile.am", target="wb_homa_rfsniffer",
         sources=["rfsniffer.cpp", "MqttConnect.cpp", "backward.cpp"],
         compiler_flags=compiler_flags,
         ldadd=["$(top_builddir)/libs/" + lib + ".la" for lib in ["libutils/libutils", "librf/librf", "libwb/libwb"]]
               + common_linker_flags,


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
