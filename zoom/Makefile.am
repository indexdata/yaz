## $Id: Makefile.am,v 1.4 2002-06-02 21:27:17 adam Exp $
## Copyright (C) 2001, Index Data

AM_CPPFLAGS = -I$(top_srcdir)/include
noinst_LTLIBRARIES = libzoom.la
libzoom_la_SOURCES = zoom-opt.c zoom-c.c zoom-p.h

zoomlibs = libzoom.la ../comstack/libcomstack.la ../z39.50/libz39.50.la ../ill/libill.la ../zutil/libzutil.la  ../odr/libodr.la ../util/libutil.la $(READLINE_LIBS)

zoomsh_LDADD = $(zoomlibs)
zoomtst1_LDADD = $(zoomlibs)
zoomtst2_LDADD = $(zoomlibs)
zoomtst3_LDADD = $(zoomlibs)
zoomtst4_LDADD = $(zoomlibs)
zoomtst5_LDADD = $(zoomlibs)
zoomtst6_LDADD = $(zoomlibs)
zoomtst7_LDADD = $(zoomlibs)
zoomtst8_LDADD = $(zoomlibs)

noinst_PROGRAMS = zoomtst1 zoomtst2 zoomtst3 zoomtst4 zoomtst5 zoomtst6 zoomtst7 zoomtst8
bin_PROGRAMS = zoomsh

zoomtst1_SOURCES = zoomtst1.c
zoomtst2_SOURCES = zoomtst2.c
zoomtst3_SOURCES = zoomtst3.c
zoomtst4_SOURCES = zoomtst4.c
zoomtst5_SOURCES = zoomtst5.c
zoomtst6_SOURCES = zoomtst6.c
zoomtst7_SOURCES = zoomtst7.c
zoomtst8_SOURCES = zoomtst8.c
zoomsh_SOURCES = zoomsh.c
