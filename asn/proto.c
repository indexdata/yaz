/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: proto.c,v $
 * Revision 1.4  1995-02-10 15:54:30  quinn
 * Small adjustments.
 *
 * Revision 1.3  1995/02/09  15:51:39  quinn
 * Works better now.
 *
 * Revision 1.2  1995/02/06  21:26:07  quinn
 * Repaired this evening's damages..
 *
 * Revision 1.1  1995/02/06  16:44:47  quinn
 * First hack at Z/SR protocol
 *
 */

#include <odr.h>

#include <proto.h>

/* ---------------------- GLOBAL DEFS ------------------- */

int z_ReferenceId(ODR o, Z_ReferenceId **p, int opt)
{
    return odr_implicit(o, odr_octetstring, (Odr_oct**) p, ODR_CONTEXT, 2, opt);
}

int z_DatabaseName(ODR o, Z_DatabaseName **p, int opt)
{
    return odr_implicit(o, odr_visiblestring, (char **) p, ODR_CONTEXT, 105,
	opt);
}

int z_ResultSetId(ODR o, char **p, int opt)
{
    return odr_implicit(o, odr_visiblestring, (char **) p, ODR_CONTEXT, 31,
	opt);
}

int z_UserInformationField(ODR o, Z_UserInformationField **p, int opt)
{
    return odr_explicit(o, odr_external, (Odr_external **)p, ODR_CONTEXT,
    	11, opt);
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
	z_UserInformationField(o, &pp->userInformationField, 1) &&
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
	z_UserInformationField(o, &pp->userInformationField, 1) &&
	odr_sequence_end(o);
}

/* ------------------------ SEARCH SERVICE ----------------------- */

int z_ElementSetName(ODR o, char **p, int opt)
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
    if (o->direction == ODR_DECODE)
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

    if (!odr_constructed_begin(o, p, ODR_CONTEXT, 19))
    	return opt;

    if (o->direction == ODR_DECODE)
    	*p = nalloc(o, sizeof(**p));

    if (odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
    	odr_constructed_end(o))
    	return 1;
    *p = 0;
    return 0;
}

/* ----------------------- RPN QUERY -----------------------*/

int z_AttributeElement(ODR o, Z_AttributeElement **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	odr_implicit(o, odr_integer, &(*p)->attributeType, ODR_CONTEXT,
	    120, 1) &&
    	odr_implicit(o, odr_integer, &(*p)->attributeValue, ODR_CONTEXT,
	    121, 1) &&
    	odr_sequence_end(o);
}

int z_AttributesPlusTerm(ODR o, Z_AttributesPlusTerm **p, int opt)
{
    if (!(odr_implicit_settag(o, ODR_CONTEXT, 102) &&
    	odr_sequence_begin(o, p, sizeof(**p))))
    	return opt;
    return
    	odr_implicit_settag(o, ODR_CONTEXT, 44) &&
    	odr_sequence_of(o, z_AttributeElement, &(*p)->attributeList,
	    &(*p)->num_attributes) &&
	odr_implicit(o, odr_octetstring, &(*p)->term, ODR_CONTEXT, 45, 0) &&
	odr_sequence_end(o);
}

int z_Operator(ODR o, Z_Operator **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_Operator_and, odr_null},
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Operator_or, odr_null},
    	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Operator_and_not, odr_null},
    	{-1, -1, -1, -1, 0}
    };
    int dummy = 999;

    if (!*p && o->direction != ODR_DECODE)
    	return opt;
    if (!odr_constructed_begin(o, p, ODR_CONTEXT, 46))
    	return opt;
    if (o->direction == ODR_DECODE)
    	*p = nalloc(o, sizeof(**p));
    else
    	(*p)->u.and = &dummy;

    if (odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
    	odr_constructed_end(o))
    	return 1;
    *p = 0;
    return opt;
}

