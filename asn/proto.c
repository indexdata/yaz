/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: proto.c,v $
 * Revision 1.2  1995-02-06 21:26:07  quinn
 * Repaired this evening's damages..
 *
 * Revision 1.1  1995/02/06  16:44:47  quinn
 * First hack at Z/SR protocol
 *
 */

#include <odr.h>

#include <proto.h>

/* ---------------------- INITIALIZE SERVICE ------------------- */

int z_ReferenceId(ODR o, Z_ReferenceId **p, int opt)
{
    return odr_implicit(o, odr_octetstring, (Odr_oct**) p, ODR_CONTEXT, 2, opt);
}

int z_DatabaseName(Odr o, Z_DatabaseName **p, int opt)
{
    return odr_implicit(o, odr_visiblestring, (char **) p, ODR_CONTEXT, 105,
	opt);
}

/* ---------------------- INITIALIZE SERVICE ------------------- */

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

/* ------------------------ SEARCH SERVICE ----------------------- */

int z_ElementSetName(ODR o, Z_ElementSetName **p, int opt)
{
    return odr_implicit(o, odr_visiblestring, (char**) p, ODR_CONTEXT, 103,
    	opt);
}

int z_PreferredRecordSyntax(ODR o, Z_PreferredRecordSyntax **p, int opt)
{
    return odr_implicit(o, odr_oid, (Odr_oid**) p, ODR_CONTEXT, 104, opt);
}

int z_DatabaseSpecificUnit(ODR o, Z_DatabaseSpecificUnit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	z_DatabaseName(o, &(*p)->databaseName, 0) &&
    	z_ElementSetName(o, &(*p)->elementSetName, 0) &&
    	odr_sequence_end(o);
}

int z_DatabaseSpecific(ODR o, Z_DatabaseSpecific **p, int opt)
{
    if (o->direction == ODR_DECODE && !*p)
    	*p = nalloc(o, sizeof(**p));
    else if (!*p)
    	return opt;

    odr_implicit_settag(o, ODR_CONTEXT, 1);
    if (odr_sequence_of(o, z_DatabaseSpecificUnit, &(*p)->elements,
    	&(*p)->num_elements))
    	return 1;
    *p = 0;
    return 0;
}

int z_ElementSetNames(ODR o, Z_ElementSetNames **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_ElementSetNames_generic,
	    z_ElementSetName},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ElementSetNames_databaseSpecific,
	    z_DatabaseSpecific},
	{-1, -1, -1, -1, 0}
    };

    if (!odr_constructed_begin(o, p, ODR_CONTEXT, 19, 0))
    	return opt;

    if (o->direction == ODR_DECODE && !*p)
    	*p = nalloc(o, sizeof(**p));

    if (odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
    	odr_constructed_enmd(o))
    	return 1;
    *p = 0;
    return 0;
}

/* ----------------------- RPN QUERY -----------------------*/

int z_RPNStructure(ODR o, Z_RPNStructure, int opt);

int z_Operand(ODR o, Z_Operand **p, int opt)
{
    Odr_arm arm[] =
    {
    	{-1, -1, -1, Z_Operand_APT, z_AttributesPlusTerm},
    	{-1, -1, -1, Z_Operand_resultSetId, z_ResultSetId},
    	{-1, -1, -1, -1, 0}
    };

    if (o->direction ==ODR_DECODE && !*p)
    	*p = nalloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt;
}

int z_Complex(ODR o, Z_Complex **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	z_RPNStructure(o, &(*p)->s1, 0) &&
    	z_RPNStructure(o, &(*p)->s2, 0) &&
    	z_Operator(o, &(*p)->operator) &&
    	odr_sequence_end(o);
}

int z_RPNStructure(ODR o, Z_RPNStructure, int opt)
{
    Odr_arm arm[] = 
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_RPNStructure_simple, z_Operand),
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_RPNStructure_complex, z_Complex},
    	{-1 -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE && !*p)
    	*p = nalloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt;
}

int z_RPNQuery(ODR o, Z_RPNQuery **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p))
    	return opt;
    return
    	odr_oid(o, &(*p)->attributeSetId, 0) &&
    	z_RPNStructure(o, &(*p)->RPNStructure, 0) &&
    	odr_sequence_end(o);
}

/* -----------------------END RPN QUERY ----------------------- */

int z_Query(ODR o, Z_Query **p, int opt)
{
    Odr_arm arm[] = 
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Query_type_1, z_RPNQuery},
    	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Query_type_2, odr_oct},
    	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE && !*p)
    	*p = nalloc(o, sizeof(**p);
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt;
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
	odr_implicit(o, z_ElementSetNames, &pp->smallSetElementSetNames,
	    ODR_CONTEXT, 100, 1) &&
	odr_implicit(o, z_ElementSetNames, &pp->mediumSetElementSetNames,
	    ODR_CONTEXT, 101, 1) &&
	z_PreferredRecordSyntax(o, &pp->preferredRecordSyntax, 1) &&
	odr_explicit(o, z_query, ODR_CONTEXT, 21, 0) &&
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
