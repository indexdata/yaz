## $Id: Makefile.am,v 1.8 2003-02-12 15:06:43 adam Exp $

noinst_LTLIBRARIES=libserver.la

libserver_la_SOURCES = eventl.c seshigh.c statserv.c requestq.c tcpdchk.c \
 eventl.h service.h session.h

EXTRA_DIST=service.c

AM_CPPFLAGS=-I$(top_srcdir)/include  $(XSLT_CFLAGS)
