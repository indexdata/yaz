# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.8 1995-03-15 11:18:17 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../asn -I../odr -I../yazlib -I../../egate/include\
 -I../../xtimosi/src -I../../alex/include
LIBDIR=../../lib
LIBINCLUDE=-L$(LIBDIR)
#CFLAGS=-Wall -pedantic -g
DEFS=$(INCLUDE)
LIB=$(LIBDIR)/libserver.a 
LIBS=$(LIBDIR)/libserver.a $(LIBDIR)/libasn.a $(LIBDIR)/libodr.a $(LIBDIR)/libcomstack.a ../../alex/lib/util.a  ../../egate/lib/util.a
PO = eventl.o 
CPP=cc -E
PROG=statserv
PROGO=statserv.o session.o

all: $(LIBDIR) $(LIB) $(PROG)

$(PROG): $(LIB) $(PROGO) 
	$(CC) $(CFLAGS) $(LIBINCLUDE) -o $(PROG) $(PROGO) $(LIBS)

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
