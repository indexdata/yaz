# Copyright (C) 1995-1998, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.15 1998-02-11 11:53:33 adam Exp $

LIBDIR=../lib

#LIBMOSI=../../xtimosi/src/libmosi.a $(LIBDIR)/librfc.a

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src
LIBINCLUDE=-L$(LIBDIR)
DEFS=$(INCLUDE) $(CDEFS) -DCCL2RPN=1
LIBS=$(LIBDIR)/libasn.a $(LIBDIR)/libutil.a \
  $(LIBDIR)/libcomstack.a ../lib/ccl.a $(LIBMOSI) $(LIBDIR)/libodr.a 
CPP=$(CC) -E
PROG=client
PROGO=client.o

all: $(PROG)

$(PROG): $(LIB) $(PROGO) $(LIBS)
	$(CC) $(CFLAGS) $(LIBINCLUDE) -o $(PROG) $(PROGO) $(LIBS) $(ELIBS)

alll:

$(LIBDIR):
	mkdir $(LIBDIR)

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
