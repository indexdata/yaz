## Copyright (C) 1994-2003, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.24 2003-12-24 16:23:43 mike Exp $

TESTS = $(check_PROGRAMS)

bin_SCRIPTS = yaz-asncomp yaz-config

EXTRA_DIST = yaz-asncomp 

AM_CPPFLAGS=-I$(top_srcdir)/include

bin_PROGRAMS = yaz-marcdump yaz-iconv
noinst_PROGRAMS = cclsh cql2pqf cql2xcql srwtst yaz-benchmark

# MARC dumper utility
yaz_marcdump_LDADD = ../src/libyaz.la 
yaz_marcdump_SOURCES = marcdump.c

# YAZ Iconv utility
yaz_iconv_LDADD = ../src/libyaz.la 
yaz_iconv_SOURCES = yaziconv.c

srwtst_SOURCES=srwtst.c
srwtst_LDADD = ../src/libyaz.la

cclsh_SOURCES=cclsh.c
cclsh_LDADD = ../src/libyaz.la $(READLINE_LIBS)

cql2pqf_SOURCES = cql2pqf.c
cql2pqf_LDADD = ../src/libyaz.la

cql2xcql_SOURCES = cql2xcql.c
cql2xcql_LDADD = ../src/libyaz.la

yaz_benchmark_LDADD = ../src/libyaz.la 
yaz_benchmark_SOURCES = benchmark.c

