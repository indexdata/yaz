# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.9 1996-10-29 13:34:56 adam Exp $

#CC=
SHELL=/bin/sh
MAKE=make
LIBDIR=../lib
LIBS=$(LIBDIR)/libodr.a $(LIBDIR)/libasn.a $(LIBDIR)/libcomstack.a \
	$(LIBDIR)/libserver.a $(LIBDIR)/libutil.a $(LIBDIR)/libret.a
LIB=$(LIBDIR)/libyaz.a

all: $(LIB)

$(LIB): $(LIBS)
	rm -f $(LIB)
	for i in $(LIBS); do echo $$i; ar x $$i; rm -f __*; done
	ar qc $(LIB) *.o
	rm -f *.o
	ranlib $(LIB)

clean:
	-rm -f *.o

depend:
