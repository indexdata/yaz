/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: siconv.c,v 1.8 2005-01-15 19:47:14 adam Exp $
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
                           char **outbuf, size_t *outbytesleft);
    int marc8_esc_mode;
    int marc8_comb_x;
    int marc8_comb_no_read;
#if HAVE_ICONV_H
    iconv_t iconv_cd;
#endif
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
            (inp[1] & 0x3f);
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

static unsigned long yaz_read_marc8 (yaz_iconv_t cd, unsigned char *inp,
                                     size_t inbytesleft, size_t *no_read)
{
    if (cd->marc8_comb_x)
    {
	unsigned long x = cd->marc8_comb_x;
	*no_read = cd->marc8_comb_no_read;
	cd->marc8_comb_x = 0;
	return x;
    }
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
	int comb = 0;
	size_t no_read_sub = 0;

	switch(cd->marc8_esc_mode)
	{
	case 'B':  /* Basic ASCII */
	case 'E':  /* ANSEL */
	case 's':  /* ASCII */
	    x = yaz_marc8_1_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	case 'g':  /* Greek */
	    x = yaz_marc8_2_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	case 'b':  /* Subscripts */
	    x = yaz_marc8_3_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	case 'p':  /* Superscripts */
	    x = yaz_marc8_4_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	case '2':  /* Basic Hebrew */
	    x = yaz_marc8_5_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	case 'N':  /* Basic Cyrillic */
	case 'Q':  /* Extended Cyrillic */
	    x = yaz_marc8_6_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	case '3':  /* Basic Arabic */
	case '4':  /* Extended Arabic */
	    x = yaz_marc8_7_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	case 'S':  /* Greek */
	    x = yaz_marc8_8_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	case '1':  /* Chinese, Japanese, Korean (EACC) */
	    x = yaz_marc8_9_conv(inp, inbytesleft, &no_read_sub, &comb);
	    break;
	default:
	    *no_read = 0;
	    cd->my_errno = YAZ_ICONV_EILSEQ;
	    return 0;
	}
#if 0
	printf ("esc mode=%c x=%04lX comb=%d\n", cd->marc8_esc_mode, x, comb);
#endif
	*no_read += no_read_sub;

	if (comb && cd->marc8_comb_x == 0)
	{
	    size_t tmp_read = 0;
	    unsigned long next_x;

	    /* read next char .. */
	    next_x = yaz_read_marc8(cd, inp + *no_read,
				    inbytesleft - *no_read, &tmp_read);
	    /* save this x for later .. */
	    cd->marc8_comb_x = x;
	    /* save next read for later .. */
	    cd->marc8_comb_no_read = tmp_read;
	    /* return next x - thereby swap */
	    x = next_x;
	}
	return x;
    }
}

static size_t yaz_write_UTF8 (yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft)
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
                                   char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = (unsigned char *) *outbuf;
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

#if HAVE_WCHAR_H
static size_t yaz_write_wchar_t (yaz_iconv_t cd, unsigned long x,
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
    cd->my_errno = YAZ_ICONV_UNKNOWN;
    cd->marc8_esc_mode = 'B';
    cd->marc8_comb_x = 0;

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

size_t yaz_iconv (yaz_iconv_t cd, char **inbuf, size_t *inbytesleft,
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
        
        x = (cd->read_handle)(cd, (unsigned char *) *inbuf, *inbytesleft,
                              &no_read);
        if (no_read == 0)
        {
            r = (size_t)(-1);
            break;
        }
	if (x)
	{
	    r = (cd->write_handle)(cd, x, outbuf, outbytesleft);
	    if (r)
		break;
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

    
