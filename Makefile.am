## This file is part of the YAZ toolkit.
## Copyright (C) Index Data
## See the file LICENSE for details.

AUTOMAKE_OPTIONS = foreign 

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include util test client ztest zoom doc etc

aclocaldir=$(datadir)/aclocal

aclocal_DATA = m4/yaz.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = yaz.pc

SPEC_FILE=$(PACKAGE).spec
EXTRA_DIST=$(SPEC_FILE) IDMETA README.md LICENSE NEWS m4/id-config.sh \
	yaz-config.in yaz.pc.in m4/yaz.m4 m4/yaz_libxml2.m4 buildconf.sh \
	Doxyfile.in m4/acx_pthread.m4 m4/ac_check_icu.m4 m4/common.nsi m4/mk_version.tcl

dist-hook:
	if test -x /usr/bin/git -a -d .git; then git log >ChangeLog ; cp ChangeLog $(distdir); fi
	test -d $(distdir)/win || mkdir $(distdir)/win
	-cp $(srcdir)/win/* $(distdir)/win

.PHONY:debian
debian:
	dpkg-buildpackage -rfakeroot


