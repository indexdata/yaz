/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-ext.c,v $
 * Revision 1.3  1995-08-21 09:10:18  quinn
 * Smallish fixes to suppport new formats.
 *
 * Revision 1.2  1995/08/17  12:45:00  quinn
 * Fixed minor problems with GRS-1. Added support in c&s.
 *
 * Revision 1.1  1995/08/15  13:37:41  quinn
 * Improved EXTERNAL
 *
 *
 */

#include <proto.h>

int z_External(ODR o, Z_External **p, int opt)
{
    oident *oid;

    static Odr_arm arm[] =
    {
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_single, odr_any},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_External_octet, odr_octetstring},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_External_arbitrary, odr_bitstring},

	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_sutrs, z_SUTRS},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_explainRecord,
	    z_ExplainRecord},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_resourceReport1,
	    z_ResourceReport1},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_resourceReport2,
	    z_ResourceReport2},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_promptObject1,
	    z_PromptObject1},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_grs1, z_GenericRecord},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_extendedService,
	    z_TaskPackage},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_itemOrder, z_ItemOrder},
	{-1, -1, -1, -1, 0}
    };
    /*
     * The table below should be moved to the ODR structure itself and
     * be an image of the association context: To help
     * map indirect references when they show up. 
     */
    static struct
    {
	oid_value dref;
	int what;          /* discriminator value for the external CHOICE */
    } tab[] =
    {
    	{VAL_SUTRS, Z_External_sutrs},
	{VAL_EXPLAIN, Z_External_explainRecord},
	{VAL_RESOURCE1, Z_External_resourceReport1},
	{VAL_RESOURCE2, Z_External_resourceReport2},
	{VAL_PROMPT1, Z_External_promptObject1},
	{VAL_GRS1, Z_External_grs1},
	{VAL_NONE, 0}
    };

    odr_implicit_settag(o, ODR_UNIVERSAL, ODR_EXTERNAL);
    if (!odr_sequence_begin(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    if (!(odr_oid(o, &(*p)->direct_reference, 1) &&
	odr_integer(o, &(*p)->indirect_reference, 1) &&
	odr_graphicstring(o, &(*p)->descriptor, 1)))
	return 0;
    /*
     * Do we know this beast?
     */
    if (o->direction == ODR_DECODE && (*p)->direct_reference &&
	(oid = oid_getentbyoid((*p)->direct_reference)))
    {
	int i;

	for (i = 0; tab[i].dref != VAL_NONE; i++)
	    if (oid->value == tab[i].dref)
	    {
		odr_choice_bias(o, tab[i].what);
		break;
	    }
    }
    return
	odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
	odr_sequence_end(o);
}
