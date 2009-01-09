## This file is part of the YAZ toolkit.
## Copyright (C) 1995-2009 Index Data

bin_SCRIPTS = yaz-asncomp yaz-config

EXTRA_DIST = yaz-asncomp yaz-icu-example.xml

DISTCLEANFILES = yaz-config

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS) $(ICU_CPPFLAGS)

bin_PROGRAMS = yaz-marcdump yaz-iconv yaz-illclient yaz-icu
noinst_PROGRAMS = cclsh cql2pqf cql2xcql srwtst yaz-benchmark \
 yaz-xmlquery

# MARC dumper utility
yaz_marcdump_SOURCES = marcdump.c
yaz_marcdump_LDADD = ../src/libyaz.la

# YAZ Iconv utility
yaz_iconv_SOURCES = yaziconv.c
yaz_iconv_LDADD = ../src/libyaz.la

srwtst_SOURCES=srwtst.c
srwtst_LDADD = ../src/libyaz.la

cclsh_SOURCES=cclsh.c
cclsh_LDADD = ../src/libyaz.la $(READLINE_LIBS)

cql2pqf_SOURCES = cql2pqf.c
cql2pqf_LDADD = ../src/libyaz.la

cql2xcql_SOURCES = cql2xcql.c
cql2xcql_LDADD = ../src/libyaz.la

yaz_benchmark_SOURCES = benchmark.c
yaz_benchmark_LDADD = ../src/libyaz.la

yaz_xmlquery_SOURCES = yaz-xmlquery.c
yaz_xmlquery_LDADD = ../src/libyaz.la

yaz_illclient_SOURCES = yaz-illclient.c
yaz_illclient_LDADD = ../src/libyaz.la

yaz_icu_SOURCES = yaz-icu.c
yaz_icu_LDADD =../src/libyaz_icu.la $(ICU_LIBS)
