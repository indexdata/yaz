## $Id: Makefile.am,v 1.3 2000-04-05 07:39:54 adam Exp $

noinst_LTLIBRARIES = libccl.la

noinst_PROGRAMS = cclsh

LIBS =

INCLUDES=-I$(srcdir)/../include

libccl_la_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c
cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.la
