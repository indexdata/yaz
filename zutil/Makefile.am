## $Id: Makefile.am,v 1.20 2003-04-23 12:31:07 adam Exp $
noinst_LTLIBRARIES = libzutil.la

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)

libzutil_la_SOURCES = zget.c yaz-ccl.c diagbib1.c logrpn.c \
  otherinfo.c pquery.c sortspec.c z3950oid.c charneg.c \
  zoom-c.c zoom-opt.c zoom-p.h grs1disp.c zgdu.c soap.c srw.c opacdisp.c

srwtst_LDADD = ../odr/libodr.la libzutil.la ../util/libutil.la
srwtst_SOURCES = srwtst.c

EXTRA_PROGRAMS = srwtst
