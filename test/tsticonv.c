/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tsticonv.c,v 1.5 2005-01-15 19:47:15 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <yaz/yaz-util.h>

/* some test strings in ISO-8859-1 format */
static const char *iso_8859_1_a[] = {
    "ax" ,
    "\330",
    "eneb\346r",
    0 };

/* same test strings in MARC-8 format */
static const char *marc8_a[] = {
    "ax",   
    "\xa2",          /* latin capital letter o with stroke */
    "eneb\xb5r",     /* latin small letter ae */
    0
};

static void marc8_tst_a()
{
    int i;
    yaz_iconv_t cd;

    cd = yaz_iconv_open("ISO-8859-1", "MARC8");
    if (!cd)
    {
	printf("tsticonv 10 yaz_iconv_open failed\n");
	exit(10);
    }
    for (i = 0; iso_8859_1_a[i]; i++)
    {
        size_t r;
        char *inbuf= (char*) marc8_a[i];
        size_t inbytesleft = strlen(inbuf);
        char outbuf0[24];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0);

        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        if (r == (size_t) (-1))
        {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv 11 i=%d e=%d\n", i, e);
	    exit(11);
        }
        if ((outbuf - outbuf0) != strlen(iso_8859_1_a[i]) 
            || memcmp(outbuf0, iso_8859_1_a[i],
		      strlen(iso_8859_1_a[i])))
        {
            printf ("tsticonv 12 i=%d\n", i);
            printf ("buf=%s   out=%s\n", iso_8859_1_a[i], outbuf0);
	    exit(12);
        }
    }
    yaz_iconv_close(cd);
}

static void marc8_tst_b()
{
    static const char *marc8_b[] = {
	/* 0 */	
	"\033$1" "\x21\x2B\x3B" /* FF1F */ "\033(B" "o",
	/* 1 */ 
	"\033$1" "\x6F\x77\x29" /* AE0E */ "\x6F\x52\x7C" /* c0F4 */ "\033(B",
	/* 2 */ 
	"\033$1"
	"\x21\x50\x6E"  /* UCS 7CFB */
	"\x21\x51\x31"  /* UCS 7D71 */
	"\x21\x3A\x67"  /* UCS 5B89 */
	"\x21\x33\x22"  /* UCS 5168 */
	"\x21\x33\x53"  /* UCS 5206 */
	"\x21\x44\x2B"  /* UCS 6790 */
	"\033(B",
	/* 3 */
	"\xB0\xB2",     /* AYN and oSLASH */
	/* 4 */
	"\xF6\x61",     /* a underscore */
	/* 5 */
	"\x61\xC2",     /* a, phonorecord mark */
	0
    };
    static const char *ucs4_b[] = {
	"\x00\x00\xFF\x1F" "\x00\x00\x00o",
	"\x00\x00\xAE\x0E" "\x00\x00\xC0\xF4",
	"\x00\x00\x7C\xFB"
	"\x00\x00\x7D\x71"
	"\x00\x00\x5B\x89"
	"\x00\x00\x51\x68"
	"\x00\x00\x52\x06"
	"\x00\x00\x67\x90",
	"\x00\x00\x02\xBB"  "\x00\x00\x00\xF8",
	"\x00\x00\x00\x61"  "\x00\x00\x03\x32",
	"\x00\x00\x00\x61"  "\x00\x00\x21\x17",
	0
    };
    int i;
    yaz_iconv_t cd;

    cd = yaz_iconv_open("UCS4", "MARC8");
    if (!cd)
    {
	printf ("tsticonv 20 yaz_iconv_open failed\n");
	exit(20);
    }
    for (i = 0; marc8_b[i]; i++)
    {
        size_t r;
	size_t len;
	size_t expect_len = i == 2 ? 24 : 8;
        char *inbuf= (char*) marc8_b[i];
        size_t inbytesleft = strlen(inbuf);
        char outbuf0[24];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0);

        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        if (r == (size_t) (-1))
        {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv 21 i=%d e=%d\n", i, e);
	    exit(21);
        }
	len = outbuf - outbuf0;
	if (len != expect_len || memcmp(outbuf0, ucs4_b[i], len))
        {
            printf ("tsticonv 22 len=%d gotlen=%d i=%d\n", expect_len, len, i);
	    exit(22);
        }
    }
    yaz_iconv_close(cd);
}

static void marc8_tst_c()
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
    yaz_iconv_t cd;

    cd = yaz_iconv_open("UTF8", "UCS4");
    if (!cd)
    {
	printf ("tsticonv 30 yaz_iconv_open failed\n");
	exit(30);
    }
    for (i = 0; ucs4_c[i]; i++)
    {
        size_t r;
	size_t len;
        char *inbuf= (char*) ucs4_c[i];
        size_t inbytesleft = 8;
        char outbuf0[24];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0);

        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        if (r == (size_t) (-1))
        {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv 31 i=%d e=%d\n", i, e);
	    exit(31);
        }
	len = outbuf - outbuf0;
	if (len != strlen(utf8_c[i]) || memcmp(outbuf0, utf8_c[i], len))
        {
            printf ("tsticonv 32 len=%d gotlen=%d i=%d\n",
		    strlen(utf8_c[i]), len, i);
	    exit(32);
        }
    }
    yaz_iconv_close(cd);
}

static void dconvert(int mandatory, const char *tmpcode)
{
    int i;
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
	if (!cd)
        {
            if (!mandatory)
                return;
            printf ("tsticonv code=%s 1\n", tmpcode);
	    exit(1);
        }
	r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	if (r == (size_t)(-1))
        {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv code=%s 2 e=%d\n", tmpcode, e);
	    exit(2);
        }
	yaz_iconv_close(cd);
        
	cd = yaz_iconv_open("ISO-8859-1", tmpcode);
	if (!cd)
        {
            if (!mandatory)
                return;
            printf ("tsticonv code=%s 3\n", tmpcode);
	    exit(3);
        }
	inbuf = outbuf0;
	inbytesleft = sizeof(outbuf0) - outbytesleft;

	outbuf = outbuf1;
	outbytesleft = sizeof(outbuf1);
	r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	if (r == (size_t)(-1)) {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv code=%s 4 e=%d\n", tmpcode, e);
	    exit(4);
	}
	if (strlen(iso_8859_1_a[i]) == 
	    (sizeof(outbuf1) - outbytesleft) &&
            memcmp(outbuf1, iso_8859_1_a[i],
		   strlen(iso_8859_1_a[i])))
        {
            printf ("tsticonv code=%s 5\n", tmpcode);
            exit(5);
	}
	yaz_iconv_close(cd);
    }
}
	
int main (int argc, char **argv)
{
    dconvert(1, "UTF-8");
    dconvert(1, "ISO-8859-1");
    dconvert(1, "UCS4");
    dconvert(1, "UCS4LE");
    dconvert(0, "CP865");
    marc8_tst_a();
    marc8_tst_b();
    marc8_tst_c();
    exit (0);
}
