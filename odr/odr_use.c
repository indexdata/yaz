/*
 * Copyright (c) 1995-1999, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_use.c,v $
 * Revision 1.9  1999-04-20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.8  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.7  1995/09/29 17:12:27  quinn
 * Smallish
 *
 * Revision 1.6  1995/09/27  15:03:00  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.5  1995/08/10  08:54:47  quinn
 * Added Explain.
 *
 * Revision 1.4  1995/06/16  13:16:12  quinn
 * Fixed Defaultdiagformat.
 *
 * Revision 1.3  1995/05/16  08:51:00  quinn
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
    return odr_implicit_tag(o, odr_cstring, p, ODR_UNIVERSAL,
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
