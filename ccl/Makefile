# Makefile for Email gateway CCL
# Europagate, 1995
#
# $Log: Makefile,v $
# Revision 1.1  1995-04-10 10:28:16  quinn
# Added copy of CCL.
#
# Revision 1.8  1995/04/10  10:22:35  quinn
# Added ccl directory.
#
# Revision 1.7  1995/03/27  12:49:03  adam
# Removed CFLAGS def.
#
# Revision 1.6  1995/02/23  08:31:58  adam
# Changed header.
#
# Revision 1.4  1995/02/22  08:50:28  adam
# Definition of CPP changed.
#
# Revision 1.3  1995/02/14  10:25:55  adam
# The constructions 'qualifier rel term ...' implemented.
#
# Revision 1.2  1995/02/13  15:15:06  adam
# Added handling of qualifiers. Not finished yet.
#
# Revision 1.1  1995/02/13  12:35:20  adam
# First version of CCL. Qualifiers aren't handled yet.
#
SHELL=/bin/sh
INCLUDE=-I../include
#CFLAGS=-g -Wall -pedantic -ansi
TPROG1=cclsh
LIB=../lib/ccl.a
PO=cclfind.o ccltoken.o cclerrms.o cclqual.o cclptree.o
CPP=$(CC) -E
DEFS=$(INCLUDE)

all: $(LIB)

$(TPROG1): $(TPROG1).o $(LIB)
	$(CC) $(CFLAGS) -o $(TPROG1) $(TPROG1).o $(LIB)

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	ranlib $(LIB)

.c.o:
	$(CC) -c $(DEFS) $(CFLAGS) $<

clean:
	rm -f *.log *.[oa] $(TPROG1) $(TPROG2) core mon.out gmon.out errlist *~

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
cclerrms.o : cclerrms.c 
cclfind.o : cclfind.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h \
  /usr/include/alloca.h /usr/include/assert.h /usr/include/string.h ../include/ccl.h 
cclptree.o : cclptree.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/assert.h /usr/include/string.h \
  /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h ../include/ccl.h 
cclqual.o : cclqual.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h \
  /usr/include/alloca.h /usr/include/assert.h /usr/include/string.h ../include/ccl.h 
cclsh.o : cclsh.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h \
  /usr/include/alloca.h /usr/include/assert.h ../include/ccl.h 
ccltoken.o : ccltoken.c /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/string.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/stddef.h \
  /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h /usr/lib/gcc-lib/i486-linux/2.5.8/include/float.h \
  /usr/include/alloca.h /usr/include/assert.h ../include/ccl.h 
