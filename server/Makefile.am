
noinst_LIBRARIES=libserver.a

libserver_a_SOURCES = eventl.c seshigh.c statserv.c requestq.c tcpdchk.c \
 eventl.h service.h session.h

EXTRA_DIST=service.c

INCLUDES=-I../include

LDADD = ../lib/libyaz.a
