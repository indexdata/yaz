## Copyright (C) 1994-2004, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.26 2004-04-28 22:45:00 adam Exp $

TESTS = $(check_PROGRAMS)

bin_SCRIPTS = yaz-asncomp yaz-config

EXTRA_DIST = yaz-asncomp 

DISTCLEANFILES = yaz-config

AM_CPPFLAGS=-I$(top_srcdir)/include

bin_PROGRAMS = yaz-marcdump yaz-iconv
noinst_PROGRAMS = cclsh cql2pqf cql2xcql srwtst yaz-benchmark

# MARC dumper utility
yaz_marcdump_SOURCES = marcdump.c
yaz_marcdump_LDADD = ../src/libyaz.la $(SSL_LIBS)

# YAZ Iconv utility
yaz_iconv_SOURCES = yaziconv.c
yaz_iconv_LDADD = ../src/libyaz.la $(SSL_LIBS)

srwtst_SOURCES=srwtst.c
srwtst_LDADD = ../src/libyaz.la $(SSL_LIBS)

cclsh_SOURCES=cclsh.c
cclsh_LDADD = ../src/libyaz.la $(READLINE_LIBS) $(SSL_LIBS)

cql2pqf_SOURCES = cql2pqf.c
cql2pqf_LDADD = ../src/libyaz.la $(SSL_LIBS)

cql2xcql_SOURCES = cql2xcql.c
cql2xcql_LDADD = ../src/libyaz.la $(SSL_LIBS)

yaz_benchmark_SOURCES = benchmark.c
yaz_benchmark_LDADD = ../src/libyaz.la $(SSL_LIBS)

