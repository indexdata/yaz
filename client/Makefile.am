## Copyright (C) 1995-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.7 2000-05-02 19:32:16 adam Exp $

bin_PROGRAMS=yaz-client

yaz_client_SOURCES=client.c admin.c admin.h

yaz_client_LDADD = ../lib/libyaz.la $(READLINE_LIBS)

INCLUDES=-I$(srcdir)/../include -DCCLRPN=1
