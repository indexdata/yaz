# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.3 1995-05-22 15:30:13 adam Exp $

LIBDIR=../lib

#LIBMOSI=../../xtimosi/src/libmosi.a $(LIBDIR)/librfc.a

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src
LIBINCLUDE=-L$(LIBDIR)
#CFLAGS=-Wall -pedantic -g
DEFS=$(INCLUDE) -DRPN_QUERY -DPREFIX_QUERY
#LIB=$(LIBDIR)/libserver.a 
LIBS=$(LIBDIR)/libasn.a $(LIBDIR)/libodr.a \
  $(LIBDIR)/libcomstack.a ../lib/ccl.a $(LIBMOSI) $(LIBDIR)/libutil.a
#PO = 
CPP=$(CC) -E
PROG=client
PROGO=client.o

all: $(PROG)

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
	rm -f *.[oa] test core mon.out gmon.out errlist tst cli $(PROG)

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
