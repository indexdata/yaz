## $Id: Makefile.am,v 1.4 2000-02-29 15:52:33 adam Exp $

AUTOMAKE_OPTIONS = foreign 

DIST_SUBDIRS=util odr z39.50 asn ill zutil comstack ccl tab retrieval server lib include client ztest doc

SUBDIRS = @SUBDIRS_VAR@

noinst_SCRIPTS = yaz-config
SPEC_FILE=$(PACKAGE).spec
EXTRA_DIST=$(SPEC_FILE) README LICENSE CHANGELOG TODO windows.txt

yaz-config:
	@echo "#!/bin/sh" >yaz-config
	@echo "# YAZ settings for developers" >>yaz-config
	@echo "YAZBIN=\"`pwd`/bin\"" >>yaz-config
	@echo "YAZTAB=\"`pwd`/tab\"" >>yaz-config
	@echo "YAZLIB=\"-L`pwd`/lib -lyaz @LIBS@\"" >>yaz-config
	@echo "YAZINC=\"-I`pwd`/include\"" >>yaz-config
	chmod +x yaz-config

dist-hook:
	mkdir $(distdir)/win
	-cp $(srcdir)/win/* $(distdir)/win

rpm: $(PACKAGE)-$(VERSION).tar.gz
	cp $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES
	rpm -ba $(SPEC_FILE)
