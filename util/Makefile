# Copyright (C) 1994-1997, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.22 1997-09-01 08:54:12 adam Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I.
LIBDIR=../lib
LIBINCLUDE=-L$(LIBDIR)
DEFS=$(INCLUDE)
#DEFS=$(INCLUDE) -DTRACE_XMALLOC=1
LIB=$(LIBDIR)/libutil.a
LIBS=
PO = options.o log.o marcdisp.o yaz-ccl.o pquery.o oid.o wrbuf.o \
	xmalloc.o readconf.o tpath.o nmem.o yaz-util.o # dmalloc.o 
CPP=$(CC) -E
RANLIB=ranlib

all: $(LIBDIR) $(LIB) marcdump

#test: test.o $(LIB) ../odr/odr.a
#	$(CC) $(CFLAGS) $(INCLUDE) -o test test.o $(LIB) ../odr/odr.a

marcdump: marcdump.o $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o marcdump marcdump.o $(LIB)

alll:

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	$(RANLIB) $(LIB)

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
