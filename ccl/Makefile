# Makefile for Email gateway CCL
# Europagate, 1995
#
# $Log: Makefile,v $
# Revision 1.5  1996-05-22 08:35:40  adam
# Removed CFLAGS definiton.
#
# Revision 1.4  1995/06/25 10:53:43  quinn
# Smallish adjustments.
#
# Revision 1.3  1995/05/30  10:25:45  quinn
# Fixed dependency control
#
# Revision 1.2  1995/04/17  09:37:36  quinn
# *** empty log message ***
#
# Revision 1.1  1995/04/10  10:28:16  quinn
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
TPROG1=cclsh
LIB=../lib/ccl.a
PO=cclfind.o ccltoken.o cclerrms.o cclqual.o cclptree.o
CPP=$(CC) -E
DEFS=$(INCLUDE)
RANLIB=ranlib

all: $(LIB)

$(TPROG1): $(TPROG1).o $(LIB)
	$(CC) $(CFLAGS) -o $(TPROG1) $(TPROG1).o $(LIB)

$(LIB): $(PO)
	rm -f $(LIB)
	ar qc $(LIB) $(PO)
	$(RANLIB) $(LIB)

.c.o:
	$(CC) -c $(DEFS) $(CFLAGS) $<

clean:
	rm -f *.log *.[oa] $(TPROG1) $(TPROG2) core mon.out gmon.out errlist *~

depend: depend2

depend1:

depend2:
	$(CPP) $(INCLUDE) -M *.c >.depend	

ifeq (.depend,$(wildcard .depend))
include .depend
endif

#Depend --- DOT NOT DELETE THIS LINE
