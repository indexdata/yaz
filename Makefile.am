## This file is part of the YAZ toolkit.
## Copyright (C) 1995-2009 Index Data

AUTOMAKE_OPTIONS = foreign 

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include util test client ztest zoom doc etc

aclocaldir=$(datadir)/aclocal

aclocal_DATA = m4/yaz.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = yaz.pc

SPEC_FILE=$(PACKAGE).spec
EXTRA_DIST=$(SPEC_FILE) README LICENSE NEWS \
	yaz-config.in yaz.pc.in m4/yaz.m4 m4/yaz_libxml2.m4 buildconf.sh \
	Doxyfile.in m4/acx_pthread.m4 m4/ac_check_icu.m4

dist-hook:
	if test -x /usr/bin/git -a -d .git; then git log >ChangeLog ; cp ChangeLog $(distdir); fi
	test -d $(distdir)/win || mkdir $(distdir)/win
	-cp $(srcdir)/win/* $(distdir)/win

.PHONY:debian
debian:
	dpkg-buildpackage -rfakeroot


