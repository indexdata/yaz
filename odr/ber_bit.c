/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_bit.c,v $
 * Revision 1.2  1995-02-03 17:04:31  quinn
 * *** empty log message ***
 *
 * Revision 1.1  1995/02/02  20:38:49  quinn
 * Updates.
 *
 *
 */

#include <odr.h>

int ber_bitstring(ODR o, Odr_bitmask *p, int cons)
{
    int res, len;
    unsigned char *base;

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
	    	while (odp_more_chunks(o, base, len))
		    if (!odr_bitstring(o, &p, 0))
		    	return 0;
		return 1;
	    }
	    /* primitive bitstring */
	    if (len < 0)
	    	return 0;
	    if (len == 0)
	    	return 1;
	    if (len - 1 > ODR_BITMASK_SIZE)
	    	return 0;
	    o->bp++;      /* silently ignore the unused-bits field */
	    o->left--;
	    len--;
	    memcpy(p->bits + p->top + 1, o->bp, len);
	    p->top += len;
	    o->bp += len;
	    o->left -= len;
	    return 1;
    	case ODR_ENCODE:
	    if ((res = ber_enclen(o->bp, p->top + 2, 5, 0)) < 0)
	    	return 0;
	    o->bp += res;
	    o->left -= res;
	    if (p->top + 2 > o->left)
	    	return 0;
	    *(o->bp++) = 0;    /* no unused bits here */
	    o->left--;
	    if (p->top < 0)
	    	return 1;
	    memcpy(o->bp, p->bits, p->top + 1);
	    o->bp += p->top + 1;
	    o->left -= p->top +1;
	    return 1;
    	case ODR_PRINT: return 1;
    	default: return 0;
    }
}
