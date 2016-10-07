AM_CPPFLAGS=$(XML2_CFLAGS)

lib_LTLIBRARIES = libutils.la
libutils_la_SOURCES = Buffer.cpp Buffer.h Exception.cpp Exception.h \
	Serializable.cpp Serializable.h XmlDocument.cpp Config.cpp Config.h \
	XmlDocument.h ConfigItem.cpp ConfigItem.h libutils.cpp libutils.h locks.cpp locks.h \
	logging.cpp logging.h md.h sha1c.cpp sha_locl.h stdafx.cpp stdafx.h strutils.cpp \
	strutils.h targetver.h thread.cpp thread.h 

AM_CFLAGS = -pthread
AM_CXXFLAGS = -pthread

libutils_la_LDFLAGS = -all-static
