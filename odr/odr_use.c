/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_use.c,v $
 * Revision 1.3  1995-05-16 08:51:00  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.2  1995/02/09  15:51:50  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/03  17:04:39  quinn
 * Initial revision
 *
 */

#include <odr.h>
#include <odr_use.h>

int odr_external(ODR o, Odr_external **p, int opt)
{
    Odr_external *pp;
    static Odr_arm arm[] =
    {
    	{ODR_EXPLICIT, ODR_CONTEXT, 0, ODR_EXTERNAL_single, odr_any},
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, ODR_EXTERNAL_octet, odr_octetstring},
    	{ODR_IMPLICIT, ODR_CONTEXT, 2, ODR_EXTERNAL_arbitrary, odr_bitstring},
    	{-1, -1, -1, -1, 0}
    };

    odr_implicit_settag(o, ODR_UNIVERSAL, ODR_EXTERNAL);
    if (!odr_sequence_begin(o, p, sizeof(Odr_external)))
    	return opt;
    pp = *p;
    return
    	odr_oid(o, &pp->direct_reference, 1) &&
    	odr_integer(o, &pp->indirect_reference, 1) &&
    	odr_graphicstring(o, &pp->descriptor, 1) &&
    	odr_choice(o, arm, &pp->u, &pp->which) &&
	odr_sequence_end(o);
}

int odr_visiblestring(ODR o, char **p, int opt)
{
    return odr_implicit(o, odr_cstring, p, ODR_UNIVERSAL, ODR_VISIBLESTRING,
    	opt);
}    

int odr_graphicstring(ODR o, char **p, int opt)
{
    return odr_implicit(o, odr_cstring, p, ODR_UNIVERSAL, ODR_GRAPHICSTRING,
    	opt);
}    
