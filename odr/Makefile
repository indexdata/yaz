# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.5 1995-03-08 12:11:59 quinn Exp $

SHELL=/bin/sh
INCLUDE=-I../include -I.
#CFLAGS= -g -Wall -pedantic -ansi -DODR_DEBUG
DEFS=$(INCLUDE)
LIB=odr.a
PO = odr_bool.o ber_bool.o ber_len.o ber_tag.o odr_util.o odr_null.o \
	ber_null.o odr_int.o ber_int.o odr_tag.o odr_cons.o odr_seq.o\
	odr_oct.o ber_oct.o odr_bit.o ber_bit.o odr_oid.o ber_oid.o odr_use.o \
	odr_choice.o odr_any.o ber_any.o odr.o
CPP=cc -E

all: $(LIB)

test: test.o $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o test test.o $(LIB)

alll:

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	ranlib $(LIB)

.c.o:
	$(CC) -c $(DEFS) $(CFLAGS) $<

clean:
	rm -f *.[oa] test core mon.out gmon.out errlist

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
