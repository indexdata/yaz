# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.1 1995-02-09 15:52:34 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../odr
CFLAGS=-g -Wall -pedantic -ansi
DEFS=$(INCLUDE)
LIB=asn.a
PO = proto.o
CPP=cc -E

all: $(LIB) test

test: test.o $(LIB) ../odr/odr.a
	$(CC) $(CFLAGS) $(INCLUDE) -o test test.o $(LIB) ../odr/odr.a

alll:

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	ranlib $(LIB)

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
