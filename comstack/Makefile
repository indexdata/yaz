# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.2 1995-06-25 10:53:44 quinn Exp $


LIBDIR=../lib

#LIBOSI=../../xtimosi/src/libmosi.a $(LIBDIR)/librfc.a
#XMOSI=xmosi.o

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src
LIBINCLUDE=-L$(LIBDIR) -L../../xtimosi/src
#CFLAGS=-Wall -pedantic -g
#CFLAGS=-g
DEFS=$(INCLUDE) -DRPN_QUERY # -DTRACE_TCPIP
LIB= $(LIBDIR)/libcomstack.a 
LIBS=$(LIBDIR)/libasn.a $(LIBDIR)/libodr.a $(LIBDIR)/libcomstack.a \
	 ../lib/ccl.a $(LIBDIR)/libutil.a $(LIBMOSI)
PO = comstack.o tcpip.o $(XMOSI)
CPP=$(CC) -E
#CC=checkergcc
RANLIB=ranlib

all: $(LIBDIR) $(LIB)

cli: tst.o $(LIB)
	$(CC) $(CFLAGS) $(LIBINCLUDE) -o cli tst.o $(LIBINCLUDE) $(LIBS)

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
	rm -f *.[oa] test core mon.out gmon.out errlist tst cli

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
