/*
 * Copyright (C) 1995-2008, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: siconv.c,v 1.50 2008-03-12 08:53:28 adam Exp $
 */
/**
 * \file siconv.c
 * \brief Implements simple ICONV
 *
 * This implements an interface similar to that of iconv and
 * is used by YAZ to interface with iconv (if present).
 * For systems where iconv is not present, this layer
 * provides a few important conversions: UTF-8, MARC-8, Latin-1.
 *
 * MARC-8 reference:
 *  http://www.loc.gov/marc/specifications/speccharmarc8.html
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#if HAVE_WCHAR_H
#include <wchar.h>
#endif

#if HAVE_ICONV_H
#include <iconv.h>
#endif

#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include "iconv-p.h"

typedef unsigned long yaz_conv_func_t(unsigned char *inp, size_t inbytesleft,
                                      size_t *no_read, int *combining,
                                      unsigned mask, int boffset);


yaz_conv_func_t yaz_marc8_42_conv;
yaz_conv_func_t yaz_marc8_45_conv;
yaz_conv_func_t yaz_marc8_67_conv;
yaz_conv_func_t yaz_marc8_62_conv;
yaz_conv_func_t yaz_marc8_70_conv;
yaz_conv_func_t yaz_marc8_32_conv;
yaz_conv_func_t yaz_marc8_4E_conv;
yaz_conv_func_t yaz_marc8_51_conv;
yaz_conv_func_t yaz_marc8_33_conv;
yaz_conv_func_t yaz_marc8_34_conv;
yaz_conv_func_t yaz_marc8_53_conv;
yaz_conv_func_t yaz_marc8_31_conv;

yaz_conv_func_t yaz_marc8r_42_conv;
yaz_conv_func_t yaz_marc8r_45_conv;
yaz_conv_func_t yaz_marc8r_67_conv;
yaz_conv_func_t yaz_marc8r_62_conv;
yaz_conv_func_t yaz_marc8r_70_conv;
yaz_conv_func_t yaz_marc8r_32_conv;
yaz_conv_func_t yaz_marc8r_4E_conv;
yaz_conv_func_t yaz_marc8r_51_conv;
yaz_conv_func_t yaz_marc8r_33_conv;
yaz_conv_func_t yaz_marc8r_34_conv;
yaz_conv_func_t yaz_marc8r_53_conv;
yaz_conv_func_t yaz_marc8r_31_conv;

struct yaz_iconv_struct {
    int my_errno;
    int init_flag;
    size_t (*init_handle)(yaz_iconv_t cd, unsigned char *inbuf,
                          size_t inbytesleft, size_t *no_read);
    unsigned long (*read_handle)(yaz_iconv_t cd, unsigned char *inbuf,
                                 size_t inbytesleft, size_t *no_read);
    size_t (*write_handle)(yaz_iconv_t cd, unsigned long x,
                           char **outbuf, size_t *outbytesleft);
    size_t (*flush_handle)(yaz_iconv_t cd,
                           char **outbuf, size_t *outbytesleft);
    int g0_mode;
    int g1_mode;

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

    unsigned write_marc8_second_half_char;
    unsigned long write_marc8_last;
    const char *write_marc8_lpage;
    const char *write_marc8_g0;
    const char *write_marc8_g1;
};


static struct {
    unsigned long x1, x2;
    unsigned y;
} latin1_comb[] = {
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

#define ESC "\033"

static size_t yaz_write_marc8_page_chr(yaz_iconv_t cd, 
                                       char **outbuf, size_t *outbytesleft,
                                       const char *page_chr);

static unsigned long yaz_read_ISO8859_1(yaz_iconv_t cd, unsigned char *inp,
                                        size_t inbytesleft, size_t *no_read)
{
    unsigned long x = inp[0];
    *no_read = 1;
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

        if (inbytesleft == 0 && cd->comb_size)
        {
            cd->my_errno = YAZ_ICONV_EINVAL;
            x = 0;
            *no_read = 0;
            break;
        }
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

static unsigned long yaz_read_marc8s(yaz_iconv_t cd, unsigned char *inp,
                                     size_t inbytesleft, size_t *no_read)
{
    unsigned long x = yaz_read_marc8(cd, inp, inbytesleft, no_read);
    if (x && cd->comb_size == 1)
    {
        /* For MARC8s we try to get a Latin-1 page code out of it */
        int i;
        for (i = 0; latin1_comb[i].x1; i++)
            if (cd->comb_x[0] == latin1_comb[i].x2 && x == latin1_comb[i].x1)
            {
                *no_read += cd->comb_no_read[0];
                cd->comb_size = 0;
                x = latin1_comb[i].y;
                break;
            }
    }
    return x;
}

static unsigned long yaz_read_marc8_comb(yaz_iconv_t cd, unsigned char *inp,
                                         size_t inbytesleft, size_t *no_read,
                                         int *comb)
{
    *no_read = 0;
    while (inbytesleft > 0 && *inp == 27)
    {
        int *modep = &cd->g0_mode;
        size_t inbytesleft0 = inbytesleft;

        inbytesleft--;
        inp++;
        if (inbytesleft == 0)
            goto incomplete;
        if (*inp == '$') /* set with multiple bytes */
        {
            inbytesleft--;
            inp++;
        }
        if (inbytesleft == 0)
            goto incomplete;
        if (*inp == '(' || *inp == ',')  /* G0 */
        {
            inbytesleft--;
            inp++;
        }
        else if (*inp == ')' || *inp == '-') /* G1 */
        {
            inbytesleft--;
            inp++;
            modep = &cd->g1_mode;
        }
        if (inbytesleft == 0)
            goto incomplete;
        if (*inp == '!') /* ANSEL is a special case */
        {
            inbytesleft--;
            inp++;
        }
        if (inbytesleft == 0)
            goto incomplete;
        *modep = *inp++; /* Final character */
        inbytesleft--;

        (*no_read) += inbytesleft0 - inbytesleft;
    }
    if (inbytesleft == 0)
        return 0;
    else if (*inp == ' ')
    {
        *no_read += 1;
        return ' ';
    }
    else
    {
        unsigned long x;
        size_t no_read_sub = 0;
        int mode = *inp < 128 ? cd->g0_mode : cd->g1_mode;
        *comb = 0;

        switch(mode)
        {
        case 'B':  /* Basic ASCII */
        case 's':  /* ASCII */
            x = yaz_marc8_42_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case 'E':  /* ANSEL */
            x = yaz_marc8_45_conv(inp, inbytesleft, &no_read_sub, comb, 127, 128);
            break;
        case 'g':  /* Greek */
            x = yaz_marc8_67_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case 'b':  /* Subscripts */
            x = yaz_marc8_62_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case 'p':  /* Superscripts */
            x = yaz_marc8_70_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case '2':  /* Basic Hebrew */
            x = yaz_marc8_32_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case 'N':  /* Basic Cyrillic */
            x = yaz_marc8_4E_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case 'Q':  /* Extended Cyrillic */
            x = yaz_marc8_51_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case '3':  /* Basic Arabic */
            x = yaz_marc8_33_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case '4':  /* Extended Arabic */
            x = yaz_marc8_34_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case 'S':  /* Greek */
            x = yaz_marc8_53_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        case '1':  /* Chinese, Japanese, Korean (EACC) */
            x = yaz_marc8_31_conv(inp, inbytesleft, &no_read_sub, comb, 127, 0);
            break;
        default:
            *no_read = 0;
            cd->my_errno = YAZ_ICONV_EILSEQ;
            return 0;
        }
        *no_read += no_read_sub;
        return x;
    }
incomplete:
    *no_read = 0;
    cd->my_errno = YAZ_ICONV_EINVAL;
    return 0;
}

static size_t yaz_write_ISO8859_1(yaz_iconv_t cd, unsigned long x,
                                  char **outbuf, size_t *outbytesleft)
{
    /* list of two char unicode sequence that, when combined, are
       equivalent to single unicode chars that can be represented in
       ISO-8859-1/Latin-1.
       Regular iconv on Linux at least does not seem to convert these,
       but since MARC-8 to UTF-8 generates these composed sequence
       we get a better chance of a successful MARC-8 -> ISO-8859-1
       conversion */
    unsigned char *outp = (unsigned char *) *outbuf;

    if (cd->compose_char)
    {
        int i;
        for (i = 0; latin1_comb[i].x1; i++)
            if (cd->compose_char == latin1_comb[i].x1 && x == latin1_comb[i].x2)
            {
                x = latin1_comb[i].y;
                break;
            }
        if (*outbytesleft < 1)
        {  /* no room. Retain compose_char and bail out */
            cd->my_errno = YAZ_ICONV_E2BIG;
            return (size_t)(-1);
        }
        if (!latin1_comb[i].x1) 
        {   /* not found. Just write compose_char */
            *outp++ = (unsigned char) cd->compose_char;
            (*outbytesleft)--;
            *outbuf = (char *) outp;
        }
        /* compose_char used so reset it. x now holds current char */
        cd->compose_char = 0;
    }

    if (x > 32 && x < 127 && cd->compose_char == 0)
    {
        cd->compose_char = x;
        return 0;
    }
    else if (x > 255 || x < 1)
    {
        cd->my_errno = YAZ_ICONV_EILSEQ;
        return (size_t) -1;
    }
    else if (*outbytesleft < 1)
    {
        cd->my_errno = YAZ_ICONV_E2BIG;
        return (size_t)(-1);
    }
    *outp++ = (unsigned char) x;
    (*outbytesleft)--;
    *outbuf = (char *) outp;
    return 0;
}

