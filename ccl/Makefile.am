## $Id: Makefile.am,v 1.7 2001-04-05 13:08:48 adam Exp $

noinst_LTLIBRARIES = libccl.la

noinst_PROGRAMS = cclsh

EXTRA_DIST = bib1

INCLUDES=-I$(top_srcdir)/include

libccl_la_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c
cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.la $(READLINE_LIBS)
