## $Id: Makefile.am,v 1.11 2003-05-27 21:42:45 adam Exp $

noinst_LTLIBRARIES = libodr.la

AM_CPPFLAGS=-I$(top_srcdir)/include

TESTS = tstodr

EXTRA_PROGRAMS = tstodr

EXTRA_DIST = tstodrcodec.c tstodrcodec.h tstodr.asn

tstodr_LDADD = libodr.la ../util/libutil.la
tstodr_SOURCES = tstodrcodec.c tstodrcodec.h tstodr.c

# Rule for generating codecs for our small ASN.1 spec
tstodrcodec.c tstodrcodec.h: $(srcdir)/tstodr.asn $(top_srcdir)/util/yaz-asncomp
	cd $(srcdir); $(top_srcdir)/util/yaz-asncomp tstodr.asn

libodr_la_SOURCES = odr_bool.c ber_bool.c ber_len.c ber_tag.c odr_util.c \
        odr_null.c ber_null.c odr_int.c ber_int.c odr_tag.c odr_cons.c \
        odr_seq.c odr_oct.c ber_oct.c odr_bit.c ber_bit.c odr_oid.c \
        ber_oid.c odr_use.c odr_choice.c odr_any.c ber_any.c odr.c odr_mem.c \
        dumpber.c odr_enum.c odr-priv.h

