
lib_LIBRARIES = libyaz.a

bin_SCRIPTS = yaz-config

libyaz_a_SOURCES=

LIB=libyaz.a

YAZLIBS=../odr/libodr.a ../@ASNMODULE@/libasn.a ../comstack/libcomstack.a \
 ../server/libserver.a ../zutil/libzutil.a ../util/libutil.a \
 ../ccl/libccl.a ../retrieval/libret.a @ILLLIB@

libyaz.a: $(YAZLIBS)
	rm -f $(LIB)
	for i in $(YAZLIBS); do echo $$i; ar x $$i; rm -f __*; done
	ar qc $(LIB) *.o
	rm -f *.o
	$(RANLIB) $(LIB)

#  Public libraries and header files
LIBDIR=$(exec_prefix)/lib
INCDIR=$(prefix)/include

yaz-config: Makefile
	@echo "#!/bin/sh" >yaz-config
	@echo "# YAZ settings for developers" >>yaz-config
	@echo "YAZBIN='$(bindir)'" >>yaz-config
	@echo "YAZTAB='$(pkgdatadir)/tab'" >>yaz-config
	@echo "YAZLIB='-L$(libdir) -lyaz $(LIBS)'" >>yaz-config
	@echo "YAZINC='-I$(includedir)'" >>yaz-config
	chmod +x yaz-config