int z_Operand(ODR o, Z_Operand **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{-1, -1, -1, Z_Operand_APT, z_AttributesPlusTerm},
    	{-1, -1, -1, Z_Operand_resultSetId, z_ResultSetId},
    	{-1, -1, -1, -1, 0}
    };

    if (o->direction ==ODR_DECODE)
    	*p = nalloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt;
}

int z_RPNStructure(ODR o, Z_RPNStructure **p, int opt);

int z_Complex(ODR o, Z_Complex **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	z_RPNStructure(o, &(*p)->s1, 0) &&
    	z_RPNStructure(o, &(*p)->s2, 0) &&
    	z_Operator(o, &(*p)->operator, 0) &&
    	odr_sequence_end(o);
}

int z_RPNStructure(ODR o, Z_RPNStructure **p, int opt)
{
    static Odr_arm arm[] = 
    {
    	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_RPNStructure_simple, z_Operand},
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_RPNStructure_complex, z_Complex},
    	{-1 -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
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
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	odr_oid(o, &(*p)->attributeSetId, 0) &&
    	z_RPNStructure(o, &(*p)->RPNStructure, 0) &&
    	odr_sequence_end(o);
}

/* -----------------------END RPN QUERY ----------------------- */

int z_Query(ODR o, Z_Query **p, int opt)
{
    static Odr_arm arm[] = 
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Query_type_1, z_RPNQuery},
    	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Query_type_2, odr_octetstring},
    	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
    	*p = nalloc(o, sizeof(**p));
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
	odr_implicit_settag(o, ODR_CONTEXT, 18) &&
	odr_sequence_of(o, z_DatabaseName, &pp->databaseNames,
	    &pp->num_databaseNames) &&
	odr_implicit(o, z_ElementSetNames, &pp->smallSetElementSetNames,
	    ODR_CONTEXT, 100, 1) &&
	odr_implicit(o, z_ElementSetNames, &pp->mediumSetElementSetNames,
	    ODR_CONTEXT, 101, 1) &&
	z_PreferredRecordSyntax(o, &pp->preferredRecordSyntax, 1) &&
	odr_explicit(o, z_Query, &pp->query, ODR_CONTEXT, 21, 0) &&
	odr_sequence_end(o);
}

/* ------------------------ RECORD ------------------------- */

int z_DatabaseRecord(ODR o, Z_DatabaseRecord **p, int opt)
{
    return odr_external(o, (Odr_external **) p, opt);
}

int z_DiagRec(ODR o, Z_DiagRec **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	odr_oid(o, &(*p)->diagnosticSetId, 1) &&       /* SHOULD NOT BE OPT */
    	odr_integer(o, &(*p)->condition, 0) &&
    	(odr_visiblestring(o, &(*p)->addinfo, 0) ||
    	odr_implicit(o, odr_cstring, &(*p)->addinfo, ODR_CONTEXT, ODR_VISIBLESTRING, 1)) &&
    	odr_sequence_end(o);
}

int z_NamePlusRecord(ODR o, Z_NamePlusRecord **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_EXPLICIT, ODR_CONTEXT, 1, Z_NamePlusRecord_databaseRecord,
	    z_DatabaseRecord},
	{ODR_EXPLICIT, ODR_CONTEXT, 2, Z_NamePlusRecord_surrogateDiagnostic,
	    z_DiagRec},
	{-1, -1, -1, -1, 0}
    };

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	odr_implicit(o, z_DatabaseName, &(*p)->databaseName, ODR_CONTEXT,
	    0, 1) &&
	odr_constructed_begin(o, &(*p)->u, ODR_CONTEXT, 1) &&
	odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
	odr_constructed_end(o) &&
	odr_sequence_end(o);
}

int z_NamePlusRecordList(ODR o, Z_NamePlusRecordList **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = nalloc(o, sizeof(**p));
    if (odr_sequence_of(o, z_NamePlusRecord, &(*p)->records,
	&(*p)->num_records))
    	return 1;
    *p = 0;
    return 0;
}

