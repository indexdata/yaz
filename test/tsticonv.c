/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tsticonv.c,v 1.9 2005-05-08 07:35:23 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <yaz/yaz-util.h>

static int compare_buffers(char *msg, int no,
			   int expect_len, const unsigned char *expect_buf,
			   int got_len, const unsigned char *got_buf)
{
    int i;
    if (expect_len == got_len
	&& !memcmp(expect_buf, got_buf, expect_len))
	return 1;
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
    exit(1);
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
        char outbuf0[32];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0);

        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        if (r == (size_t) (-1))
        {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv 11 i=%d e=%d\n", i, e);
	    exit(11);
        }
	compare_buffers("tsticonv 11", i,
			strlen(iso_8859_1_a[i]), iso_8859_1_a[i],
			outbuf - outbuf0, outbuf0);
    }
    yaz_iconv_close(cd);
}

static void tst_marc8_to_ucs4b()
{
    static struct {
	const unsigned char *marc8_b;
	int len;
	const unsigned char *ucs4_b;
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
    {
	0, 0, 0
    }
    };
    int i;
    yaz_iconv_t cd;

    cd = yaz_iconv_open("UCS4", "MARC8");
    if (!cd)
    {
	printf ("tsticonv 20 yaz_iconv_open failed\n");
	exit(20);
    }
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
		if (e != YAZ_ICONV_E2BIG)
	        {
                    printf ("tsticonv 21 i=%d e=%d\n", i, e);
	            exit(21);
		}
	    }
	    else
		break;
        }
	compare_buffers("tsticonv 22", i,
			expect_len, ar[i].ucs4_b,
			outbuf - outbuf0, outbuf0);
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
	compare_buffers("tsticonv 32", i,
			strlen(utf8_c[i]), utf8_c[i],
			outbuf - outbuf0, outbuf0);
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
            printf ("tsticonv code=%s i=%d 1\n", tmpcode, i);
	    exit(1);
        }
	r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	if (r == (size_t)(-1))
        {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv code=%s i=%d 2 e=%d\n", tmpcode, i, e);
	    exit(2);
        }
	yaz_iconv_close(cd);
        
	cd = yaz_iconv_open("ISO-8859-1", tmpcode);
	if (!cd)
        {
            if (!mandatory)
                return;
            printf ("tsticonv code=%s i=%d 3\n", tmpcode, i);
	    exit(3);
        }
	inbuf = outbuf0;
	inbytesleft = sizeof(outbuf0) - outbytesleft;

	outbuf = outbuf1;
	outbytesleft = sizeof(outbuf1);
	r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	if (r == (size_t)(-1)) {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv code=%s i=%d 4 e=%d\n", tmpcode, i, e);
	    exit(4);
	}
	compare_buffers("dconvert", i,
			strlen(iso_8859_1_a[i]), iso_8859_1_a[i],
			sizeof(outbuf1) - outbytesleft, outbuf1);
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
    tst_marc8_to_iso_8859_1();
    tst_marc8_to_ucs4b();
    tst_ucs4b_to_utf8();
    exit(0);
}
