/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-dia.c,v $
 * Revision 1.5  1998-02-11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.4  1996/01/22 09:46:34  quinn
 * Added Sort PDU. Moved StringList to main protocol file.
 *
 * Revision 1.3  1995/09/29  17:11:54  quinn
 * Smallish
 *
 * Revision 1.2  1995/09/27  15:02:41  quinn
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
	(odr_sequence_of(o, (Odr_fun)z_Specification, &(*p)->goodOnes,
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
	(odr_sequence_of(o, (Odr_fun)z_AttributeList, &(*p)->alternatives,
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
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Proximity_resultSets, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Proximity_badSet,
	    (Odr_fun)z_InternationalString},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Proximity_relation, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_Proximity_unit, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_Proximity_distance, (Odr_fun)odr_integer},
	{ODR_EXPLICIT, ODR_CONTEXT, 6, Z_Proximity_attributes, (Odr_fun)z_AttributeList},
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_Proximity_ordered, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 8, Z_Proximity_exclusion, (Odr_fun)odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_Proximity *)odr_malloc(o, sizeof(**p));
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
	*p = (Z_AttrListList *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, (Odr_fun)z_AttributeList, &(*p)->lists, &(*p)->num_lists))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Scan(ODR o, Z_Scan **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_ScanD_nonZeroStepSize, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ScanD_specifiedStepSize, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_ScanD_termList1, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_ScanD_termList2, (Odr_fun)z_AttrListList},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_ScanD_posInResponse, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 6, Z_ScanD_resources, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_ScanD_endOfList, (Odr_fun)odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_Scan *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Sort(ODR o, Z_Sort **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_SortD_sequence, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_SortD_noRsName, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_SortD_tooMany, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_SortD_incompatible, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_SortD_generic, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_SortD_dbSpecific, (Odr_fun)odr_null},
#if 0
	{ODR_EXPLICIT, ODR_CONTEXT, 6, Z_SortD_sortElement, (Odr_fun)z_SortElement},
#endif
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_SortD_key, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 8, Z_SortD_action, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 9, Z_SortD_illegal, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 10, Z_SortD_inputTooLarge, (Odr_fun)z_StringList},
	{ODR_IMPLICIT, ODR_CONTEXT, 11, Z_SortD_aggregateTooLarge, (Odr_fun)odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_Sort *)odr_malloc(o, sizeof(**p));
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
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_SegmentationD_segments, (Odr_fun)odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_Segmentation *)odr_malloc(o, sizeof(**p));
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
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_ExtServicesD_req, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ExtServicesD_permission, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ExtServicesD_immediate, (Odr_fun)odr_integer},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_ExtServices *)odr_malloc(o, sizeof(**p));
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
	*p = (Z_OidList *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, (Odr_fun)odr_oid, &(*p)->oids, &(*p)->num_oids))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_AccessCtrl(ODR o, Z_AccessCtrl **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_AccessCtrlD_noUser, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_AccessCtrlD_refused, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_AccessCtrlD_simple, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_AccessCtrlD_oid, (Odr_fun)z_OidList},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_AccessCtrlD_alternative, (Odr_fun)z_OidList},
	{ODR_IMPLICIT, ODR_CONTEXT, 6, Z_AccessCtrlD_pwdInv, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_AccessCtrlD_pwdExp, (Odr_fun)odr_null},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_AccessCtrl *)odr_malloc(o, sizeof(**p));
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
	(odr_sequence_of(o, (Odr_fun)odr_oid, &(*p)->suggestedAlternatives,
	    &(*p)->num_suggestedAlternatives) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_DiagFormat(ODR o, Z_DiagFormat **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1000, Z_DiagFormat_tooMany, (Odr_fun)z_TooMany},
	{ODR_IMPLICIT, ODR_CONTEXT, 1001, Z_DiagFormat_badSpec, (Odr_fun)z_BadSpec},
	{ODR_IMPLICIT, ODR_CONTEXT, 1002, Z_DiagFormat_dbUnavail, (Odr_fun)z_DbUnavail},
	{ODR_IMPLICIT, ODR_CONTEXT, 1003, Z_DiagFormat_unSupOp, (Odr_fun)odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 1004, Z_DiagFormat_attribute, (Odr_fun)z_Attribute},
	{ODR_IMPLICIT, ODR_CONTEXT, 1005, Z_DiagFormat_attCombo, (Odr_fun)z_AttCombo},
	{ODR_IMPLICIT, ODR_CONTEXT, 1006, Z_DiagFormat_term, (Odr_fun)z_DiagTerm},
	{ODR_EXPLICIT, ODR_CONTEXT, 1007, Z_DiagFormat_proximity, (Odr_fun)z_Proximity},
	{ODR_EXPLICIT, ODR_CONTEXT, 1008, Z_DiagFormat_scan, (Odr_fun)z_Scan},
	{ODR_EXPLICIT, ODR_CONTEXT, 1009, Z_DiagFormat_sort, (Odr_fun)z_Sort},
	{ODR_EXPLICIT, ODR_CONTEXT, 1010, Z_DiagFormat_segmentation,
	    (Odr_fun)z_Segmentation},
	{ODR_EXPLICIT, ODR_CONTEXT, 1011, Z_DiagFormat_extServices,
	    (Odr_fun)z_ExtServices},
	{ODR_EXPLICIT, ODR_CONTEXT, 1012, Z_DiagFormat_accessCtrl,
	    (Odr_fun)z_AccessCtrl},
	{ODR_IMPLICIT, ODR_CONTEXT, 1013, Z_DiagFormat_recordSyntax,
	    (Odr_fun)z_RecordSyntax},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_DiagFormat *)odr_malloc(o, sizeof(**p));
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
	    (Odr_fun)z_DefaultDiagFormat},
	{ODR_EXPLICIT, ODR_CONTEXT, 2, Z_Diagnostic_explicitDiagnostic,
	    (Odr_fun)z_DiagFormat},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_Diagnostic *)odr_malloc(o, sizeof(**p));
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

int z_DiagnosticFormat(ODR o, Z_DiagnosticFormat **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = (Z_DiagnosticFormat *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, (Odr_fun)z_DiagnosticUnit, &(*p)->diagnostics,
	&(*p)->num_diagnostics))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}
