## $Id: Makefile.am,v 1.10 2003-06-23 10:22:21 adam Exp $

noinst_LTLIBRARIES = libccl.la

noinst_PROGRAMS = cclsh

check_PROGRAMS = tstccl

TESTS = $(check_PROGRAMS)

EXTRA_DIST = bib1

AM_CPPFLAGS=-I$(top_srcdir)/include

libccl_la_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c

cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.la ../util/libutil.la $(READLINE_LIBS)

tstccl_SOURCES = tstccl.c
tstccl_LDADD = libccl.la ../util/libutil.la

