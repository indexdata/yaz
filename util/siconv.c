/*
 * Copyright (c) 1997-2002, Index Data
 * See the file LICENSE for details.
 *
 * $Id: siconv.c,v 1.1 2002-08-27 14:02:13 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <ctype.h>

#if HAVE_ICONV_H
#include <iconv.h>
#endif

#include <yaz/yaz-util.h>

struct yaz_iconv_struct {
    int my_errno;
    unsigned long (*read_handle)(yaz_iconv_t cd, char **inbuf,
                                 size_t *inbytesleft);
    size_t (*write_handle)(yaz_iconv_t cd, unsigned long x,
                           char **outbuf, size_t *outbytesleft);
#if HAVE_ICONV_H
    iconv_t iconv_cd;
#endif
};


static unsigned long yaz_read_ISO8859_1 (yaz_iconv_t cd,
                                         char **inbuf, size_t *inbytesleft)
{
    unsigned char *inp = *inbuf;
    unsigned long x = 0;
    x = inp[0];
    (*inbytesleft)--;
    inp++;
    *inbuf = inp;
    return x;
}

static unsigned long yaz_read_UTF8 (yaz_iconv_t cd,
                                    char **inbuf, size_t *inbytesleft)
{
    unsigned char *inp = *inbuf;
    unsigned long x = 0;
    if (inp[0] <= 0x7f)
    {
        x = inp[0];
        
        (*inbytesleft)--;
        inp++;
    }
    else if (inp[0] <= 0xdf && *inbytesleft >= 2)
    {
        x = ((inp[0] & 0x1f) << 6) + (inp[1] & 0x3f);
        
        (*inbytesleft) -= 2;
        inp += 2;
    }
    else if (inp[0] <= 0xef && *inbytesleft >= 3)
    {
        x =  ((inp[0] & 0x0f) << 12) +
            ((inp[1] & 0x3f) << 6) +  (inp[1] & 0x3f);
        
        (*inbytesleft) -= 3;
        inp += 3;
    }
    else if (inp[0] <= 0xef && *inbytesleft >= 4)
    {
        x =  ((inp[0] & 0x07) << 18) +
            ((inp[1] & 0x3f) << 12) + ((inp[2] & 0x3f) << 6) +
            (inp[3] & 0x3f);
        
        (*inbytesleft) -= 4;
        inp += 4;
    }
    else
    {
        cd->my_errno = YAZ_ICONV_EINVAL;
    }
    *inbuf = inp;
    return x;
}

static unsigned long yaz_read_UCS4 (yaz_iconv_t cd,
                                    char **inbuf, size_t *inbytesleft)
{
    unsigned char *inp = *inbuf;
    unsigned long x = 0;
    
    if (*inbytesleft < 4)
    {
        cd->my_errno = YAZ_ICONV_EINVAL; /* incomplete input */
        return 0;
    }
    memcpy (&x, inp, sizeof(x));
    (*inbytesleft) -= 4;
    inp += 4;
    *inbuf = inp;
    return x;
}

static size_t yaz_write_UTF8 (yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = *outbuf;
    if (x <= 0x7f && *outbytesleft >= 1)
    {
        *outp++ = x;
        (*outbytesleft)--;
    } 
    else if (x <= 0x7ff && *outbytesleft >= 2)
    {
        *outp++ = (x >> 6) | 0xc0;
        *outp++ = (x & 0x3f) | 0x80;
        (*outbytesleft) -= 2;
    }
    else if (x <= 0xffff && *outbytesleft >= 3)
    {
        *outp++ = (x >> 12) | 0xe0;
        *outp++ = ((x >> 6) & 0x3f) | 0x80;
        *outp++ = (x & 0x3f) | 0x80;
        (*outbytesleft) -= 3;
    }
    else if (x <= 0x1fffff && *outbytesleft >= 4)
    {
        *outp++ = (x >> 18) | 0xf0;
        *outp++ = ((x >> 12) & 0x3f) | 0x80;
        *outp++ = ((x >> 6) & 0x3f) | 0x80;
        *outp++ = (x & 0x3f) | 0x80;
        (*outbytesleft) -= 4;
    }
    else if (x > 0x1fffff)
    {
        cd->my_errno = YAZ_ICONV_EILSEQ;  /* invalid sequence */
        return (size_t)(-1);
    }
    else 
    {
        cd->my_errno = YAZ_ICONV_E2BIG;  /* not room for output */
        return (size_t)(-1);
    }
    *outbuf = outp;
    return 0;
}

static size_t yaz_write_ISO8859_1 (yaz_iconv_t cd, unsigned long x,
                                   char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = *outbuf;
    if (x > 255 || x < 1)
    {
        cd->my_errno = YAZ_ICONV_EILSEQ;
        return (size_t) -1;
    }
    else if (*outbytesleft >= 1)
    {
        *outp++ = x;
        (*outbytesleft)--;
    }
    else 
    {
        cd->my_errno = YAZ_ICONV_E2BIG;
        return (size_t)(-1);
    }
    *outbuf = outp;
    return 0;
}


static size_t yaz_write_UCS4 (yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = *outbuf;
    if (x < 1 || x > 0x1fffff)
    {
        cd->my_errno = YAZ_ICONV_EILSEQ;
        return (size_t)(-1);
    }
    else if (*outbytesleft >= 4)
    {
        memcpy (outp, &x, sizeof(x));
        outp += 4;
        (*outbytesleft) -= 4;
    }
    else
    {
        cd->my_errno = YAZ_ICONV_E2BIG;
        return (size_t)(-1);
    }
    *outbuf = outp;
    return 0;
}

yaz_iconv_t yaz_iconv_open (const char *tocode, const char *fromcode)
{
    yaz_iconv_t cd = xmalloc (sizeof(*cd));

    cd->write_handle = 0;
    cd->read_handle = 0;
    cd->my_errno = YAZ_ICONV_UNKNOWN;

    if (!strcmp(fromcode, "UTF-8"))
        cd->read_handle = yaz_read_UTF8;
    else if (!strcmp(fromcode, "ISO-8859-1"))
        cd->read_handle = yaz_read_ISO8859_1;
    else if (!strcmp(fromcode, "UCS-4"))
        cd->read_handle = yaz_read_UCS4;


    if (!strcmp(tocode, "UTF-8"))
        cd->write_handle = yaz_write_UTF8;
    else if (!strcmp (tocode, "ISO-8859-1"))
        cd->write_handle = yaz_write_ISO8859_1;
    else if (!strcmp (tocode, "UCS-4"))
        cd->write_handle = yaz_write_UCS4;

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
    if (!cd->to_UCS4 || !cd->from_UCS4)
    {
        xfree (cd);
        return 0;
    }
#endif
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
            switch (errno)
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
        return 0;
    inbuf0 = *inbuf;
    while (1)
    {
        unsigned long x;

        if (*inbytesleft == 0)
        {
            r = *inbuf - inbuf0;
            break;
        }
        
        x = (cd->read_handle)(cd, inbuf, inbytesleft);
        if (x == 0)
        {
            r = (size_t)(-1);
            break;
        }
        r = (cd->write_handle)(cd, x, outbuf, outbytesleft);
        if (r)
            break;
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

    
