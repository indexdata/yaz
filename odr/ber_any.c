/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: ber_any.c,v 1.22 2003-02-14 18:49:23 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>

#include "odr-priv.h"

int ber_any(ODR o, Odr_any **p)
{
    int res;
    int left = o->size - (o->bp - o->buf);

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = completeBER(o->bp, left)) <= 0)        /* FIX THIS */
	    {
	    	o->error = OPROTO;
	    	return 0;
	    }
	    (*p)->buf = (unsigned char *)odr_malloc(o, res);
	    memcpy((*p)->buf, o->bp, res);
	    (*p)->len = (*p)->size = res;
	    o->bp += res;
	    return 1;
    	case ODR_ENCODE:
	    if (odr_write(o, (*p)->buf, (*p)->len) < 0)
	    	return 0;
	    return 1;
    	default: o->error = OOTHER; return 0;
    }
}

/*
 * Return length of BER-package or 0.
 */
int completeBER(const unsigned char *buf, int len)
{
    int res, ll, zclass, tag, cons;
    const unsigned char *b = buf;
    
    if (!len)
    	return 0;
    if (!buf[0] && !buf[1])
    	return 0;
    if (len > 5 && buf[0] >= 0x20 && buf[0] < 0x7f
		&& buf[1] >= 0x20 && buf[1] < 0x7f
		&& buf[2] >= 0x20 && buf[2] < 0x7f)
    {
        /* deal with HTTP request/response */
	int i = 2, content_len = 0;

        while (i <= len-4)
        {
            if (buf[i] == '\r' && buf[i+1] == '\n')
            {
                i += 2;
                if (buf[i] == '\r' && buf[i+1] == '\n')
                {
                    /* i += 2 seems not to work with GCC -O2 .. 
                       so i+2 is used instead .. */
                    if (len >= (i+2)+ content_len)
                        return (i+2)+ content_len;
                    break;
                }
                if (i < len-18)
                {
                    if (!memcmp(buf+i, "Content-Length:", 15))
                    {
                        i+= 15;
                        if (buf[i] == ' ')
                            i++;
                        content_len = 0;
                        while (i <= len-4 && isdigit(buf[i]))
                            content_len = content_len*10 + (buf[i++] - '0');
                        if (content_len < 0) /* prevent negative offsets */
                            content_len = 0;
                    }
                }
            }
            else
                i++;
        }
        return 0;
    }
    /* BER from now on .. */
    if ((res = ber_dectag(b, &zclass, &tag, &cons)) <= 0)
    	return 0;
    if (res > len)
    	return 0;
    b += res;
    len -= res;
    if ((res = ber_declen(b, &ll)) <= 0)
    	return 0;
    if (res > len)
    	return 0;
    b += res;
    len -= res;
    if (ll >= 0)
    	return (len >= ll ? ll + (b-buf) : 0);
    if (!cons)
    	return 0;    
    /* constructed - cycle through children */
    while (len >= 2)
    {
	if (*b == 0 && *(b + 1) == 0)
	    break;
	if (!(res = completeBER(b, len)))
	    return 0;
	b += res;
	len -= res;
    }
    if (len < 2)
    	return 0;
    return (b - buf) + 2;
}
