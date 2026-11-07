## This file is part of the YAZ toolkit.
## Copyright (C) Index Data

bin_PROGRAMS=yaz-client 
EXTRA_PROGRAMS=bertorture

check_PROGRAMS = test_client_history
check_SCRIPTS = test_client_history.sh
TESTS = $(check_PROGRAMS) $(check_SCRIPTS)
EXTRA_DIST = default.bib $(check_SCRIPTS)

COMMON=admin.c admin.h tabcomplete.c tabcomplete.h fhistory.c fhistory.h
yaz_client_SOURCES=client.c $(COMMON)

test_client_history_SOURCES = test_client_history.c $(COMMON)
test_client_history_LDADD = ../src/libyaz.la $(READLINE_LIBS)

AM_CPPFLAGS=-I$(top_srcdir)/src $(XML2_CFLAGS)

yaz_client_LDADD = ../src/libyaz.la $(READLINE_LIBS) 
bertorture_LDADD = ../src/libyaz.la 

bertorture_SOURCES=bertorture.c


