/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: proto.c,v $
 * Revision 1.38  1995-09-27 15:02:40  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.37  1995/08/21  09:10:15  quinn
 * Smallish fixes to suppport new formats.
 *
 * Revision 1.36  1995/08/15  11:59:39  quinn
 * Updated External
 *
 * Revision 1.35  1995/08/10  08:53:59  quinn
 * Added Explain
 *
 * Revision 1.34  1995/06/19  17:01:48  quinn
 * This should bring us in sync with the version distributed as 1.0b
 *
 * Revision 1.33  1995/06/19  13:39:56  quinn
 * *** empty log message ***
 *
 * Revision 1.32  1995/06/19  12:37:28  quinn
 * Fixed a bug in the compspec.
 *
 * Revision 1.31  1995/06/16  13:15:56  quinn
 * Fixed Defaultdiagformat.
 *
 * Revision 1.30  1995/06/15  15:42:01  quinn
 * Fixed some v3 bugs
 *
 * Revision 1.29  1995/06/15  07:44:49  quinn
 * Moving to v3.
 *
 * Revision 1.28  1995/06/14  15:26:35  quinn
 * *** empty log message ***
 *
 * Revision 1.27  1995/06/07  14:36:22  quinn
 * Added CLOSE
 *
 * Revision 1.26  1995/06/02  09:49:13  quinn
 * Adding access control
 *
 * Revision 1.25  1995/05/25  11:00:08  quinn
 * *** empty log message ***
 *
 * Revision 1.24  1995/05/22  13:58:18  quinn
 * Fixed an ODR_NULLVAL.
 *
 * Revision 1.23  1995/05/22  11:30:18  quinn
 * Adding Z39.50-1992 stuff to proto.c. Adding zget.c
 *
 * Revision 1.22  1995/05/17  08:40:56  quinn
 * Added delete. Fixed some sequence_begins. Smallish.
 *
 * Revision 1.21  1995/05/16  08:50:24  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.20  1995/05/15  11:55:25  quinn
 * Smallish.
 *
 * Revision 1.19  1995/04/11  11:58:35  quinn
 * Fixed bug.
 *
 * Revision 1.18  1995/04/11  11:52:02  quinn
 * Fixed possible buf in proto.c
 *
 * Revision 1.17  1995/04/10  10:22:22  quinn
 * Added SCAN.
 *
 * Revision 1.16  1995/03/30  10:26:43  quinn
 * Added Term structure
 *
 * Revision 1.15  1995/03/30  09:08:39  quinn
 * Added Resource control protocol
 *
 * Revision 1.14  1995/03/29  08:06:13  quinn
 * Added a few v3 elements
 *
 * Revision 1.13  1995/03/20  11:26:52  quinn
 * *** empty log message ***
 *
 * Revision 1.12  1995/03/20  09:45:09  quinn
 * Working towards v3
 *
 * Revision 1.11  1995/03/17  10:17:25  quinn
 * Added memory management.
 *
 * Revision 1.10  1995/03/15  11:17:40  quinn
 * Fixed some return-checks from choice.. need better ay to handle those..
 *
 * Revision 1.9  1995/03/15  08:37:06  quinn
 * Fixed protocol bugs.
 *
 * Revision 1.8  1995/03/14  16:59:24  quinn
 * Fixed OPTIONAL flag in attributeelement
 *
 * Revision 1.7  1995/03/07  16:29:33  quinn
 * Added authentication stuff.
 *
 * Revision 1.6  1995/03/01  14:46:03  quinn
 * Fixed protocol bug in 8777query.
 *
 * Revision 1.5  1995/02/14  11:54:22  quinn
 * Fixing include.
 *
 * Revision 1.4  1995/02/10  15:54:30  quinn
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

/*
 * We'll use a general octetstring here, since string operations are
 * clumsy on long strings.
 */
int MDF z_SUTRS(ODR o, Odr_oct **p, int opt)
{
    return odr_implicit(o, odr_octetstring, p, ODR_UNIVERSAL,
	ODR_GENERALSTRING, opt);
}

int z_ReferenceId(ODR o, Z_ReferenceId **p, int opt)
{
    return odr_implicit(o, odr_octetstring, (Odr_oct**) p, ODR_CONTEXT, 2, opt);
}

int MDF z_DatabaseName(ODR o, Z_DatabaseName **p, int opt)
{
    return odr_implicit(o, odr_visiblestring, (char **) p, ODR_CONTEXT, 105,
	opt);
}

int z_ResultSetId(ODR o, char **p, int opt)
{
    return odr_implicit(o, odr_visiblestring, (char **) p, ODR_CONTEXT, 31,
	opt);
}

int MDF z_ElementSetName(ODR o, char **p, int opt)
{
    return odr_implicit(o, odr_visiblestring, p, ODR_CONTEXT, 103, opt);
}

int z_UserInformationField(ODR o, Z_External **p, int opt)
{
    return odr_explicit(o, z_External, (Z_External **)p, ODR_CONTEXT,
    	11, opt);
}

