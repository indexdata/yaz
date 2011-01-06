## This file is part of the YAZ toolkit.
## Copyright (C) 1995-2011 Index Data

bin_PROGRAMS=yaz-ztest
noinst_PROGRAMS=gfs-example

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c dummy-opac.c ztest.h
gfs_example_SOURCES=gfs-example.c

EXTRA_DIST=dummy-words dummy-grs ztest.pem config1.xml

LDADD=../src/libyaz_server.la ../src/libyaz.la

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)
