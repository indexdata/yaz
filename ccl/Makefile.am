## $Id: Makefile.am,v 1.2 2000-02-28 11:44:39 adam Exp $

noinst_LIBRARIES = libccl.a

noinst_PROGRAMS = cclsh

INCLUDES=-I../include

libccl_a_SOURCES = cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c
cclsh_SOURCES=cclsh.c
cclsh_LDADD = libccl.a
