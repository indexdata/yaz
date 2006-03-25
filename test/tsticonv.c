/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tsticonv.c,v 1.16 2006-03-25 14:42:16 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <yaz/yaz-util.h>
#include <yaz/test.h>

static int compare_buffers(char *msg, int no,
                           int expect_len, const char *expect_buf,
                           int got_len, const char *got_buf)
{
    if (expect_len == got_len
        && !memcmp(expect_buf, got_buf, expect_len))
        return 1;
    
    if (0) /* use 1 see how the buffers differ (for debug purposes) */
    {
        int i;
        printf("tsticonv test=%s i=%d failed\n", msg, no);
        printf("off got exp\n");
        for (i = 0; i<got_len || i<expect_len; i++)
        {
            char got_char[10];
            char expect_char[10];
            
            if (i < got_len)
                sprintf(got_char, "%02X", got_buf[i]);
            else
                sprintf(got_char, "?  ");
            
            if (i < expect_len)
                sprintf(expect_char, "%02X", expect_buf[i]);
            else
                sprintf(expect_char, "?  ");
            
            printf("%02d  %s  %s %c\n",
                   i, got_char, expect_char, got_buf[i] == expect_buf[i] ?
                   ' ' : '*');
            
        }
    }
    return 0;
}

/* some test strings in ISO-8859-1 format */
static const char *iso_8859_1_a[] = {
    "ax" ,
    "\xd8",
    "eneb\346r",
    "\xe5" "\xd8",
    "\xe5" "\xd8" "b",
    "\xe5" "\xe5",
    0 };

/* same test strings in MARC-8 format */
static const char *marc8_a[] = {
    "ax",   
    "\xa2",          /* latin capital letter o with stroke */
    "eneb\xb5r",     /* latin small letter ae */
    "\xea" "a\xa2",
    "\xea" "a\xa2" "b",
    "\xea" "a"  "\xea" "a",
    0
};

static void tst_marc8_to_iso_8859_1()
{
    int i;
    yaz_iconv_t cd;
    int ret;

    cd = yaz_iconv_open("ISO-8859-1", "MARC8");
    YAZ_CHECK(cd);
    if (!cd)
        return;
    for (i = 0; iso_8859_1_a[i]; i++)
    {
        size_t r;
        char *inbuf= (char*) marc8_a[i];
        size_t inbytesleft = strlen(inbuf);
        char outbuf0[32];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0);

        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        YAZ_CHECK(r != (size_t)(-1));
        if (r == (size_t) (-1))
            break;

        ret = compare_buffers("tsticonv 11", i,
                              strlen(iso_8859_1_a[i]), iso_8859_1_a[i],
                              outbuf - outbuf0, outbuf0);
        YAZ_CHECK(ret);
    }
    yaz_iconv_close(cd);
}

