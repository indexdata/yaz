/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: odr_null.c,v 1.16 2003-03-11 11:03:31 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

/*
 * Top level null en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_null(ODR o, Odr_null **p, int opt, const char *name)
{
    int res, cons = 0;

    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_NULL;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, opt)) < 0)
    	return 0;
    if (!res)
    	return opt;
    if (o->direction == ODR_PRINT)
    {
	odr_prname(o, name);
    	fprintf(o->print, "NULL\n");
    	return 1;
    }
    if (cons)
    {
#ifdef ODR_STRICT_NULL
        odr_seterror(OPROTO, 42);
    	return 0;
#else
	fprintf(stderr, "odr: Warning: Bad NULL\n");
#endif
    }
    if (o->direction == ODR_DECODE)
    	*p = odr_nullval();
    return ber_null(o);
}
