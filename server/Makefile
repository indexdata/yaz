# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.17 1995-04-10 10:23:34 quinn Exp $

LIBDIR=../lib

# Uncomment this when you want to use XTIMOSI
#LIBMOSI=../../xtimosi/src/libmosi.a $(LIBDIR)/librfc.a

SHELL=/bin/sh
INCLUDE=-I../include -I. -I../../xtimosi/src -I../../alex/include
LIBINCLUDE=-L$(LIBDIR)
#CFLAGS=-Wall -pedantic -g
DEFS=$(INCLUDE)
LIB=$(LIBDIR)/libserver.a 
LIBS=$(LIBDIR)/libserver.a $(LIBDIR)/libasn.a $(LIBDIR)/libodr.a \
$(LIBDIR)/libcomstack.a $(LIBMOSI) $(LIBDIR)/libutil.a
PO = eventl.o seshigh.o statserv.o
CPP=$(CC) -E
PROG=ztest
PROGO=ztest.o

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
	rm -f *.[oa] test core mon.out gmon.out errlist tst cli $(PROG)

depend: depend1
:
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
eventl.o : eventl.c /usr/include/assert.h /usr/include/sys/time.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/linux/types.h /usr/include/linux/time.h \
  /usr/include/time.h /usr/include/sys/types.h /usr/include/unistd.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h \
  /usr/include/alloca.h /usr/include/string.h ./eventl.h ../include/dmalloc.h 
pwrite.o : pwrite.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/errno.h /usr/include/linux/errno.h 
seshigh.o : seshigh.c /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/unistd.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h /usr/include/sys/types.h /usr/include/linux/types.h \
  /usr/include/assert.h ../include/comstack.h ../include/dmalloc.h ./eventl.h \
  ./session.h ../include/odr.h /usr/include/string.h ../include/prt.h ../include/oid.h \
  ../include/proto.h ../include/odr_use.h ../include/log.h ../include/backend.h 
session.o : session.c /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h /usr/include/alloca.h /usr/include/assert.h \
  ../include/comstack.h ../include/dmalloc.h ./eventl.h ./session.h ../include/odr.h \
  /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h \
  ../include/prt.h ../include/oid.h ../include/proto.h ../include/odr_use.h 
statserv.o : statserv.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/unistd.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/sys/types.h /usr/include/linux/types.h /usr/include/fcntl.h /usr/include/linux/fcntl.h \
  /usr/include/sys/wait.h /usr/include/waitflags.h /usr/include/linux/wait.h /usr/include/waitstatus.h \
  /usr/include/endian.h /usr/include/bytesex.h /usr/include/signal.h /usr/include/linux/signal.h \
  /usr/include/errno.h /usr/include/linux/errno.h ../include/options.h ./eventl.h \
  ./session.h ../include/comstack.h ../include/dmalloc.h ../include/odr.h /usr/include/string.h \
  ../include/prt.h ../include/oid.h ../include/tcpip.h /usr/include/sys/socket.h \
  /usr/include/linux/socket.h /usr/include/linux/sockios.h /usr/include/netdb.h \
  /usr/include/paths.h /usr/include/netinet/in.h /usr/include/linux/in.h /usr/include/arpa/inet.h \
  ../include/log.h 
ztest.o : ztest.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h \
  /usr/include/alloca.h /usr/include/ctype.h ../include/statserv.h ../include/backend.h \
  ../include/proto.h ../include/odr.h /usr/include/string.h ../include/prt.h ../include/dmalloc.h \
  ../include/odr_use.h 
