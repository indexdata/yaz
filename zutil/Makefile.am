## $Id: Makefile.am,v 1.15 2003-02-12 15:06:44 adam Exp $
noinst_LTLIBRARIES = libzutil.la

AM_CPPFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/srw $(XSLT_CFLAGS)

libzutil_la_SOURCES = zget.c yaz-ccl.c diagbib1.c logrpn.c \
  otherinfo.c pquery.c sortspec.c z3950oid.c charneg.c \
  zoom-c.c zoom-opt.c zoom-p.h grs1disp.c zgdu.c soap.c srw.c

srwtst_LDADD = ../odr/libodr.la libzutil.la $(XSLT_LIB) ../util/libutil.la
srwtst_SOURCES = srwtst.c

noinst_PROGRAMS = srwtst