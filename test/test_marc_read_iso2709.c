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
#include <yaz/json.h>
#include <yaz/wrbuf.h>
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
    *len = 38 + body_len;
    char *buf = xmalloc(*len);
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

    char *buf = record("245", valid, sizeof(valid) - 1, &len);
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

/* Writers must not go beyond the end of a subfield or the indicators. */
static void tst_write_bounds(void)
{
    static const int modes[] = { YAZ_MARC_LINE, YAZ_MARC_MARCXML,
        YAZ_MARC_TURBOMARC, YAZ_MARC_XCHANGE, YAZ_MARC_JSON };
    yaz_marc_t mt = yaz_marc_create();
    yaz_iconv_t cd = yaz_iconv_open("UTF-8", "MARC-8");
    WRBUF wr = wrbuf_alloc();
    const char *result;
    size_t result_len;
    char *buf;
    int len;
    size_t i;

    YAZ_CHECK(cd != 0);
    for (i = 0; i < sizeof(modes) / sizeof(*modes); i++)
    {
        yaz_marc_xml(mt, modes[i]);
        /* Something for a short subfield to run into, if not stopped. */
        buf = record("245", "  \037aXXXXXXXXXXXXXXXXXXXXXXX\036", 28, &len);
        YAZ_CHECK_EQ(yaz_marc_decode_buf(mt, buf, len, &result, &result_len),
                     len);
        YAZ_CHECK(strstr(result, "XXXXXXXX") != 0);
        xfree(buf);
        /* Identifier length 9, but the subfield is one byte. */
        buf = record("245", "  \037a\036", 5, &len);
        buf[11] = '9';
        YAZ_CHECK_EQ(yaz_marc_decode_buf(mt, buf, len, &result, &result_len),
                     len);
        YAZ_CHECK(strchr(result, 'X') == 0);
        if (modes[i] == YAZ_MARC_LINE)
            YAZ_CHECK(!strcmp(result, "00043nam a2900037   4500\n"
                              "245    $a \n\n"));
        xfree(buf);
    }

    /* Seven indicators, then two where the last one is an incomplete MARC-8
       character. The conversion is set after reading, as record_conv does. */
    buf = record("245", "XXXXXXX\037aY\036", 11, &len);
    buf[10] = '7';
    YAZ_CHECK_EQ(yaz_marc_read_iso2709(mt, buf, len), len);
    xfree(buf);
    buf = record("245", "1\342\036", 3, &len);
    YAZ_CHECK_EQ(yaz_marc_read_iso2709(mt, buf, len), len);
    yaz_marc_iconv(mt, cd);
    YAZ_CHECK_EQ(yaz_marc_write_marcxml(mt, wr), 0);
    YAZ_CHECK(strstr(wrbuf_cstr(wr), " ind2=") != 0);
    YAZ_CHECK(strstr(wrbuf_cstr(wr), " ind3=") == 0);
    xfree(buf);

    yaz_marc_iconv(mt, 0);
    yaz_iconv_close(cd);
    wrbuf_destroy(wr);
    yaz_marc_destroy(mt);
}

/* Two identical fields; each directory entry names the field it refers to. */
static char *record_dir(const int *fields, int entries, int *len)
{
    const char body[] = "  \037aX\036";
    int body_len = sizeof(body) - 1;
    int base = 24 + 12 * entries + 1;
    char *buf;
    int i;
    *len = base + 2 * body_len + 1;
    buf = xmalloc(*len);
    memcpy(buf, "00000nam a2200000   4500", 24);
    put_number(buf, 5, *len);
    put_number(buf + 12, 5, base);
    for (i = 0; i < entries; i++)
    {
        char *p = buf + 24 + 12 * i;
        memcpy(p, "245", 3);
        put_number(p + 3, 4, body_len);
        put_number(p + 7, 5, fields[i] * body_len);
    }
    buf[base - 1] = ISO2709_FS;
    memcpy(buf + base, body, body_len);
    memcpy(buf + base + body_len, body, body_len);
    buf[*len - 1] = ISO2709_RS;
    return buf;
}

static void tst_overlap(void)
{
    yaz_marc_t mt = yaz_marc_create();
    int fields[100];
    char *buf;
    int len, i;
    yaz_marc_xml(mt, YAZ_MARC_LINE);

    fields[0] = 0;
    fields[1] = 1;
    buf = record_dir(fields, 2, &len);
    check_record(mt, buf, len, 1);
    xfree(buf);
    fields[0] = 1;
    fields[1] = 0;
    buf = record_dir(fields, 2, &len);
    check_record(mt, buf, len, 1); /* directory order differs from data */
    xfree(buf);
    fields[2] = 0;
    buf = record_dir(fields, 3, &len);
    check_record(mt, buf, len, 0); /* more field data than the record has */
    xfree(buf);
    for (i = 0; i < 100; i++)
        fields[i] = 0;
    buf = record_dir(fields, 100, &len);
    check_record(mt, buf, len, 0);
    xfree(buf);
    yaz_marc_destroy(mt);
}

