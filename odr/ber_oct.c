/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_oct.c,v $
 * Revision 1.1  1995-02-02 16:21:52  quinn
 * First kick.
 *
 */

#include <odr.h>

static int more_chunks(ODR o, unsigned char *base, int len)
{
    if (!len)
    	return 0;
    if (len < 0) /* indefinite length */
    {
	if (*o->bp == 0 && *(o->bp + 1) == 0)
	{
	    o->bp += 2;
	    o->left -= 2;
	    return 0;
	}
	else
	    return 1;
    }
    else
        return o->bp - base < len;
}

int ber_octetstring(ODR o, ODR_OCT *p, int cons)
{
    int res, len;
    unsigned char *base, *c;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = ber_declen(o->bp, &len)) < 0)
	    	return 0;
	    o->bp += res;
	    o->left -= res;
	    if (cons)       /* fetch component strings */
	    {
	    	base = o->bp;
	    	while (more_chunks(o, base, len))
		    if (!odr_octetstring(o, &p, 0))
		    	return 0;
		return 1;
	    }
	    /* primitive octetstring */
	    if (len < 0)
	    	return 0;
	    if (len == 0)
	    	return 1;
	    if (len > p->size - p->len)
	    {
	    	c = nalloc(o, p->size += len);
	    	if (p->len)
		    memcpy(c, p->buf, p->len);
		p->buf = c;
	    }
	    memcpy(p->buf + p->len, o->bp, len);
	    p->len += len;
	    o->bp += len;
	    o->left -= len;
	    return 1;
    	case ODR_ENCODE:
	    if ((res = ber_enclen(o->bp, p->len, 5, 0)) < 0)
	    	return 0;
	    o->bp += res;
	    o->left -= res;
	    if (p->len == 0)
	    	return 1;
	    if (p->len > o->left)
	    	return 0;
	    memcpy(o->bp, p->buf, p->len);
	    o->bp += p->len;
	    o->left -= p->len;
	    return 1;
    	case ODR_PRINT: return 1;
    	default: return 0;
    }
}
