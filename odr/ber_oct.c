/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_oct.c,v $
 * Revision 1.5  1995-03-08 12:12:10  quinn
 * Added better error checking.
 *
 * Revision 1.4  1995/02/10  15:55:28  quinn
 * Bug fixes, mostly.
 *
 * Revision 1.3  1995/02/03  17:04:34  quinn
 * *** empty log message ***
 *
 * Revision 1.2  1995/02/02  20:38:50  quinn
 * Updates.
 *
 * Revision 1.1  1995/02/02  16:21:52  quinn
 * First kick.
 *
 */

#include <odr.h>

int ber_octetstring(ODR o, Odr_oct *p, int cons)
{
    int res, len;
    unsigned char *base, *c;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = ber_declen(o->bp, &len)) < 0)
	    {
	    	o->error = OPROTO;
	    	return 0;
	    }
	    o->bp += res;
	    o->left -= res;
	    if (cons)       /* fetch component strings */
	    {
	    	base = o->bp;
	    	while (odp_more_chunks(o, base, len))
		    if (!odr_octetstring(o, &p, 0))
		    	return 0;
		return 1;
	    }
	    /* primitive octetstring */
	    if (len < 0)
	    {
	    	o->error = OOTHER;
	    	return 0;
	    }
	    if (len + 1 > p->size - p->len)
	    {
	    	c = nalloc(o, p->size += len + 1);
	    	if (p->len)
		    memcpy(c, p->buf, p->len);
		p->buf = c;
	    }
	    if (len)
		memcpy(p->buf + p->len, o->bp, len);
	    p->len += len;
	    o->bp += len;
	    o->left -= len;
	    return 1;
    	case ODR_ENCODE:
	    if ((res = ber_enclen(o->bp, p->len, 5, 0)) < 0)
	    {
	    	o->error = OOTHER;
	    	return 0;
	    }
	    o->bp += res;
	    o->left -= res;
	    if (p->len == 0)
	    	return 1;
	    if (p->len > o->left)
	    {
	    	o->error = OSPACE;
	    	return 0;
	    }
	    memcpy(o->bp, p->buf, p->len);
	    o->bp += p->len;
	    o->left -= p->len;
	    return 1;
    	case ODR_PRINT: return 1;
    	default: o->error = OOTHER; return 0;
    }
}
