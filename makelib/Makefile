# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.1 1995-03-16 13:28:25 quinn Exp $

#CC=
SHELL=/bin/sh
MAKE=make
LIBDIR=../../lib
LIBS=$(LIBDIR)/libodr.a $(LIBDIR)/libasn.a $(LIBDIR)/libcomstack.a \
	$(LIBDIR)/libserver.a
LIB=$(LIBDIR)/libyaz.a

all: $(LIB)

$(LIB): $(LIBS)
	rm -f $(LIB)
	for i in $(LIBS); do ar x $$i; done
	-rm -f __*
	ar qc $(LIB) *.o
	ranlib $(LIB)
	rm -f *.o
