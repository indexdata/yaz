# Copyright (C) 1995-1997, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.2 1997-09-17 12:10:43 adam Exp $

LIBDIR=../lib

#LIBMOSI=../../xtimosi/src/libmosi.a $(LIBDIR)/librfc.a

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src
LIBINCLUDE=-L$(LIBDIR)
DEFS=$(INCLUDE) $(CDEFS)
LIB=$(LIBDIR)/libserver.a 
LIBS=$(LIBDIR)/libserver.a $(LIBDIR)/libasn.a $(LIBDIR)/libodr.a \
$(LIBDIR)/libcomstack.a $(LIBMOSI) $(LIBDIR)/libutil.a $(ELIBS)
CPP=$(CC) -E
PROG=ztest
PROGO=ztest.o read-grs.o
RANLIB=ranlib

all: $(PROG)

$(PROG): $(LIB) $(PROGO) 
	$(CC) $(CFLAGS) $(LIBINCLUDE) -o $(PROG) $(PROGO) $(LIBS)

alll:

.c.o:
	$(CC) -c $(DEFS) $(CFLAGS) $<

clean:
	rm -f *.[oa] test core mon.out gmon.out errlist tst cli $(PROG)

depend: depend2

depend1:
	sed '/^#Depend/q' <Makefile >Makefile.tmp
	$(CPP) $(DEFS) -M *.c >>Makefile.tmp
	mv -f Makefile.tmp Makefile

depend2:
	$(CPP) $(DEFS) -M *.c >.depend	

ifeq (.depend,$(wildcard .depend))
include .depend
endif

#Depend --- DOT NOT DELETE THIS LINE
