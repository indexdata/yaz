/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <yaz/test.h>
#include <yaz/marcdisp.h>
#include <yaz/xmalloc.h>

static void put_number(char *p, int width, int value)
{
    char tmp[16];
    sprintf(tmp, "%0*d", width, value);
    memcpy(p, tmp, width);
}

/* Allocate exactly the advertised size: no NUL or spare trailing bytes. */
static char *record(const char *tag, const char *body, int body_len, int *len)
{
    char *buf;
    *len = 38 + body_len;
    buf = xmalloc(*len);
    memcpy(buf, "00000nam a2200037   4500", 24);
    put_number(buf, 5, *len);
    memcpy(buf + 24, tag, 3);
    put_number(buf + 27, 4, body_len);
    memcpy(buf + 31, "00000", 5);
    buf[36] = ISO2709_FS;
    memcpy(buf + 37, body, body_len);
    buf[*len - 1] = ISO2709_RS;
    return buf;
}

static void check_record(yaz_marc_t mt, const char *buf, int len, int ok)
{
    const char *result;
    size_t result_len;
    int expected = ok ? len : -1;
    YAZ_CHECK_EQ(yaz_marc_read_iso2709(mt, buf, len), expected);
    YAZ_CHECK_EQ(yaz_marc_decode_buf(mt, buf, len, &result, &result_len),
                 expected);
}

static void tst_bounds(void)
{
    yaz_marc_t mt = yaz_marc_create();
    const char valid[] = "  \037aX\036";
    char *buf;
    int len, i, mode;
    yaz_marc_xml(mt, YAZ_MARC_LINE);
    for (i = 0; i < 24; i++)
    {
        char *short_buf = xmalloc(i ? i : 1);
        memset(short_buf, '0', i);
        check_record(mt, short_buf, i, 0);
        xfree(short_buf);
    }
    check_record(mt, 0, 0, 0);
    YAZ_CHECK_EQ(yaz_marc_read_iso2709(mt, 0, -1), -1);
    YAZ_CHECK_EQ(yaz_marc_read_iso2709(mt, "", -2), -1);

    buf = record("245", valid, sizeof(valid) - 1, &len);
    check_record(mt, buf, len, 1);
    YAZ_CHECK_EQ(yaz_marc_read_iso2709(mt, buf, -1), len);
    for (i = 24; i < len; i++)
        check_record(mt, buf, i, 0);
    put_number(buf + 27, 4, 9999);
    check_record(mt, buf, len, 0);
    put_number(buf + 27, 4, sizeof(valid) - 1);
    put_number(buf + 31, 5, 99999);
    check_record(mt, buf, len, 0);
    put_number(buf + 31, 5, 0);
    put_number(buf + 12, 5, len);
    check_record(mt, buf, len, 0);
    put_number(buf + 12, 5, 0);
    check_record(mt, buf, len, 0);
    xfree(buf);

    /* Exercise both raw UTF-8 and iconv character-width probing. */
    for (mode = 0; mode < 2; mode++)
    {
        yaz_iconv_t cd = mode ? yaz_iconv_open("UTF-8", "UTF-8") : 0;
        if (mode)
            YAZ_CHECK(cd != 0);
        yaz_marc_iconv(mt, cd);
        buf = record("001", "\036", 1, &len);
        check_record(mt, buf, len, 1); /* short control field, no indicators */
        xfree(buf);
        buf = record("245", "\036", 1, &len);
        check_record(mt, buf, len, 0);
        xfree(buf);
        buf = record("245", "a\036", 2, &len);
        check_record(mt, buf, len, 0);
        xfree(buf);
        buf = record("245", "\303\036", 2, &len);
        check_record(mt, buf, len, 0);
        xfree(buf);
        buf = record("245", "\303\251 \037aX\036", 7, &len);
        check_record(mt, buf, len, 1); /* two indicators, three UTF-8 bytes */
        buf[10] = '9';
        check_record(mt, buf, len, 0);
        xfree(buf);
        buf = record("001", "  \037aX\036", 6, &len);
        check_record(mt, buf, len, 1); /* Danmarc control-tag data field */
        xfree(buf);
        buf = record("001", "X  \037aX\036", 7, &len);
        check_record(mt, buf, len, 1); /* Danmarc extra leading byte */
        xfree(buf);
        yaz_marc_iconv(mt, 0);
        if (cd)
            yaz_iconv_close(cd);
    }
    yaz_marc_destroy(mt);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_bounds();
    YAZ_CHECK_TERM;
}
