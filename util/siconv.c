/*
 * Copyright (c) 1997-2002, Index Data
 * See the file LICENSE for details.
 *
 * $Id: siconv.c,v 1.4 2002-08-30 11:27:44 adam Exp $
 */

/* mini iconv and wrapper for system iconv library (if present) */

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
    int init_flag;
    size_t (*init_handle)(yaz_iconv_t cd, unsigned char *inbuf,
                          size_t inbytesleft, size_t *no_read);
    unsigned long (*read_handle)(yaz_iconv_t cd, unsigned char *inbuf,
                                 size_t inbytesleft, size_t *no_read);
    size_t (*write_handle)(yaz_iconv_t cd, unsigned long x,
                           char **outbuf, size_t *outbytesleft);
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

static size_t yaz_write_UTF8 (yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = *outbuf;
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
        *outp++ = (unsigned char) x;
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
    if (*outbytesleft >= 4)
    {
        *outp++ = (unsigned char) (x<<24);
        *outp++ = (unsigned char) (x<<16);
        *outp++ = (unsigned char) (x<<8);
        *outp++ = (unsigned char) x;
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

static size_t yaz_write_UCS4LE (yaz_iconv_t cd, unsigned long x,
                                char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = *outbuf;
    if (*outbytesleft >= 4)
    {
        *outp++ = (unsigned char) x;
        *outp++ = (unsigned char) (x<<8);
        *outp++ = (unsigned char) (x<<16);
        *outp++ = (unsigned char) (x<<24);
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
    cd->init_handle = 0;
    cd->my_errno = YAZ_ICONV_UNKNOWN;

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
    
    if (!yaz_matchstr(tocode, "UTF8"))
        cd->write_handle = yaz_write_UTF8;
    else if (!yaz_matchstr(tocode, "ISO88591"))
        cd->write_handle = yaz_write_ISO8859_1;
    else if (!yaz_matchstr (tocode, "UCS4"))
        cd->write_handle = yaz_write_UCS4;
    else if (!yaz_matchstr(tocode, "UCS4LE"))
        cd->write_handle = yaz_write_UCS4LE;

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
            size_t r = (cd->init_handle)(cd, *inbuf, *inbytesleft, &no_read);
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
        
        x = (cd->read_handle)(cd, *inbuf, *inbytesleft, &no_read);
        if (no_read == 0)
        {
            r = (size_t)(-1);
            break;
        }
        r = (cd->write_handle)(cd, x, outbuf, outbytesleft);
        if (r)
            break;
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

    
