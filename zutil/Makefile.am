## $Id: Makefile.am,v 1.16 2003-02-17 22:35:48 adam Exp $
noinst_LTLIBRARIES = libzutil.la

AM_CPPFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/srw $(XML2_CFLAGS)

libzutil_la_SOURCES = zget.c yaz-ccl.c diagbib1.c logrpn.c \
  otherinfo.c pquery.c sortspec.c z3950oid.c charneg.c \
  zoom-c.c zoom-opt.c zoom-p.h grs1disp.c zgdu.c soap.c srw.c

srwtst_LDADD = ../odr/libodr.la libzutil.la $(XML2_LIBS) ../util/libutil.la
srwtst_SOURCES = srwtst.c

noinst_PROGRAMS = srwtst