static size_t yaz_flush_ISO8859_1(yaz_iconv_t cd,
                                  char **outbuf, size_t *outbytesleft)
{
    if (cd->compose_char)
    {
        unsigned char *outp = (unsigned char *) *outbuf;
        if (*outbytesleft < 1)
        {
            cd->my_errno = YAZ_ICONV_E2BIG;
            return (size_t)(-1);
        }
        *outp++ = (unsigned char) cd->compose_char;
        (*outbytesleft)--;
        *outbuf = (char *) outp;
        cd->compose_char = 0;
    }
    return 0;
}

static unsigned long lookup_marc8(yaz_iconv_t cd,
                                  unsigned long x, int *comb,
                                  const char **page_chr)
{
    char utf8_buf[7];
    char *utf8_outbuf = utf8_buf;
    size_t utf8_outbytesleft = sizeof(utf8_buf)-1, r;

    r = yaz_write_UTF8(cd, x, &utf8_outbuf, &utf8_outbytesleft);
    if (r == (size_t)(-1))
    {
        cd->my_errno = YAZ_ICONV_EILSEQ;
        return 0;
    }
    else
    {
        unsigned char *inp;
        size_t inbytesleft, no_read_sub = 0;
        unsigned long x;

        *utf8_outbuf = '\0';        
        inp = (unsigned char *) utf8_buf;
        inbytesleft = strlen(utf8_buf);

        x = yaz_marc8r_42_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(B";
            return x;
        }
        x = yaz_marc8r_45_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(B";
            return x;
        }
        x = yaz_marc8r_62_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "b";
            return x;
        }
        x = yaz_marc8r_70_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "p";
            return x;
        }
        x = yaz_marc8r_32_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(2";
            return x;
        }
        x = yaz_marc8r_4E_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(N";
            return x;
        }
        x = yaz_marc8r_51_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(Q";
            return x;
        }
        x = yaz_marc8r_33_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(3";
            return x;
        }
        x = yaz_marc8r_34_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(4";
            return x;
        }
        x = yaz_marc8r_53_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(S";
            return x;
        }
        x = yaz_marc8r_31_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "$1";
            return x;
        }
        cd->my_errno = YAZ_ICONV_EILSEQ;
        return x;
    }
}

static size_t flush_combos(yaz_iconv_t cd,
                           char **outbuf, size_t *outbytesleft)
{
    unsigned long y = cd->write_marc8_last;
    unsigned char byte;
    char out_buf[4];
    size_t out_no = 0;

    if (!y)
        return 0;

    assert(cd->write_marc8_lpage);
    if (cd->write_marc8_lpage)
    {
        size_t r = yaz_write_marc8_page_chr(cd, outbuf, outbytesleft,
                                            cd->write_marc8_lpage);
        if (r)
            return r;
    }

    byte = (unsigned char )((y>>16) & 0xff);
    if (byte)
        out_buf[out_no++] = byte;
    byte = (unsigned char)((y>>8) & 0xff);
    if (byte)
        out_buf[out_no++] = byte;
    byte = (unsigned char )(y & 0xff);
    if (byte)
        out_buf[out_no++] = byte;

    if (out_no + 2 >= *outbytesleft)
    {
        cd->my_errno = YAZ_ICONV_E2BIG;
        return (size_t) (-1);
    }

    memcpy(*outbuf, out_buf, out_no);
    *outbuf += out_no;
    (*outbytesleft) -= out_no;
    if (cd->write_marc8_second_half_char)
    {
        *(*outbuf)++ = cd->write_marc8_second_half_char;
        (*outbytesleft)--;
    }        

    cd->write_marc8_last = 0;
    cd->write_marc8_lpage = 0;
    cd->write_marc8_second_half_char = 0;
    return 0;
}

