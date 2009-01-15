/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
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

#define ESC "\x1b"

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

static int tst_convert_l(yaz_iconv_t cd, size_t in_len, const char *in_buf,
                         size_t expect_len, const char *expect_buf)
{
    size_t r;
    char *inbuf= (char*) in_buf;
    size_t inbytesleft = in_len > 0 ? in_len : strlen(in_buf);
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
            if (e != YAZ_ICONV_E2BIG)
                return 0;
        }
        else
        {
            yaz_iconv(cd, 0, 0, &outbuf, &outbytesleft);
            break;
        }
    }

    return compare_buffers("tsticonv 22", 0,
                           expect_len, expect_buf,
                           outbuf - outbuf0, outbuf0);
}

static int tst_convert_x(yaz_iconv_t cd, const char *buf, const char *cmpbuf,
                         int expect_error)
{
    int ret = 1;
    WRBUF b = wrbuf_alloc();
    char outbuf[16];
    size_t inbytesleft = strlen(buf);
    const char *inp = buf;
    int rounds = 0;
    for (rounds = 0; inbytesleft && rounds < sizeof(outbuf); rounds++)
    {
        size_t outbytesleft = sizeof(outbuf);
        char *outp = outbuf;
        size_t r = yaz_iconv(cd, (char**) &inp,  &inbytesleft,
                             &outp, &outbytesleft);
        wrbuf_write(b, outbuf, outp - outbuf);
        if (r == (size_t) (-1))
        {
            int e = yaz_iconv_error(cd);
            if (e != YAZ_ICONV_E2BIG)
            {
                if (expect_error != -1)
                    if (e != expect_error)
                        ret = 0;
                break;
            }
        }
        else
        {
            size_t outbytesleft = sizeof(outbuf);
            char *outp = outbuf;
            r = yaz_iconv(cd, 0, 0, &outp, &outbytesleft);
            wrbuf_write(b, outbuf, outp - outbuf);
            if (expect_error != -1)
                if (expect_error)
                    ret = 0;
            break;
        }
    }
    if (wrbuf_len(b) == strlen(cmpbuf) 
        && !memcmp(cmpbuf, wrbuf_buf(b), wrbuf_len(b)))
        ;
    else
    {
        WRBUF w = wrbuf_alloc();

        ret = 0;
        wrbuf_rewind(w);
        wrbuf_puts_escaped(w, buf);
        yaz_log(YLOG_LOG, "input %s", wrbuf_cstr(w));

        wrbuf_rewind(w);
        wrbuf_write_escaped(w, wrbuf_buf(b), wrbuf_len(b));
        yaz_log(YLOG_LOG, "got %s", wrbuf_cstr(w));
        
        wrbuf_rewind(w);
        wrbuf_puts_escaped(w, cmpbuf);
        yaz_log(YLOG_LOG, "exp %s", wrbuf_cstr(w));

        wrbuf_destroy(w);
    }

    wrbuf_destroy(b);
    return ret;
}

static int tst_convert(yaz_iconv_t cd, const char *buf, const char *cmpbuf)
{
    return tst_convert_x(cd, buf, cmpbuf, 0);
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

static void tst_marc8_to_ucs4b(void)
{
    yaz_iconv_t cd = yaz_iconv_open("UCS4", "MARC8");
    YAZ_CHECK(cd);
    if (!cd)
        return;
    
    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\033$1" "\x21\x2B\x3B" /* FF1F */ "\033(B" "o",
                  8, 
                  "\x00\x00\xFF\x1F" "\x00\x00\x00o"));
    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\033$1" "\x6F\x77\x29" /* AE0E */
                  "\x6F\x52\x7C" /* c0F4 */ "\033(B",
                  8,
                  "\x00\x00\xAE\x0E" "\x00\x00\xC0\xF4"));
    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\033$1"
                  "\x21\x50\x6E"  /* UCS 7CFB */
                  "\x21\x51\x31"  /* UCS 7D71 */
                  "\x21\x3A\x67"  /* UCS 5B89 */
                  "\x21\x33\x22"  /* UCS 5168 */
                  "\x21\x33\x53"  /* UCS 5206 */
                  "\x21\x44\x2B"  /* UCS 6790 */
                  "\033(B",
                  24, 
                  "\x00\x00\x7C\xFB"
                  "\x00\x00\x7D\x71"
                  "\x00\x00\x5B\x89"
                  "\x00\x00\x51\x68"
                  "\x00\x00\x52\x06"
                  "\x00\x00\x67\x90"));

    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\xB0\xB2",     /* AYN and oSLASH */
                  8, 
                  "\x00\x00\x02\xBB"  "\x00\x00\x00\xF8"));
    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\xF6\x61",     /* a underscore */
                  8, 
                  "\x00\x00\x00\x61"  "\x00\x00\x03\x32"));

    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\x61\xC2",     /* a, phonorecord mark */
                  8,
                  "\x00\x00\x00\x61"  "\x00\x00\x21\x17"));

    /* bug #258 */
    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "el" "\xe8" "am\xe8" "an", /* elaman where a is a" */
                  32,
                  "\x00\x00\x00" "e"
                  "\x00\x00\x00" "l"
                  "\x00\x00\x00" "a"
                  "\x00\x00\x03\x08"
                  "\x00\x00\x00" "m"
                  "\x00\x00\x00" "a"
                  "\x00\x00\x03\x08"
                  "\x00\x00\x00" "n"));
    /* bug #260 */
    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\xe5\xe8\x41",
                  12, 
                  "\x00\x00\x00\x41" "\x00\x00\x03\x04" "\x00\x00\x03\x08"));
    /* bug #416 */
    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\xEB\x74\xEC\x73",
                  12,
                  "\x00\x00\x00\x74" "\x00\x00\x03\x61" "\x00\x00\x00\x73"));
    /* bug #416 */
    YAZ_CHECK(tst_convert_l(
                  cd,
                  0,
                  "\xFA\x74\xFB\x73",
                  12, 
                  "\x00\x00\x00\x74" "\x00\x00\x03\x60" "\x00\x00\x00\x73"));

    yaz_iconv_close(cd);
}

