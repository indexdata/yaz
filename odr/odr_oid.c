/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_oid.c,v $
 * Revision 1.13  1998-02-11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.12  1995/09/29 17:12:25  quinn
 * Smallish
 *
 * Revision 1.11  1995/09/27  15:03:00  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.10  1995/05/29  08:11:44  quinn
 * Moved oid from odr/asn to util.
 *
 * Revision 1.9  1995/05/16  08:50:57  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.8  1995/03/17  10:17:55  quinn
 * Added memory management.
 *
 * Revision 1.7  1995/03/08  12:12:29  quinn
 * Added better error checking.
 *
 * Revision 1.6  1995/03/01  08:40:56  quinn
 * Smallish changes.
 *
 * Revision 1.5  1995/02/10  18:57:26  quinn
 * More in the way of error-checking.
 *
 * Revision 1.4  1995/02/10  15:55:29  quinn
 * Bug fixes, mostly.
 *
 * Revision 1.3  1995/02/09  15:51:49  quinn
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
#include <oid.h>

/*
 * Top level oid en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_oid(ODR o, Odr_oid **p, int opt)
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
    	o->error = OPROTO;
	return 0;
    }
    if (o->direction == ODR_PRINT)
    {
    	int i;

    	fprintf(o->print, "%sOID:", odr_indent(o));
    	for (i = 0; (*p)[i] > -1; i++)
	    fprintf(o->print, " %d", (*p)[i]);
	fprintf(o->print, "\n");
    	return 1;
    }
    if (o->direction == ODR_DECODE)
    	*p = (int *)odr_malloc(o, OID_SIZE * sizeof(**p));
    return ber_oidc(o, *p);
}
