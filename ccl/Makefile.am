## $Id: Makefile.am,v 1.6 2001-03-21 15:02:39 adam Exp $

noinst_LTLIBRARIES = libccl.la

noinst_PROGRAMS = cclsh

EXTRA_DIST = bib1

LIBS = $(READLINE_LIBS)

INCLUDES=-I$(top_srcdir)/include

libccl_la_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c
cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.la
