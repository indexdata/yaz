/*
 * Copyright (c) 1995-2002, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: odr_choice.c,v 1.20 2002-07-25 12:51:08 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

int odr_choice(ODR o, Odr_arm arm[], void *p, void *whichp,
	       const char *name)
{
    int i, cl = -1, tg, cn, *which = (int *)whichp, bias = o->choice_bias;

    if (o->error)
    	return 0;
    if (o->direction != ODR_DECODE && !*(char**)p)
    	return 0;

    if (o->direction == ODR_DECODE)
    {
        *which = -1;
        *(char**)p = 0;
    }
    o->choice_bias = -1;

    if (o->direction == ODR_PRINT)
    {
	if (name)
	{
	    odr_prname(o, name);
	    fprintf (o->print, "choice\n");
	}
    }
    for (i = 0; arm[i].fun; i++)
    {
    	if (o->direction == ODR_DECODE)
	{
	    if (bias >= 0 && bias != arm[i].which)
		continue;
    	    *which = arm[i].which;
	}
	else if (*which != arm[i].which)
	    continue;

    	if (arm[i].tagmode != ODR_NONE)
    	{
	    if (o->direction == ODR_DECODE && cl < 0)
	    {
	    	if (o->op->stackp > -1 && !odr_constructed_more(o))
		    return 0;
	    	if (ber_dectag(o->bp, &cl, &tg, &cn) <= 0)
		    return 0;
	    }
	    else if (o->direction != ODR_DECODE)
	    {
	    	cl = arm[i].zclass;
	    	tg = arm[i].tag;
	    }
	    if (tg == arm[i].tag && cl == arm[i].zclass)
	    {
	    	if (arm[i].tagmode == ODR_IMPLICIT)
	    	{
		    odr_implicit_settag(o, cl, tg);
	    	    return (*arm[i].fun)(o, (char **)p, 0, arm[i].name);
		}
		/* explicit */
		if (!odr_constructed_begin(o, p, cl, tg, 0))
		    return 0;
		return (*arm[i].fun)(o, (char **)p, 0, arm[i].name) &&
		    odr_constructed_end(o);
	    }
	}
	else  /* no tagging. Have to poll type */
	{
	    if ((*arm[i].fun)(o, (char **)p, 1, arm[i].name) && *(char**)p)
	    	return 1;
	}
    }
    return 0;
}

void odr_choice_bias(ODR o, int what)
{
    if (o->enable_bias)
        o->choice_bias = what;
}

void odr_choice_enable_bias (ODR o, int mode)
{
    o->enable_bias = mode;
}
