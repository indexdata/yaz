/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-ext.c,v $
 * Revision 1.8  1996-02-20 12:51:44  quinn
 * Completed SCAN. Fixed problems with EXTERNAL.
 *
 * Revision 1.7  1995/10/12  10:34:38  quinn
 * Added Espec-1.
 *
 * Revision 1.6  1995/09/29  17:11:55  quinn
 * Smallish
 *
 * Revision 1.5  1995/09/27  15:02:42  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.4  1995/08/29  11:17:16  quinn
 * *** empty log message ***
 *
 * Revision 1.3  1995/08/21  09:10:18  quinn
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

/*
 * The table below should be moved to the ODR structure itself and
 * be an image of the association context: To help
 * map indirect references when they show up. 
 */
static Z_ext_typeent type_table[] =
{
    {VAL_SUTRS, Z_External_sutrs, z_SUTRS},
    {VAL_EXPLAIN, Z_External_explainRecord, z_ExplainRecord},
    {VAL_RESOURCE1, Z_External_resourceReport1, z_ResourceReport1},
    {VAL_RESOURCE2, Z_External_resourceReport2, z_ResourceReport2},
    {VAL_PROMPT1, Z_External_promptObject1, z_PromptObject1 },
    {VAL_GRS1, Z_External_grs1, z_GenericRecord},
    {VAL_EXTENDED, Z_External_extendedService, z_TaskPackage},
    {VAL_ITEMORDER, Z_External_itemOrder, z_ItemOrder},
    {VAL_DIAG1, Z_External_diag1, z_DiagnosticFormat},
    {VAL_ESPEC1, Z_External_espec1, z_Espec1},
    {VAL_NONE, 0, 0}
};

Z_ext_typeent *z_ext_getentbyref(oid_value val)
{
    Z_ext_typeent *i;

    for (i = type_table; i->dref != VAL_NONE; i++)
	if (i->dref == val)
	    return i;
    return 0;
}

int z_External(ODR o, Z_External **p, int opt)
{
    oident *oid;
    Z_ext_typeent *type;

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
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_diag1, z_DiagnosticFormat},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_espec1, z_Espec1},
	{-1, -1, -1, -1, 0}
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
	(oid = oid_getentbyoid((*p)->direct_reference)) &&
	(type = z_ext_getentbyref(oid->value)))
    {
	int class, tag, cons;

	/*
	 * We know it. If it's represented as an ASN.1 type, bias the CHOICE.
	 */
	if (!odr_peektag(o, &class, &tag, &cons))
	    return opt && odr_ok(o);
	if (class == ODR_CONTEXT && tag == 0 && cons == 1)
	    odr_choice_bias(o, type->what);
    }
    return
	odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
	odr_sequence_end(o);
}
