## This file is part of the YAZ toolkit.
## Copyright (C) 1995-2009 Index Data

bin_PROGRAMS=yaz-client 
EXTRA_PROGRAMS=bertorture

EXTRA_DIST = default.bib

COMMON=admin.c admin.h tabcomplete.c tabcomplete.h fhistory.c fhistory.h
yaz_client_SOURCES=client.c $(COMMON)

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)

yaz_client_LDADD = ../src/libyaz.la $(READLINE_LIBS) 
bertorture_LDADD = ../src/libyaz.la 

bertorture_SOURCES=bertorture.c


