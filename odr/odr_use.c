/*
 * Copyright (c) 1995-2002, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: odr_use.c,v 1.12 2002-07-25 12:51:08 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

int odr_external(ODR o, Odr_external **p, int opt, const char *name)
{
    Odr_external *pp;
    static Odr_arm arm[] =
    {
    	{ODR_EXPLICIT, ODR_CONTEXT, 0, ODR_EXTERNAL_single, 
	 (Odr_fun)odr_any, "single"},
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, ODR_EXTERNAL_octet,
	 (Odr_fun)odr_octetstring, "octet"},
    	{ODR_IMPLICIT, ODR_CONTEXT, 2, ODR_EXTERNAL_arbitrary,
	 (Odr_fun)odr_bitstring, "arbitrary"},
    	{-1, -1, -1, -1, 0, 0}
    };

    odr_implicit_settag(o, ODR_UNIVERSAL, ODR_EXTERNAL);
    if (!odr_sequence_begin(o, p, sizeof(Odr_external), name))
    	return opt;
    pp = *p;
    return
    	odr_oid(o, &pp->direct_reference, 1, "direct") &&
    	odr_integer(o, &pp->indirect_reference, 1, "indirect") &&
    	odr_graphicstring(o, &pp->descriptor, 1, "descriptor") &&
    	odr_choice(o, arm, &pp->u, &pp->which, 0) &&
	odr_sequence_end(o);
}

int odr_visiblestring(ODR o, char **p, int opt, const char *name)
{
    return odr_implicit_tag(o, odr_cstring, p, ODR_UNIVERSAL,
			    ODR_VISIBLESTRING, opt, name);
}    

/*
 * a char may not be sufficient to hold a general string, but we'll deal
 * with that once we start looking more closely at UniCode & co.
 */
int odr_generalstring(ODR o, char **p, int opt, const char *name)
{
    return odr_implicit_tag(o, odr_iconv_string, p, ODR_UNIVERSAL,
			    ODR_GENERALSTRING,opt, name);
}    

int odr_graphicstring(ODR o, char **p, int opt, const char *name)
{
    return odr_implicit_tag(o, odr_cstring, p, ODR_UNIVERSAL,
			ODR_GRAPHICSTRING, opt, name);
}    

int odr_generalizedtime(ODR o, char **p, int opt, const char *name)
{
    return odr_implicit_tag(o, odr_cstring, p, ODR_UNIVERSAL,
			    ODR_GENERALIZEDTIME, opt, name);
}
