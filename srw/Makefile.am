# $Id: Makefile.am,v 1.2 2003-01-15 14:26:55 adam Exp $
AM_CPPFLAGS = -I$(top_srcdir)/include $(GSOAP_INCLUDE) $(XSLT_CFLAGS)

EXTRA_DIST = zing.h

if SRW
noinst_LTLIBRARIES = libsrw.la

libsrw_la_SOURCES = srw_C.c srw-namespace.c srw-xcql.c srw-xslt.c srw_Client.c srw-diag.c srw-server.c srw_Server.c

include_HEADERS = srw_H.h srw_Stub.h 

LDADD = $(GSOAP_LIB)

# Dependency for stdsoap2.h needed!
stdsoap2.h srw_C.c srw_Client.c srw_Server.c srw_H.h srw_Stub.h: zing.h
	$(GSOAP_PREFIX)/bin/soapcpp2 -p srw_ -c zing.h

srw-namespace.o: srw_H.h

srw_C.o: srw_C.c
	$(COMPILE) -c soapC.c 
srw_Client.o: soapClient.c
	$(COMPILE) -c srw_Client.c 
srw_Server.o: srw_Server.c
	$(COMPILE) -c srw_Server.c 
endif
