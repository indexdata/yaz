/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_any.c,v $
 * Revision 1.1  1995-02-09 15:51:45  quinn
 * Works better now.
 *
 */

#include <odr.h>

int ber_any(ODR o, Odr_any **p)
{
    int res;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = completeBER(o->bp, o->left)) <= 0)
	    	return 0;
	    (*p)->buf = nalloc(o, res);
	    memcpy((*p)->buf, o->bp, res);
	    (*p)->len = (*p)->size = res;
	    o->bp += res;
	    o->left -= res;
	    return 1;
    	case ODR_ENCODE:
	    if ((*p)->len > o->left)
	    	return 0;
	    memcpy(o->bp , (*p)->buf, (*p)->len);
	    o->bp += (*p)->len;
	    o->left -= (*p)->len;
	    return 1;
    	default: return 0;
    }
}

/*
 * Return length of BER-package or -1.
 */
int completeBER(unsigned char *buf, int len)
{
    int res, ll, class, tag, cons;
    unsigned char *b = buf;
    
    if (!buf[0] && !buf[1])
    	return -1;
    if ((res = ber_dectag(b, &class, &tag, &cons)) <= 0)
    	return 0;
    if (res > len)
    	return -1;
    b += res;
    len -= res;
    if ((res = ber_declen(b, &ll)) <= 0)
    	return -1;
    if (res > len)
    	return -1;
    b += res;
    len -= res;
    if (ll >= 0)
    	return (len >= ll ? len + (b-buf) : -1);
    if (!cons)
    	return -1;    
    while (1)
    {
	if ((res = completeBER(b, len)) < 0)
	    return -1;
	b += res;
	len -= res;
	if (len < 2)
	    return -1;
	if (*b == 0 && *(b + 1) == 0)
	    return (b - buf) + 2;
    }
}
