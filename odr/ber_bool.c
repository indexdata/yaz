/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_bool.c,v $
 * Revision 1.1  1995-02-02 16:21:51  quinn
 * First kick.
 *
 */

#include <stdio.h>
#include <odr.h>


int ber_boolean(ODR o, int *val)
{
    unsigned char *b = o->bp;
    int res, len;

    switch (o->direction)
    {
    	case ODR_ENCODE:
	    if (ber_enclen(o->bp, 1, 1, 1) != 1)
		return 0;
	    o->bp++;
	    o->left--;
	    *(o->bp++) = (unsigned char) *val;
	    fprintf(stderr, "[val=%d]\n", *val);
	    o->left--;
	    return 1;
	case ODR_DECODE:
	    if ((res = ber_declen(b, &len)) < 0)
		return 0;
	    if (len != 1)
		return 0;
	    o->bp+= res;
	    o->left -= res;
	    *val = *b;
	    o->bp++;
	    o->left--;
	    fprintf(stderr, "[val=%d]\n", *val);
	    return 1;
	case ODR_PRINT:
	    return 1;
	default: return 0;
    }
}
