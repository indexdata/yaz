/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: odr_oid.c,v 1.19 2003-03-11 11:03:31 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"
#include <yaz/oid.h>

/*
 * Top level oid en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_oid(ODR o, Odr_oid **p, int opt, const char *name)
{
    int res, cons = 0;

    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_OID;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, opt)) < 0)
    	return 0;
    if (!res)
    	return opt;
    if (cons)
    {
        odr_seterror(o, OPROTO, 46);
	return 0;
    }
    if (o->direction == ODR_PRINT)
    {
    	int i;

	odr_prname(o, name);
    	fprintf(o->print, "OID:");
    	for (i = 0; (*p)[i] > -1; i++)
	    fprintf(o->print, " %d", (*p)[i]);
	fprintf(o->print, "\n");
    	return 1;
    }
    if (o->direction == ODR_DECODE)
    	*p = (int *)odr_malloc(o, OID_SIZE * sizeof(**p));
    return ber_oidc(o, *p);
}
