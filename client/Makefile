# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.11 1996-10-07 15:29:01 quinn Exp $

LIBDIR=../lib

#LIBMOSI=../../xtimosi/src/libmosi.a $(LIBDIR)/librfc.a

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src
LIBINCLUDE=-L$(LIBDIR)
DEFS=$(INCLUDE) -DCCL2RPN=1
LIBS=$(LIBDIR)/libasn.a $(LIBDIR)/libodr.a \
  $(LIBDIR)/libcomstack.a ../lib/ccl.a $(LIBMOSI) $(LIBDIR)/libutil.a $(ELIBS)
CPP=$(CC) -E
PROG=client
PROGO=client.o

all: $(PROG)

$(PROG): $(LIB) $(PROGO)
	$(CC) $(CFLAGS) $(LIBINCLUDE) -o $(PROG) $(PROGO) $(LIBS)

qparser: qparser.o
	$(CC) $(CFLAGS) $(LIBINCLUDE) -o qparser qparser.o $(LIBS)

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
	$(CPP) $(INCLUDE) -M *.c >.depend	

ifeq (.depend,$(wildcard .depend))
include .depend
endif

#Depend --- DOT NOT DELETE THIS LINE