static void tst_ucs4b_to_utf8(void)
{
    yaz_iconv_t cd = yaz_iconv_open("UTF8", "UCS4");
    YAZ_CHECK(cd);
    if (!cd)
        return;
    YAZ_CHECK(tst_convert_l(
                  cd,
                  8,
                  "\x00\x00\xFF\x1F\x00\x00\x00o",
                  4,
                  "\xEF\xBC\x9F\x6F"));

    YAZ_CHECK(tst_convert_l(
                  cd,
                  8, 
                  "\x00\x00\xAE\x0E\x00\x00\xC0\xF4",
                  6,
                  "\xEA\xB8\x8E\xEC\x83\xB4"));
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

        r = yaz_iconv(cd, 0, 0, &outbuf, &outbytesleft);
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

        r = yaz_iconv(cd, 0, 0, &outbuf, &outbytesleft);
        if (r == (size_t)(-1))
        {
            fprintf(stderr, "failed\n");
        }
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
        
static void tst_marc8_to_utf8(void)
{
    yaz_iconv_t cd = yaz_iconv_open("UTF-8", "MARC8");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK(tst_convert(cd, "Cours de math", 
                          "Cours de math"));
    /* COMBINING ACUTE ACCENT */
    YAZ_CHECK(tst_convert(cd, "Cours de mathâe", 
                          "Cours de mathe\xcc\x81"));

    YAZ_CHECK(tst_convert(cd, "\xea" "a", "a\xcc\x8a"));
    YAZ_CHECK(tst_convert(cd, "a" "\xea" "\x1e", "a" "\x1e\xcc\x8a"));
    YAZ_CHECK(tst_convert(cd, "a" "\xea" "p", "a" "p\xcc\x8a"));

    YAZ_CHECK(tst_convert_x(cd, "a\xea", "a", YAZ_ICONV_EINVAL));
    YAZ_CHECK(tst_convert(cd, "p", "\xcc\x8a")); /* note: missing p */
    yaz_iconv(cd, 0, 0, 0, 0);     /* incomplete. so we have to reset */

    /* bug #2115 */
    YAZ_CHECK(tst_convert(cd, ESC "(N" ESC ")Qp" ESC "(B", "\xd0\x9f"));

    YAZ_CHECK(tst_convert_x(cd, ESC , "", YAZ_ICONV_EINVAL));
    YAZ_CHECK(tst_convert_x(cd, ESC "(", "", YAZ_ICONV_EINVAL));
    YAZ_CHECK(tst_convert_x(cd, ESC "(B", "", 0));

    YAZ_CHECK(tst_convert(cd, ESC "(B" "\x31", "1"));  /* ASCII in G0 */
    YAZ_CHECK(tst_convert(cd, ESC ")B" "\xB1", "1"));  /* ASCII in G1 */

    yaz_iconv_close(cd);
}

static void tst_marc8s_to_utf8(void)
{
    yaz_iconv_t cd = yaz_iconv_open("UTF-8", "MARC8s");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK(tst_convert(cd, "Cours de math", 
                          "Cours de math"));
    /* E9: LATIN SMALL LETTER E WITH ACUTE */
    YAZ_CHECK(tst_convert(cd, "Cours de mathâe", 
                          "Cours de math\xc3\xa9"));

    yaz_iconv_close(cd);
}


static void tst_marc8_to_latin1(void)
{
    yaz_iconv_t cd = yaz_iconv_open("ISO-8859-1", "MARC8");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK(tst_convert(cd, "ax", "ax"));

    /* latin capital letter o with stroke */
    YAZ_CHECK(tst_convert(cd, "\xa2", "\xd8"));

    /* with latin small letter ae */
    YAZ_CHECK(tst_convert(cd, "eneb\xb5r", "eneb\346r"));

    YAZ_CHECK(tst_convert(cd, "\xea" "a\xa2", "\xe5" "\xd8"));

    YAZ_CHECK(tst_convert(cd, "\xea" "a\xa2" "b", "\xe5" "\xd8" "b"));

    YAZ_CHECK(tst_convert(cd, "\xea" "a"  "\xea" "a", "\xe5" "\xe5"));

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

static void tst_utf8_to_marc8(const char *marc8_type)
{
    yaz_iconv_t cd = yaz_iconv_open(marc8_type, "UTF-8");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK(tst_convert(cd, "Cours ", "Cours "));

    /** Pure ASCII. 11 characters (sizeof(outbuf)-1) */
    YAZ_CHECK(tst_convert(cd, "Cours de mat", "Cours de mat"));

    /** Pure ASCII. 12 characters (sizeof(outbuf)) */
    YAZ_CHECK(tst_convert(cd, "Cours de math", "Cours de math"));

    /** Pure ASCII. 13 characters (sizeof(outbuf)+1) */
    YAZ_CHECK(tst_convert(cd, "Cours de math.", "Cours de math."));

    /** UPPERCASE SCANDINAVIAN O */
    YAZ_CHECK(tst_convert(cd, "S\xc3\x98", "S\xa2"));

    /** ARING */
    YAZ_CHECK(tst_convert(cd, "A" "\xCC\x8A", "\xEA" "A"));

    /** A MACRON + UMLAUT, DIAERESIS */
    YAZ_CHECK(tst_convert(cd, "A" "\xCC\x84" "\xCC\x88",
                          "\xE5\xE8\x41"));
    
    /* Ligature spanning two characters */
    YAZ_CHECK(tst_convert(cd,
                          "\x74" "\xCD\xA1" "\x73",  /* UTF-8 */
                          "\xEB\x74\xEC\x73"));      /* MARC-8 */

    /* Double title spanning two characters */
    YAZ_CHECK(tst_convert(cd,
                          "\x74" "\xCD\xA0" "\x73",  /* UTF-8 */
                          "\xFA\x74\xFB\x73"));      /* MARC-8 */

    /** Ideographic question mark (Unicode FF1F) */
    YAZ_CHECK(tst_convert(cd,
                          "\xEF\xBC\x9F" "o",        /* UTF-8 */
                          "\033$1" "\x21\x2B\x3B" "\033(B" "o" ));


    /** Ideographic space per ANSI Z39.64 */
    YAZ_CHECK(tst_convert(cd,
                          "\xe3\x80\x80" "o",        /* UTF-8 */
                          "\033$1" "\x21\x23\x21" "\033(B" "o" ));

    /** Superscript 0 . bug #642 */
    YAZ_CHECK(tst_convert(cd,
                          "(\xe2\x81\xb0)",        /* UTF-8 */
                          "(\033p0\x1bs)"));
    
    
    /** bug #1778 */
    YAZ_CHECK(tst_convert(cd,
                          /* offset 0x530 in UTF-8 rec marccol4.u8.marc */
                          "\xE3\x83\xB3" "\xE3\x82\xBF" 
                          "\xCC\x84" "\xCC\x84" "\xE3\x83\xBC" /* UTF-8 */,
                          "\x1B\x24\x31" "\x69\x25\x73"
                          "\x1B\x28\x42" "\xE5\xE5" "\x1B\x24\x31" 
                          "\x69\x25\x3F"
                          "\x69\x21\x3C" "\x1B\x28\x42"));

    
    /** bug #2120 */
    YAZ_CHECK(tst_convert(cd, 
                          "\xCE\x94\xCE\xB5\xCF\x84"
                          "\xCE\xBF\xCF\x81\xCE\xB1"
                          "\xCE\xBA\xCE\xB7\xCF\x82\x2C",

                          "\x1B\x28\x53\x45\x66\x78\x72\x75"
                          "\x61\x6D\x6A\x77"
                          "\x1B\x28\x42\x2C"
                  ));
 
    {
        char *inbuf0 = "\xe2\x81\xb0";
        char *inbuf = inbuf0;
        size_t inbytesleft = strlen(inbuf);
        char outbuf0[64];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0)-1;
        size_t r;
#if 0
        int i;
#endif
        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        YAZ_CHECK(r != (size_t) (-1));

#if 0
        *outbuf = '\0';  /* so we know when to stop printing */
        for (i = 0; outbuf0[i]; i++)
        {
            int ch = outbuf0[i] & 0xff;
            yaz_log(YLOG_LOG, "ch%d %02X %c", i, ch, ch >= ' ' ? ch : '?');
        }
#endif

        r = yaz_iconv(cd, 0, 0, &outbuf, &outbytesleft);
        YAZ_CHECK(r != (size_t) (-1));
        *outbuf = '\0';  /* for strcmp test below and printing */
#if 0
        for (i = 0; outbuf0[i]; i++)
        {
            int ch = outbuf0[i] & 0xff;
            yaz_log(YLOG_LOG, "ch%d %02X %c", i, ch, ch >= ' ' ? ch : '?');
        }
#endif
        YAZ_CHECK(strcmp("\033p0\x1bs", outbuf0) == 0);
    }
    yaz_iconv(cd, 0, 0, 0, 0);
    yaz_iconv_close(cd);
}

static void tst_advance_to_utf8(void)
{
    yaz_iconv_t cd = yaz_iconv_open("utf-8", "advancegreek");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK(tst_convert(cd, "Cours ", "Cours "));
    yaz_iconv_close(cd);
}

static void tst_utf8_to_advance(void)
{
    yaz_iconv_t cd = yaz_iconv_open("advancegreek", "utf-8");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK(tst_convert(cd, "Cours ", "Cours "));
    yaz_iconv_close(cd);
}

static void tst_latin1_to_marc8(void)
{
    yaz_iconv_t cd = yaz_iconv_open("MARC8", "ISO-8859-1");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK(tst_convert(cd, "Cours ", "Cours "));

    /** Pure ASCII. 11 characters (sizeof(outbuf)-1) */
    YAZ_CHECK(tst_convert(cd, "Cours de mat", "Cours de mat"));

    /** Pure ASCII. 12 characters (sizeof(outbuf)) */
    YAZ_CHECK(tst_convert(cd, "Cours de math", "Cours de math"));

    /** Pure ASCII. 13 characters (sizeof(outbuf)) */
    YAZ_CHECK(tst_convert(cd, "Cours de math.", "Cours de math."));

    /** D8: UPPERCASE SCANDINAVIAN O */
    YAZ_CHECK(tst_convert(cd, "S\xd8", "S\xa2"));

    /** E9: LATIN SMALL LETTER E WITH ACUTE */
    YAZ_CHECK(tst_convert(cd, "Cours de math\xe9", "Cours de mathâe"));
    YAZ_CHECK(tst_convert(cd, "Cours de math", "Cours de math"
                  ));
    YAZ_CHECK(tst_convert(cd, "Cours de mathé", "Cours de mathâe" ));
    YAZ_CHECK(tst_convert(cd, "12345678é","12345678âe"));
    YAZ_CHECK(tst_convert(cd, "123456789é", "123456789âe"));
    YAZ_CHECK(tst_convert(cd, "1234567890é","1234567890âe"));
    YAZ_CHECK(tst_convert(cd, "12345678901é", "12345678901âe"));
    YAZ_CHECK(tst_convert(cd, "Cours de mathém", "Cours de mathâem"));
    YAZ_CHECK(tst_convert(cd, "Cours de mathématiques",
                          "Cours de mathâematiques"));
    yaz_iconv_close(cd);
}

static void tst_utf8_codes(void)
{
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
}

static void tst_danmarc_to_latin1(void)
{
    yaz_iconv_t cd = yaz_iconv_open("iso-8859-1", "danmarc");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK(tst_convert(cd, "ax", "ax"));

    YAZ_CHECK(tst_convert(cd, "a@@b", "a@b"));
    YAZ_CHECK(tst_convert(cd, "a@@@@b", "a@@b"));
    YAZ_CHECK(tst_convert(cd, "@000ab", "\nb"));

    YAZ_CHECK(tst_convert(cd, "@\xe5", "aa"));
    YAZ_CHECK(tst_convert(cd, "@\xc5.", "Aa."));
    
    yaz_iconv_close(cd);
}


int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);

    tst_utf8_codes();

    tst_marc8_to_utf8();

    tst_marc8s_to_utf8();

    tst_marc8_to_latin1();

    tst_advance_to_utf8();
    tst_utf8_to_advance();

    tst_utf8_to_marc8("marc8");
    tst_utf8_to_marc8("marc8lossy");
    tst_utf8_to_marc8("marc8lossless");

    tst_danmarc_to_latin1();

    tst_latin1_to_marc8();

    tst_marc8_to_ucs4b();
    tst_ucs4b_to_utf8();

    dconvert(1, "UTF-8");
    dconvert(1, "ISO-8859-1");
    dconvert(1, "UCS4");
    dconvert(1, "UCS4LE");
    dconvert(0, "CP865");

    YAZ_CHECK_TERM;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

