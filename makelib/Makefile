# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.6 1995-04-10 10:22:57 quinn Exp $

#CC=
SHELL=/bin/sh
MAKE=make
LIBDIR=../lib
LIBS=$(LIBDIR)/libodr.a $(LIBDIR)/libasn.a $(LIBDIR)/libcomstack.a \
	$(LIBDIR)/libserver.a $(LIBDIR)/libutil.a
LIB=$(LIBDIR)/libyaz.a

all: $(LIB)

$(LIB): $(LIBS)
	rm -f $(LIB)
	for i in $(LIBS); do echo $$i; ar x $$i; done
	-rm -f __*
	ar qc $(LIB) *.o
	ranlib $(LIB)
	rm -f *.o

clean:
	-rm -f *.o

depend:
