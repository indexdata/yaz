/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: siconv.c,v 1.17 2006-02-10 12:45:39 adam Exp $
 */
/**
 * \file siconv.c
 * \brief Implements simple ICONV
 *
 * This implements an interface similar to that of iconv and
 * is used by YAZ to interface with iconv (if present).
 * For systems where iconv is not present, this layer
 * provides a few important conversion: UTF-8, MARC-8, Latin-1.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <ctype.h>
#if HAVE_WCHAR_H
#include <wchar.h>
#endif

#if HAVE_ICONV_H
#include <iconv.h>
#endif

#include <yaz/yaz-util.h>

unsigned long yaz_marc8_1_conv (unsigned char *inp, size_t inbytesleft,
                              size_t *no_read, int *combining);
unsigned long yaz_marc8_2_conv (unsigned char *inp, size_t inbytesleft,
                                size_t *no_read, int *combining);
unsigned long yaz_marc8_3_conv (unsigned char *inp, size_t inbytesleft,
                                size_t *no_read, int *combining);
unsigned long yaz_marc8_4_conv (unsigned char *inp, size_t inbytesleft,
                                size_t *no_read, int *combining);
unsigned long yaz_marc8_5_conv (unsigned char *inp, size_t inbytesleft,
                                size_t *no_read, int *combining);
unsigned long yaz_marc8_6_conv (unsigned char *inp, size_t inbytesleft,
                                size_t *no_read, int *combining);
unsigned long yaz_marc8_7_conv (unsigned char *inp, size_t inbytesleft,
                                size_t *no_read, int *combining);
unsigned long yaz_marc8_8_conv (unsigned char *inp, size_t inbytesleft,
                                size_t *no_read, int *combining);
unsigned long yaz_marc8_9_conv (unsigned char *inp, size_t inbytesleft,
                                size_t *no_read, int *combining);
    
struct yaz_iconv_struct {
    int my_errno;
    int init_flag;
    size_t (*init_handle)(yaz_iconv_t cd, unsigned char *inbuf,
                          size_t inbytesleft, size_t *no_read);
    unsigned long (*read_handle)(yaz_iconv_t cd, unsigned char *inbuf,
                                 size_t inbytesleft, size_t *no_read);
    size_t (*write_handle)(yaz_iconv_t cd, unsigned long x,
                           char **outbuf, size_t *outbytesleft,
                           int last);
    int marc8_esc_mode;

    int comb_offset;
    int comb_size;
    unsigned long comb_x[8];
    size_t comb_no_read[8];
    size_t no_read_x;
    unsigned long unget_x;
#if HAVE_ICONV_H
    iconv_t iconv_cd;
#endif
    unsigned long compose_char;
};

static unsigned long yaz_read_ISO8859_1 (yaz_iconv_t cd, unsigned char *inp,
                                         size_t inbytesleft, size_t *no_read)
{
    unsigned long x = inp[0];
    *no_read = 1;
    return x;
}

static size_t yaz_init_UTF8 (yaz_iconv_t cd, unsigned char *inp,
                             size_t inbytesleft, size_t *no_read)
{
    if (inp[0] != 0xef)
    {
        *no_read = 0;
        return 0;
    }
    if (inbytesleft < 3)
    {
        cd->my_errno = YAZ_ICONV_EINVAL;
        return (size_t) -1;
    }
    if (inp[1] != 0xbb || inp[2] != 0xbf)
    {
        cd->my_errno = YAZ_ICONV_EILSEQ;
        return (size_t) -1;
    }
    *no_read = 3;
    return 0;
}

static unsigned long yaz_read_UTF8 (yaz_iconv_t cd, unsigned char *inp,
                                    size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;

    if (inp[0] <= 0x7f)
    {
        x = inp[0];
        *no_read = 1;
    }
    else if (inp[0] <= 0xbf || inp[0] >= 0xfe)
    {
        *no_read = 0;
        cd->my_errno = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xdf && inbytesleft >= 2)
    {
        x = ((inp[0] & 0x1f) << 6) | (inp[1] & 0x3f);
        if (x >= 0x80)
            *no_read = 2;
        else
        {
            *no_read = 0;
            cd->my_errno = YAZ_ICONV_EILSEQ;
        }
    }
    else if (inp[0] <= 0xef && inbytesleft >= 3)
    {
        x = ((inp[0] & 0x0f) << 12) | ((inp[1] & 0x3f) << 6) |
            (inp[2] & 0x3f);
        if (x >= 0x800)
            *no_read = 3;
        else
        {
            *no_read = 0;
            cd->my_errno = YAZ_ICONV_EILSEQ;
        }
    }
    else if (inp[0] <= 0xf7 && inbytesleft >= 4)
    {
        x =  ((inp[0] & 0x07) << 18) | ((inp[1] & 0x3f) << 12) |
            ((inp[2] & 0x3f) << 6) | (inp[3] & 0x3f);
        if (x >= 0x10000)
            *no_read = 4;
        else
        {
            *no_read = 0;
            cd->my_errno = YAZ_ICONV_EILSEQ;
        }
    }
    else if (inp[0] <= 0xfb && inbytesleft >= 5)
    {
        x =  ((inp[0] & 0x03) << 24) | ((inp[1] & 0x3f) << 18) |
            ((inp[2] & 0x3f) << 12) | ((inp[3] & 0x3f) << 6) |
            (inp[4] & 0x3f);
        if (x >= 0x200000)
            *no_read = 5;
        else
        {
            *no_read = 0;
            cd->my_errno = YAZ_ICONV_EILSEQ;
        }
    }
    else if (inp[0] <= 0xfd && inbytesleft >= 6)
    {
        x =  ((inp[0] & 0x01) << 30) | ((inp[1] & 0x3f) << 24) |
            ((inp[2] & 0x3f) << 18) | ((inp[3] & 0x3f) << 12) |
            ((inp[4] & 0x3f) << 6) | (inp[5] & 0x3f);
        if (x >= 0x4000000)
            *no_read = 6;
        else
        {
            *no_read = 0;
            cd->my_errno = YAZ_ICONV_EILSEQ;
        }
    }
    else
    {
        *no_read = 0;
        cd->my_errno = YAZ_ICONV_EINVAL;
    }
    return x;
}

static unsigned long yaz_read_UCS4 (yaz_iconv_t cd, unsigned char *inp,
                                    size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    
    if (inbytesleft < 4)
    {
        cd->my_errno = YAZ_ICONV_EINVAL; /* incomplete input */
        *no_read = 0;
    }
    else
    {
        x = (inp[0]<<24) | (inp[1]<<16) | (inp[2]<<8) | inp[3];
        *no_read = 4;
    }
    return x;
}

static unsigned long yaz_read_UCS4LE (yaz_iconv_t cd, unsigned char *inp,
                                      size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    
    if (inbytesleft < 4)
    {
        cd->my_errno = YAZ_ICONV_EINVAL; /* incomplete input */
        *no_read = 0;
    }
    else
    {
        x = (inp[3]<<24) | (inp[2]<<16) | (inp[1]<<8) | inp[0];
        *no_read = 4;
    }
    return x;
}

#if HAVE_WCHAR_H
static unsigned long yaz_read_wchar_t (yaz_iconv_t cd, unsigned char *inp,
                                       size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    
    if (inbytesleft < sizeof(wchar_t))
    {
        cd->my_errno = YAZ_ICONV_EINVAL; /* incomplete input */
        *no_read = 0;
    }
    else
    {
        wchar_t wch;
        memcpy (&wch, inp, sizeof(wch));
        x = wch;
        *no_read = sizeof(wch);
    }
    return x;
}
#endif


static unsigned long yaz_read_marc8_comb (yaz_iconv_t cd, unsigned char *inp,
                                          size_t inbytesleft, size_t *no_read,
                                          int *comb);

static unsigned long yaz_read_marc8 (yaz_iconv_t cd, unsigned char *inp,
                                     size_t inbytesleft, size_t *no_read)
{
    unsigned long x;
    if (cd->comb_offset < cd->comb_size)
    {
        *no_read = cd->comb_no_read[cd->comb_offset];
        x = cd->comb_x[cd->comb_offset];

        /* special case for double-diacritic combining characters, 
           INVERTED BREVE and DOUBLE TILDE.
           We'll increment the no_read counter by 1, since we want to skip over
           the processing of the closing ligature character
        */
        /* this code is no longer necessary.. our handlers code in
           yaz_marc8_?_conv (generated by charconv.tcl) now returns
           0 and no_read=1 when a sequence does not match the input.
           The SECOND HALFs in codetables.xml produces a non-existant
           entry in the conversion trie.. Hence when met, the input byte is
           skipped as it should (in yaz_iconv)
        */
#if 0
        if (x == 0x0361 || x == 0x0360)
            *no_read += 1;
#endif
        cd->comb_offset++;
        return x;
    }

    cd->comb_offset = 0;
    for (cd->comb_size = 0; cd->comb_size < 8; cd->comb_size++)
    {
        int comb = 0;
        x = yaz_read_marc8_comb(cd, inp, inbytesleft, no_read, &comb);
        if (!comb || !x)
            break;
        cd->comb_x[cd->comb_size] = x;
        cd->comb_no_read[cd->comb_size] = *no_read;
        inp += *no_read;
        inbytesleft = inbytesleft - *no_read;
    }
    return x;
}

