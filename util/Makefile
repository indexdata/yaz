# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.5 1995-04-10 10:23:49 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I.
LIBDIR=../lib
LIBINCLUDE=-L$(LIBDIR)
#CFLAGS=-g -Wall -pedantic -ansi
DEFS=$(INCLUDE)
LIB=$(LIBDIR)/libutil.a
LIBS=
PO = dmalloc.o options.o log.o marcdisp.o
CPP=$(CC) -E

all: $(LIBDIR) $(LIB) marcdump

#test: test.o $(LIB) ../odr/odr.a
#	$(CC) $(CFLAGS) $(INCLUDE) -o test test.o $(LIB) ../odr/odr.a

marcdump: marcdump.o $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o marcdump marcdump.o $(LIB)

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
dmalloc.o : dmalloc.c /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h 
log.o : log.c ../include/log.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/stdlib.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h /usr/include/alloca.h /usr/include/ctype.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stdarg.h 
marcdisp.o : marcdisp.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/ctype.h ../include/marcdisp.h 
marcdump.o : marcdump.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h \
  /usr/include/alloca.h ../include/marcdisp.h 
options.o : options.c /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h /usr/include/alloca.h ../include/options.h 
query.o : query.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/ctype.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/odr.h ../include/prt.h \
  ../include/dmalloc.h ../include/proto.h ../include/odr_use.h 
