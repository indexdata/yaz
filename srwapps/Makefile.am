## $Id: Makefile.am,v 1.1 2003-01-06 08:20:28 adam Exp $
## Copyright (C) 2002-2003, Index Data

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/srw $(CFLAGSTHREADS)

if SRW
bin_PROGRAMS = srw-gateway srw-client
endif

if ISTHR
extra=../lib/libyazthread.la
endif

LDADD = $(extra) ../lib/libyaz.la $(GSOAP_LIB) $(XSLT_LIB) $(LIBTHREAD)

srw_gateway_SOURCES = srw-gateway.c 
srw_client_SOURCES = srw-client.c