int MDF z_InternationalString(ODR o, char **p, int opt)
{
    return odr_generalstring(o, p, opt);
}

int z_InfoCategory(ODR o, Z_InfoCategory **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_oid, &(*p)->categoryTypeId, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, odr_integer, &(*p)->categoryValue, ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

int MDF z_OtherInformationUnit(ODR o, Z_OtherInformationUnit **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_OtherInfo_characterInfo,
	    odr_visiblestring},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_OtherInfo_binaryInfo,
	    odr_octetstring},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_OtherInfo_externallyDefinedInfo,
	    z_External},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_OtherInfo_oid, odr_oid},
	{-1, -1, -1, -1, 0}
    };

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, z_InfoCategory, &(*p)->category, ODR_CONTEXT, 1, 1) &&
	odr_choice(o, arm, &(*p)->which, &(*p)->information) &&
	odr_sequence_end(o);
}
    
int MDF z_OtherInformation(ODR o, Z_OtherInformation **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    odr_implicit_settag(o, ODR_CONTEXT, 201);
    if (odr_sequence_of(o, z_OtherInformationUnit, &(*p)->list,
	&(*p)->num_elements))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int MDF z_StringOrNumeric(ODR o, Z_StringOrNumeric **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_StringOrNumeric_string,
	    odr_visiblestring},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_StringOrNumeric_numeric,
	    odr_integer},
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

/*
 * check tagging!!
 */
int MDF z_Unit(ODR o, Z_Unit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_visiblestring, &(*p)->unitSystem, ODR_CONTEXT,
	    1, 1) &&
	odr_explicit(o, z_StringOrNumeric, &(*p)->unitType, ODR_CONTEXT,
	    2, 1) &&
	odr_explicit(o, z_StringOrNumeric, &(*p)->unit, ODR_CONTEXT, 3, 1) &&
	odr_implicit(o, odr_integer, &(*p)->scaleFactor, ODR_CONTEXT, 4, 1) &&
	odr_sequence_end(o);
}

int MDF z_IntUnit(ODR o, Z_IntUnit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_integer, &(*p)->value, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, z_Unit, &(*p)->unitUsed, ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

/* ---------------------- INITIALIZE SERVICE ------------------- */

#if 0
int MDF z_NSRAuthentication(ODR o, Z_NSRAuthentication **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_visiblestring(o, &(*p)->user, 0) &&
    	odr_visiblestring(o, &(*p)->password, 0) &&
    	odr_visiblestring(o, &(*p)->account, 0) &&
    	odr_sequence_end(o);
}
#endif

int z_IdPass(ODR o, Z_IdPass **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_visiblestring, &(*p)->groupId, ODR_CONTEXT, 0, 1) &&
    	odr_implicit(o, odr_visiblestring, &(*p)->userId, ODR_CONTEXT, 1, 1) &&
    	odr_implicit(o, odr_visiblestring, &(*p)->password, ODR_CONTEXT, 2,
	    1) &&
    	odr_sequence_end(o);
}

int MDF z_StrAuthentication(ODR o, char **p, int opt)
{
    return odr_visiblestring(o, p, opt);
}

