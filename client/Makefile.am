## Copyright (C) 1995-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.6 2000-04-05 07:39:54 adam Exp $

bin_PROGRAMS=yaz-client

yaz_client_SOURCES=client.c admin.c admin.h

yaz_client_LDADD = ../lib/libyaz.la

INCLUDES=-I$(srcdir)/../include -DCCLRPN=1
