

noinst_LTLIBRARIES=libserver.la

libserver_la_SOURCES = eventl.c seshigh.c statserv.c requestq.c tcpdchk.c \
 eventl.h service.h session.h

EXTRA_DIST=service.c

AM_CPPFLAGS=-I$(top_srcdir)/include  $(XML2_CFLAGS) -DDOCDIR=\"$(datadir)/doc/$(PACKAGE)\"
