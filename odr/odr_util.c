/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: odr_util.c,v 1.23 2003-05-20 19:55:30 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "odr-priv.h"
#include <yaz/oid.h>

void odr_prname(ODR o, const char *name)
{
    if (name)
	fprintf (o->print, "%*s%s ", o->indent*4, "", name);
    else
	fprintf (o->print, "%*s", o->indent*4, "");
}

int odp_more_chunks(ODR o, const unsigned char *base, int len)
{
    if (!len)
    	return 0;
    if (len < 0) /* indefinite length */
    {
	if (*o->bp == 0 && *(o->bp + 1) == 0)
	{
	    o->bp += 2;
	    return 0;
	}
	else
	    return 1;
    }
    else
        return o->bp - base < len;
}

Odr_oid *odr_oiddup_nmem(NMEM nmem, Odr_oid *o)
{
    Odr_oid *r;

    if (!o)
    	return 0;
    if (!(r = (int *)nmem_malloc(nmem, (oid_oidlen(o) + 1) * sizeof(int))))
    	return 0;
    oid_oidcpy(r, o);
    return r;
}

Odr_oid *odr_oiddup(ODR odr, Odr_oid *o)
{
    return odr_oiddup_nmem (odr->mem, o);
}

Odr_oid *odr_getoidbystr_nmem(NMEM nmem, const char *str)
{
    int num = 1, i = 0;
    const char *p = str;
    Odr_oid *ret;

    if (!isdigit(*str))
	return 0;
    while ((p = strchr(p, '.')))
	num++, p++;
    ret = (int *)nmem_malloc(nmem, sizeof(*ret)*(num + 1));
    p = str;
    do
	ret[i++] = atoi(p);
    while ((p = strchr(p, '.')) && *++p);
    ret[i] = -1;
    return ret;
}

Odr_oid *odr_getoidbystr(ODR o, const char *str)
{
    return odr_getoidbystr_nmem (o->mem, str);
}

int odr_missing(ODR o, int opt, const char *name)
{
    if (o->error)
        return 0;
    if (!opt)
    {
        printf ("odr_missing set error : %s\n", name);
        odr_seterror(o, OREQUIRED, 55);
        odr_setaddinfo(o, name);
    }
    return opt;
}
