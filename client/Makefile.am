## Copyright (C) 1995-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.8 2000-05-25 19:58:00 adam Exp $

bin_PROGRAMS=yaz-client

yaz_client_SOURCES=client.c admin.c admin.h

yaz_client_LDADD = ../lib/libyaz.la $(READLINE_LIBS)

INCLUDES=-I$(srcdir)/../include -DCCL2RPN=1
