/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-esp.c,v $
 * Revision 1.3  1998-02-11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.2  1998/02/10 15:31:46  adam
 * Implemented date and time structure. Changed the Update Extended
 * Service.
 *
 * Revision 1.1  1995/10/12 10:34:37  quinn
 * Added Espec-1.
 *
 *
 */

#include <proto.h>

int z_OccurValues(ODR o, Z_OccurValues **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_integer, &(*p)->start, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->howMany, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_Occurrences(ODR o, Z_Occurrences **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Occurrences_all, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Occurrences_last, (Odr_fun)odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Occurrences_values, (Odr_fun)z_OccurValues},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_Occurrences *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_SpecificTag(ODR o, Z_SpecificTag **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
        odr_implicit(o, odr_oid, &(*p)->schemaId, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_integer, &(*p)->tagType, ODR_CONTEXT, 1, 1) &&
	odr_explicit(o, z_StringOrNumeric, &(*p)->tagValue, ODR_CONTEXT,
	    2, 0) &&
	odr_explicit(o, z_Occurrences, &(*p)->occurrences, ODR_CONTEXT, 3, 1) &&
	odr_sequence_end(o);
}

int z_ETagUnit(ODR o, Z_ETagUnit **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ETagUnit_specificTag, (Odr_fun)z_SpecificTag},
	{ODR_EXPLICIT, ODR_CONTEXT, 2, Z_ETagUnit_wildThing, (Odr_fun)z_Occurrences},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_ETagUnit_wildPath, (Odr_fun)odr_null},
	{-1, -1, -1 -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_ETagUnit *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ETagPath(ODR o, Z_ETagPath **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = (Z_ETagPath *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, (Odr_fun)z_ETagUnit, &(*p)->tags, &(*p)->num_tags))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_SimpleElement(ODR o, Z_SimpleElement **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, z_ETagPath, &(*p)->path, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, z_Variant, &(*p)->variantRequest, ODR_CONTEXT,
	    2, 1) &&
	odr_sequence_end(o);
}

int z_CompoPrimitives(ODR o, Z_CompoPrimitives **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = (Z_CompoPrimitives *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, z_InternationalString, &(*p)->primitives,
	&(*p)->num_primitives))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_CompoSpecs(ODR o, Z_CompoSpecs **p, int opt)
{
    if (o->direction == ODR_DECODE)
	*p = (Z_CompoSpecs *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_sequence_of(o, (Odr_fun)z_SimpleElement, &(*p)->specs, &(*p)->num_specs))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_CompositeElement(ODR o, Z_CompositeElement **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_CompoElement_primitives,
	    (Odr_fun)z_CompoPrimitives},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_CompoElement_specs,
	    (Odr_fun)z_CompoSpecs},
	{-1, -1, -1, -1, 0}
    };

    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
        odr_constructed_begin(o, &(*p)->elementList, ODR_CONTEXT, 1) &&
	odr_choice(o, arm, &(*p)->elementList, &(*p)->which) &&
	odr_constructed_end(o) &&
	odr_implicit(o, z_ETagPath, &(*p)->deliveryTag, ODR_CONTEXT, 2, 0) &&
	odr_implicit(o, z_Variant, &(*p)->variantRequest, ODR_CONTEXT, 3, 1) &&
	odr_sequence_end(o);
}

int z_ElementRequest(ODR o, Z_ElementRequest **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ERequest_simpleElement,
	    (Odr_fun)z_SimpleElement},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ERequest_compositeElement,
	    (Odr_fun)z_CompositeElement},
	{-1, -1, -1 -1, 0}
    };

    if (o->direction == ODR_DECODE)
	*p = (Z_ElementRequest *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Espec1(ODR o, Z_Espec1 **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    return
        odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	(odr_sequence_of(o, z_InternationalString, &(*p)->elementSetNames,
	    &(*p)->num_elementSetNames) || odr_ok(o)) &&
	odr_implicit(o, odr_oid, &(*p)->defaultVariantSetId, ODR_CONTEXT,
	    2, 1) &&
	odr_implicit(o, z_Variant, &(*p)->defaultVariantRequest, ODR_CONTEXT,
	    3, 1) &&
	odr_implicit(o, odr_integer, &(*p)->defaultTagType, ODR_CONTEXT,
	    4, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 5) &&
	(odr_sequence_of(o, (Odr_fun)z_ElementRequest, &(*p)->elements,
	    &(*p)->num_elements) || odr_ok(o)) &&
	odr_sequence_end(o);
}
