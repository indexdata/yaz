/*
 * Copyright (c) 1995-2002, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: odr_tag.c,v 1.12 2002-07-25 12:51:08 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

int odr_peektag(ODR o, int *zclass, int *tag, int *cons)
{
    if (o->direction != ODR_DECODE)
    {
	o->error = OOTHER;
	return 0;
    }
    if (o->op->stackp > -1 && !odr_constructed_more(o))
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