static size_t yaz_write_marc8_page_chr(yaz_iconv_t cd, 
                                       char **outbuf, size_t *outbytesleft,
                                       const char *page_chr)
{
    const char **old_page_chr = &cd->write_marc8_g0;

    /* are we going to a G1-set (such as such as ESC ")!E") */
    if (page_chr && page_chr[1] == ')')
        old_page_chr = &cd->write_marc8_g1;

    if (!*old_page_chr || strcmp(page_chr, *old_page_chr))
    {
        size_t plen = 0;
        const char *page_out = page_chr;
        
        if (*outbytesleft < 8)
        {
            cd->my_errno = YAZ_ICONV_E2BIG;
            
            return (size_t) (-1);
        }

        if (*old_page_chr)
        {
            if (!strcmp(*old_page_chr, ESC "p") 
                || !strcmp(*old_page_chr, ESC "g")
                || !strcmp(*old_page_chr, ESC "b"))
            {
                page_out = ESC "s";
                /* Technique 1 leave */
                if (strcmp(page_chr, ESC "(B")) /* Not going ASCII page? */
                {
                    /* Must leave script + enter new page */
                    plen = strlen(page_out);
                    memcpy(*outbuf, page_out, plen);
                    (*outbuf) += plen;
                    (*outbytesleft) -= plen;
                    page_out = ESC "(B";
                }
            }
        }
        *old_page_chr = page_chr;
        plen = strlen(page_out);
        memcpy(*outbuf, page_out, plen);
        (*outbuf) += plen;
        (*outbytesleft) -= plen;
    }
    return 0;
}


static size_t yaz_write_marc8_2(yaz_iconv_t cd, unsigned long x,
                                char **outbuf, size_t *outbytesleft)
{
    int comb = 0;
    const char *page_chr = 0;
    unsigned long y = lookup_marc8(cd, x, &comb, &page_chr);

    if (!y)
        return (size_t) (-1);

    if (comb)
    {
        if (page_chr)
        {
            size_t r = yaz_write_marc8_page_chr(cd, outbuf, outbytesleft,
                                                page_chr);
            if (r)
                return r;
        }
        if (x == 0x0361)
            cd->write_marc8_second_half_char = 0xEC;
        else if (x == 0x0360)
            cd->write_marc8_second_half_char = 0xFB;

        if (*outbytesleft <= 1)
        {
            cd->my_errno = YAZ_ICONV_E2BIG;
            return (size_t) (-1);
        }
        *(*outbuf)++ = y;
        (*outbytesleft)--;
    }
    else
    {
        size_t r = flush_combos(cd, outbuf, outbytesleft);
        if (r)
            return r;

        cd->write_marc8_last = y;
        cd->write_marc8_lpage = page_chr;
    }
    return 0;
}

static size_t yaz_flush_marc8(yaz_iconv_t cd,
                              char **outbuf, size_t *outbytesleft)
{
    size_t r = flush_combos(cd, outbuf, outbytesleft);
    if (r)
        return r;
    cd->write_marc8_g1 = 0;
    return yaz_write_marc8_page_chr(cd, outbuf, outbytesleft, ESC "(B");
}

static size_t yaz_write_marc8(yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft)
{
    int i;
    for (i = 0; latin1_comb[i].x1; i++)
    {
        if (x == latin1_comb[i].y)
        {
            size_t r ;
            /* save the output pointers .. */
            char *outbuf0 = *outbuf;
            size_t outbytesleft0 = *outbytesleft;
            int last_ch = cd->write_marc8_last;
            const char *lpage = cd->write_marc8_lpage;

            r = yaz_write_marc8_2(cd, latin1_comb[i].x1,
                                  outbuf, outbytesleft);
            if (r)
                return r;
            r = yaz_write_marc8_2(cd, latin1_comb[i].x2,
                                  outbuf, outbytesleft);
            if (r && cd->my_errno == YAZ_ICONV_E2BIG)
            {
                /* not enough room. reset output to original values */
                *outbuf = outbuf0;
                *outbytesleft = outbytesleft0;
                cd->write_marc8_last = last_ch;
                cd->write_marc8_lpage = lpage;
            }
            return r;
        }
    }
    return yaz_write_marc8_2(cd, x, outbuf, outbytesleft);
}


