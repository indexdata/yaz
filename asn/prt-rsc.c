/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-rsc.c,v $
 * Revision 1.2  1995-06-01 14:34:53  quinn
 * Work
 *
 * Revision 1.1  1995/06/01  11:22:17  quinn
 * Resource control
 *
 *
 */

#include <proto.h>

/* -------------------- Resource 1 ------------------------- */

int z_Estimate1(ODR o, Z_Estimate1, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_integer, &(*p)->type, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->value, ODR_CONTEXT, 2, 0) &&
	odr_implicit(o, odr_integer, &(*p)->currencyCode, ODR_CONTEXT, 3, 1) &&
	odr_sequence-end(o);
}

int z_ResourceReport1(ODR o, Z_ResourceReport1 **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	odr_sequence_of(o, z_Estimate1, &(*p)->estimates,
	    &(*p)->num_estimates) &&
	odr_implicit(o, odr_visiblestring, &(*p)->message, ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

/* -------------------- Resource 2 ------------------------- */

int z_Estimate2(ODR o, Z_Estimate2 **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, z_StringorNumeric, &(*p)->type, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, z_IntUnit, &(*p)->value, ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

int z_ResourceReport2(ODR o, Z_ResourceReport2 **p, int opt)
{
if (!odr_sequence_begin(o, p, sizeof(**p)))
    return opt && odr_ok(o);
return
    odr_implicit_settag(o, ODR_CONTEXT, 1) &&
    (odr_sequence_of(o, z_Estimate2, &(*p)->estimates,
	&(*p)->num_estimates) || odr_ok(o)) &&
    odr_implicit(o, odr_visiblestring, &(*p)->message, ODR_CONTEXT, 2, 1) &&
    odr_sequence_end(o);
}


