## $Id: Makefile.am,v 1.7 2002-09-11 21:25:57 adam Exp $

noinst_LTLIBRARIES=libasn.la

libasn_la_SOURCES = proto.c prt-rsc.c prt-acc.c prt-exp.c prt-grs.c prt-exd.c \
	prt-dia.c prt-esp.c prt-arc.c prt-add.c prt-dat.c prt-univ.c

pkginclude_HEADERS = \
	prt-acc.h prt-add.h prt-arc.h prt-dat.h prt-dia.h prt-esp.h prt-exd.h \
	prt-exp.h prt-ext.h prt-grs.h prt-proto.h prt-rsc.h prt-univ.h

AM_CPPFLAGS=-I$(top_srcdir)/include
