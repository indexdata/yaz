## This file is part of the YAZ toolkit.
## Copyright (C) 1994-2005, Index Data, All rights reserved.
## $Id: Makefile.am,v 1.33 2006-04-21 12:54:36 marc Exp $

AUTOMAKE_OPTIONS = foreign 

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include util test client ztest zoom doc etc

aclocaldir=$(datadir)/aclocal

aclocal_DATA = m4/yaz.m4

SPEC_FILE=$(PACKAGE).spec
EXTRA_DIST=$(SPEC_FILE) README LICENSE NEWS TODO windows.txt \
	yaz-config.in m4/yaz.m4 buildconf.sh Doxyfile.in

dist-hook:
	mkdir $(distdir)/win
	-cp $(srcdir)/win/* $(distdir)/win
	mkdir $(distdir)/debian
	cp $(srcdir)/debian/control $(distdir)/debian
	cp $(srcdir)/debian/changelog $(distdir)/debian
	cp $(srcdir)/debian/rules $(distdir)/debian
	cp $(srcdir)/debian/*.postinst $(distdir)/debian
	cp $(srcdir)/debian/*.install $(distdir)/debian