int z_IdAuthentication(ODR o, Z_IdAuthentication **p, int opt)
{
    static Odr_arm arm[] =
    {
	{-1, -1, -1, Z_IdAuthentication_open, z_StrAuthentication},
	{-1, -1, -1, Z_IdAuthentication_idPass, z_IdPass},
	{-1, -1, -1, Z_IdAuthentication_anonymous, odr_null},
	{-1, -1, -1, Z_IdAuthentication_other, z_External},
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

int z_InitRequest(ODR o, Z_InitRequest **p, int opt)
{
    Z_InitRequest *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
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
	odr_explicit(o, z_IdAuthentication, &pp->idAuthentication, ODR_CONTEXT,
	    7, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationId, ODR_CONTEXT,
	    110, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationName, ODR_CONTEXT,
	    111, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationVersion,
	    ODR_CONTEXT, 112, 1) &&
	z_UserInformationField(o, &pp->userInformationField, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

int z_InitResponse(ODR o, Z_InitResponse **p, int opt)
{
    Z_InitResponse *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
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
	odr_implicit(o, odr_visiblestring, &pp->implementationId, ODR_CONTEXT,
	    110, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationName, ODR_CONTEXT,
	    111, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationVersion,
	    ODR_CONTEXT, 112, 1) &&
	z_UserInformationField(o, &pp->userInformationField, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

/* ------------------ RESOURCE CONTROL ----------------*/

int z_TriggerResourceControlRequest(ODR o, Z_TriggerResourceControlRequest **p,
				    int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit(o, odr_integer, &(*p)->requestedAction, ODR_CONTEXT,
	    46, 0) &&
	odr_implicit(o, odr_oid, &(*p)->prefResourceReportFormat,
	    ODR_CONTEXT, 47, 1) &&
	odr_implicit(o, odr_bool, &(*p)->resultSetWanted, ODR_CONTEXT,
	    48, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

int z_ResourceControlRequest(ODR o, Z_ResourceControlRequest **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit(o, odr_bool, &(*p)->suspendedFlag, ODR_CONTEXT, 39, 1)&&
	odr_explicit(o, z_External, &(*p)->resourceReport, ODR_CONTEXT,
	    40, 1) &&
	odr_implicit(o, odr_integer, &(*p)->partialResultsAvailable,
	    ODR_CONTEXT, 41, 1) &&
	odr_implicit(o, odr_bool, &(*p)->responseRequired, ODR_CONTEXT,
	    42, 0) &&
	odr_implicit(o, odr_bool, &(*p)->triggeredRequestFlag,
	    ODR_CONTEXT, 43, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

int z_ResourceControlResponse(ODR o, Z_ResourceControlResponse **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit(o, odr_bool, &(*p)->continueFlag, ODR_CONTEXT, 44, 0) &&
	odr_implicit(o, odr_bool, &(*p)->resultSetWanted, ODR_CONTEXT,
	    45, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

/* ------------------------ SEARCH SERVICE ----------------------- */

int z_DatabaseSpecificUnit(ODR o, Z_DatabaseSpecificUnit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_DatabaseName(o, &(*p)->databaseName, 0) &&
    	z_ElementSetName(o, &(*p)->elementSetName, 0) &&
    	odr_sequence_end(o);
}

int z_DatabaseSpecific(ODR o, Z_DatabaseSpecific **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
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

    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt && odr_ok(o);

    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return 0;
}

/* ----------------------- RPN QUERY -----------------------*/

int z_ComplexAttribute(ODR o, Z_ComplexAttribute **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
    	odr_sequence_of(o, z_StringOrNumeric, &(*p)->list,
	    &(*p)->num_list) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, odr_integer, &(*p)->semanticAction,
	    &(*p)->num_semanticAction) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_AttributeElement(ODR o, Z_AttributeElement **p, int opt)
{
#ifdef Z_95
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 121, Z_AttributeValue_numeric,
	    odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 224, Z_AttributeValue_complex,
	    z_ComplexAttribute},
	{-1, -1, -1, -1, 0}
    };
#endif

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
#ifdef Z_95
    	odr_implicit(o, odr_oid, &(*p)->attributeSet, ODR_CONTEXT, 1, 1) &&
#endif
    	odr_implicit(o, odr_integer, &(*p)->attributeType, ODR_CONTEXT,
	    120, 0) &&
#ifdef Z_95
	odr_choice(o, arm, &(*p)->value, &(*p)->which) &&
#else
    	odr_implicit(o, odr_integer, &(*p)->attributeValue, ODR_CONTEXT,
	    121, 0) &&
#endif
    	odr_sequence_end(o);
}

int MDF z_Term(ODR o, Z_Term **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 45, Z_Term_general, odr_octetstring},
	{ODR_IMPLICIT, ODR_CONTEXT, 215, Z_Term_numeric, odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 216, Z_Term_characterString,
	    odr_visiblestring},
	{ODR_IMPLICIT, ODR_CONTEXT, 217, Z_Term_oid, odr_oid},
	{ODR_IMPLICIT, ODR_CONTEXT, 218, Z_Term_dateTime, odr_cstring},
	{ODR_IMPLICIT, ODR_CONTEXT, 219, Z_Term_external, z_External},
	/* add intUnit here */
	{ODR_IMPLICIT, ODR_CONTEXT, 221, Z_Term_null, odr_null},
    	{-1, -1, -1, -1, 0}
    };

    if (o->direction ==ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_AttributesPlusTerm(ODR o, Z_AttributesPlusTerm **p, int opt)
{
    if (!(odr_implicit_settag(o, ODR_CONTEXT, 102) &&
    	odr_sequence_begin(o, p, sizeof(**p))))
    	return opt && odr_ok(o);
    return
    	odr_implicit_settag(o, ODR_CONTEXT, 44) &&
    	odr_sequence_of(o, z_AttributeElement, &(*p)->attributeList,
	    &(*p)->num_attributes) &&
	z_Term(o, &(*p)->term, 0) &&
	odr_sequence_end(o);
}

int z_ResultSetPlusAttributes(ODR o, Z_ResultSetPlusAttributes **p, int opt)
{
    if (!(odr_implicit_settag(o, ODR_CONTEXT, 214) &&
    	odr_sequence_begin(o, p, sizeof(**p))))
    	return opt && odr_ok(o);
    return
    	z_ResultSetId(o, &(*p)->resultSet, 0) &&
    	odr_implicit_settag(o, ODR_CONTEXT, 44) &&
    	odr_sequence_of(o, z_AttributeElement, &(*p)->attributeList,
	    &(*p)->num_attributes) &&
	odr_sequence_end(o);
}

int z_ProximityOperator(ODR o, Z_ProximityOperator **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ProxCode_known, odr_integer},
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ProxCode_private, odr_integer},
    	{-1, -1, -1, -1, 0}
    };

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_bool, &(*p)->exclusion, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, odr_integer, &(*p)->distance, ODR_CONTEXT, 2, 0) &&
	odr_implicit(o, odr_bool, &(*p)->ordered, ODR_CONTEXT, 3, 0) &&
	odr_implicit(o, odr_integer, &(*p)->relationType, ODR_CONTEXT, 4, 0) &&
	odr_constructed_begin(o, &(*p)->proximityUnitCode, ODR_CONTEXT, 5) &&
	odr_choice(o, arm, &(*p)->proximityUnitCode, &(*p)->which) &&
	odr_constructed_end(o) &&
	odr_sequence_end(o);
}

int z_Operator(ODR o, Z_Operator **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_Operator_and, odr_null},
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Operator_or, odr_null},
    	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Operator_and_not, odr_null},
    	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Operator_prox, z_ProximityOperator},
    	{-1, -1, -1, -1, 0}
    };

    if (!*p && o->direction != ODR_DECODE)
    	return opt;
    if (!odr_constructed_begin(o, p, ODR_CONTEXT, 46))
    	return opt && odr_ok(o);
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));

    if (odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
    	odr_constructed_end(o))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Operand(ODR o, Z_Operand **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{-1, -1, -1, Z_Operand_APT, z_AttributesPlusTerm},
    	{-1, -1, -1, Z_Operand_resultSetId, z_ResultSetId},
    	{-1, -1, -1, Z_Operand_resultAttr, z_ResultSetPlusAttributes},
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

int z_RPNStructure(ODR o, Z_RPNStructure **p, int opt);

int z_Complex(ODR o, Z_Complex **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
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
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_RPNQuery(ODR o, Z_RPNQuery **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
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
    	{ODR_EXPLICIT, ODR_CONTEXT, 2, Z_Query_type_2, odr_octetstring},
    	{ODR_EXPLICIT, ODR_CONTEXT, 101, Z_Query_type_101, z_RPNQuery},
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

int z_SearchRequest(ODR o, Z_SearchRequest **p, int opt)
{
    Z_SearchRequest *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
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
	odr_explicit(o, z_ElementSetNames, &pp->smallSetElementSetNames,
	    ODR_CONTEXT, 100, 1) &&
	odr_explicit(o, z_ElementSetNames, &pp->mediumSetElementSetNames,
	    ODR_CONTEXT, 101, 1) &&
	odr_implicit(o, odr_oid, &pp->preferredRecordSyntax,
	    ODR_CONTEXT, 104, 1) &&
	odr_explicit(o, z_Query, &pp->query, ODR_CONTEXT, 21, 0) &&
#ifdef Z_95
	odr_implicit(o, z_OtherInformation, &(*p)->additionalSearchInfo,
	    ODR_CONTEXT, 203, 1) &&
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

/* ------------------------ RECORD ------------------------- */

int z_DatabaseRecord(ODR o, Z_DatabaseRecord **p, int opt)
{
    return z_External(o, (Z_External **) p, opt);
}

#ifdef Z_95

int MDF z_DefaultDiagFormat(ODR o, Z_DefaultDiagFormat **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{-1, -1, -1, Z_DiagForm_v2AddInfo, odr_visiblestring},
	{-1, -1, -1, Z_DiagForm_v3AddInfo, z_InternationalString},
    	{ODR_IMPLICIT, ODR_CONTEXT, ODR_VISIBLESTRING, Z_DiagForm_v2AddInfo,
	    odr_visiblestring}, /* To cater to a bug in the CNIDR servers */
	{-1, -1, -1, -1, 0}
    };
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_oid(o, &(*p)->diagnosticSetId, 1) && /* SHOULD NOT BE OPT! */
    	odr_integer(o, &(*p)->condition, 0) &&
	/*
	 * I no longer recall what server tagged the addinfo.. but it isn't
	 * hurting anyone, so...
	 * We need to turn it into a choice, or something, because of
	 * that damn generalstring in v3.
	 */
	odr_choice(o, arm, &(*p)->addinfo, &(*p)->which) &&
    	odr_sequence_end(o);
}

int MDF z_DiagRec(ODR o, Z_DiagRec **p, int opt)
{
    static Odr_arm arm[] = 
    {
    	{-1, -1, -1, Z_DiagRec_defaultFormat, z_DefaultDiagFormat},
    	{-1, -1, -1, Z_DiagRec_externallyDefined, z_External},
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

#else

int z_DiagRec(ODR o, Z_DiagRec **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_oid(o, &(*p)->diagnosticSetId, 1) && /* SHOULD NOT BE OPT! */
    	odr_integer(o, &(*p)->condition, 0) &&
	/*
	 * I no longer recall what server tagged the addinfo.. but it isn't
	 * hurting anyone, so...
	 * We need to turn it into a choice, or something, because of
	 * that damn generalstring in v3.
	 */
    	(odr_visiblestring(o, &(*p)->addinfo, 0) ||
	    odr_implicit(o, odr_cstring, &(*p)->addinfo, ODR_CONTEXT,
	    ODR_VISIBLESTRING, 1)) &&
    	odr_sequence_end(o);
}

#endif

int z_DiagRecs(ODR o, Z_DiagRecs **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;

	if (odr_sequence_of(o, z_DiagRec, &(*p)->diagRecs,
    	&(*p)->num_diagRecs))
    	return 1;
    *p = 0;
    return 0;
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
    	return opt && odr_ok(o);
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
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_NamePlusRecord, &(*p)->records,
	&(*p)->num_records))
    	return 1;
    *p = 0;
    return 0;
}

int z_Records(ODR o, Z_Records **p, int opt)
{
    static Odr_arm arm[] = 
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 28, Z_Records_DBOSD, z_NamePlusRecordList},
    	{ODR_IMPLICIT, ODR_CONTEXT, 130, Z_Records_NSD, z_DiagRec},
    	{ODR_IMPLICIT, ODR_CONTEXT, 205, Z_Records_multipleNSD,
	    z_DiagRecs},
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

/* ------------------------ ACCESS CTRL SERVICE ----------------------- */

int z_AccessControlRequest(ODR o, Z_AccessControlRequest **p, int opt)
{
    static Odr_arm arm[] = 
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 37, Z_AccessRequest_simpleForm,
	    odr_octetstring},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_AccessRequest_externallyDefined,
	    z_External},
    	{-1, -1, -1, -1, 0}
    };
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

int z_AccessControlResponse(ODR o, Z_AccessControlResponse **p, int opt)
{
    static Odr_arm arm[] = 
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 38, Z_AccessResponse_simpleForm,
	    odr_octetstring},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_AccessResponse_externallyDefined,
	    z_External},
    	{-1, -1, -1, -1, 0}
    };
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
	odr_explicit(o, z_DiagRec, &(*p)->diagnostic, ODR_CONTEXT, 223, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

/* ------------------------ SCAN SERVICE -------------------- */

int MDF z_AttributeList(ODR o, Z_AttributeList **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;

    odr_implicit_settag(o, ODR_CONTEXT, 44);
    if (odr_sequence_of(o, z_AttributeElement, &(*p)->attributes,
    	&(*p)->num_attributes))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

/*
 * This is a temporary hack. We don't know just *what* old version of the
 * protocol willow uses, so we'll just patiently wait for them to update
 */
static int willow_scan = 0;

int z_WillowAttributesPlusTerm(ODR o, Z_AttributesPlusTerm **p, int opt)
{
    if (!*p && o->direction != ODR_DECODE)
    	return opt;
    if (!odr_constructed_begin(o, p, ODR_CONTEXT, 4))
    {
    	o->t_class = -1;
    	return opt && odr_ok(o);
    }
    if (!odr_constructed_begin(o, p, ODR_CONTEXT, 1))
    	return 0;
    if (!odr_constructed_begin(o, p, ODR_UNIVERSAL, ODR_SEQUENCE))
    	return 0;
    if (!odr_implicit_settag(o, ODR_CONTEXT, 44))
    	return 0;
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    if (!odr_sequence_of(o, z_AttributeElement, &(*p)->attributeList,
	&(*p)->num_attributes))
	return 0;
    if (!odr_sequence_end(o) || !odr_sequence_end(o))
    	return 0;
    if (!z_Term(o, &(*p)->term, 0))
    	return 0;
    if (!odr_constructed_end(o))
    	return 0;
    willow_scan = 1;
    return 1;
}

int z_AlternativeTerm(ODR o, Z_AlternativeTerm **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    {
    	o->t_class = -1;
    	return opt && odr_ok(o);
    }

    if (odr_sequence_of(o, z_AttributesPlusTerm, &(*p)->terms,
    	&(*p)->num_terms))
    	return 1;
    *p = 0;
    return opt && !o->error;
}

int z_OccurrenceByAttributes(ODR o, Z_OccurrenceByAttributes **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_explicit(o, z_AttributeList, &(*p)->attributes, ODR_CONTEXT, 1, 1)&&
	odr_explicit(o, odr_integer, &(*p)->global, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_TermInfo(ODR o, Z_TermInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	(willow_scan ? 
    	    odr_implicit(o, z_Term, &(*p)->term, ODR_CONTEXT, 1, 0) :
	    z_Term(o, &(*p)->term, 0)) &&
	z_AttributeList(o, &(*p)->suggestedAttributes, 1) &&
	odr_implicit(o, z_AlternativeTerm, &(*p)->alternativeTerm,
	    ODR_CONTEXT, 4, 1) &&
	odr_implicit(o, odr_integer, &(*p)->globalOccurrences, ODR_CONTEXT,
	    2, 1) &&
	odr_implicit(o, z_OccurrenceByAttributes, &(*p)->byAttributes,
	    ODR_CONTEXT, 3, 1) &&
	odr_sequence_end(o);
}

int z_Entry(ODR o, Z_Entry **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Entry_termInfo, z_TermInfo},
	{ODR_EXPLICIT, ODR_CONTEXT, 2, Z_Entry_surrogateDiagnostic,
	    z_DiagRec},
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

int z_Entries(ODR o, Z_Entries **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;

    if (odr_sequence_of(o, z_Entry, &(*p)->entries,
    	&(*p)->num_entries))
    	return 1;
    *p = 0;
    return 0;
}

int z_ListEntries(ODR o, Z_ListEntries **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ListEntries_entries, z_Entries},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ListEntries_nonSurrogateDiagnostics,
	    z_DiagRecs},
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

int z_ScanRequest(ODR o, Z_ScanRequest **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    willow_scan = 0;
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	odr_sequence_of(o, z_DatabaseName, &(*p)->databaseNames,
	    &(*p)->num_databaseNames) &&
	odr_oid(o, &(*p)->attributeSet, 1) &&
	(z_AttributesPlusTerm(o, &(*p)->termListAndStartPoint, 1) ?
	    ((*p)->termListAndStartPoint ? 1 : 
	z_WillowAttributesPlusTerm(o, &(*p)->termListAndStartPoint, 0)) : 0) &&
	odr_implicit(o, odr_integer, &(*p)->stepSize, ODR_CONTEXT, 5, 1) &&
	odr_implicit(o, odr_integer, &(*p)->numberOfTermsRequested,
	    ODR_CONTEXT, 6, 0) &&
	odr_implicit(o, odr_integer, &(*p)->preferredPositionInResponse,
	    ODR_CONTEXT, 7, 1) &&
	odr_sequence_end(o);
}

int z_ScanResponse(ODR o, Z_ScanResponse **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit(o, odr_integer, &(*p)->stepSize, ODR_CONTEXT, 3, 1) &&
	odr_implicit(o, odr_integer, &(*p)->scanStatus, ODR_CONTEXT, 4, 0) &&
	odr_implicit(o, odr_integer, &(*p)->numberOfEntriesReturned,
	    ODR_CONTEXT, 5, 0) &&
	odr_implicit(o, odr_integer, &(*p)->positionOfTerm, ODR_CONTEXT, 6, 1)&&
	odr_explicit(o, z_ListEntries, &(*p)->entries, ODR_CONTEXT, 7, 1) &&
	odr_implicit(o, odr_oid, &(*p)->attributeSet, ODR_CONTEXT, 8, 1) &&
	odr_sequence_end(o);
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
    	return opt && odr_ok(o);
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	odr_implicit(o, odr_integer, &pp->resultCount, ODR_CONTEXT, 23, 0) &&
    	z_NumberOfRecordsReturned(o, &pp->numberOfRecordsReturned, 0) &&
    	z_NextResultSetPosition(o, &pp->nextResultSetPosition, 0) &&
    	odr_implicit(o, odr_bool, &pp->searchStatus, ODR_CONTEXT, 22, 0) &&
    	odr_implicit(o, odr_integer, &pp->resultSetStatus, ODR_CONTEXT, 26,
	    1) &&
    	z_PresentStatus(o, &pp->presentStatus, 1) &&
    	z_Records(o, &pp->records, 1) &&
#ifdef Z_95
	odr_implicit(o, z_OtherInformation, &(*p)->additionalSearchInfo,
	    ODR_CONTEXT, 203, 1) &&
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

/* --------------------- PRESENT SERVICE ---------------------- */

int z_ElementSpec(ODR o, Z_ElementSpec **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ElementSpec_elementSetName,
	    odr_visiblestring},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ElementSpec_externalSpec,
	    z_External},
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

int MDF z_Specification(ODR o, Z_Specification **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_oid, &(*p)->schema, ODR_CONTEXT, 1, 1) &&
	z_ElementSpec(o, &(*p)->elementSpec, 1) &&
	odr_sequence_end(o);
}

int z_DbSpecific(ODR o, Z_DbSpecific **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_explicit(o, z_DatabaseName, &(*p)->databaseName, ODR_CONTEXT,
	    1, 0) &&
	odr_implicit(o, z_Specification, &(*p)->spec, ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

int z_CompSpec(ODR o, Z_CompSpec **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_bool, &(*p)->selectAlternativeSyntax, ODR_CONTEXT,
	    1, 0) &&
	odr_implicit(o, z_Specification, &(*p)->generic, ODR_CONTEXT, 2, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, z_DbSpecific, &(*p)->dbSpecific,
	    &(*p)->num_dbSpecific) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 4) &&
	(odr_sequence_of(o, odr_oid, &(*p)->recordSyntax,
	    &(*p)->num_recordSyntax) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_RecordComposition(ODR o, Z_RecordComposition **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_EXPLICIT, ODR_CONTEXT, 19, Z_RecordComp_simple,
	    z_ElementSetNames},
	{ODR_IMPLICIT, ODR_CONTEXT, 209, Z_RecordComp_complex,
	    z_CompSpec},
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

int z_Range(ODR o, Z_Range **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_integer, &(*p)->startingPosition, ODR_CONTEXT,
	    1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->numberOfRecords, ODR_CONTEXT,
	    2, 0) &&
	odr_sequence_end(o);
}

int z_PresentRequest(ODR o, Z_PresentRequest **p, int opt)
{
    Z_PresentRequest *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	z_ResultSetId(o, &pp->resultSetId, 0) &&
    	odr_implicit(o, odr_integer, &pp->resultSetStartPoint, ODR_CONTEXT,
	    30, 0) &&
	odr_implicit(o, odr_integer, &pp->numberOfRecordsRequested, ODR_CONTEXT,
	    29, 0) &&
#ifdef Z_95
	odr_implicit_settag(o, ODR_CONTEXT, 212) &&
	(odr_sequence_of(o, z_Range, &(*p)->additionalRanges,
	    &(*p)->num_ranges) || odr_ok(o)) &&
	z_RecordComposition(o, &(*p)->recordComposition, 1) &&
#else
	odr_explicit(o, z_ElementSetNames, &pp->elementSetNames, ODR_CONTEXT,
	    19, 1) &&
#endif
	odr_implicit(o, odr_oid, &(*p)->preferredRecordSyntax, ODR_CONTEXT,
	    104, 1) &&
#ifdef Z_95
	odr_implicit(o, odr_integer, &(*p)->maxSegmentCount, ODR_CONTEXT,
	    204, 1) &&
	odr_implicit(o, odr_integer, &(*p)->maxRecordSize, ODR_CONTEXT,
	    206, 1) &&
	odr_implicit(o, odr_integer, &(*p)->maxSegmentSize, ODR_CONTEXT,
	    207, 1) &&
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

int z_PresentResponse(ODR o, Z_PresentResponse **p, int opt)
{
    Z_PresentResponse *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	z_NumberOfRecordsReturned(o, &pp->numberOfRecordsReturned, 0) &&
    	z_NextResultSetPosition(o, &pp->nextResultSetPosition, 0) &&
    	z_PresentStatus(o, &pp->presentStatus, 0) &&
    	z_Records(o, &pp->records, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
    	odr_sequence_end(o);
}

/* ----------------------DELETE -------------------------- */

int z_DeleteSetStatus(ODR o, int **p, int opt)
{
    return odr_implicit(o, odr_integer, p, ODR_CONTEXT, 33, opt);
}

int z_ListStatus(ODR o, Z_ListStatus **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ResultSetId(o, &(*p)->id, 0) &&
	z_DeleteSetStatus(o, &(*p)->status, 0) &&
	odr_sequence_end(o);
}

int z_DeleteResultSetRequest(ODR o, Z_DeleteResultSetRequest **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit(o, odr_integer, &(*p)->deleteFunction, ODR_CONTEXT, 32,
	    0) &&
	(odr_sequence_of(o, z_ListStatus, &(*p)->resultSetList,
	    &(*p)->num_ids) || odr_ok(o)) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

int z_DeleteResultSetResponse(ODR o, Z_DeleteResultSetResponse **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit(o, z_DeleteSetStatus, &(*p)->deleteOperationStatus,
	    ODR_CONTEXT, 0, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	(odr_sequence_of(o, z_ListStatus, &(*p)->deleteListStatuses,
	    &(*p)->num_statuses) || odr_ok(o)) &&
	odr_implicit(o, odr_integer, &(*p)->numberNotDeleted, ODR_CONTEXT,
	    34, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 35) &&
	(odr_sequence_of(o, z_ListStatus, &(*p)->bulkStatuses,
	    &(*p)->num_bulkStatuses) || odr_ok(o)) &&
	odr_implicit(o, odr_visiblestring, &(*p)->deleteMessage, ODR_CONTEXT,
	    36, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

/* ------------------------ SEGMENT SERVICE -------------- */

int z_Segment(ODR o, Z_Segment **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit(o, odr_integer, &(*p)->numberOfRecordsReturned,
	    ODR_CONTEXT, 24, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 0) &&
	odr_sequence_of(o, z_NamePlusRecord, &(*p)->segmentRecords,
	    &(*p)->num_segmentRecords) &&
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
	odr_sequence_end(o);
}

/* ------------------------ CLOSE SERVICE ---------------- */

int z_Close(ODR o, Z_Close **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	z_ReferenceId(o, &(*p)->referenceId, 1) &&
	odr_implicit(o, odr_integer, &(*p)->closeReason, ODR_CONTEXT, 211, 0) &&
	odr_implicit(o, odr_visiblestring, &(*p)->diagnosticInformation,
	    ODR_CONTEXT, 3, 1) &&
	odr_implicit(o, odr_oid, &(*p)->resourceReportFormat, ODR_CONTEXT,
	    4, 1) &&
	odr_implicit(o, z_External, &(*p)->resourceReport, ODR_CONTEXT,
	    5, 1) &&
#ifdef Z_95
	z_OtherInformation(o, &(*p)->otherInfo, 1) &&
#endif
	odr_sequence_end(o);
}

/* ------------------------ APDU ------------------------- */

int MDF z_Permissions(ODR o, Z_Permissions **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->userId, ODR_CONTEXT,
	    1, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	odr_sequence_of(o, odr_integer, &(*p)->allowableFunctions,
	    &(*p)->num_allowableFunctions) &&
	odr_sequence_end(o);
}

int z_ExtendedServicesRequest(ODR o, Z_ExtendedServicesRequest **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        z_ReferenceId(o, &(*p)->referenceId, 1) &&
        odr_implicit(o, odr_integer, &(*p)->function, ODR_CONTEXT, 3, 0) &&
        odr_implicit(o, odr_oid, &(*p)->packageType, ODR_CONTEXT, 4, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->packageName, ODR_CONTEXT, 5, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->userId, ODR_CONTEXT, 6, 1) &&
        odr_implicit(o, z_IntUnit, &(*p)->retentionTime, ODR_CONTEXT, 7, 1) &&
        odr_implicit(o, z_Permissions, &(*p)->permissions, ODR_CONTEXT, 8, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->description, ODR_CONTEXT, 9, 1) &&
        odr_implicit(o, z_External, &(*p)->taskSpecificParameters, ODR_CONTEXT, 10, 1) &&
        odr_implicit(o, odr_integer, &(*p)->waitAction, ODR_CONTEXT, 11, 0) &&
        z_ElementSetName(o, &(*p)->elements, 1) &&
        z_OtherInformation(o, &(*p)->otherInfo, 1) &&
        odr_sequence_end(o);
}

int z_ExtendedServicesResponse(ODR o, Z_ExtendedServicesResponse **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        z_ReferenceId(o, &(*p)->referenceId, 1) &&
        odr_implicit(o, odr_integer, &(*p)->operationStatus, ODR_CONTEXT, 3, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 4) &&
	(odr_sequence_of(o, z_DiagRec, &(*p)->diagnostics,
	    &(*p)->num_diagnostics) || odr_ok(o)) &&
        odr_implicit(o, z_External, &(*p)->taskPackage, ODR_CONTEXT, 5, 1) &&
        z_OtherInformation(o, &(*p)->otherInfo, 1) &&
        odr_sequence_end(o);
}

/* ------------------------ APDU ------------------------- */

int MDF z_APDU(ODR o, Z_APDU **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 20, Z_APDU_initRequest, z_InitRequest},
    	{ODR_IMPLICIT, ODR_CONTEXT, 21, Z_APDU_initResponse, z_InitResponse},
    	{ODR_IMPLICIT, ODR_CONTEXT, 22, Z_APDU_searchRequest, z_SearchRequest},
    	{ODR_IMPLICIT, ODR_CONTEXT, 23, Z_APDU_searchResponse,
	    z_SearchResponse},
    	{ODR_IMPLICIT, ODR_CONTEXT, 24, Z_APDU_presentRequest,
	    z_PresentRequest},
    	{ODR_IMPLICIT, ODR_CONTEXT, 25, Z_APDU_presentResponse,
	    z_PresentResponse},
	{ODR_IMPLICIT, ODR_CONTEXT, 26, Z_APDU_deleteResultSetRequest,
	    z_DeleteResultSetRequest},
	{ODR_IMPLICIT, ODR_CONTEXT, 27, Z_APDU_deleteResultSetResponse,
	    z_DeleteResultSetResponse},
    	{ODR_IMPLICIT, ODR_CONTEXT, 30, Z_APDU_resourceControlRequest,
	    z_ResourceControlRequest},
    	{ODR_IMPLICIT, ODR_CONTEXT, 31, Z_APDU_resourceControlResponse,
	    z_ResourceControlResponse},
    	{ODR_IMPLICIT, ODR_CONTEXT, 32, Z_APDU_triggerResourceControlRequest,
	    z_TriggerResourceControlRequest},
	{ODR_IMPLICIT, ODR_CONTEXT, 35, Z_APDU_scanRequest, z_ScanRequest},
	{ODR_IMPLICIT, ODR_CONTEXT, 36, Z_APDU_scanResponse, z_ScanResponse},
	{ODR_IMPLICIT, ODR_CONTEXT, 45, Z_APDU_segmentRequest, z_Segment},
	{ODR_IMPLICIT, ODR_CONTEXT, 46, Z_APDU_extendedServicesRequest,
	    z_ExtendedServicesRequest},
	{ODR_IMPLICIT, ODR_CONTEXT, 47, Z_APDU_extendedServicesResponse,
	    z_ExtendedServicesResponse},
	{ODR_IMPLICIT, ODR_CONTEXT, 48, Z_APDU_close, z_Close},

    	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    if (!odr_choice(o, arm, &(*p)->u, &(*p)->which))
    {
    	if (o->direction == ODR_DECODE)
	    *p = 0;
	return opt && odr_ok(o);
    }
    return 1;
}
