/*
 * Copyright (c) 2002-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: tsticonv.c,v 1.5 2003-06-22 11:48:33 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <yaz/yaz-util.h>

/* some test strings in ISO-8859-1 format */
const char *buf[] = {
    "ax" ,
    "\330",
    "eneb\346r",
    "\xfc",
    "\xfb",
    "\xfbr",
    0 };

/* some test strings in MARC-8 format */
const char *marc8_strings[] = {
    "ax",   
    "\xa2",          /* latin capital letter o with stroke */
    "eneb\xb5r",     /* latin small letter ae */
    "\xe8\x75",      /* latin small letter u with umlaut */
    "\xe3\x75",      /* latin small letter u with circumflex */
    "\xe3\x75r",     /* latin small letter u with circumflex */
    0
};

static void marc8_tst()
{
    int i;
    yaz_iconv_t cd;

    cd = yaz_iconv_open("ISO-8859-1", "MARC8");
    for (i = 0; buf[i]; i++)
    {
        size_t r;
        char *inbuf= (char*) marc8_strings[i];
        size_t inbytesleft = strlen(inbuf);
        char outbuf0[24];
        char *outbuf = outbuf0;
        size_t outbytesleft = sizeof(outbuf0);

        r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        if (r == (size_t) (-1))
        {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv 6 i=%d e=%d\n", i, e);
	    exit(6);
        }
        if ((outbuf - outbuf0) != strlen(buf[i]) 
            || memcmp(outbuf0, buf[i], strlen(buf[i])))
        {
            printf ("tsticonv 7 i=%d\n", i);
            printf ("buf=%s   out=%s\n", buf[i], outbuf0);
	    exit(7);
        }
    }
    yaz_iconv_close(cd);
}

static void dconvert(int mandatory, const char *tmpcode)
{
    int i;
    yaz_iconv_t cd;
    for (i = 0; buf[i]; i++)
    {
        size_t r;
	char *inbuf = (char*) buf[i];
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
            printf ("tsticonv 1\n");
	    exit(1);
        }
	r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	if (r == (size_t)(-1))
        {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv 2 e=%d\n", e);
	    exit(2);
        }
	yaz_iconv_close(cd);
        
	cd = yaz_iconv_open("ISO-8859-1", tmpcode);
	if (!cd)
        {
            if (!mandatory)
                return;
            printf ("tsticonv 3\n");
	    exit(3);
        }
	inbuf = outbuf0;
	inbytesleft = sizeof(outbuf0) - outbytesleft;

	outbuf = outbuf1;
	outbytesleft = sizeof(outbuf1);
	r = yaz_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	if (r == (size_t)(-1)) {
            int e = yaz_iconv_error(cd);

            printf ("tsticonv 4 e=%d\n", e);
	    exit(4);
	}
	if (strlen(buf[i]) == (sizeof(outbuf1) - outbytesleft) &&
            memcmp(outbuf1, buf[i], strlen(buf[i])))
        {
            printf ("tsticonv 5\n");
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
    dconvert(0, "CP865");
    marc8_tst();
    exit (0);
}
