/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: proto.c,v $
 * Revision 1.1  1995-02-06 16:44:47  quinn
 * First hack at Z/SR protocol
 *
 */

#include <odr.h>

#include <proto.h>

int z_ReferenceId(ODR o, Z_ReferenceId **p, int opt)
{
    return odr_implicit(o, odr_octetstring, (Odr_oct**) p, ODR_CONTEXT, 2, opt);
}

int z_DatabaseName(Odr o, Z_DatabaseName **p, int opt)
{
    return odr_implicit(o, odr_visiblestring, (char **) p, ODR_CONTEXT, 105,
	opt);
}

int z_InitRequest(ODR o, Z_InitRequest **p, int opt)
{
    Z_InitRequest *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	odr_implicit(o, odr_bitstring, &pp->protocolVersion, ODR_CONTEXT, 
	    3, 0) &&
	odr_implicit(o, odr_bitstring, &pp->options, ODR_CONTEXT, 4, 0) &&
	odr_implicit(o, odr_integer, &pp->preferredMessageSize, ODR_CONTEXT,
	    5, 0) &&
	odr_implicit(o, odr_integer, &pp->maximumRecordSize, ODR_CONTEXT,
	    6, 0) &&
	odr_implicit(o, odr_visiblestring, &pp->idAuthentication, ODR_CONTEXT,
	    7, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationId, ODR_CONTEXT,
	    110, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationName, ODR_CONTEXT,
	    111, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationVersion,
	    ODR_CONTEXT, 112, 1) &&
	odr_sequence_end(o);
}

int z_InitResponse(ODR o, Z_InitResponse **p, int opt)
{
    Z_InitResponse *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	odr_implicit(o, odr_bitstring, &pp->protocolVersion, ODR_CONTEXT, 
	    3, 0) &&
	odr_implicit(o, odr_bitstring, &pp->options, ODR_CONTEXT, 4, 0) &&
	odr_implicit(o, odr_integer, &pp->preferredMessageSize, ODR_CONTEXT,
	    5, 0) &&
	odr_implicit(o, odr_integer, &pp->maximumRecordSize, ODR_CONTEXT,
	    6, 0) &&
	odr_implicit(o, odr_bool, &pp->result, ODR_CONTEXT, 12, 0) &&
	odr_implicit(o, odr_visiblestring, &pp->idAuthentication, ODR_CONTEXT,
	    7, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationId, ODR_CONTEXT,
	    110, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationName, ODR_CONTEXT,
	    111, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationVersion,
	    ODR_CONTEXT, 112, 1) &&
	odr_sequence_end(o);
}

int z_SearchRequest(ODR o, Z_SearchRequest **p, int opt)
{
    Z_SearchRequest *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	odr_implicit(o, odr_integer, &pp->smallSetUpperBound, ODR_CONTEXT,
	    13, 0) &&
	odr_implicit(o, odr_integer, &pp->largeSetLowerBound, ODR_CONTEXT,
	    14, 0) &&
	odr_implicit(o, odr_integer, &pp->mediumSetPresentNumber, ODR_CONTEXT,
	    15, 0) &&
	odr_implicit(o, odr_bool, &pp->replaceIndicator, ODR_CONTEXT, 16, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->resultSetName, ODR_CONTEXT,
	    17, 9) &&

        /* MORE */

	odr_sequence_end(o);
}

int z_APDU(ODR o, Z_APDU **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 20, Z_APDU_InitRequest, z_InitRequest},
    	{ODR_IMPLICIT, ODR_CONTEXT, 21, Z_APDU_InitResponse, z_InitResponse},
    	{ODR_IMPLICIT, ODR_CONTEXT, 22, Z_APDU_SearchRequest, z_SearchRequest},

    	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE && !*p)
    	*p = nalloc(o, sizeof(**p));
    if (!odr_choice(o, arm, &(*p)->u, &(*p)->which))
    {
    	if (o->direction == ODR_DECODE)
    	{
	    *p = 0;
	    return opt;
	}
    }
    return 1;
}
