## $Id: Makefile.am,v 1.8 2003-02-12 15:06:44 adam Exp $
## Copyright (C) 2001, Index Data

AM_CPPFLAGS = -I$(top_srcdir)/include

noinst_PROGRAMS = zoomtst1 zoomtst2 zoomtst3 zoomtst4 zoomtst5 zoomtst6 zoomtst7 zoomtst8
bin_PROGRAMS = zoomsh

zoomtst1_LDADD = ../lib/libyaz.la $(XSLT_LIB)
zoomtst2_LDADD = ../lib/libyaz.la $(XSLT_LIB)
zoomtst3_LDADD = ../lib/libyaz.la $(XSLT_LIB)
zoomtst4_LDADD = ../lib/libyaz.la $(XSLT_LIB)
zoomtst5_LDADD = ../lib/libyaz.la $(XSLT_LIB)
zoomtst6_LDADD = ../lib/libyaz.la $(XSLT_LIB)
zoomtst7_LDADD = ../lib/libyazmalloc.la ../lib/libyaz.la $(XSLT_LIB)
zoomtst8_LDADD = ../lib/libyaz.la $(XSLT_LIB)
zoomsh_LDADD = ../lib/libyaz.la $(XSLT_LIB) $(READLINE_LIBS) 

zoomtst1_SOURCES = zoomtst1.c
zoomtst2_SOURCES = zoomtst2.c
zoomtst3_SOURCES = zoomtst3.c
zoomtst4_SOURCES = zoomtst4.c
zoomtst5_SOURCES = zoomtst5.c
zoomtst6_SOURCES = zoomtst6.c
zoomtst7_SOURCES = zoomtst7.c
zoomtst8_SOURCES = zoomtst8.c
zoomsh_SOURCES = zoomsh.c
