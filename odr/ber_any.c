/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: ber_any.c,v 1.24 2003-03-11 11:03:31 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

int ber_any(ODR o, Odr_any **p)
{
    int res;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = completeBER(o->bp, odr_max(o))) <= 0)        /* FIX THIS */
	    {
                odr_seterror(o, OPROTO, 2);
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
    	default: odr_seterror(o, OOTHER, 3); return 0;
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
    if ((res = ber_dectag(b, &zclass, &tag, &cons, len)) <= 0)
    	return 0;
    if (res > len)
    	return 0;
    b += res;
    len -= res;
    if ((res = ber_declen(b, &ll, len)) <= 0)
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
