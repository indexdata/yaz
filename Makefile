# Copyright (C) 1994, Index Data I/S 
# All rights reserved.
# Sebastian Hammer, Adam Dickmeiss
# $Id: Makefile,v 1.31 1996-11-08 11:03:02 adam Exp $

# Uncomment the lines below to enable mOSI communcation.
#ODEFS=-DUSE_XTIMOSI
#RFC1006=rfc1006
#LIBMOSI=../../xtimosi/src/libmosi.a ../lib/librfc.a
#XMOSI=xmosi.o

CDEFS=$(ODEFS) 
#CC=
SHELL=/bin/sh
MAKE=make
SUBDIR=util odr asn $(RFC1006) ccl comstack retrieval client server makelib
# Add external libraries to the ELIBS macro
ELIBS=
CONTROL=RANLIB="ranlib" ELIBS="$(ELIBS)"

# Installation directories, etc.
#  Binaries
BINDIR=/usr/local/bin
#  Public libraries and header files
LIBDIR=/usr/local/lib
INCDIR=/usr/local/include
#  Misc tables, etc.
YAZDIR=/usr/local/lib/yaz

all:
	for i in $(SUBDIR); do cd $$i; if $(MAKE) $(CONTROL) \
	CFLAGS="$(CFLAGS) $(CDEFS)" LIBMOSI="$(LIBMOSI)" XMOSI="$(XMOSI)";\
	then cd ..; else exit 1; fi; done

dep depend:
	for i in $(SUBDIR); do cd $$i; if $(MAKE) depend; then cd ..; else exit 1; fi; done

clean:
	for i in $(SUBDIR); do (cd $$i; $(MAKE) clean); done
	-rm lib/*.a

oclean:
	for i in $(SUBDIR); do (cd $$i; rm -f *.o); done
	mv lib/libyaz.a .; rm -f lib/*.a; mv libyaz.a lib
	cd client; strip client
	cd server; strip ztest

cleanup:
	rm -f `find $(SUBDIR) -name "*.[oa]" -print`
	rm -f `find $(SUBDIR) -name "core" -print`
	rm -f `find $(SUBDIR) -name "errlist" -print`
	rm -f `find $(SUBDIR) -name "a.out" -print`

distclean: clean cleandepend

cleandepend: 
	for i in $(SUBDIR); do (cd $$i; \
		if sed '/^#Depend/q' <Makefile >Makefile.tmp; then \
		mv -f Makefile.tmp Makefile; fi; rm -f .depend); done

taildepend:
	for i in $(SUBDIR); do (cd $$i; \
		if sed 's/^if/#if/' <Makefile|sed 's/^include/#include/'| \
		sed 's/^endif/#endif/' | \
		sed 's/^depend: depend2/depend: depend1/g' | \
		sed '/^#Depend/q' >Makefile.tmp; then \
		mv -f Makefile.tmp Makefile; fi); done

gnudepend:
	for i in $(SUBDIR); do (cd $$i; \
		if sed '/^#Depend/q' <Makefile| \
		sed 's/^#if/if/' |sed 's/^#include/include/'| \
		sed 's/^#endif/endif/' | \
		sed 's/^depend: depend1/depend: depend2/g' >Makefile.tmp;then \
		mv -f Makefile.tmp Makefile; fi); done

install: all install.misc install.lib install.bin

install.bin:
	@if [ ! -d $(BINDIR) ]; then \
		echo "Making directory $(BINDIR)"; \
		mkdir $(BINDIR); \
	fi
	@echo "Installing client -> $(BINDIR)"; \
	cp client/client $(BINDIR)/client; chmod 755 $(BINDIR)/client
	@echo "Installing ztest -> $(BINDIR)"; \
	cp server/ztest $(BINDIR)/ztest; chmod 755 $(BINDIR)/ztest

install.lib:
	@if [ ! -d $(LIBDIR) ]; then \
		echo "Making directory $(LIBDIR)"; \
		mkdir $(LIBDIR); \
	fi
	@echo "Installing libyaz.a -> $(LIBDIR)"; \
	cp lib/libyaz.a $(LIBDIR)/libyaz.a; \
	chmod 644 $(LIBDIR)/libyaz.a
	@if [ -f lib/librfc.a ]; then \
		echo "Installing librfc.a -> $(LIBDIR)"; \
		cp lib/librfc.a $(LIBDIR)/librfc.a; \
		chmod 644 $(LIBDIR)/librfc.a; \
        fi
	@if [ ! -d $(INCDIR) ]; then \
		echo "Making directory $(INCDIR)"; \
		mkdir $(INCDIR); \
	fi
	@cd include; for f in *.h; do \
		echo "Installing $$f -> $(INCDIR)"; \
		cp $$f $(INCDIR)/$$f; chmod 644 $(INCDIR)/$$f; \
	done

install.misc:
	@if [ ! -d $(YAZDIR) ]; then \
		echo "Making directory $(YAZDIR)"; \
		mkdir $(YAZDIR); \
	fi
	@cd tab; for f in *; do \
		if [ -f $$f ]; then \
			echo "Installing $$f -> $(YAZDIR)"; \
			cp $$f $(YAZDIR)/$$f; chmod 644 $(YAZDIR)/$$f; \
		fi; \
	done

wc:
	wc `find . -name '*.[ch]'`
	
