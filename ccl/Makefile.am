## $Id: Makefile.am,v 1.5 2001-03-19 21:30:06 adam Exp $

noinst_LTLIBRARIES = libccl.la

noinst_PROGRAMS = cclsh

EXTRA_DIST = bib1

LIBS = $(READLINE_LIBS)

INCLUDES=-I$(srcdir)/../include

libccl_la_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c
cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.la
