/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/yaz-util.h>
#include <yaz/test.h>

static const char *expected_charset;
static int conversions;

static yaz_iconv_t test_iconv_open(const char *to, const char *from)
{
    conversions++;
    YAZ_CHECK(!strcmp(from, expected_charset));
    return yaz_iconv_open(to, from);
}

/* Exercise the real display path, observing charset selection without
 * relying on shared-library symbol interposition. */
/* config.h is already loaded and has no include guard. */
#undef HAVE_CONFIG_H
#define main client_main
#define yaz_iconv_open test_iconv_open
#include "../client/client.c"
#undef yaz_iconv_open
#undef main

static void check_record(const Odr_oid *oid, const char *data, int len,
                         const char *charset)
{
    Z_External record;
    Odr_oct octets;
    char *buf = xmalloc(len ? len : 1);
    memcpy(buf, data, len);
    memset(&record, 0, sizeof(record));
    record.direct_reference = (Odr_oid *) oid;
    record.which = Z_External_octet;
    octets.buf = buf;
    octets.len = len;
    record.u.octet_aligned = &octets;
    expected_charset = charset;
    conversions = 0;
    display_record(&record);
    YAZ_CHECK_EQ(conversions, 1);
    record_last = 0;
    xfree(buf);
}

int main(int argc, char **argv)
{
    const char *short_record = "00000nam a22000000a 4500";
    int len;
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    marcCharset = "auto";
    outputCharset = "UTF-8";
    for (len = 0; len < 24; len++)
        check_record(yaz_oid_recsyn_usmarc, short_record, len,
                     len > 9 ? "UTF-8" : "MARC-8");
    check_record(yaz_oid_recsyn_usmarc, "00000nam  22000000a 4500", 10, "MARC-8");
    check_record(yaz_oid_recsyn_usmarc, "00026nam a2200025   4500\036\035",
                 26, "UTF-8");
    check_record(yaz_oid_recsyn_usmarc, "00026nam  2200025   4500\036\035",
                 26, "MARC-8");
    check_record(yaz_oid_recsyn_unimarc, short_record, 1, "ISO-8859-1");
    marcCharset = "ISO-8859-1";
    check_record(yaz_oid_recsyn_usmarc, short_record, 1, "ISO-8859-1");
    YAZ_CHECK_TERM;
}
