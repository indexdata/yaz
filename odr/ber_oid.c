/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_oid.c,v $
 * Revision 1.2  1995-02-14 20:39:55  quinn
 * Fixed bugs in completeBER and (serious one in) ber_oid.
 *
 * Revision 1.1  1995/02/03  17:04:36  quinn
 * Initial revision
 *
 */

#include <odr.h>

int ber_oid(ODR o, Odr_oid *p)
{
    int len;
    unsigned char *lenp;
    int pos, n, res, id;
    unsigned char octs[8];

    switch (o->direction)
    {
	case ODR_DECODE:
	    if ((res = ber_declen(o->bp, &len)) < 1)
	    	return 0;
	    if (len < 0)
	    	return 0;
	    o->bp += res;
	    o->left -= res;
	    if (len == 0)
	    {
	    	*p = -1;
	    	return 1;
	    }
	    p[0] = *o->bp / 40;
	    if (p[0] > 2)
	    	p[0] = 2;
	    p[1] = *o->bp - p[0] * 40;
	    o->bp++;
	    o->left--;
	    pos = 2;
	    len--;
	    while (len)
	    {
	    	p[pos] = 0;
	    	do
	    	{
		    if (!len)
		    	return 0;
		    p[pos] <<= 7;
		    p[pos] |= *o->bp & 0X7F;
		    len--;
		    o->left--;
		}
		while (*(o->bp++) & 0X80);
		pos++;
	    }
	    p[pos] = -1;
	    return 1;
    	case ODR_ENCODE:
	    /* we'll allow ourselves the quiet luxury of only doing encodings
    	       shorter than 127 */
            lenp = o->bp;
            o->bp++;
            o->left--;
            if (p[0] < 0 && p[1] <= 0)
            	return 0;
	    p[1] = p[0] * 40 + p[1];
	    for (pos = 1; p[pos] >= 0; pos++)
	    {
	    	id = p[pos];
	    	n = 0;
	    	do
	    	{
		    octs[n++] = id & 0X7F;
		    id >>= 7;
		}
		while (id);
		if (n > o->left)
		    return 0;
		o->left -= n;
		while (n--)
		    *(o->bp++) = octs[n] | ((n > 0) << 7);
	    }
	    if (ber_enclen(lenp, (o->bp - lenp) - 1, 1, 1) != 1)
	    	return 0;
	    return 1;
    	default: return 0;
    }
}
