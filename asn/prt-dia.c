/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-dia.c,v $
 * Revision 1.2  1995-09-27 15:02:41  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.1  1995/08/29  11:19:31  quinn
 * Added Diagnostic Format
 *
 *
 */

#include <proto.h>

int z_TooMany(ODR o, Z_TooMany **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
        odr_implicit(o, odr_integer, &(*p)->tooManyWhat, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->max, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_BadSpec(ODR o, Z_BadSpec **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_Specification, &(*p)->spec, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, z_DatabaseName, &(*p)->db, ODR_CONTEXT, 2, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, z_Specification, &(*p)->goodOnes,
	    &(*p)->num_goodOnes) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_DbUnavailWhy(ODR o, Z_DbUnavailWhy **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
        odr_implicit(o, odr_integer, &(*p)->reasonCode, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->message, ODR_CONTEXT,
	    2, 1) &&
	odr_sequence_end(o);
}

int z_DbUnavail(ODR o, Z_DbUnavail **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_DatabaseName, &(*p)->db, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, z_DbUnavailWhy, &(*p)->why, ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

int z_Attribute(ODR o, Z_Attribute **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
        odr_implicit(o, odr_oid, &(*p)->id, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->type, ODR_CONTEXT, 2, 1) &&
	odr_implicit(o, odr_integer, &(*p)->value, ODR_CONTEXT, 3, 1) &&
	odr_explicit(o, z_Term, &(*p)->term, ODR_CONTEXT, 4, 1) &&
	odr_sequence_end(o);
}

int z_AttCombo(ODR o, Z_AttCombo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
	odr_implicit(o, z_AttributeList, &(*p)->unsupportedCombination,
	    ODR_CONTEXT, 1, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_AttributeList, &(*p)->alternatives,
	    &(*p)->num_alternatives) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_DiagTerm(ODR o, Z_DiagTerm **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_integer, &(*p)->problem, ODR_CONTEXT, 1, 1) &&
	odr_explicit(o, z_Term, &(*p)->term, ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

int z_Proximity(ODR o, Z_Proximity **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Proximity_resultSets, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Proximity_badSet,
	    z_InternationalString},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Proximity_relation, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_Proximity_unit, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_Proximity_distance, odr_integer},
	{ODR_EXPLICIT, ODR_CONTEXT, 6, Z_Proximity_attributes, z_AttributeList},
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_Proximity_ordered, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 8, Z_Proximity_exclusion, odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_AttrListList(ODR o, Z_AttrListList **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, z_AttributeList, &(*p)->lists, &(*p)->num_lists))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Scan(ODR o, Z_Scan **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_ScanD_nonZeroStepSize, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ScanD_specifiedStepSize, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_ScanD_termList1, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_ScanD_termList2, z_AttrListList},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_ScanD_posInResponse, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 6, Z_ScanD_resources, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_ScanD_endOfList, odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_StringList(ODR o, Z_StringList **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, z_InternationalString, &(*p)->strings,
	&(*p)->num_strings))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Sort(ODR o, Z_Sort **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_SortD_sequence, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_SortD_noRsName, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_SortD_tooMany, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_SortD_incompatible, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_SortD_generic, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_SortD_dbSpecific, odr_null},
#if 0
	{ODR_EXPLICIT, ODR_CONTEXT, 6, Z_SortD_sortElement, z_SortElement},
#endif
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_SortD_key, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 8, Z_SortD_action, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 9, Z_SortD_illegal, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 10, Z_SortD_inputTooLarge, z_StringList},
	{ODR_IMPLICIT, ODR_CONTEXT, 11, Z_SortD_aggregateTooLarge, odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Segmentation(ODR o, Z_Segmentation **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_SegmentationD_segments, odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ExtServices(ODR o, Z_ExtServices **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_ExtServicesD_req, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ExtServicesD_permission, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ExtServicesD_immediate, odr_integer},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_OidList(ODR o, Z_OidList **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, odr_oid, &(*p)->oids, &(*p)->num_oids))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_AccessCtrl(ODR o, Z_AccessCtrl **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_AccessCtrlD_noUser, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_AccessCtrlD_refused, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_AccessCtrlD_simple, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_AccessCtrlD_oid, z_OidList},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_AccessCtrlD_alternative, z_OidList},
	{ODR_IMPLICIT, ODR_CONTEXT, 6, Z_AccessCtrlD_pwdInv, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_AccessCtrlD_pwdExp, odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_RecordSyntax(ODR o, Z_RecordSyntax **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_oid, &(*p)->unsupportedSyntax, ODR_CONTEXT, 1, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, odr_oid, &(*p)->suggestedAlternatives,
	    &(*p)->num_suggestedAlternatives) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_DiagFormat(ODR o, Z_DiagFormat **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1000, Z_DiagFormat_tooMany, z_TooMany},
	{ODR_IMPLICIT, ODR_CONTEXT, 1001, Z_DiagFormat_badSpec, z_BadSpec},
	{ODR_IMPLICIT, ODR_CONTEXT, 1002, Z_DiagFormat_dbUnavail, z_DbUnavail},
	{ODR_IMPLICIT, ODR_CONTEXT, 1003, Z_DiagFormat_unSupOp, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 1004, Z_DiagFormat_attribute, z_Attribute},
	{ODR_IMPLICIT, ODR_CONTEXT, 1005, Z_DiagFormat_attCombo, z_AttCombo},
	{ODR_IMPLICIT, ODR_CONTEXT, 1006, Z_DiagFormat_term, z_DiagTerm},
	{ODR_EXPLICIT, ODR_CONTEXT, 1007, Z_DiagFormat_proximity, z_Proximity},
	{ODR_EXPLICIT, ODR_CONTEXT, 1008, Z_DiagFormat_scan, z_Scan},
	{ODR_EXPLICIT, ODR_CONTEXT, 1009, Z_DiagFormat_sort, z_Sort},
	{ODR_EXPLICIT, ODR_CONTEXT, 1010, Z_DiagFormat_segmentation,
	    z_Segmentation},
	{ODR_EXPLICIT, ODR_CONTEXT, 1011, Z_DiagFormat_extServices,
	    z_ExtServices},
	{ODR_EXPLICIT, ODR_CONTEXT, 1012, Z_DiagFormat_accessCtrl,
	    z_AccessCtrl},
	{ODR_IMPLICIT, ODR_CONTEXT, 1013, Z_DiagFormat_recordSyntax,
	    z_RecordSyntax},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Diagnostic(ODR o, Z_Diagnostic **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Diagnostic_defaultDiagRec,
	    z_DefaultDiagFormat},
	{ODR_EXPLICIT, ODR_CONTEXT, 2, Z_Diagnostic_explicitDiagnostic,
	    z_DiagFormat},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_DiagnosticUnit(ODR o, Z_DiagnosticUnit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
    	odr_explicit(o, z_Diagnostic, &(*p)->diagnostic, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->message, ODR_CONTEXT,
	    2, 1) &&
	odr_sequence_end(o);
}

int MDF z_DiagnosticFormat(ODR o, Z_DiagnosticFormat **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, z_DiagnosticUnit, &(*p)->diagnostics,
	&(*p)->num_diagnostics))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}