/* Comments quote record data and must not be able to end the XML comment. */
static void tst_xml_comment(void)
{
    static const int formats[] = {
        YAZ_MARC_MARCXML, YAZ_MARC_TURBOMARC, YAZ_MARC_XCHANGE
    };
    yaz_marc_t mt = yaz_marc_create();
    WRBUF wr = wrbuf_alloc();
    const char valid[] = "  \037aX\036";
    char comment[] = "trailing-";
    char *buf;
    int len, i, libxml2;

    buf = record("245", valid, sizeof(valid) - 1, &len);
    memcpy(buf + 24, "--><x/><!---", 12); /* bad directory entry */
#if YAZ_HAVE_XML2
    for (libxml2 = 0; libxml2 < 2; libxml2++)
#else
    for (libxml2 = 0; libxml2 < 1; libxml2++)
#endif
    {
        yaz_marc_write_using_libxml2(mt, libxml2);
        for (i = 0; i < 3; i++)
        {
            yaz_marc_xml(mt, formats[i]);
            YAZ_CHECK_EQ(yaz_marc_read_iso2709(mt, buf, len), len);
            yaz_marc_add_comment(mt, comment);
            wrbuf_rewind(wr);
            YAZ_CHECK_EQ(yaz_marc_write_mode(mt, wr), 0);
            YAZ_CHECK(strstr(wrbuf_cstr(wr), "(- -><x/><!- - -)"));
            YAZ_CHECK(strstr(wrbuf_cstr(wr), "trailing- "));
            YAZ_CHECK(!strstr(wrbuf_cstr(wr), "--><x/>"));
            YAZ_CHECK(!strstr(wrbuf_cstr(wr), "--->"));
        }
    }
    xfree(buf);
    wrbuf_destroy(wr);
    yaz_marc_destroy(mt);
}

/* The ISO2709 writer writes no field or record terminators but its own. */
static void tst_write_terminators(void)
{
    yaz_marc_t mt = yaz_marc_create();
    yaz_iconv_t cd = yaz_iconv_open("UTF-8", "MARC-8");
    /* MARC-8 0x9D and 0x9E are 0x1D and 0x1E when ASCII is the G1 set */
    const char marc8[] = "  \037ak\033)B\235\236b\235\036";
    const char expect1[] = "00048nam a2200037   4500" "245001000000\036"
        "  \037ak  b \036\035";
    const char *json = "{\"leader\":\"00000nam a2200000 a 4500\",\"fields\":"
        "[{\"2\\u001e\\u001d\":{\"ind1\":\"\\u001d\",\"ind2\":\"0\","
        "\"subfields\":[{\"a\":\"x\\u001dy\"}]}}]}";
    const char expect2[] = "00046nam a2200037 a 4500" "2  000800000\036"
        " 0\037ax y\036\035";
    struct json_node *n = json_parse(json, 0);
    WRBUF w = wrbuf_alloc();
    const char *result;
    size_t result_len;
    char *buf;
    int len;

    yaz_marc_xml(mt, YAZ_MARC_ISO2709);

    YAZ_CHECK(cd != 0);
    yaz_marc_iconv(mt, cd);
    buf = record("245", marc8, sizeof(marc8) - 1, &len);
    YAZ_CHECK_EQ(yaz_marc_decode_buf(mt, buf, len, &result, &result_len), len);
    YAZ_CHECK_EQ(result_len, sizeof(expect1) - 1);
    YAZ_CHECK(result_len == sizeof(expect1) - 1 &&
              !memcmp(result, expect1, result_len));
    xfree(buf);
    yaz_marc_iconv(mt, 0);
    if (cd)
        yaz_iconv_close(cd);

    YAZ_CHECK(n != 0);
    yaz_marc_reset(mt);
    YAZ_CHECK_EQ(yaz_marc_read_json_node(mt, n), 0);
    YAZ_CHECK_EQ(yaz_marc_write_mode(mt, w), 0);
    YAZ_CHECK_EQ(wrbuf_len(w), sizeof(expect2) - 1);
    YAZ_CHECK(wrbuf_len(w) == sizeof(expect2) - 1 &&
              !memcmp(wrbuf_buf(w), expect2, wrbuf_len(w)));
    json_remove_node(n);
    wrbuf_destroy(w);
    yaz_marc_destroy(mt);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_bounds();
    tst_write_bounds();
    tst_overlap();
    tst_xml_comment();
    tst_write_terminators();
    YAZ_CHECK_TERM;
}
