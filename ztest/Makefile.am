## This file is part of the YAZ toolkit.
## Copyright (C) Index Data

bin_PROGRAMS=yaz-ztest
noinst_PROGRAMS=gfs-example
check_PROGRAMS=test_xml_fetch
TESTS=$(check_PROGRAMS)

test_xml_fetch_SOURCES=test_xml_fetch.c dummy-opac.c

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c dummy-opac.c ztest.h
gfs_example_SOURCES=gfs-example.c

EXTRA_DIST=dummy-words dummy-grs ztest.pem config1.xml

LDADD=../src/libyaz_server.la ../src/libyaz.la

AM_CPPFLAGS=-I$(top_srcdir)/src $(XML2_CFLAGS)

clean-local:
	rm -f socket
