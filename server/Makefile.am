## $Id: Makefile.am,v 1.2 2000-02-28 11:44:39 adam Exp $

noinst_LIBRARIES=libserver.a

libserver_a_SOURCES = eventl.c seshigh.c statserv.c requestq.c tcpdchk.c \
 eventl.h service.h session.h

EXTRA_DIST=service.c

INCLUDES=-I../include

LDADD = ../lib/libyaz.a
