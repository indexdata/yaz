# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.10 1998-02-11 11:53:35 adam Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src
LIBDIR=../lib
LIBINCLUDE=-L$(LIBDIR)
DEFS=$(INCLUDE) # -DTRACE_TRANSPORT
LIB=$(LIBDIR)/librfc.a
LIBS=-lodr
PO = rfct.o makensap.o
CPP=$(CC) -E
RANLIB=ranlib

all: $(LIBDIR) $(INCDIR) $(LIB)

test: test.o $(LIB)
	$(CC) $(CFLAGS) $(LIBINCLUDE) -o test test.o $(LIBS)

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
	rm -f *.[oa] test core mon.out gmon.out errlist

depend: depend2

depend1:
	sed '/^#Depend/q' <Makefile >Makefile.tmp
	$(CPP) $(DEFS) -M *.c >>Makefile.tmp
	mv -f Makefile.tmp Makefile

depend2:
	$(CPP) $(INCLUDE) -M *.c >.depend	

#ifeq (.depend,$(wildcard .depend))
#include .depend
#endif

#Depend --- DOT NOT DELETE THIS LINE
