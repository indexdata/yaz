## $Id: Makefile.am,v 1.4 2000-04-06 12:32:56 adam Exp $

lib_LTLIBRARIES = libyaz.la

bin_SCRIPTS = yaz-config

# If the line below is omitted the LINK variable is
# not set in Makefile.in and it will NOT work ..
EXTRA_libyaz_la_SOURCES=x.c

# No real sources
libyaz_la_SOURCES=

LDFLAGS=-version-info 0:0:0

libyaz_la_LIBADD=../odr/libodr.la ../@ASNMODULE@/libasn.la \
 ../comstack/libcomstack.la ../server/libserver.la \
 ../zutil/libzutil.la ../util/libutil.la ../ccl/libccl.la \
 ../retrieval/libret.la @ILLLIB@

#  Public libraries and header files
LIBDIR=$(exec_prefix)/lib
INCDIR=$(prefix)/include

yaz-config: Makefile
	@echo "#!/bin/sh" >yaz-config
	@echo "# YAZ settings for developers" >>yaz-config
	@echo "YAZBIN='$(bindir)'" >>yaz-config
	@echo "YAZTAB='$(pkgdatadir)/tab'" >>yaz-config
	@echo "YAZLIB='-L$(libdir) -lyaz $(LIBS)'" >>yaz-config
	@echo "YAZINC='-I$(includedir)'" >>yaz-config
	chmod +x yaz-config