static unsigned long yaz_read_marc8_comb (yaz_iconv_t cd, unsigned char *inp,
                                          size_t inbytesleft, size_t *no_read,
                                          int *comb)
{
    *no_read = 0;
    while(inbytesleft >= 1 && inp[0] == 27)
    {
        size_t inbytesleft0 = inbytesleft;
        inp++;
        inbytesleft--;
        while(inbytesleft > 0 && strchr("(,$!", *inp))
        {
            inbytesleft--;
            inp++;
        }
        if (inbytesleft <= 0)
        {
            *no_read = 0;
            cd->my_errno = YAZ_ICONV_EINVAL;
            return 0;
        }
        cd->marc8_esc_mode = *inp++;
        inbytesleft--;
        (*no_read) += inbytesleft0 - inbytesleft;
    }
    if (inbytesleft <= 0)
        return 0;
    else
    {
        unsigned long x;
        size_t no_read_sub = 0;
        *comb = 0;

        switch(cd->marc8_esc_mode)
        {
        case 'B':  /* Basic ASCII */
        case 'E':  /* ANSEL */
        case 's':  /* ASCII */
            x = yaz_marc8_1_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'g':  /* Greek */
            x = yaz_marc8_2_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'b':  /* Subscripts */
            x = yaz_marc8_3_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'p':  /* Superscripts */
            x = yaz_marc8_4_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case '2':  /* Basic Hebrew */
            x = yaz_marc8_5_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'N':  /* Basic Cyrillic */
        case 'Q':  /* Extended Cyrillic */
            x = yaz_marc8_6_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case '3':  /* Basic Arabic */
        case '4':  /* Extended Arabic */
            x = yaz_marc8_7_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'S':  /* Greek */
            x = yaz_marc8_8_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case '1':  /* Chinese, Japanese, Korean (EACC) */
            x = yaz_marc8_9_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        default:
            *no_read = 0;
            cd->my_errno = YAZ_ICONV_EILSEQ;
            return 0;
        }
        *no_read += no_read_sub;
        return x;
    }
}

static size_t yaz_write_UTF8 (yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft,
                              int last)
{
    unsigned char *outp = (unsigned char *) *outbuf;
    if (x <= 0x7f && *outbytesleft >= 1)
    {
        *outp++ = (unsigned char) x;
        (*outbytesleft)--;
    } 
    else if (x <= 0x7ff && *outbytesleft >= 2)
    {
        *outp++ = (unsigned char) ((x >> 6) | 0xc0);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 2;
    }
    else if (x <= 0xffff && *outbytesleft >= 3)
    {
        *outp++ = (unsigned char) ((x >> 12) | 0xe0);
        *outp++ = (unsigned char) (((x >> 6) & 0x3f) | 0x80);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 3;
    }
    else if (x <= 0x1fffff && *outbytesleft >= 4)
    {
        *outp++ = (unsigned char) ((x >> 18) | 0xf0);
        *outp++ = (unsigned char) (((x >> 12) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 6)  & 0x3f) | 0x80);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 4;
    }
    else if (x <= 0x3ffffff && *outbytesleft >= 5)
    {
        *outp++ = (unsigned char) ((x >> 24) | 0xf8);
        *outp++ = (unsigned char) (((x >> 18) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 12) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 6)  & 0x3f) | 0x80);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 5;
    }
    else if (*outbytesleft >= 6)
    {
        *outp++ = (unsigned char) ((x >> 30) | 0xfc);
        *outp++ = (unsigned char) (((x >> 24) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 18) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 12) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 6)  & 0x3f) | 0x80);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 6;
    }
    else 
    {
        cd->my_errno = YAZ_ICONV_E2BIG;  /* not room for output */
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}


