## Copyright (C) 1995-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.3 2000-02-29 13:44:55 adam Exp $

AUTOMAKE_OPTIONS=no-dependencies

bin_PROGRAMS=yaz-client

yaz_client_SOURCES=client.c

LDADD = ../lib/libyaz.a

INCLUDES=-I../include -DCCLRPN=1
