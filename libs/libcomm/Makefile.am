METASOURCES = AUTO
lib_LTLIBRARIES = libcomm.la
libcomm_la_SOURCES = Connection.cpp Connection.h IPConnection.cpp \
	IPConnection.h IPListener.cpp IPListener.h IPSupervisor.cpp IPSupervisor.h \
	SerialConnection.cpp SerialConnection.h SupervisorClient.cpp SupervisorClient.h libcomm.cpp \
	libcomm.h stdafx.cpp stdafx.h targetver.h WebClient.h WebClient.cpp
AM_CFLAGS = -pthread
AM_CXXFLAGS = -pthread -g
libcomm_la_LDFLAGS = -all-static
