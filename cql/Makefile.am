# $Id: Makefile.am,v 1.2 2003-04-11 15:53:39 adam Exp $
AM_YFLAGS=-p cql_

AM_CPPFLAGS=-I$(top_srcdir)/include

noinst_LTLIBRARIES = libcql.la
noinst_PROGRAMS = cql2pqf cql2xcql

EXTRA_DIST=lexer.c

libcql_la_SOURCES=cql.y cqlstdio.c cqltransform.c \
 cqlutil.c xcqlutil.c cqlstring.c cql.h

cql2pqf_SOURCES = cql2pqf.c
cql2xcql_SOURCES = cql2xcql.c

LDADD=libcql.la ../util/libutil.la
