# Copyright (C) 1994-1997, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.32 1997-09-01 08:52:58 adam Exp $

LIBDIR=../lib

#LIBMOSI=../../xtimosi/src/libmosi.a $(LIBDIR)/librfc.a

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src
LIBINCLUDE=-L$(LIBDIR)
DEFS=$(INCLUDE)
LIB=$(LIBDIR)/libserver.a 
LIBS=$(LIBDIR)/libserver.a $(LIBDIR)/libasn.a $(LIBDIR)/libodr.a \
$(LIBDIR)/libcomstack.a $(LIBMOSI) $(LIBDIR)/libutil.a $(ELIBS)
PO = eventl.o seshigh.o statserv.o requestq.o
CPP=$(CC) -E
RANLIB=ranlib

all: $(LIBDIR) $(LIB) 

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	$(RANLIB) $(LIB)

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
