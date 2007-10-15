/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: siconv.c,v 1.48 2007-10-15 20:45:05 adam Exp $
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


#include <yaz/yaz-util.h>

unsigned long yaz_marc8_42_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_45_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_67_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_62_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_70_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_32_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_4E_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_51_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_33_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_34_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_53_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);
unsigned long yaz_marc8_31_conv(unsigned char *inp, size_t inbytesleft,
                               size_t *no_read, int *combining);


unsigned long yaz_marc8r_42_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_45_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_67_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_62_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_70_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_32_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_4E_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_51_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_33_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_34_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_53_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);
unsigned long yaz_marc8r_31_conv(unsigned char *inp, size_t inbytesleft,
                                 size_t *no_read, int *combining);

#define ESC "\033"

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

static size_t yaz_write_marc8_page_chr(yaz_iconv_t cd, 
                                       char **outbuf, size_t *outbytesleft,
                                       const char *page_chr);

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
    if (inp[1] != 0xbb && inp[2] == 0xbf)
        *no_read = 3;
    else
        *no_read = 0;
    return 0;
}

unsigned long yaz_read_UTF8_char(unsigned char *inp,
                                 size_t inbytesleft, size_t *no_read,
                                 int *error)
{
    unsigned long x = 0;

    *no_read = 0; /* by default */
    if (inp[0] <= 0x7f)
    {
        x = inp[0];
        *no_read = 1;
    }
    else if (inp[0] <= 0xbf || inp[0] >= 0xfe)
    {
        *error = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xdf && inbytesleft >= 2)
    {
        if ((inp[1] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x1f) << 6) | (inp[1] & 0x3f);
            if (x >= 0x80)
                *no_read = 2;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xef && inbytesleft >= 3)
    {
        if ((inp[1] & 0xc0) == 0x80 && (inp[2] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x0f) << 12) | ((inp[1] & 0x3f) << 6) |
                (inp[2] & 0x3f);
            if (x >= 0x800)
                *no_read = 3;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }            
    else if (inp[0] <= 0xf7 && inbytesleft >= 4)
    {
        if ((inp[1] & 0xc0) == 0x80 && (inp[2] & 0xc0) == 0x80
            && (inp[3] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x07) << 18) | ((inp[1] & 0x3f) << 12) |
                ((inp[2] & 0x3f) << 6) | (inp[3] & 0x3f);
            if (x >= 0x10000)
                *no_read = 4;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xfb && inbytesleft >= 5)
    {
        if ((inp[1] & 0xc0) == 0x80 && (inp[2] & 0xc0) == 0x80
            && (inp[3] & 0xc0) == 0x80 && (inp[4] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x03) << 24) | ((inp[1] & 0x3f) << 18) |
                ((inp[2] & 0x3f) << 12) | ((inp[3] & 0x3f) << 6) |
                (inp[4] & 0x3f);
            if (x >= 0x200000)
                *no_read = 5;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xfd && inbytesleft >= 6)
    {
        if ((inp[1] & 0xc0) == 0x80 && (inp[2] & 0xc0) == 0x80
            && (inp[3] & 0xc0) == 0x80 && (inp[4] & 0xc0) == 0x80
            && (inp[5] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x01) << 30) | ((inp[1] & 0x3f) << 24) |
                ((inp[2] & 0x3f) << 18) | ((inp[3] & 0x3f) << 12) |
                ((inp[4] & 0x3f) << 6) | (inp[5] & 0x3f);
            if (x >= 0x4000000)
                *no_read = 6;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }
    else
        *error = YAZ_ICONV_EINVAL;  /* incomplete sentence */

    return x;
}

