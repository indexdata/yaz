# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.4 1995-05-29 08:11:54 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src
LIBDIR=../lib
LIBINCLUDE=-L$(LIBDIR)
CFLAGS= -g -Wall -pedantic -ansi
DEFS=$(INCLUDE) #-DTRACE_TRANSPORT
LIB=$(LIBDIR)/librfc.a
LIBS=-lodr
PO = rfct.o makensap.o
CPP=$(CC) -E

all: $(LIBDIR) $(INCDIR) $(LIB)

test: test.o $(LIB)
	$(CC) $(CFLAGS) $(LIBINCLUDE) -o test test.o $(LIBS)

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
	rm -f *.[oa] test core mon.out gmon.out errlist

depend: depend2

depend1:
	mv Makefile Makefile.tmp
	sed '/^#Depend/q' <Makefile.tmp >Makefile
	$(CPP) $(INCLUDE) -M *.c >>Makefile
	-rm Makefile.tmp

depend2:
	$(CPP) $(INCLUDE) -M *.c >.depend	

ifeq (.depend,$(wildcard .depend))
include .depend
endif

#Depend --- DOT NOT DELETE THIS LINE
