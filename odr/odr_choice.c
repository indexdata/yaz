/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_choice.c,v $
 * Revision 1.18  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.17  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.16  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.15  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.14  1997/05/14 06:53:57  adam
 * C++ support.
 *
 * Revision 1.13  1997/04/30 08:52:10  quinn
 * Null
 *
 * Revision 1.12  1996/10/08  12:58:17  adam
 * New ODR function, odr_choice_enable_bias, to control behaviour of
 * odr_choice_bias.
 *
 * Revision 1.11  1995/09/29  17:12:23  quinn
 * Smallish
 *
 * Revision 1.10  1995/09/27  15:02:58  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.9  1995/08/15  12:00:23  quinn
 * Updated External
 *
 * Revision 1.8  1995/06/19  17:01:51  quinn
 * This should bring us in sync with the version distributed as 1.0b
 *
 * Revision 1.7  1995/06/19  13:06:50  quinn
 * Fixed simple bug in the code to handle untagged choice elements.
 *
 * Revision 1.6  1995/05/16  08:50:53  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.5  1995/03/18  12:16:31  quinn
 * Minor changes.
 *
 * Revision 1.4  1995/03/14  16:59:38  quinn
 * Added odr_constructed_more check
 *
 * Revision 1.3  1995/03/08  12:12:22  quinn
 * Added better error checking.
 *
 * Revision 1.2  1995/02/09  15:51:48  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/07  17:52:59  quinn
 * A damn mess, but now things work, I think.
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/odr.h>

int odr_choice(ODR o, Odr_arm arm[], void *p, void *whichp,
	       const char *name)
{
    int i, cl = -1, tg, cn, *which = (int *)whichp, bias = o->choice_bias;

    if (o->error)
    	return 0;
    if (o->direction != ODR_DECODE && !*(char**)p)
    	return 0;
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
	    	if (o->stackp > -1 && !odr_constructed_more(o))
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
    *which = -1;
    *(char*)p = 0;
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