int z_Records(ODR o, Z_Records **p, int opt)
{
    Odr_arm arm[] = 
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 28, Z_Records_DBOSD, z_NamePlusRecordList},
    	{ODR_IMPLICIT, ODR_CONTEXT, 130, Z_Records_NSD, z_DiagRec},
    	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
    	*p = nalloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt;
}

/* ------------------------ SEARCHRESPONSE ----------------*/

int z_NumberOfRecordsReturned(ODR o, int **p, int opt)
{
    return odr_implicit(o, odr_integer, p, ODR_CONTEXT, 24, opt);
}

int z_NextResultSetPosition(ODR o, int **p, int opt)
{
    return odr_implicit(o, odr_integer, p, ODR_CONTEXT, 25, opt);
}

int z_PresentStatus(ODR o, int **p, int opt)
{
    return odr_implicit(o, odr_integer, p, ODR_CONTEXT, 27, opt);
}

int z_SearchResponse(ODR o, Z_SearchResponse **p, int opt)
{
    Z_SearchResponse *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	odr_implicit(o, odr_integer, &pp->resultCount, ODR_CONTEXT, 23, 0) &&
    	z_NumberOfRecordsReturned(o, &pp->numberOfRecordsReturned, 0) &&
    	z_NextResultSetPosition(o, &pp->nextResultSetPosition, 0) &&
    	odr_implicit(o, odr_bool, &pp->searchStatus, ODR_CONTEXT, 22, 0) &&
    	odr_implicit(o, odr_integer, &pp->resultSetStatus, ODR_CONTEXT, 26, 1) &&
    	z_PresentStatus(o, &pp->presentStatus, 1) &&
    	z_Records(o, &pp->records, 1) &&
	odr_sequence_end(o);
}

/* --------------------- PRESENT SERVICE ---------------------- */

int z_PresentRequest(ODR o, Z_PresentRequest **p, int opt)
{
    Z_PresentRequest *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	z_ResultSetId(o, &pp->resultSetId, 0) &&
    	odr_implicit(o, odr_integer, &pp->resultSetStartPoint, ODR_CONTEXT,
	    30, 0) &&
	odr_implicit(o, odr_integer, &pp->numberOfRecordsRequested, ODR_CONTEXT,
	    29, 0) &&
	z_ElementSetNames(o, &pp->elementSetNames, 1) &&
	z_PreferredRecordSyntax(o, &pp->preferredRecordSyntax, 1) &&
	odr_sequence_end(o);
}

int z_PresentResponse(ODR o, Z_PresentResponse **p, int opt)
{
    Z_PresentResponse *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	z_NumberOfRecordsReturned(o, &pp->numberOfRecordsReturned, 0) &&
    	z_NextResultSetPosition(o, &pp->nextResultSetPosition, 0) &&
    	z_PresentStatus(o, &pp->presentStatus, 0) &&
    	z_Records(o, &pp->records, 1) &&
    	odr_sequence_end(o);
}

/* ------------------------ APDU ------------------------- */

int z_APDU(ODR o, Z_APDU **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 20, Z_APDU_initRequest, z_InitRequest},
    	{ODR_IMPLICIT, ODR_CONTEXT, 21, Z_APDU_initResponse, z_InitResponse},
    	{ODR_IMPLICIT, ODR_CONTEXT, 22, Z_APDU_searchRequest, z_SearchRequest},
    	{ODR_IMPLICIT, ODR_CONTEXT, 23, Z_APDU_searchResponse, z_SearchResponse},
    	{ODR_IMPLICIT, ODR_CONTEXT, 24, Z_APDU_presentRequest, z_PresentRequest},
    	{ODR_IMPLICIT, ODR_CONTEXT, 25, Z_APDU_presentResponse, z_PresentResponse},

    	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
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
