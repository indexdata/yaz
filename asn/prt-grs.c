/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-grs.c,v $
 * Revision 1.7  1998-02-11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.6  1997/05/14 06:53:23  adam
 * C++ support.
 *
 * Revision 1.5  1995/10/18 16:12:20  quinn
 * Added a couple of special cases to handle the WAIS server.
 *
 * Revision 1.4  1995/09/29  17:11:55  quinn
 * Smallish
 *
 * Revision 1.3  1995/09/27  15:02:43  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/08/28  10:58:58  quinn
 * Added extra choice-entry to data to catch visiblestring.
 *
 * Revision 1.1  1995/08/17  12:47:09  quinn
 * Added GRS-1.
 *
 *
 */

#include <proto.h>

int z_TaggedElement(ODR o, Z_TaggedElement **p, int opt);
int z_ElementData(ODR o, Z_ElementData **p, int opt);
int z_ElementMetaData(ODR o, Z_ElementMetaData **p, int opt);
int z_TagUnit(ODR o, Z_TagUnit **p, int opt);
int z_TagPath(ODR o, Z_TagPath **p, int opt);
int z_Order(ODR o, Z_Order **p, int opt);
int z_Usage(ODR o, Z_Usage **p, int opt);
int z_HitVector(ODR o, Z_HitVector **p, int opt);
int z_Triple(ODR o, Z_Triple **p, int opt);
int z_Variant(ODR o, Z_Variant **p, int opt);

int z_GenericRecord(ODR o, Z_GenericRecord **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = (Z_GenericRecord *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, (Odr_fun)z_TaggedElement, &(*p)->elements, &(*p)->num_elements))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_TaggedElement(ODR o, Z_TaggedElement **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_integer, &(*p)->tagType, ODR_CONTEXT, 1, 1) &&
        odr_explicit(o, z_StringOrNumeric, &(*p)->tagValue, ODR_CONTEXT, 2, 0) &&
        odr_implicit(o, odr_integer, &(*p)->tagOccurrence, ODR_CONTEXT, 3, 1) &&
        odr_explicit(o, z_ElementData, &(*p)->content, ODR_CONTEXT, 4, 0) &&
        odr_implicit(o, z_ElementMetaData, &(*p)->metaData, ODR_CONTEXT, 5, 1) &&
        odr_implicit(o, z_Variant, &(*p)->appliedVariant, ODR_CONTEXT, 6, 1) &&
	odr_sequence_end(o);
}
    
int z_ElementData(ODR o, Z_ElementData **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_NONE, -1, -1, Z_ElementData_octets, (Odr_fun)odr_octetstring},
	{ODR_NONE, -1, -1, Z_ElementData_numeric, (Odr_fun)odr_integer},
	{ODR_NONE, -1, -1, Z_ElementData_date, (Odr_fun)odr_generalizedtime},
	{ODR_NONE, -1, -1, Z_ElementData_ext, (Odr_fun)z_External},
	{ODR_NONE, -1, -1, Z_ElementData_string, (Odr_fun)z_InternationalString},
	/* The entry below provides some backwards compatibility */
	{ODR_NONE, -1, -1, Z_ElementData_string, (Odr_fun)odr_visiblestring},
	{ODR_NONE, -1, -1, Z_ElementData_trueOrFalse, (Odr_fun)odr_bool},
	{ODR_NONE, -1, -1, Z_ElementData_oid, (Odr_fun)odr_oid},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ElementData_intUnit, (Odr_fun)z_IntUnit},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ElementData_elementNotThere, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_ElementData_elementEmpty, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_ElementData_noDataRequested, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_ElementData_diagnostic, (Odr_fun)z_External},
	{ODR_EXPLICIT, ODR_CONTEXT, 6, Z_ElementData_subtree, (Odr_fun)z_GenericRecord},
	{-1, -1, -1, -1, 0}
    };
    
    if (o->direction == ODR_DECODE)
	*p = (Z_ElementData *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ElementMetaData(ODR o, Z_ElementMetaData **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_Order, &(*p)->seriesOrder, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, z_Usage, &(*p)->usageRight, ODR_CONTEXT, 2, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, (Odr_fun)z_HitVector, &(*p)->hits, &(*p)->num_hits) ||
	    odr_ok(o)) &&
        odr_implicit(o, z_InternationalString, &(*p)->displayName, ODR_CONTEXT,
	    4, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 5) &&
	(odr_sequence_of(o, (Odr_fun)z_Variant, &(*p)->supportedVariants,
	    &(*p)->num_supportedVariants) || odr_ok(o)) &&
        odr_implicit(o, z_InternationalString, &(*p)->message, ODR_CONTEXT,
	    6, 1) &&
        odr_implicit(o, odr_octetstring, &(*p)->elementDescriptor, ODR_CONTEXT,
	    7, 1) &&
        odr_implicit(o, z_TagPath, &(*p)->surrogateFor, ODR_CONTEXT, 8, 1) &&
        odr_implicit(o, z_TagPath, &(*p)->surrogateElement, ODR_CONTEXT, 9, 1) &&
        odr_implicit(o, z_External, &(*p)->other, ODR_CONTEXT, 99, 1) &&
        odr_sequence_end(o);
}

