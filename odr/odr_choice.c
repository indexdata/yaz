/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_choice.c,v $
 * Revision 1.3  1995-03-08 12:12:22  quinn
 * Added better error checking.
 *
 * Revision 1.2  1995/02/09  15:51:48  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/07  17:52:59  quinn
 * A damn mess, but now things work, I think.
 *
 */

#include <odr.h>

int odr_choice(ODR o, Odr_arm arm[], void *p, int *which)
{
    int i, cl = -1, tg, cn;

    if (o->error)
    	return 0;
    if (o->direction != ODR_DECODE && !*(char**)p)
    	return 0;
    for (i = 0; arm[i].fun; i++)
    {
    	if (o->direction == ODR_DECODE)
    	    *which = arm[i].which;
	else if (*which != arm[i].which)
	    continue;

    	if (arm[i].tagmode != ODR_NONE)
    	{
	    if (o->direction == ODR_DECODE && cl < 0)
	    {
	    	if (ber_dectag(o->bp, &cl, &tg, &cn) <= 0)
		    return 0;
	    }
	    else if (o->direction != ODR_DECODE)
	    {
	    	cl = arm[i].class;
	    	tg = arm[i].tag;
	    }
	    if (tg == arm[i].tag && cl == arm[i].class)
	    {
	    	if (arm[i].tagmode == ODR_IMPLICIT)
	    	{
		    odr_implicit_settag(o, cl, tg);
	    	    return (*arm[i].fun)(o, p, 0);
		}
		/* explicit */
		if (!odr_constructed_begin(o, p, cl, tg))
		    return 0;
		return (*arm[i].fun)(o, p, 0) &&
		    odr_constructed_end(o);
	    }
	}
	else  /* no tagging. Have to poll type */
	    if ((*arm[i].fun)(o, p, 0))
	    	return 1;
    }
    *which = -1;
    *(char*)p = 0;
    return 0;
}
