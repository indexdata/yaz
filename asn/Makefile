# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.9 1995-04-10 10:22:21 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I.
LIBDIR=../lib
LIBINCLUDE=-L$(LIBDIR)
#CFLAGS=-g -Wall -pedantic -ansi
DEFS=$(INCLUDE)
LIB=$(LIBDIR)/libasn.a
LIBS=-lodr -lasn
PO = proto.o diagbib1.o oid.o
CPP=$(CC) -E

all: $(LIB)

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

depend: depend1

depend1:
	mv Makefile Makefile.tmp
	sed '/^#Depend/q' <Makefile.tmp >Makefile
	$(CPP) $(INCLUDE) -M *.c >>Makefile
	-rm Makefile.tmp

depend2:
	$(CPP) $(INCLUDE) -M *.c >.depend	

#ifeq (.depend,$(wildcard .depend))
#include .depend
#endif

#Depend --- DOT NOT DELETE THIS LINE
diagbib1.o : diagbib1.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h ../include/diagbib1.h 
oid.o : oid.c ../include/oid.h ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
pnew.o : pnew.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h \
  ../include/proto.h ../include/odr_use.h 
proto.o : proto.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h \
  ../include/proto.h ../include/odr_use.h 
test.o : test.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h ../include/odr.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h \
  ../include/odr_use.h ../include/proto.h 
