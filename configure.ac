AC_INIT([rfsniffer], [0.2], [yuri.pechatnov@wirenboard.ru])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_LANG([C++])
AM_PROG_AR

AC_PROG_CXX([g++])

#AM_PROG_LIBTOOL
LT_INIT
AC_CONFIG_MACRO_DIR([m4])

AC_CHECK_HEADERS([json/json.h])
AC_CHECK_HEADERS([jsoncpp/json/json.h])
AM_CONFIG_HEADER(haconfig.inc)

AC_DEFINE([RFSNIFFER],[1],[Wiren board rfm sniffer])
AC_DEFINE([USE_CONFIG], [1], [Use JSON Config])

AC_SEARCH_LIBS([_ZNK4Json5Value4sizeEv], [jsoncpp], [], [])

AM_SILENT_RULES([no])

AC_CONFIG_FILES([
 Makefile
 libs/libutils/Makefile
 libs/librf/Makefile
 libs/libwb/Makefile
 tests/Makefile
 rfsniffer/Makefile
])
AC_OUTPUT
