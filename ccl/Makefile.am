## $Id: Makefile.am,v 1.8 2002-04-15 09:44:41 adam Exp $

noinst_LTLIBRARIES = libccl.la

noinst_PROGRAMS = cclsh

EXTRA_DIST = bib1

AM_CPPFLAGS=-I$(top_srcdir)/include

libccl_la_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c
cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.la $(READLINE_LIBS)