#if HAVE_WCHAR_H
static size_t yaz_write_wchar_t(yaz_iconv_t cd, unsigned long x,
                                char **outbuf, size_t *outbytesleft)
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
    cd->flush_handle = 0;
    cd->my_errno = YAZ_ICONV_UNKNOWN;

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
        else if (!yaz_matchstr(fromcode, "MARC8s"))
            cd->read_handle = yaz_read_marc8s;
        else if (!yaz_matchstr(fromcode, "advancegreek"))
            cd->read_handle = yaz_read_advancegreek;
        else if (!yaz_matchstr(fromcode, "iso54281984"))
            cd->read_handle = yaz_read_iso5428_1984;
        else if (!yaz_matchstr(fromcode, "iso5428:1984"))
            cd->read_handle = yaz_read_iso5428_1984;
#if HAVE_WCHAR_H
        else if (!yaz_matchstr(fromcode, "WCHAR_T"))
            cd->read_handle = yaz_read_wchar_t;
#endif
        
        if (!yaz_matchstr(tocode, "UTF8"))
            cd->write_handle = yaz_write_UTF8;
        else if (!yaz_matchstr(tocode, "ISO88591"))
        {
            cd->write_handle = yaz_write_ISO8859_1;
            cd->flush_handle = yaz_flush_ISO8859_1;
        }
        else if (!yaz_matchstr (tocode, "UCS4"))
            cd->write_handle = yaz_write_UCS4;
        else if (!yaz_matchstr(tocode, "UCS4LE"))
            cd->write_handle = yaz_write_UCS4LE;
        else if (!yaz_matchstr(tocode, "MARC8"))
        {
            cd->write_handle = yaz_write_marc8;
            cd->flush_handle = yaz_flush_marc8;
        }
        else if (!yaz_matchstr(tocode, "MARC8s"))
        {
            cd->write_handle = yaz_write_marc8;
            cd->flush_handle = yaz_flush_marc8;
        }
        else if (!yaz_matchstr(tocode, "advancegreek"))
        {
            cd->write_handle = yaz_write_advancegreek;
        }
        else if (!yaz_matchstr(tocode, "iso54281984"))
        {
            cd->write_handle = yaz_write_iso5428_1984;
        }
        else if (!yaz_matchstr(tocode, "iso5428:1984"))
        {
            cd->write_handle = yaz_write_iso5428_1984;
        }
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
    char *inbuf0 = 0;
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

    if (inbuf)
        inbuf0 = *inbuf;

    if (cd->init_flag)
    {
        cd->my_errno = YAZ_ICONV_UNKNOWN;
        cd->g0_mode = 'B';
        cd->g1_mode = 'E';
        
        cd->comb_offset = cd->comb_size = 0;
        cd->compose_char = 0;
        
        cd->write_marc8_second_half_char = 0;
        cd->write_marc8_last = 0;
        cd->write_marc8_lpage = 0;
        cd->write_marc8_g0 = ESC "(B";
        cd->write_marc8_g1 = 0;
        
        cd->unget_x = 0;
        cd->no_read_x = 0;
    }

    if (cd->init_flag)
    {
        if (cd->init_handle && inbuf && *inbuf)
        {
            size_t no_read = 0;
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
    }
    cd->init_flag = 0;

    if (!inbuf || !*inbuf)
    {
        if (outbuf && *outbuf)
        {
            if (cd->unget_x)
                r = (*cd->write_handle)(cd, cd->unget_x, outbuf, outbytesleft);
            if (cd->flush_handle)
                r = (*cd->flush_handle)(cd, outbuf, outbytesleft);
        }
        if (r == 0)
            cd->init_flag = 1;
        cd->unget_x = 0;
        return r;
    }
    while (1)
    {
        unsigned long x;
        size_t no_read;

        if (cd->unget_x)
        {
            x = cd->unget_x;
            no_read = cd->no_read_x;
        }
        else
        {
            if (*inbytesleft == 0)
            {
                r = *inbuf - inbuf0;
                break;
            }
            x = (*cd->read_handle)(cd, (unsigned char *) *inbuf, *inbytesleft,
                                   &no_read);
            if (no_read == 0)
            {
                r = (size_t)(-1);
                break;
            }
        }
        if (x)
        {
            r = (*cd->write_handle)(cd, x, outbuf, outbytesleft);
            if (r)
            {
                /* unable to write it. save it because read_handle cannot
                   rewind .. */
                if (cd->my_errno == YAZ_ICONV_E2BIG)
                {
                    cd->unget_x = x;
                    cd->no_read_x = no_read;
                    break;
                }
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

void yaz_iconv_set_errno(yaz_iconv_t cd, int no)
{
    cd->my_errno = no;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
