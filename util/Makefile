# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.12 1995-05-30 10:25:47 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I.
LIBDIR=../lib
LIBINCLUDE=-L$(LIBDIR)
#CFLAGS=-g -Wall -pedantic -ansi
DEFS=$(INCLUDE)
LIB=$(LIBDIR)/libutil.a
LIBS=
PO = options.o log.o marcdisp.o yaz-ccl.o pquery.o oid.o # dmalloc.o
CPP=$(CC) -E

all: $(LIBDIR) $(LIB) marcdump

#test: test.o $(LIB) ../odr/odr.a
#	$(CC) $(CFLAGS) $(INCLUDE) -o test test.o $(LIB) ../odr/odr.a

marcdump: marcdump.o $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o marcdump marcdump.o $(LIB)

alll:

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	ranlib $(LIB)

$(LIBDIR):
	mkdir $(LIBDIR)

.c.o:
	$(CC) -c $(DEFS) $(CFLAGS) $<

clean:
	rm -f *.[oa] test marcdump core mon.out gmon.out errlist

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
