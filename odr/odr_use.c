/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_use.c,v $
 * Revision 1.1  1995-02-03 17:04:39  quinn
 * Initial revision
 *
 */

#include <odr.h>
#include <odr_use.h>

int odr_external(ODR o, Odr_external **p, int opt)
{
    Odr_external *pp;
    
    odr_implicit_settag(o, ODR_UNIVERSAL, ODR_EXTERNAL);
    if (!odr_sequence_begin(o, p, sizeof(Odr_external)))
    	return opt;
    pp = *p;
    return
    	odr_oid(o, &pp->direct_reference, 1) &&
    	odr_integer(o, &pp->indirect_reference, 1) &&
    	odr_graphicstring(o, &pp->descriptor, 1) &&
	odr_implicit(o, odr_octetstring, &pp->octet_aligned, ODR_CONTEXT,
	    1, 0) &&
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
