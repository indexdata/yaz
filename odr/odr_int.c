/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: odr_int.c,v 1.18 2003-03-11 11:03:31 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

/*
 * Top level integer en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_integer(ODR o, int **p, int opt, const char *name)
{
    int res, cons = 0;

    if (o->error)
        return 0;
    if (o->t_class < 0)
    {
        o->t_class = ODR_UNIVERSAL;
        o->t_tag = ODR_INTEGER;
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
        odr_seterror(o, OPROTO, 1);
        return 0;
    }
    if (o->direction == ODR_DECODE)
        *p = (int *)odr_malloc(o, sizeof(int));
    return ber_integer(o, *p);
}