static void tst_marc8_to_ucs4b()
{
    static struct {
        const char *marc8_b;
        int len;
        const char *ucs4_b;
    } ar[] = {
    { 
        "\033$1" "\x21\x2B\x3B" /* FF1F */ "\033(B" "o",
        8, "\x00\x00\xFF\x1F" "\x00\x00\x00o"
    }, {
        "\033$1" "\x6F\x77\x29" /* AE0E */ "\x6F\x52\x7C" /* c0F4 */ "\033(B",
        8, "\x00\x00\xAE\x0E" "\x00\x00\xC0\xF4",
    }, {
        "\033$1"
        "\x21\x50\x6E"  /* UCS 7CFB */
        "\x21\x51\x31"  /* UCS 7D71 */
        "\x21\x3A\x67"  /* UCS 5B89 */
        "\x21\x33\x22"  /* UCS 5168 */
        "\x21\x33\x53"  /* UCS 5206 */
        "\x21\x44\x2B"  /* UCS 6790 */
        "\033(B",
        24, "\x00\x00\x7C\xFB"
        "\x00\x00\x7D\x71"
        "\x00\x00\x5B\x89"
        "\x00\x00\x51\x68"
        "\x00\x00\x52\x06"
        "\x00\x00\x67\x90"
    }, {
        "\xB0\xB2",     /* AYN and oSLASH */
        8, "\x00\x00\x02\xBB"  "\x00\x00\x00\xF8"
    }, {
        "\xF6\x61",     /* a underscore */
        8, "\x00\x00\x00\x61"  "\x00\x00\x03\x32"
    }, {
        "\x61\xC2",     /* a, phonorecord mark */
        8, "\x00\x00\x00\x61"  "\x00\x00\x21\x17"
    },
    {  /* bug #258 */
        "el" "\xe8" "am\xe8" "an", /* elaman where a is a" */
        32,
        "\x00\x00\x00" "e"
        "\x00\x00\x00" "l"
        "\x00\x00\x00" "a"
        "\x00\x00\x03\x08"
        "\x00\x00\x00" "m"
        "\x00\x00\x00" "a"
        "\x00\x00\x03\x08"
        "\x00\x00\x00" "n"
    }, 
    { /* bug #260 */
        "\xe5\xe8\x41",
        12, "\x00\x00\x00\x41" "\x00\x00\x03\x04" "\x00\x00\x03\x08"
    }, 
    { /* bug #416 */
        "\xEB\x74\xEC\x73",
        12, "\x00\x00\x00\x74" "\x00\x00\x03\x61" "\x00\x00\x00\x73"
    },
    { /* bug #416 */
        "\xFA\x74\xFB\x73",
        12, "\x00\x00\x00\x74" "\x00\x00\x03\x60" "\x00\x00\x00\x73"
    },
    {
        0, 0, 0
    }
    };
    int i;
    int ret;
    yaz_iconv_t cd;

    cd = yaz_iconv_open("UCS4", "MARC8");
    YAZ_CHECK(cd);
    if (!cd)
        return;
    for (i = 0; ar[i].len; i++)
    {
        size_t r;
        size_t expect_len = ar[i].len;
        char *inbuf= (char*) ar[i].marc8_b;
        size_t inbytesleft = strlen(inbuf);
        char outbuf0[64];
        char *outbuf = outbuf0;

        while (inbytesleft)
        {
            size_t outbytesleft = outbuf0 + sizeof(outbuf0) - outbuf;
            if (outbytesleft > 12)
                outbytesleft = 12;
            r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if (r == (size_t) (-1))
            {
                int e = yaz_iconv_error(cd);
                YAZ_CHECK(e == YAZ_ICONV_E2BIG);
                if (e != YAZ_ICONV_E2BIG)
                    return;
            }
            else
                break;
        }
        ret = compare_buffers("tsticonv 22", i,
                              expect_len, ar[i].ucs4_b,
                              outbuf - outbuf0, outbuf0);
        YAZ_CHECK(ret);
    }
    yaz_iconv_close(cd);
}

static void tst_ucs4b_to_utf8()
{
    static const char *ucs4_c[] = {
        "\x00\x00\xFF\x1F\x00\x00\x00o",
        "\x00\x00\xAE\x0E\x00\x00\xC0\xF4",
        0
    };
    static const char *utf8_c[] = {
        "\xEF\xBC\x9F\x6F",
        "\xEA\xB8\x8E\xEC\x83\xB4",
        0
    };
    
    int i;
    int ret;
    yaz_iconv_t cd;

    cd = yaz_iconv_open("UTF8", "UCS4");
    YAZ_CHECK(cd);
    if (!cd)
        return;
    for (i = 0; ucs4_c[i]; i++)
    {
        size_t r;
        char *inbuf= (char*) ucs4_c[i];
        size_t inbytesleft = 8;
        char outbuf0[24];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0);

        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        YAZ_CHECK(r != (size_t) (-1));
        if (r == (size_t) (-1))
            return;
        ret = compare_buffers("tsticonv 32", i,
                              strlen(utf8_c[i]), utf8_c[i],
                              outbuf - outbuf0, outbuf0);
        YAZ_CHECK(ret);
    }
    yaz_iconv_close(cd);
}

static void dconvert(int mandatory, const char *tmpcode)
{
    int i;
    int ret;
    yaz_iconv_t cd;
    for (i = 0; iso_8859_1_a[i]; i++)
    {
        size_t r;
        char *inbuf = (char*) iso_8859_1_a[i];
        size_t inbytesleft = strlen(inbuf);
        char outbuf0[24];
        char outbuf1[10];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0);

        cd = yaz_iconv_open(tmpcode, "ISO-8859-1");
        YAZ_CHECK(cd || !mandatory);
        if (!cd)
            return;
        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        YAZ_CHECK(r != (size_t) (-1));
        yaz_iconv_close(cd);
        if (r == (size_t) (-1))
            return;
        
        cd = yaz_iconv_open("ISO-8859-1", tmpcode);
        YAZ_CHECK(cd || !mandatory);
        if (!cd)
            return;
        inbuf = outbuf0;
        inbytesleft = sizeof(outbuf0) - outbytesleft;

        outbuf = outbuf1;
        outbytesleft = sizeof(outbuf1);
        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        YAZ_CHECK(r != (size_t) (-1));
        if (r != (size_t)(-1)) 
        {
            ret = compare_buffers("dconvert", i,
                                  strlen(iso_8859_1_a[i]), iso_8859_1_a[i],
                              sizeof(outbuf1) - outbytesleft, outbuf1);
            YAZ_CHECK(ret);
        }
        yaz_iconv_close(cd);
    }
}

