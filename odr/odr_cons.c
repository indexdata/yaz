/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_cons.c,v $
 * Revision 1.3  1995-02-09 15:51:48  quinn
 * Works better now.
 *
 * Revision 1.2  1995/02/07  17:52:59  quinn
 * A damn mess, but now things work, I think.
 *
 * Revision 1.1  1995/02/02  16:21:53  quinn
 * First kick.
 *
 */

#include <odr.h>

int odr_constructed_begin(ODR o, void *p, int class, int tag)
{
    int res;
    int cons = 1;

    if (o->t_class < 0)
    {
	o->t_class = class;
	o->t_tag = tag;
    }
    if (o->direction == ODR_DECODE)
    	*(char**)p = 0;
    if ((res = ber_tag(o, *(char**)p, o->t_class, o->t_tag, &cons)) < 0)
    	return 0;
    if (!res || !cons)
    	return 0;

    o->stack[++(o->stackp)].lenb = o->bp;
    if (o->direction == ODR_ENCODE || o->direction == ODR_PRINT)
    {
	o->stack[o->stackp].lenlen = 1;
	o->bp++;
	o->left--;
    }
    else if (o->direction == ODR_DECODE)
    {
    	if ((res = ber_declen(o->bp, &o->stack[o->stackp].len)) < 0)
	    return 0;
	o->stack[o->stackp].lenlen = res;
	o->bp += res;
	o->left -= res;
    }
    else return 0;

    o->stack[o->stackp].base = o->bp;
    return 1;
}

int odr_constructed_end(ODR o)
{
    int res;

    if (o->stackp < 0)
    	return 0;
    switch (o->direction)
    {
    	case ODR_DECODE:
	    if (o->stack[o->stackp].len < 0)
	    {
	    	if (*o->bp++ == 0 && *(o->bp++) == 0)
	    	{
		    o->left -= 2;
		    return 1;
		}
		else
		    return 0;
	    }
	    else if (o->bp - o->stack[o->stackp].base !=
		o->stack[o->stackp].len)
	    	return 0;
	    o->stackp--;
	    return 1;
    	case ODR_ENCODE:
	    if ((res = ber_enclen(o->stack[o->stackp].lenb,
		o->bp - o->stack[o->stackp].base,
		o->stack[o->stackp].lenlen, 1)) < 0)
		    return 0;
	    if (res == 0)   /* indefinite encoding */
	    {
	    	*(o->bp++) = *(o->bp++) = 0;
	    	o->left--;
	    }
	    o->stackp--;
	    return 1;
    	case ODR_PRINT: return 1;
    	default: return 0;
    }
}
