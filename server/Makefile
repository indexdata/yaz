# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.1 1995-03-10 18:22:44 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../asn -I../odr -I../../egate/include -I../../xtimosi/src
#CFLAGS=-Wall -pedantic -g
#CFLAGS=-g
DEFS=$(INCLUDE)
LIB= server.a 
PO = eventl.o 
CPP=cc -E
#CC=checkergcc

all: $(LIB)

alll:

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	ranlib $(LIB)

.c.o:
	$(CC) -c $(DEFS) $(CFLAGS) $<

clean:
	rm -f *.[oa] test core mon.out gmon.out errlist tst cli

depend: depend2
:
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
