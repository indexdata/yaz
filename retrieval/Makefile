# $Id: Makefile,v 1.5 1996-02-20 16:33:02 quinn Exp $

SHELL=/bin/sh
RANLIB=ranlib
INCLUDE=-I../include -I.
CFLAGS=-g -Wall -pedantic -ansi
DEFS=$(INCLUDE)
LIB=../lib/libret.a
PO = d1_read.o d1_attset.o d1_tagset.o d1_absyn.o d1_grs.o \
	d1_sutrs.o d1_varset.o d1_espec.o \
	d1_doespec.o d1_map.o d1_marc.o d1_write.o d1_expout.o
CPP=$(CC) -E

all: $(LIB)

tst: tst.c $(LIB)
	$(CC) -g -o tst $(INCLUDE) tst.c \
	../lib/data1.a $(YAZ)/lib/libyaz.a

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	$(RANLIB) $(LIB)

.c.o:
	$(CC) -c $(DEFS) $(CFLAGS) $<

clean:
	rm -f *.[oa] $(TPROG) core mon.out gmon.out errlist test isam-test issh

depend: depend2

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
