/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-ext.c,v $
 * Revision 1.20  1999-04-20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.19  1998/03/31 15:13:19  adam
 * Development towards compiled ASN.1.
 *
 * Revision 1.18  1998/03/31 11:07:44  adam
 * Furhter work on UNIverse resource report.
 * Added Extended Services handling in frontend server.
 *
 * Revision 1.17  1998/03/20 14:46:06  adam
 * Added UNIverse Resource Reports.
 *
 * Revision 1.16  1998/02/11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.15  1998/02/10 15:31:46  adam
 * Implemented date and time structure. Changed the Update Extended
 * Service.
 *
 * Revision 1.14  1998/01/05 09:04:57  adam
 * Fixed bugs in encoders/decoders - Not operator (!) missing.
 *
 * Revision 1.13  1997/05/14 06:53:22  adam
 * C++ support.
 *
 * Revision 1.12  1997/04/30 08:52:02  quinn
 * Null
 *
 * Revision 1.11  1996/10/10  12:35:13  quinn
 * Added Update extended service.
 *
 * Revision 1.10  1996/10/09  15:54:55  quinn
 * Added SearchInfoReport
 *
 * Revision 1.9  1996/06/10  08:53:36  quinn
 * Added Summary,OPAC,ResourceReport
 *
 * Revision 1.8  1996/02/20  12:51:44  quinn
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
    {VAL_SUTRS, Z_External_sutrs, (Odr_fun) z_SUTRS},
    {VAL_EXPLAIN, Z_External_explainRecord, (Odr_fun)z_ExplainRecord},
    {VAL_RESOURCE1, Z_External_resourceReport1, (Odr_fun)z_ResourceReport1},
    {VAL_RESOURCE2, Z_External_resourceReport2, (Odr_fun)z_ResourceReport2},
    {VAL_PROMPT1, Z_External_promptObject1, (Odr_fun)z_PromptObject1 },
    {VAL_GRS1, Z_External_grs1, (Odr_fun)z_GenericRecord},
    {VAL_EXTENDED, Z_External_extendedService, (Odr_fun)z_TaskPackage},
#ifdef ASN_COMPILED
    {VAL_ITEMORDER, Z_External_itemOrder, (Odr_fun)z_IOItemOrder},
#else
    {VAL_ITEMORDER, Z_External_itemOrder, (Odr_fun)z_ItemOrder},
#endif
    {VAL_DIAG1, Z_External_diag1, (Odr_fun)z_DiagnosticFormat},
    {VAL_ESPEC1, Z_External_espec1, (Odr_fun)z_Espec1},
    {VAL_SUMMARY, Z_External_summary, (Odr_fun)z_BriefBib},
    {VAL_OPAC, Z_External_OPAC, (Odr_fun)z_OPACRecord},
    {VAL_SEARCHRES1, Z_External_searchResult1, (Odr_fun)z_SearchInfoReport},
    {VAL_DBUPDATE, Z_External_update, (Odr_fun)z_IUUpdate},
    {VAL_DATETIME, Z_External_dateTime, (Odr_fun)z_DateTime},
    {VAL_UNIVERSE_REPORT, Z_External_universeReport, (Odr_fun)z_UniverseReport},
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

int z_External(ODR o, Z_External **p, int opt, const char *name)
{
    oident *oid;
    Z_ext_typeent *type;

    static Odr_arm arm[] =
    {
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_single,
	 (Odr_fun)odr_any, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_External_octet,
	 (Odr_fun)odr_octetstring, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_External_arbitrary,
	 (Odr_fun)odr_bitstring, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_sutrs,
	 (Odr_fun)z_SUTRS, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_explainRecord,
	 (Odr_fun)z_ExplainRecord, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_resourceReport1,
	 (Odr_fun)z_ResourceReport1, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_resourceReport2,
	 (Odr_fun)z_ResourceReport2, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_promptObject1,
	 (Odr_fun)z_PromptObject1, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_grs1,
	 (Odr_fun)z_GenericRecord, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_extendedService,
	 (Odr_fun)z_TaskPackage, 0},
#ifdef ASN_COMPILED
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_itemOrder,
	 (Odr_fun)z_IOItemOrder, 0},
#else
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_itemOrder,
	 (Odr_fun)z_ItemOrder, 0},
#endif
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_diag1,
	 (Odr_fun)z_DiagnosticFormat, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_espec1,
	 (Odr_fun)z_Espec1, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_summary,
	 (Odr_fun)z_BriefBib, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_OPAC,
	 (Odr_fun)z_OPACRecord, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_searchResult1,
	 (Odr_fun)z_SearchInfoReport, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_update,
	 (Odr_fun)z_IUUpdate, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_dateTime,
	 (Odr_fun)z_DateTime, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_universeReport,
	 (Odr_fun)z_UniverseReport, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    odr_implicit_settag(o, ODR_UNIVERSAL, ODR_EXTERNAL);
    if (!odr_sequence_begin(o, p, sizeof(**p), name))
	return opt && odr_ok(o);
    if (!(odr_oid(o, &(*p)->direct_reference, 1, 0) &&
	  odr_integer(o, &(*p)->indirect_reference, 1, 0) &&
	  odr_graphicstring(o, &(*p)->descriptor, 1, 0)))
	return 0;
    /*
     * Do we know this beast?
     */
    if (o->direction == ODR_DECODE && (*p)->direct_reference &&
	(oid = oid_getentbyoid((*p)->direct_reference)) &&
	(type = z_ext_getentbyref(oid->value)))
    {
	int zclass, tag, cons;
	
	/*
	 * We know it. If it's represented as an ASN.1 type, bias the CHOICE.
	 */
	if (!odr_peektag(o, &zclass, &tag, &cons))
	    return opt && odr_ok(o);
	if (zclass == ODR_CONTEXT && tag == 0 && cons == 1)
	    odr_choice_bias(o, type->what);
    }
    return
	odr_choice(o, arm, &(*p)->u, &(*p)->which, name) &&
	odr_sequence_end(o);
}

