## $Id: Makefile.am,v 1.16 2002-09-25 07:43:29 adam Exp $

AUTOMAKE_OPTIONS = foreign 

SUBDIRS = util odr comstack z39.50 ill zutil ccl tab retrieval server include lib client ztest zoom doc

aclocaldir=$(datadir)/aclocal

aclocal_DATA = yaz.m4

SPEC_FILE=$(PACKAGE).spec
EXTRA_DIST=$(SPEC_FILE) README LICENSE CHANGELOG TODO windows.txt \
	yaz-config.in yaz.m4

dist-hook:
	cd doc; make
	mkdir $(distdir)/win
	-cp $(srcdir)/win/* $(distdir)/win
	mkdir $(distdir)/debian
	cp $(srcdir)/debian/control $(distdir)/debian
	cp $(srcdir)/debian/changelog $(distdir)/debian
	cp $(srcdir)/debian/rules $(distdir)/debian
	cp $(srcdir)/debian/postinst $(distdir)/debian

rpm: $(PACKAGE)-$(VERSION).tar.gz
	cp $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES
	rpm -ba $(SPEC_FILE)
