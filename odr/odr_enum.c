/*
 * Copyright (c) 1995-2002, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: odr_enum.c,v 1.6 2002-07-25 12:51:08 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

/*
 * Top level enum en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_enum(ODR o, int **p, int opt, const char *name)
{
    int res, cons = 0;

    if (o->error)
        return 0;
    if (o->t_class < 0)
    {
        o->t_class = ODR_UNIVERSAL;
        o->t_tag = ODR_ENUM;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, opt)) < 0)
        return 0;
    if (!res)
        return opt;
    if (o->direction == ODR_PRINT)
    {
	odr_prname(o, name);
        fprintf(o->print, "%d\n", **p);
        return 1;
    }
    if (cons)
    {
        o->error = OPROTO;
        return 0;
    }
    if (o->direction == ODR_DECODE)
        *p = (int *)odr_malloc(o, sizeof(int));
    return ber_integer(o, *p);
}