int utf8_check(unsigned c)
{
    if (sizeof(c) >= 4)
    {
        size_t r;
        char src[4];
        char dst[4];
        char utf8buf[6];
        char *inbuf = src;
        size_t inbytesleft = 4;
        char *outbuf = utf8buf;
        size_t outbytesleft = sizeof(utf8buf);
        int i;
        yaz_iconv_t cd = yaz_iconv_open("UTF-8", "UCS4LE");
        if (!cd)
            return 0;
        for (i = 0; i<4; i++)
            src[i] = c >> (i*8);
        
        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        yaz_iconv_close(cd);

        if (r == (size_t)(-1))
            return 0;

        cd = yaz_iconv_open("UCS4LE", "UTF-8");
        if (!cd)
            return 0;
        inbytesleft = sizeof(utf8buf) - outbytesleft;
        inbuf = utf8buf;

        outbuf = dst;
        outbytesleft = 4;

        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        if (r == (size_t)(-1))
            return 0;

        yaz_iconv_close(cd);

        if (memcmp(src, dst, 4))
            return 0;
    }
    return 1;
}
        

static int tst_convert(yaz_iconv_t cd, const char *buf, const char *cmpbuf)
{
    int ret = 0;
    WRBUF b = wrbuf_alloc();
    char outbuf[12];
    size_t inbytesleft = strlen(buf);
    const char *inp = buf;
    while (inbytesleft)
    {
        size_t outbytesleft = sizeof(outbuf);
        char *outp = outbuf;
        size_t r = yaz_iconv(cd, (char**) &inp,  &inbytesleft,
                             &outp, &outbytesleft);
        if (r == (size_t) (-1))
        {
            int e = yaz_iconv_error(cd);
            if (e != YAZ_ICONV_E2BIG)
                break;
        }
        wrbuf_write(b, outbuf, outp - outbuf);
    }
    if (wrbuf_len(b) == strlen(cmpbuf) 
        && !memcmp(cmpbuf, wrbuf_buf(b), wrbuf_len(b)))
        ret = 1;
    else
        yaz_log(YLOG_LOG, "GOT (%.*s)", wrbuf_len(b), wrbuf_buf(b));
    wrbuf_free(b, 1);
    return ret;
}

static void tst_x()
{
    yaz_iconv_t cd = yaz_iconv_open("ISO-8859-1", "MARC8");

    YAZ_CHECK(cd);

    YAZ_CHECK(tst_convert(cd, "Cours de math", 
                          "Cours de math"));
    YAZ_CHECK(tst_convert(cd, "Cours de mathâe", 
                          "Cours de mathé"));
    YAZ_CHECK(tst_convert(cd, "12345678âe", 
                          "12345678é"));
    YAZ_CHECK(tst_convert(cd, "123456789âe", 
                          "123456789é"));
    YAZ_CHECK(tst_convert(cd, "1234567890âe", 
                          "1234567890é"));
    YAZ_CHECK(tst_convert(cd, "12345678901âe", 
                          "12345678901é"));
    YAZ_CHECK(tst_convert(cd, "Cours de mathâem", 
                          "Cours de mathém"));
    YAZ_CHECK(tst_convert(cd, "Cours de mathâematiques", 
                          "Cours de mathématiques"));

    yaz_iconv_close(cd);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);

    tst_x();

    YAZ_CHECK(utf8_check(3));
    YAZ_CHECK(utf8_check(127));
    YAZ_CHECK(utf8_check(128));
    YAZ_CHECK(utf8_check(255));
    YAZ_CHECK(utf8_check(256));
    YAZ_CHECK(utf8_check(900));
    YAZ_CHECK(utf8_check(1000));
    YAZ_CHECK(utf8_check(10000));
    YAZ_CHECK(utf8_check(100000));
    YAZ_CHECK(utf8_check(1000000));
    YAZ_CHECK(utf8_check(10000000));
    YAZ_CHECK(utf8_check(100000000));

    dconvert(1, "UTF-8");
    dconvert(1, "ISO-8859-1");
    dconvert(1, "UCS4");
    dconvert(1, "UCS4LE");
    dconvert(0, "CP865");
    tst_marc8_to_iso_8859_1();
    tst_marc8_to_ucs4b();
    tst_ucs4b_to_utf8();

    YAZ_CHECK_TERM;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
