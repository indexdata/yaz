/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_oid.c,v $
 * Revision 1.3  1995-02-09 15:51:49  quinn
 * Works better now.
 *
 * Revision 1.2  1995/02/07  14:13:46  quinn
 * Bug fixes.
 *
 * Revision 1.1  1995/02/03  17:04:38  quinn
 * Initial revision
 *
 *
 */

#include <odr.h>

/*
 * Top level oid en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_oid(ODR o, Odr_oid **p, int opt)
{
    int res, cons = 0;

    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_OID;
    }
    if (o->direction == ODR_DECODE)
    	*p =0;
    if ((res = ber_tag(o, *p, o->t_class, o->t_tag, &cons)) < 0)
    {
    	*p = 0;
    	return 0;
    }
    if (!res || cons)
    {
    	*p = 0;
    	return opt;
    }
    if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "%sOID\n", odr_indent(o));
    	return 1;
    }
    if (o->direction == ODR_DECODE)
    	*p = nalloc(o, ODR_OID_SIZE);
    return ber_oid(o, *p);
}
