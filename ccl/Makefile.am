## $Id: Makefile.am,v 1.4 2001-03-18 20:45:39 ja7 Exp $

noinst_LTLIBRARIES = libccl.la

noinst_PROGRAMS = cclsh

LIBS = $(READLINE_LIBS)

INCLUDES=-I$(srcdir)/../include

libccl_la_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c
cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.la
