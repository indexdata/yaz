/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_tag.c,v $
 * Revision 1.9  1998-02-11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.8  1997/05/14 06:53:59  adam
 * C++ support.
 *
 * Revision 1.7  1996/02/20 12:52:54  quinn
 * Added odr_peektag
 *
 * Revision 1.6  1995/12/14  16:28:26  quinn
 * More explain stuff.
 *
 * Revision 1.5  1995/09/29  17:12:27  quinn
 * Smallish
 *
 * Revision 1.4  1995/09/27  15:03:00  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.3  1995/05/16  08:51:00  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.2  1995/03/08  12:12:31  quinn
 * Added better error checking.
 *
 * Revision 1.1  1995/02/02  16:21:54  quinn
 * First kick.
 *
 */

#include <odr.h>

int odr_peektag(ODR o, int *zclass, int *tag, int *cons)
{
    if (o->direction != ODR_DECODE)
    {
	o->error = OOTHER;
	return 0;
    }
    if (o->stackp > -1 && !odr_constructed_more(o))
	return 0;
    if (ber_dectag(o->bp, zclass, tag, cons) <= 0)
    {
	o->error = OREQUIRED;
	return 0;
    }
    return 1;
}

int odr_implicit_settag(ODR o, int zclass, int tag)
{
    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
	o->t_class = zclass;
	o->t_tag = tag;
    }
    return 1;
}

int odr_initmember(ODR o, void *p, int size)
{
    char **pp = (char **) p;

    if (o->error)
	return 0;
    if (o->direction == ODR_DECODE)
	*pp = (char *)odr_malloc(o, size);
    else if (!*pp)
    {
	o->t_class = -1;
	return 0;
    }
    return 1;
}
