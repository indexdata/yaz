## $Id: Makefile.am,v 1.9 2002-06-06 13:02:01 adam Exp $

noinst_LTLIBRARIES = libccl.la

noinst_PROGRAMS = cclsh

EXTRA_DIST = bib1

AM_CPPFLAGS=-I$(top_srcdir)/include

libccl_la_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c
cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.la ../util/libutil.la $(READLINE_LIBS)