static unsigned long yaz_read_UTF8 (yaz_iconv_t cd, unsigned char *inp,
                                    size_t inbytesleft, size_t *no_read)
{
    return yaz_read_UTF8_char(inp, inbytesleft, no_read, &cd->my_errno);
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

static unsigned long yaz_read_iso5428_1984(yaz_iconv_t cd, unsigned char *inp,
                                           size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    int tonos = 0;
    int dialitika = 0;

    *no_read = 0;
    while (inbytesleft > 0)
    {
        if (*inp == 0xa2)
        {
            tonos = 1;
        }
        else if (*inp == 0xa3)
        {
            dialitika = 1;
        }
        else
            break;
        inp++;
        --inbytesleft;
        (*no_read)++;
    }    
    if (inbytesleft == 0)
    {
        cd->my_errno = YAZ_ICONV_EINVAL; /* incomplete input */
        *no_read = 0;
        return 0;
    }
    switch (*inp) {
    case 0xe1: /*  alpha small */
            if (tonos) 
                x = 0x03ac;
            else 
                x = 0x03b1;
            break;
    case 0xc1: /*  alpha capital */
            if (tonos) 
                x = 0x0386;
            else 
                x = 0x0391;
            break;

    case 0xe2: /*  Beta small */
            x = 0x03b2;
            break;
    case 0xc2: /*  Beta capital */
            x = 0x0392;
            break;

    case 0xe4: /*  Gamma small */
            x = 0x03b3;
            break;
    case 0xc4: /*  Gamma capital */
            x = 0x0393;
            break;

    case 0xe5: /*  Delta small */
            x = 0x03b4;
            break;
    case 0xc5: /*  Delta capital */
            x = 0x0394;
            break;
    case 0xe6: /*  epsilon small */
            if (tonos) 
                x = 0x03ad;
            else 
                x = 0x03b5;
            break;
    case 0xc6: /*  epsilon capital */
            if (tonos) 
                x = 0x0388;
            else 
                x = 0x0395;
            break;
    case 0xe9: /*  Zeta small */
            x = 0x03b6;
            break;
    case 0xc9: /*  Zeta capital */
            x = 0x0396;
            break;
    case 0xea: /*  Eta small */
            if (tonos) 
                x = 0x03ae;
            else 
                x = 0x03b7;
            break;
    case 0xca: /*  Eta capital */
            if (tonos) 
                x = 0x0389;
            else 
                x = 0x0397;
            break;
    case 0xeb: /*  Theta small */
            x = 0x03b8;
            break;
    case 0xcb: /*  Theta capital */
            x = 0x0398;
            break;
    case 0xec: /*  Iota small */
            if (tonos) 
                if (dialitika) 
                    x = 0x0390;
                else 
                    x = 0x03af;
            else 
                if (dialitika) 
                    x = 0x03ca;
                else 
                    x = 0x03b9;
            break;
    case 0xcc: /*  Iota capital */
            if (tonos) 
                x = 0x038a;
            else 
                if (dialitika) 
                    x = 0x03aa;
                else 
                    x = 0x0399;
            break;
    case 0xed: /*  Kappa small */
            x = 0x03ba;
            break;
    case 0xcd: /*  Kappa capital */
            x = 0x039a;
            break;
    case 0xee: /*  Lambda small */
            x = 0x03bb;
            break;
    case 0xce: /*  Lambda capital */
            x = 0x039b;
            break;
    case 0xef: /*  Mu small */
            x = 0x03bc;
            break;
    case 0xcf: /*  Mu capital */
            x = 0x039c;
            break;
    case 0xf0: /*  Nu small */
            x = 0x03bd;
            break;
    case 0xd0: /*  Nu capital */
            x = 0x039d;
            break;
    case 0xf1: /*  Xi small */
            x = 0x03be;
            break;
    case 0xd1: /*  Xi capital */
            x = 0x039e;
            break;
    case 0xf2: /*  Omicron small */
            if (tonos) 
                x = 0x03cc;
            else 
                x = 0x03bf;
            break;
    case 0xd2: /*  Omicron capital */
            if (tonos) 
                x = 0x038c;
            else 
                x = 0x039f;
            break;
    case 0xf3: /*  Pi small */
            x = 0x03c0;
            break;
    case 0xd3: /*  Pi capital */
            x = 0x03a0;
            break;
    case 0xf5: /*  Rho small */
            x = 0x03c1;
            break;
    case 0xd5: /*  Rho capital */
            x = 0x03a1;
            break;
    case 0xf7: /*  Sigma small (end of words) */
            x = 0x03c2;
            break;
    case 0xf6: /*  Sigma small */
            x = 0x03c3;
            break;
    case 0xd6: /*  Sigma capital */
            x = 0x03a3;
            break;
    case 0xf8: /*  Tau small */
            x = 0x03c4;
            break;
    case 0xd8: /*  Tau capital */
            x = 0x03a4;
            break;
    case 0xf9: /*  Upsilon small */
            if (tonos) 
                if (dialitika) 
                    x = 0x03b0;
                else 
                    x = 0x03cd;
            else 
                if (dialitika) 
                    x = 0x03cb;
                else 
                    x = 0x03c5;
            break;
    case 0xd9: /*  Upsilon capital */
            if (tonos) 
                x = 0x038e;
            else 
                if (dialitika) 
                    x = 0x03ab;
                else 
                    x = 0x03a5;
            break;
    case 0xfa: /*  Phi small */
            x = 0x03c6;
            break;
    case 0xda: /*  Phi capital */
            x = 0x03a6;
            break;
    case 0xfb: /*  Chi small */
            x = 0x03c7;
            break;
    case 0xdb: /*  Chi capital */
            x = 0x03a7;
            break;
    case 0xfc: /*  Psi small */
            x = 0x03c8;
            break;
    case 0xdc: /*  Psi capital */
            x = 0x03a8;
            break;
    case 0xfd: /*  Omega small */
            if (tonos) 
                x = 0x03ce;
            else 
                x = 0x03c9;
            break;
    case 0xdd: /*  Omega capital */
            if (tonos) 
                x = 0x038f;
            else 
                x = 0x03a9;
            break;
    default:
        x = *inp;
        break;
    }
    (*no_read)++;
    
    return x;
}

static size_t yaz_write_iso5428_1984(yaz_iconv_t cd, unsigned long x,
                                     char **outbuf, size_t *outbytesleft)
{
    size_t k = 0;
    unsigned char *out = (unsigned char*) *outbuf;
    if (*outbytesleft < 3)
    {
        cd->my_errno = YAZ_ICONV_E2BIG;  /* not room for output */
        return (size_t)(-1);
    }
    switch (x)
    {
    case 0x03ac : out[k++]=0xa2; out[k++]=0xe1; break;
    case 0x03b1 : out[k++]=0xe1; break;
    case 0x0386 : out[k++]=0xa2; out[k++]=0xc1; break;
    case 0x0391 : out[k++]=0xc1; break;
    case 0x03b2 : out[k++]=0xe2; break;
    case 0x0392 : out[k++]=0xc2; break;
    case 0x03b3 : out[k++]=0xe4; break;
    case 0x0393 : out[k++]=0xc4; break;
    case 0x03b4 : out[k++]=0xe5; break;
    case 0x0394 : out[k++]=0xc5; break;
    case 0x03ad : out[k++]=0xa2; out[k++]=0xe6; break;
    case 0x03b5 : out[k++]=0xe6; break;
    case 0x0388 : out[k++]=0xa2; out[k++]=0xc6; break;
    case 0x0395 : out[k++]=0xc6; break;
    case 0x03b6 : out[k++]=0xe9; break;
    case 0x0396 : out[k++]=0xc9; break;
    case 0x03ae : out[k++]=0xa2; out[k++]=0xea; break;
    case 0x03b7 : out[k++]=0xea; break;
    case 0x0389 : out[k++]=0xa2; out[k++]=0xca; break;
    case 0x0397 : out[k++]=0xca; break;
    case 0x03b8 : out[k++]=0xeb; break;
    case 0x0398 : out[k++]=0xcb; break;
    case 0x0390 : out[k++]=0xa2; out[k++]=0xa3; out[k++]=0xec; break;
    case 0x03af : out[k++]=0xa2; out[k++]=0xec; break;
    case 0x03ca : out[k++]=0xa3; out[k++]=0xec; break;
    case 0x03b9 : out[k++]=0xec; break;
    case 0x038a : out[k++]=0xa2; out[k++]=0xcc; break;
    case 0x03aa : out[k++]=0xa3; out[k++]=0xcc; break;
    case 0x0399 : out[k++]=0xcc; break;
    case 0x03ba : out[k++]=0xed; break;
    case 0x039a : out[k++]=0xcd; break;
    case 0x03bb : out[k++]=0xee; break;
    case 0x039b : out[k++]=0xce; break;
    case 0x03bc : out[k++]=0xef; break;
    case 0x039c : out[k++]=0xcf; break;
    case 0x03bd : out[k++]=0xf0; break;
    case 0x039d : out[k++]=0xd0; break;
    case 0x03be : out[k++]=0xf1; break;
    case 0x039e : out[k++]=0xd1; break;
    case 0x03cc : out[k++]=0xa2; out[k++]=0xf2; break;
    case 0x03bf : out[k++]=0xf2; break;
    case 0x038c : out[k++]=0xa2; out[k++]=0xd2; break;
    case 0x039f : out[k++]=0xd2; break;
    case 0x03c0 : out[k++]=0xf3; break;
    case 0x03a0 : out[k++]=0xd3; break;
    case 0x03c1 : out[k++]=0xf5; break;
    case 0x03a1 : out[k++]=0xd5; break;
    case 0x03c2 : out[k++]=0xf7; break;
    case 0x03c3 : out[k++]=0xf6; break;
    case 0x03a3 : out[k++]=0xd6; break;
    case 0x03c4 : out[k++]=0xf8; break;
    case 0x03a4 : out[k++]=0xd8; break;
    case 0x03b0 : out[k++]=0xa2; out[k++]=0xa3; out[k++]=0xf9; break;
    case 0x03cd : out[k++]=0xa2; out[k++]=0xf9; break;
    case 0x03cb : out[k++]=0xa3; out[k++]=0xf9; break;
    case 0x03c5 : out[k++]=0xf9; break;
    case 0x038e : out[k++]=0xa2; out[k++]=0xd9; break;
    case 0x03ab : out[k++]=0xa3; out[k++]=0xd9; break;
    case 0x03a5 : out[k++]=0xd9; break;
    case 0x03c6 : out[k++]=0xfa; break;
    case 0x03a6 : out[k++]=0xda; break;
    case 0x03c7 : out[k++]=0xfb; break;
    case 0x03a7 : out[k++]=0xdb; break;
    case 0x03c8 : out[k++]=0xfc; break;
    case 0x03a8 : out[k++]=0xdc; break;
    case 0x03ce : out[k++]=0xa2; out[k++]=0xfd; break;
    case 0x03c9 : out[k++]=0xfd; break;
    case 0x038f : out[k++]=0xa2; out[k++]=0xdd; break;
    case 0x03a9 : out[k++]=0xdd; break;
    default:
        if (x > 255)
        {
            cd->my_errno = YAZ_ICONV_EILSEQ;
            return (size_t) -1;
        }
        out[k++] = x;
        break;
    }
    *outbytesleft -= k;
    (*outbuf) += k;
    return 0;
}

static unsigned long yaz_read_advancegreek(yaz_iconv_t cd, unsigned char *inp,
                                           size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    int shift = 0;
    int tonos = 0;
    int dialitika = 0;

    *no_read = 0;
    while (inbytesleft > 0)
    {
        if (*inp == 0x9d)
        {
            tonos = 1;
        }
        else if (*inp == 0x9e)
        {
            dialitika = 1;
        }
        else if (*inp == 0x9f)
        {
            shift = 1;
        }
        else
            break;
        inp++;
        --inbytesleft;
        (*no_read)++;
    }    
    if (inbytesleft == 0)
    {
        cd->my_errno = YAZ_ICONV_EINVAL; /* incomplete input */
        *no_read = 0;
        return 0;
    }
    switch (*inp) {
    case 0x81:
        if (shift) 
            if (tonos) 
                x = 0x0386;
            else 
                x = 0x0391;
        else 
            if (tonos) 
                x = 0x03ac;
            else 
                x = 0x03b1;
        break;
    case 0x82:
        if (shift) 
            x = 0x0392;
        else 
            x = 0x03b2;
        
        break;
    case 0x83:
        if (shift) 
            x = 0x0393;
        else 
            x = 0x03b3;
        break;
    case 0x84:
        if (shift) 
            x = 0x0394;
        else 
            x = 0x03b4;
        break;
    case 0x85:
        if (shift) 
            if (tonos) 
                x = 0x0388;
            else 
                x = 0x0395;
        else 
            if (tonos) 
                x = 0x03ad;
            else 
                x = 0x03b5;
        break;
    case 0x86:
        if (shift) 
            x = 0x0396;
        else 
            x = 0x03b6;
        break;
    case 0x87:
        if (shift) 
            if (tonos) 
                x = 0x0389;
            else 
                x = 0x0397;
        else 
            if (tonos) 
                x = 0x03ae;
            else 
                x = 0x03b7;
        break;
    case 0x88:
        if (shift) 
            x = 0x0398;
        else 
            x = 0x03b8;
        break;
    case 0x89:
        if (shift) 
            if (tonos) 
                x = 0x038a;
            else 
                if (dialitika) 
                    x = 0x03aa;
                else 
                    x = 0x0399;
        else 
            if (tonos) 
                if (dialitika) 
                    x = 0x0390;
                else 
                    x = 0x03af;
        
            else 
                if (dialitika) 
                    x = 0x03ca;
                else 
                    x = 0x03b9;
        break;
    case 0x8a:
        if (shift) 
            x = 0x039a;
        else 
            x = 0x03ba;
        
        break;
    case 0x8b:
        if (shift) 
            x = 0x039b;
        else 
            x = 0x03bb;
        break;
    case 0x8c:
        if (shift) 
            x = 0x039c;
        else 
            x = 0x03bc;
        
        break;
    case 0x8d:
        if (shift) 
            x = 0x039d;
        else 
            x = 0x03bd;
        break;
    case 0x8e:
        if (shift) 
            x = 0x039e;
        else 
            x = 0x03be;
        break;
    case 0x8f:
        if (shift) 
            if (tonos) 
                x = 0x038c;
            else 
                x = 0x039f;
        else 
            if (tonos) 
                x = 0x03cc;
            else 
                x = 0x03bf;
        break;
    case 0x90:
        if (shift) 
            x = 0x03a0;
        else 
            x = 0x03c0;
        break;
    case 0x91:
        if (shift) 
            x = 0x03a1;
        else 
            x = 0x03c1;
        break;
    case 0x92:
        x = 0x03c2;
        break;
    case 0x93:
        if (shift) 
            x = 0x03a3;
        else 
            x = 0x03c3;
        break;
    case 0x94:
        if (shift) 
            x = 0x03a4;
        else 
            x = 0x03c4;
        break;
    case 0x95:
        if (shift) 
            if (tonos) 
                x = 0x038e;
            else 
                if (dialitika) 
                    x = 0x03ab;
                else 
                    x = 0x03a5;
        else 
            if (tonos) 
                if (dialitika) 
                    x = 0x03b0;
                else 
                    x = 0x03cd;
        
            else 
                if (dialitika) 
                    x = 0x03cb;
                else 
                    x = 0x03c5;
        break;
    case 0x96:
        if (shift) 
            x = 0x03a6;
        else 
            x = 0x03c6;
        break;
    case 0x97:
        if (shift) 
            x = 0x03a7;
        else 
            x = 0x03c7;
        break;
    case 0x98:
        if (shift) 
            x = 0x03a8;
        else 
            x = 0x03c8;
        
        break;
        
    case 0x99:
        if (shift) 
            if (tonos) 
                x = 0x038f;
            else 
                x = 0x03a9;
        else 
            if (tonos) 
                x = 0x03ce;
            else 
                x = 0x03c9;
        break;
    default:
        x = *inp;
        break;
    }
    (*no_read)++;
    
    return x;
}

static size_t yaz_write_advancegreek(yaz_iconv_t cd, unsigned long x,
                                     char **outbuf, size_t *outbytesleft)
{
    size_t k = 0;
    unsigned char *out = (unsigned char*) *outbuf;
    if (*outbytesleft < 3)
    {
        cd->my_errno = YAZ_ICONV_E2BIG;  /* not room for output */
        return (size_t)(-1);
    }
    switch (x)
    {
    case 0x03ac : out[k++]=0x9d; out[k++]=0x81; break;
    case 0x03ad : out[k++]=0x9d; out[k++]=0x85; break;
    case 0x03ae : out[k++]=0x9d; out[k++]=0x87; break;
    case 0x03af : out[k++]=0x9d; out[k++]=0x89; break;
    case 0x03cc : out[k++]=0x9d; out[k++]=0x8f; break;
    case 0x03cd : out[k++]=0x9d; out[k++]=0x95; break;
    case 0x03ce : out[k++]=0x9d; out[k++]=0x99; break;
    case 0x0390 : out[k++]=0x9d; out[k++]=0x9e; out[k++]=0x89; break;
    case 0x03b0 : out[k++]=0x9d; out[k++]=0x9e; out[k++]=0x95; break;
    case 0x0386 : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x81; break;
    case 0x0388 : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x85; break;
    case 0x0389 : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x87; break;
    case 0x038a : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x89; break;
    case 0x038c : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x8f; break;
    case 0x038e : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x95; break;
    case 0x038f : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x99; break;
    case 0x03ca : out[k++]=0x9e; out[k++]=0x89; break;
    case 0x03cb : out[k++]=0x9e; out[k++]=0x95; break;
    case 0x03aa : out[k++]=0x9e; out[k++]=0x9f; out[k++]=0x89; break;
    case 0x03ab : out[k++]=0x9e; out[k++]=0x9f; out[k++]=0x95; break;
    case 0x0391 : out[k++]=0x9f; out[k++]=0x81; break;
    case 0x0392 : out[k++]=0x9f; out[k++]=0x82; break;
    case 0x0393 : out[k++]=0x9f; out[k++]=0x83; break;
    case 0x0394 : out[k++]=0x9f; out[k++]=0x84; break;
    case 0x0395 : out[k++]=0x9f; out[k++]=0x85; break;
    case 0x0396 : out[k++]=0x9f; out[k++]=0x86; break;
    case 0x0397 : out[k++]=0x9f; out[k++]=0x87; break;
    case 0x0398 : out[k++]=0x9f; out[k++]=0x88; break;
    case 0x0399 : out[k++]=0x9f; out[k++]=0x89; break;
    case 0x039a : out[k++]=0x9f; out[k++]=0x8a; break;
    case 0x039b : out[k++]=0x9f; out[k++]=0x8b; break;
    case 0x039c : out[k++]=0x9f; out[k++]=0x8c; break;
    case 0x039d : out[k++]=0x9f; out[k++]=0x8d; break;
    case 0x039e : out[k++]=0x9f; out[k++]=0x8e; break;
    case 0x039f : out[k++]=0x9f; out[k++]=0x8f; break;
    case 0x03a0 : out[k++]=0x9f; out[k++]=0x90; break;
    case 0x03a1 : out[k++]=0x9f; out[k++]=0x91; break;
    case 0x03a3 : out[k++]=0x9f; out[k++]=0x93; break;
    case 0x03a4 : out[k++]=0x9f; out[k++]=0x94; break;
    case 0x03a5 : out[k++]=0x9f; out[k++]=0x95; break;
    case 0x03a6 : out[k++]=0x9f; out[k++]=0x96; break;
    case 0x03a7 : out[k++]=0x9f; out[k++]=0x97; break;
    case 0x03a8 : out[k++]=0x9f; out[k++]=0x98; break;
    case 0x03a9 : out[k++]=0x9f; out[k++]=0x99; break;
    case 0x03b1 : out[k++]=0x81; break;
    case 0x03b2 : out[k++]=0x82; break;
    case 0x03b3 : out[k++]=0x83; break;
    case 0x03b4 : out[k++]=0x84; break;
    case 0x03b5 : out[k++]=0x85; break;
    case 0x03b6 : out[k++]=0x86; break;
    case 0x03b7 : out[k++]=0x87; break;
    case 0x03b8 : out[k++]=0x88; break;
    case 0x03b9 : out[k++]=0x89; break;
    case 0x03ba : out[k++]=0x8a; break;
    case 0x03bb : out[k++]=0x8b; break;
    case 0x03bc : out[k++]=0x8c; break;
    case 0x03bd : out[k++]=0x8d; break;
    case 0x03be : out[k++]=0x8e; break;
    case 0x03bf : out[k++]=0x8f; break;
    case 0x03c0 : out[k++]=0x90; break;
    case 0x03c1 : out[k++]=0x91; break;
    case 0x03c2 : out[k++]=0x92; break;
    case 0x03c3 : out[k++]=0x93; break;
    case 0x03c4 : out[k++]=0x94; break;
    case 0x03c5 : out[k++]=0x95; break;
    case 0x03c6 : out[k++]=0x96; break;
    case 0x03c7 : out[k++]=0x96; break;
    case 0x03c8 : out[k++]=0x98; break;
    case 0x03c9 : out[k++]=0x99; break;
    default:
        if (x > 255)
        {
            cd->my_errno = YAZ_ICONV_EILSEQ;
            return (size_t) -1;
        }
        out[k++] = x;
        break;
    }
    *outbytesleft -= k;
    (*outbuf) += k;
    return 0;
}


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
    while(inbytesleft >= 1 && inp[0] == 27)
    {
        size_t inbytesleft0 = inbytesleft;
        inp++;
        inbytesleft--;
        while(inbytesleft > 0 && strchr("(,$!)-", *inp))
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
    else if (*inp == ' ')
    {
        *no_read += 1;
        return ' ';
    }
    else
    {
        unsigned long x;
        size_t no_read_sub = 0;
        *comb = 0;

        switch(cd->marc8_esc_mode)
        {
        case 'B':  /* Basic ASCII */
        case 's':  /* ASCII */
        case 'E':  /* ANSEL */
            x = yaz_marc8_42_conv(inp, inbytesleft, &no_read_sub, comb);
            if (!x)
            {
                no_read_sub = 0;
                x = yaz_marc8_45_conv(inp, inbytesleft, &no_read_sub, comb);
            }
            break;
        case 'g':  /* Greek */
            x = yaz_marc8_67_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'b':  /* Subscripts */
            x = yaz_marc8_62_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'p':  /* Superscripts */
            x = yaz_marc8_70_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case '2':  /* Basic Hebrew */
            x = yaz_marc8_32_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'N':  /* Basic Cyrillic */
            x = yaz_marc8_4E_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'Q':  /* Extended Cyrillic */
            x = yaz_marc8_51_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case '3':  /* Basic Arabic */
            x = yaz_marc8_33_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case '4':  /* Extended Arabic */
            x = yaz_marc8_34_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case 'S':  /* Greek */
            x = yaz_marc8_53_conv(inp, inbytesleft, &no_read_sub, comb);
            break;
        case '1':  /* Chinese, Japanese, Korean (EACC) */
            x = yaz_marc8_31_conv(inp, inbytesleft, &no_read_sub, comb);
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

static size_t yaz_write_UTF8(yaz_iconv_t cd, unsigned long x,
                             char **outbuf, size_t *outbytesleft)
{
    return yaz_write_UTF8_char(x, outbuf, outbytesleft, &cd->my_errno);
}

size_t yaz_write_UTF8_char(unsigned long x,
                           char **outbuf, size_t *outbytesleft,
                           int *error)
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
        *error = YAZ_ICONV_E2BIG;  /* not room for output */
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}

static size_t yaz_write_ISO8859_1 (yaz_iconv_t cd, unsigned long x,
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

static size_t yaz_write_UCS4 (yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft)
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
                                char **outbuf, size_t *outbytesleft)
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

        x = yaz_marc8r_42_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "(B";
            return x;
        }
        x = yaz_marc8r_45_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "(B";
            return x;
        }
        x = yaz_marc8r_67_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "g";
            return x;
        }
        x = yaz_marc8r_62_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "b";
            return x;
        }
        x = yaz_marc8r_70_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "p";
            return x;
        }
        x = yaz_marc8r_32_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "(2";
            return x;
        }
        x = yaz_marc8r_4E_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "(N";
            return x;
        }
        x = yaz_marc8r_51_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "(Q";
            return x;
        }
        x = yaz_marc8r_33_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "(3";
            return x;
        }
        x = yaz_marc8r_34_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "(4";
            return x;
        }
        x = yaz_marc8r_53_conv(inp, inbytesleft, &no_read_sub, comb);
        if (x)
        {
            *page_chr = ESC "(S";
            return x;
        }
        x = yaz_marc8r_31_conv(inp, inbytesleft, &no_read_sub, comb);
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
        cd->marc8_esc_mode = 'B';
        
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

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