int z_TagUnit(ODR o, Z_TagUnit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_integer, &(*p)->tagType, ODR_CONTEXT, 1, 1) &&
        odr_explicit(o, z_StringOrNumeric, &(*p)->tagValue, ODR_CONTEXT, 2, 0) &&
        odr_implicit(o, odr_integer, &(*p)->tagOccurrence, ODR_CONTEXT, 3, 1) &&
        odr_sequence_end(o);
}

int z_TagPath(ODR o, Z_TagPath **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = (Z_TagPath *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, (Odr_fun)z_TagUnit, &(*p)->tags, &(*p)->num_tags))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Order(ODR o, Z_Order **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_bool, &(*p)->ascending, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, odr_integer, &(*p)->order, ODR_CONTEXT, 2, 0) &&
        odr_sequence_end(o);
}

int z_Usage(ODR o, Z_Usage **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_integer, &(*p)->type, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->restriction, ODR_CONTEXT,
	    2, 1) &&
        odr_sequence_end(o);
}

int z_HitVector(ODR o, Z_HitVector **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        z_Term(o, &(*p)->satisfier, 1) &&
        odr_implicit(o, z_IntUnit, &(*p)->offsetIntoElement, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, z_IntUnit, &(*p)->length, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, odr_integer, &(*p)->hitRank, ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, odr_octetstring, &(*p)->targetToken, ODR_CONTEXT,
	    4, 1) &&
        odr_sequence_end(o);
}

int z_Triple(ODR o, Z_Triple **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_NONE, -1, -1, Z_Triple_integer, (Odr_fun)odr_integer},
	{ODR_NONE, -1, -1, Z_Triple_internationalString, (Odr_fun)z_InternationalString},
	/* The entry below provides some backwards compatibility */
	{ODR_NONE, -1, -1, Z_Triple_internationalString, (Odr_fun)odr_visiblestring},
	{ODR_NONE, -1, -1, Z_Triple_octetString, (Odr_fun)odr_octetstring},
	{ODR_NONE, -1, -1, Z_Triple_oid, (Odr_fun)odr_oid},
	{ODR_NONE, -1, -1, Z_Triple_boolean, (Odr_fun)odr_bool},
	{ODR_NONE, -1, -1, Z_Triple_null, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Triple_unit, (Odr_fun)z_Unit},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Triple_valueAndUnit, (Odr_fun)z_IntUnit},
	{-1, -1, -1, -1, 0}
    };

    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_oid, &(*p)->variantSetId, ODR_CONTEXT, 0, 1) &&
	odr_implicit(o, odr_integer, &(*p)->zclass, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->type, ODR_CONTEXT, 2, 0) &&
	odr_constructed_begin(o, &(*p)->value, ODR_CONTEXT, 3) &&
	odr_choice(o, arm, &(*p)->value, &(*p)->which) &&
	odr_constructed_end(o) &&
	odr_sequence_end(o);
}

int z_Variant(ODR o, Z_Variant **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_oid, &(*p)->globalVariantSetId, ODR_CONTEXT,
	    1, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	odr_sequence_of(o, (Odr_fun)z_Triple, &(*p)->triples, &(*p)->num_triples) &&
	odr_sequence_end(o);
}