static size_t yaz_write_ISO8859_1 (yaz_iconv_t cd, unsigned long x,
                                   char **outbuf, size_t *outbytesleft,
                                   int last)
{
    /* list of two char unicode sequence that, when combined, are
       equivalent to single unicode chars that can be represented in
       ISO-8859-1/Latin-1.
       Regular iconv on Linux at least does not seem to convert these,
       but since MARC-8 to UTF-8 generates these composed sequence
       we get a better chance of a successful MARC-8 -> ISO-8859-1
       conversion */
    static struct {
        unsigned long x1, x2;
        unsigned y;
    } comb[] = {
        { 'A', 0x0300, 0xc0}, /* LATIN CAPITAL LETTER A WITH GRAVE */
        { 'A', 0x0301, 0xc1}, /* LATIN CAPITAL LETTER A WITH ACUTE */
        { 'A', 0x0302, 0xc2}, /* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
        { 'A', 0x0303, 0xc3}, /* LATIN CAPITAL LETTER A WITH TILDE */
        { 'A', 0x0308, 0xc4}, /* LATIN CAPITAL LETTER A WITH DIAERESIS */
        { 'A', 0x030a, 0xc5}, /* LATIN CAPITAL LETTER A WITH RING ABOVE */
        /* no need for 0xc6      LATIN CAPITAL LETTER AE */
        { 'C', 0x0327, 0xc7}, /* LATIN CAPITAL LETTER C WITH CEDILLA */
        { 'E', 0x0300, 0xc8}, /* LATIN CAPITAL LETTER E WITH GRAVE */
        { 'E', 0x0301, 0xc9}, /* LATIN CAPITAL LETTER E WITH ACUTE */
        { 'E', 0x0302, 0xca}, /* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
        { 'E', 0x0308, 0xcb}, /* LATIN CAPITAL LETTER E WITH DIAERESIS */
        { 'I', 0x0300, 0xcc}, /* LATIN CAPITAL LETTER I WITH GRAVE */
        { 'I', 0x0301, 0xcd}, /* LATIN CAPITAL LETTER I WITH ACUTE */
        { 'I', 0x0302, 0xce}, /* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
        { 'I', 0x0308, 0xcf}, /* LATIN CAPITAL LETTER I WITH DIAERESIS */
        { 'N', 0x0303, 0xd1}, /* LATIN CAPITAL LETTER N WITH TILDE */
        { 'O', 0x0300, 0xd2}, /* LATIN CAPITAL LETTER O WITH GRAVE */
        { 'O', 0x0301, 0xd3}, /* LATIN CAPITAL LETTER O WITH ACUTE */
        { 'O', 0x0302, 0xd4}, /* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
        { 'O', 0x0303, 0xd5}, /* LATIN CAPITAL LETTER O WITH TILDE */
        { 'O', 0x0308, 0xd6}, /* LATIN CAPITAL LETTER O WITH DIAERESIS */
        /* omitted:    0xd7      MULTIPLICATION SIGN */
        /* omitted:    0xd8      LATIN CAPITAL LETTER O WITH STROKE */
        { 'U', 0x0300, 0xd9}, /* LATIN CAPITAL LETTER U WITH GRAVE */
        { 'U', 0x0301, 0xda}, /* LATIN CAPITAL LETTER U WITH ACUTE */
        { 'U', 0x0302, 0xdb}, /* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
        { 'U', 0x0308, 0xdc}, /* LATIN CAPITAL LETTER U WITH DIAERESIS */
        { 'Y', 0x0301, 0xdd}, /* LATIN CAPITAL LETTER Y WITH ACUTE */
        /* omitted:    0xde      LATIN CAPITAL LETTER THORN */
        /* omitted:    0xdf      LATIN SMALL LETTER SHARP S */
        { 'a', 0x0300, 0xe0}, /* LATIN SMALL LETTER A WITH GRAVE */
        { 'a', 0x0301, 0xe1}, /* LATIN SMALL LETTER A WITH ACUTE */
        { 'a', 0x0302, 0xe2}, /* LATIN SMALL LETTER A WITH CIRCUMFLEX */
        { 'a', 0x0303, 0xe3}, /* LATIN SMALL LETTER A WITH TILDE */
        { 'a', 0x0308, 0xe4}, /* LATIN SMALL LETTER A WITH DIAERESIS */
        { 'a', 0x030a, 0xe5}, /* LATIN SMALL LETTER A WITH RING ABOVE */
        /* omitted:    0xe6      LATIN SMALL LETTER AE */
        { 'c', 0x0327, 0xe7}, /* LATIN SMALL LETTER C WITH CEDILLA */
        { 'e', 0x0300, 0xe8}, /* LATIN SMALL LETTER E WITH GRAVE */
        { 'e', 0x0301, 0xe9}, /* LATIN SMALL LETTER E WITH ACUTE */
        { 'e', 0x0302, 0xea}, /* LATIN SMALL LETTER E WITH CIRCUMFLEX */
        { 'e', 0x0308, 0xeb}, /* LATIN SMALL LETTER E WITH DIAERESIS */
        { 'i', 0x0300, 0xec}, /* LATIN SMALL LETTER I WITH GRAVE */
        { 'i', 0x0301, 0xed}, /* LATIN SMALL LETTER I WITH ACUTE */
        { 'i', 0x0302, 0xee}, /* LATIN SMALL LETTER I WITH CIRCUMFLEX */
        { 'i', 0x0308, 0xef}, /* LATIN SMALL LETTER I WITH DIAERESIS */
        /* omitted:    0xf0      LATIN SMALL LETTER ETH */
        { 'n', 0x0303, 0xf1}, /* LATIN SMALL LETTER N WITH TILDE */
        { 'o', 0x0300, 0xf2}, /* LATIN SMALL LETTER O WITH GRAVE */
        { 'o', 0x0301, 0xf3}, /* LATIN SMALL LETTER O WITH ACUTE */
        { 'o', 0x0302, 0xf4}, /* LATIN SMALL LETTER O WITH CIRCUMFLEX */
        { 'o', 0x0303, 0xf5}, /* LATIN SMALL LETTER O WITH TILDE */
        { 'o', 0x0308, 0xf6}, /* LATIN SMALL LETTER O WITH DIAERESIS */
        /* omitted:    0xf7      DIVISION SIGN */
        /* omitted:    0xf8      LATIN SMALL LETTER O WITH STROKE */
        { 'u', 0x0300, 0xf9}, /* LATIN SMALL LETTER U WITH GRAVE */
        { 'u', 0x0301, 0xfa}, /* LATIN SMALL LETTER U WITH ACUTE */
        { 'u', 0x0302, 0xfb}, /* LATIN SMALL LETTER U WITH CIRCUMFLEX */
        { 'u', 0x0308, 0xfc}, /* LATIN SMALL LETTER U WITH DIAERESIS */
        { 'y', 0x0301, 0xfd}, /* LATIN SMALL LETTER Y WITH ACUTE */
        /* omitted:    0xfe      LATIN SMALL LETTER THORN */
        { 'y', 0x0308, 0xff}, /* LATIN SMALL LETTER Y WITH DIAERESIS */
        
        { 0, 0, 0}
    };
    unsigned char *outp = (unsigned char *) *outbuf;

    if (!last && x > 32 && x < 127 && cd->compose_char == 0)
    {
        cd->compose_char = x;
        return 0;
    }
    else if (cd->compose_char)
    {
        int i;
        for (i = 0; comb[i].x1; i++)
            if (cd->compose_char == comb[i].x1 && x == comb[i].x2)
            {
                x = comb[i].y;
                break;
            }
        if (!comb[i].x1) 
        {   /* not found */
            if (*outbytesleft >= 1)
            {
                *outp++ = (unsigned char) cd->compose_char;
                (*outbytesleft)--;
                *outbuf = (char *) outp;
                if (!last && x > 32 && x < 127)
                {
                    cd->compose_char = x;
                    return 0;
                }
            }
            else
            {
                cd->my_errno = YAZ_ICONV_E2BIG;
                return (size_t)(-1);
            }
        }
        /* compose_char and old x combined to one new char: x */
        cd->compose_char = 0;
    }
    if (x > 255 || x < 1)
    {
        cd->my_errno = YAZ_ICONV_EILSEQ;
        return (size_t) -1;
    }
    else if (*outbytesleft >= 1)
    {
        *outp++ = (unsigned char) x;
        (*outbytesleft)--;
    }
    else 
    {
        cd->my_errno = YAZ_ICONV_E2BIG;
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}


static size_t yaz_write_UCS4 (yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft,
                              int last)
{
    unsigned char *outp = (unsigned char *) *outbuf;
    if (*outbytesleft >= 4)
    {
        *outp++ = (unsigned char) (x>>24);
        *outp++ = (unsigned char) (x>>16);
        *outp++ = (unsigned char) (x>>8);
        *outp++ = (unsigned char) x;
        (*outbytesleft) -= 4;
    }
    else
    {
        cd->my_errno = YAZ_ICONV_E2BIG;
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}

static size_t yaz_write_UCS4LE (yaz_iconv_t cd, unsigned long x,
                                char **outbuf, size_t *outbytesleft,
                                int last)
{
    unsigned char *outp = (unsigned char *) *outbuf;
    if (*outbytesleft >= 4)
    {
        *outp++ = (unsigned char) x;
        *outp++ = (unsigned char) (x>>8);
        *outp++ = (unsigned char) (x>>16);
        *outp++ = (unsigned char) (x>>24);
        (*outbytesleft) -= 4;
    }
    else
    {
        cd->my_errno = YAZ_ICONV_E2BIG;
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}

#if HAVE_WCHAR_H
static size_t yaz_write_wchar_t (yaz_iconv_t cd, unsigned long x,
                                 char **outbuf, size_t *outbytesleft,
                                 int last)
{
    unsigned char *outp = (unsigned char *) *outbuf;

    if (*outbytesleft >= sizeof(wchar_t))
    {
        wchar_t wch = x;
        memcpy(outp, &wch, sizeof(wch));
        outp += sizeof(wch);
        (*outbytesleft) -= sizeof(wch);
    }
    else
    {
        cd->my_errno = YAZ_ICONV_E2BIG;
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}
#endif

int yaz_iconv_isbuiltin(yaz_iconv_t cd)
{
    return cd->read_handle && cd->write_handle;
}

yaz_iconv_t yaz_iconv_open (const char *tocode, const char *fromcode)
{
    yaz_iconv_t cd = (yaz_iconv_t) xmalloc (sizeof(*cd));

    cd->write_handle = 0;
    cd->read_handle = 0;
    cd->init_handle = 0;
    cd->my_errno = YAZ_ICONV_UNKNOWN;
    cd->marc8_esc_mode = 'B';
    cd->comb_offset = cd->comb_size = 0;
    cd->compose_char = 0;

    /* a useful hack: if fromcode has leading @,
       the library not use YAZ's own conversions .. */
    if (fromcode[0] == '@')
        fromcode++;
    else
    {
        if (!yaz_matchstr(fromcode, "UTF8"))
        {
            cd->read_handle = yaz_read_UTF8;
            cd->init_handle = yaz_init_UTF8;
        }
        else if (!yaz_matchstr(fromcode, "ISO88591"))
            cd->read_handle = yaz_read_ISO8859_1;
        else if (!yaz_matchstr(fromcode, "UCS4"))
            cd->read_handle = yaz_read_UCS4;
        else if (!yaz_matchstr(fromcode, "UCS4LE"))
            cd->read_handle = yaz_read_UCS4LE;
        else if (!yaz_matchstr(fromcode, "MARC8"))
            cd->read_handle = yaz_read_marc8;
#if HAVE_WCHAR_H
        else if (!yaz_matchstr(fromcode, "WCHAR_T"))
            cd->read_handle = yaz_read_wchar_t;
#endif
        
        if (!yaz_matchstr(tocode, "UTF8"))
            cd->write_handle = yaz_write_UTF8;
        else if (!yaz_matchstr(tocode, "ISO88591"))
            cd->write_handle = yaz_write_ISO8859_1;
        else if (!yaz_matchstr (tocode, "UCS4"))
            cd->write_handle = yaz_write_UCS4;
        else if (!yaz_matchstr(tocode, "UCS4LE"))
            cd->write_handle = yaz_write_UCS4LE;
#if HAVE_WCHAR_H
        else if (!yaz_matchstr(tocode, "WCHAR_T"))
            cd->write_handle = yaz_write_wchar_t;
#endif
    }
#if HAVE_ICONV_H
    cd->iconv_cd = 0;
    if (!cd->read_handle || !cd->write_handle)
    {
        cd->iconv_cd = iconv_open (tocode, fromcode);
        if (cd->iconv_cd == (iconv_t) (-1))
        {
            xfree (cd);
            return 0;
        }
    }
#else
    if (!cd->read_handle || !cd->write_handle)
    {
        xfree (cd);
        return 0;
    }
#endif
    cd->init_flag = 1;
    return cd;
}

size_t yaz_iconv(yaz_iconv_t cd, char **inbuf, size_t *inbytesleft,
                 char **outbuf, size_t *outbytesleft)
{
    char *inbuf0;
    size_t r = 0;
#if HAVE_ICONV_H
    if (cd->iconv_cd)
    {
        size_t r =
            iconv(cd->iconv_cd, inbuf, inbytesleft, outbuf, outbytesleft);
        if (r == (size_t)(-1))
        {
            switch (yaz_errno())
            {
            case E2BIG:
                cd->my_errno = YAZ_ICONV_E2BIG;
                break;
            case EINVAL:
                cd->my_errno = YAZ_ICONV_EINVAL;
                break;
            case EILSEQ:
                cd->my_errno = YAZ_ICONV_EILSEQ;
                break;
            default:
                cd->my_errno = YAZ_ICONV_UNKNOWN;
            }
        }
        return r;
    }
#endif
    if (inbuf == 0 || *inbuf == 0)
    {
        cd->init_flag = 1;
        cd->my_errno = YAZ_ICONV_UNKNOWN;
        return 0;
    }
    inbuf0 = *inbuf;

    if (cd->init_flag)
    {
        if (cd->init_handle)
        {
            size_t no_read;
            size_t r = (cd->init_handle)(cd, (unsigned char *) *inbuf,
                                         *inbytesleft, &no_read);
            if (r)
            {
                if (cd->my_errno == YAZ_ICONV_EINVAL)
                    return r;
                cd->init_flag = 0;
                return r;
            }
            *inbytesleft -= no_read;
            *inbuf += no_read;
        }
        cd->init_flag = 0;
        cd->unget_x = 0;
        cd->no_read_x = 0;
    }
    while (1)
    {
        unsigned long x;
        size_t no_read;

        if (*inbytesleft == 0)
        {
            r = *inbuf - inbuf0;
            break;
        }
        if (!cd->unget_x)
        {
            x = (cd->read_handle)(cd, (unsigned char *) *inbuf, *inbytesleft,
                                  &no_read);
            if (no_read == 0)
            {
                r = (size_t)(-1);
                break;
            }
        }
        else
        {
            x = cd->unget_x;
            no_read = cd->no_read_x;
        }
        if (x)
        {
            r = (cd->write_handle)(cd, x, outbuf, outbytesleft,
                                   (*inbytesleft - no_read) == 0 ? 1 : 0);
            if (r)
            {
                /* unable to write it. save it because read_handle cannot
                   rewind .. */
                cd->unget_x = x;
                cd->no_read_x = no_read;
                break;
            }
            cd->unget_x = 0;
        }
        *inbytesleft -= no_read;
        (*inbuf) += no_read;
    }
    return r;
}

int yaz_iconv_error (yaz_iconv_t cd)
{
    return cd->my_errno;
}

int yaz_iconv_close (yaz_iconv_t cd)
{
#if HAVE_ICONV_H
    if (cd->iconv_cd)
        iconv_close (cd->iconv_cd);
#endif
    xfree (cd);
    return 0;
}

    
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

