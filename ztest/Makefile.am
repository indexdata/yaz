## This file is part of the YAZ toolkit.
## Copyright (C) 1995-2008 Index Data

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c ztest.h

EXTRA_DIST=dummy-records dummy-words dummy-grs ztest.pem config1.xml

yaz_ztest_LDADD=../src/libyaz_server.la $(PTHREAD_LIBS)

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)
