# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.15 1995-04-10 10:23:10 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I.
LIBDIR=../lib
LIBINCLUDE=-L$(LIBDIR)
CFLAGS= -g -Wall -pedantic -ansi # -DODR_DEBUG
DEFS=$(INCLUDE)
LIB=$(LIBDIR)/libodr.a
LIBS=-lodr
PO = odr_bool.o ber_bool.o ber_len.o ber_tag.o odr_util.o odr_null.o \
	ber_null.o odr_int.o ber_int.o odr_tag.o odr_cons.o odr_seq.o\
	odr_oct.o ber_oct.o odr_bit.o ber_bit.o odr_oid.o ber_oid.o odr_use.o \
	odr_choice.o odr_any.o ber_any.o odr.o odr_mem.o
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
ber_any.o : ber_any.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
ber_bit.o : ber_bit.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
ber_bool.o : ber_bool.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h ../include/odr.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
ber_int.o : ber_int.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h \
  /usr/include/sys/types.h /usr/include/linux/types.h /usr/include/netinet/in.h \
  /usr/include/sys/socket.h /usr/include/linux/socket.h /usr/include/linux/sockios.h \
  /usr/include/linux/in.h 
ber_len.o : ber_len.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h ../include/odr.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
ber_null.o : ber_null.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
ber_oct.o : ber_oct.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
ber_oid.o : ber_oid.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
ber_tag.o : ber_tag.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h ../include/odr.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr.o : odr.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h \
  /usr/include/alloca.h ../include/odr.h /usr/include/string.h ../include/prt.h \
  ../include/dmalloc.h 
odr_any.o : odr_any.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_bit.o : odr_bit.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_bool.o : odr_bool.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h ../include/odr.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_choice.o : odr_choice.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_cons.o : odr_cons.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_int.o : odr_int.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_mem.o : odr_mem.c /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h /usr/include/alloca.h ../include/odr.h \
  /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  ../include/prt.h ../include/dmalloc.h 
odr_null.o : odr_null.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_oct.o : odr_oct.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_oid.o : odr_oid.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_priv.o : odr_priv.c 
odr_seq.o : odr_seq.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h \
  /usr/include/assert.h 
odr_tag.o : odr_tag.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h 
odr_use.o : odr_use.c ../include/odr.h /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h \
  ../include/odr_use.h 
odr_util.o : odr_util.c /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h /usr/include/alloca.h ../include/odr.h \
  /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  ../include/prt.h ../include/dmalloc.h 
test.o : test.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h ../include/odr.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/prt.h ../include/dmalloc.h \
  ../include/odr_use.h 
