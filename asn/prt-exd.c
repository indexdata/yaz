/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-exd.c,v $
 * Revision 1.10  1999-05-26 13:49:12  adam
 * DB Update implemented in client (very basic).
 *
 * Revision 1.9  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.8  1998/02/11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.7  1998/02/10 15:31:46  adam
 * Implemented date and time structure. Changed the Update Extended
 * Service.
 *
 * Revision 1.6  1998/01/05 09:04:57  adam
 * Fixed bugs in encoders/decoders - Not operator (!) missing.
 *
 * Revision 1.5  1997/04/30 08:52:02  quinn
 * Null
 *
 * Revision 1.4  1996/10/10  12:35:12  quinn
 * Added Update extended service.
 *
 * Revision 1.3  1995/09/29  17:11:54  quinn
 * Smallish
 *
 * Revision 1.2  1995/09/27  15:02:41  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.1  1995/08/21  09:16:32  quinn
 * Added Extended services + Item Order
 *
 *
 */

#include <proto.h>

int z_TaskPackage(ODR o, Z_TaskPackage **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_oid, &(*p)->packageType, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->packageName,
		     ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->userId,
		     ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, z_IntUnit, &(*p)->retentionTime, ODR_CONTEXT, 4, 1) &&
        odr_implicit(o, z_Permissions, &(*p)->permissions,
		     ODR_CONTEXT, 5, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->description,
		     ODR_CONTEXT, 6, 1) &&
        odr_implicit(o, odr_octetstring, &(*p)->targetReference,
		     ODR_CONTEXT, 7, 0) &&
        odr_implicit(o, odr_generalizedtime, &(*p)->creationDateTime,
		     ODR_CONTEXT, 8, 1) &&
        odr_implicit(o, odr_integer, &(*p)->taskStatus,
		     ODR_CONTEXT, 9, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 10) &&
	(odr_sequence_of(o, (Odr_fun)z_DiagRec, &(*p)->packageDiagnostics,
			 &(*p)->num_packageDiagnostics, 0) || odr_ok(o)) &&
        odr_implicit(o, z_External, &(*p)->taskSpecificParameters,
		     ODR_CONTEXT, 11, 0) &&
	odr_sequence_end(o);
}

/* ----------------------- ITEM ORDER --------------------- */

int z_IOTargetPart(ODR o, Z_IOTargetPart **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, z_External, &(*p)->itemRequest, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_External, &(*p)->statusOrErrorReport, ODR_CONTEXT,
	    2, 0) &&
	odr_implicit(o, odr_integer, &(*p)->auxiliaryStatus, ODR_CONTEXT,
	    3, 1) &&
	odr_sequence_end(o);
}

int z_IOResultSetItem(ODR o, Z_IOResultSetItem **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, z_InternationalString, &(*p)->resultSetId, ODR_CONTEXT,
	    1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->item, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_IOOriginPartNotToKeep(ODR o, Z_IOOriginPartNotToKeep **p, int opt,
			    const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, z_IOResultSetItem, &(*p)->resultSetItem, ODR_CONTEXT,
	    1, 0) &&
	odr_implicit(o, z_External, &(*p)->itemRequest, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_IOContact(ODR o, Z_IOContact **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    1, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->phone, ODR_CONTEXT,
	    2, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->email, ODR_CONTEXT,
	    3, 1) &&
	odr_sequence_end(o);
}

int z_IOCreditCardInfo(ODR o, Z_IOCreditCardInfo **p, int opt,
		       const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
    	odr_implicit(o, z_InternationalString, &(*p)->nameOnCard, ODR_CONTEXT,
	    1, 0) &&
	odr_implicit(o, z_InternationalString, &(*p)->expirationDate,
	    ODR_CONTEXT, 2, 0) &&
	odr_implicit(o, z_InternationalString, &(*p)->cardNumber, ODR_CONTEXT,
	    3, 0) &&
	odr_sequence_end(o);
}

int z_IOBilling(ODR o, Z_IOBilling **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_IOBilling_billInvoice,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_IOBilling_prepay,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_IOBilling_depositAccount,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_IOBilling_creditCard,
	 (Odr_fun)z_IOCreditCardInfo, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_IOBilling_cardInfoPreviouslySupplied,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_IOBilling_privateKnown,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 6, Z_IOBilling_privateNotKnown,
	 (Odr_fun)z_External, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_constructed_begin(o, &(*p)->paymentMethod,
			      ODR_CONTEXT, 1, 0) &&
	odr_choice(o, arm, &(*p)->paymentMethod, &(*p)->which, 0) &&
	odr_constructed_end(o) &&
	odr_implicit(o, z_InternationalString, &(*p)->customerReference,
		     ODR_CONTEXT, 2, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->customerPONumber,
		     ODR_CONTEXT, 3, 1) &&
	odr_sequence_end(o);
}

int z_IOOriginPartToKeep(ODR o, Z_IOOriginPartToKeep **p, int opt,
			 const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_External, &(*p)->supplDescription,
		     ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_IOContact, &(*p)->contact, ODR_CONTEXT, 2, 1) &&
	odr_implicit(o, z_IOBilling, &(*p)->addlBilling,
		     ODR_CONTEXT, 3, 1) &&
	odr_sequence_end(o);
}

int z_IORequest(ODR o, Z_IORequest **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return 
    	odr_implicit(o, z_IOOriginPartToKeep, &(*p)->toKeep,
		     ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_IOOriginPartNotToKeep, &(*p)->notToKeep,
		     ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

int z_IOTaskPackage(ODR o, Z_IOTaskPackage **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_IOOriginPartToKeep, &(*p)->originPart,
		     ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_IOTargetPart, &(*p)->targetPart,
		     ODR_CONTEXT, 2, 0) &&
	odr_sequence_end(o);
}

int z_ItemOrder(ODR o, Z_ItemOrder **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ItemOrder_esRequest,
	 (Odr_fun)z_IORequest, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ItemOrder_taskPackage,
	 (Odr_fun)z_IOTaskPackage, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (o->direction == ODR_DECODE)
	*p = (Z_ItemOrder *)odr_malloc(o, sizeof(**p));
    else if (!*p)
	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

/* ----------------------- ITEM UPDATE -------------------- */

int z_IUSuppliedRecordsId (ODR o, Z_IUSuppliedRecordsId **p, int opt,
			   const char *name)
{
    static Odr_arm arm[] = {
        {ODR_IMPLICIT, ODR_CONTEXT, 1, Z_IUSuppliedRecordsId_timeStamp,
         odr_generalizedtime, 0},
        {ODR_IMPLICIT, ODR_CONTEXT, 2, Z_IUSuppliedRecordsId_versionNumber,
         z_InternationalString, 0},
        {ODR_IMPLICIT, ODR_CONTEXT, 3, Z_IUSuppliedRecordsId_previousVersion,
         (Odr_fun)odr_external, 0},
        {-1, -1, -1, -1, 0, 0}
    };
    if (!odr_initmember(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
        return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_IUCorrelationInfo (ODR o, Z_IUCorrelationInfo **p, int opt,
			 const char *name)
{
    if (!odr_sequence_begin (o, p, sizeof(**p), 0))
        return opt && odr_ok (o);
    return
        odr_implicit (o, z_InternationalString,
		      &(*p)->note, ODR_CONTEXT, 1, 1) &&
        odr_implicit (o, odr_integer,
		      &(*p)->id, ODR_CONTEXT, 2, 1) &&
        odr_sequence_end (o);
}

int z_IUSuppliedRecords_elem (ODR o, Z_IUSuppliedRecords_elem **p, int opt,
			      const char *name)
{
    static Odr_arm arm[] = {
        {ODR_IMPLICIT, ODR_CONTEXT, 1, Z_IUSuppliedRecords_number,
         (Odr_fun)odr_integer, 0},
        {ODR_IMPLICIT, ODR_CONTEXT, 2, Z_IUSuppliedRecords_string,
         (Odr_fun)z_InternationalString, 0},
        {ODR_IMPLICIT, ODR_CONTEXT, 3, Z_IUSuppliedRecords_opaque,
         (Odr_fun)odr_octetstring, 0},
        {-1, -1, -1, -1, 0, 0}
    };
    if (!odr_sequence_begin (o, p, sizeof(**p), 0))
        return opt && odr_ok (o);
    return
        ((odr_constructed_begin (o, &(*p)->u, ODR_CONTEXT, 1, 0) &&
	  odr_choice (o, arm, &(*p)->u, &(*p)->which, 0) &&
	  odr_constructed_end (o)) || odr_ok(o)) &&
        odr_explicit (o, z_IUSuppliedRecordsId,
		      &(*p)->supplementalId, ODR_CONTEXT, 2, 1) &&
        odr_implicit (o, z_IUCorrelationInfo,
		      &(*p)->correlationInfo, ODR_CONTEXT, 3, 1) &&
        odr_implicit (o, z_External,
		      &(*p)->record, ODR_CONTEXT, 4, 0) &&
        odr_sequence_end (o);
}

int z_IUSuppliedRecords (ODR o, Z_IUSuppliedRecords **p, int opt,
			 const char *name)
{
    if (!odr_initmember (o, p, sizeof(**p)))
        return opt && odr_ok(o);
    if (odr_sequence_of (o, (Odr_fun)z_IUSuppliedRecords_elem, &(*p)->elements,
			 &(*p)->num, 0))
        return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_IUTaskPackageRecordStructure (ODR o, Z_IUTaskPackageRecordStructure **p,
				    int opt, const char *name)
{
    static Odr_arm arm[] = {
        {ODR_IMPLICIT, ODR_CONTEXT, 1, Z_IUTaskPackageRecordStructure_record,
         (Odr_fun)odr_external, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 2,
	 Z_IUTaskPackageRecordStructure_surrogateDiagnostics,
         (Odr_fun)z_DiagRecs, 0},
        {-1, -1, -1, -1, 0, 0}
    };
    if (!odr_sequence_begin (o, p, sizeof(**p), 0))
        return opt && odr_ok (o);
    return
        ((odr_constructed_begin (o, &(*p)->u, ODR_CONTEXT, 1, 0) &&
	  odr_choice (o, arm, &(*p)->u, &(*p)->which, 0) &&
	  odr_constructed_end (o)) || odr_ok(o)) &&
        odr_implicit (o, z_IUCorrelationInfo,
		      &(*p)->correlationInfo, ODR_CONTEXT, 2, 1) &&
        odr_implicit (o, odr_integer,
		      &(*p)->recordStatus, ODR_CONTEXT, 3, 0) &&
	odr_implicit (o, z_DiagRecs, 
		      &(*p)->supplementalDiagnostics, ODR_CONTEXT, 4, 1) &&
        odr_sequence_end (o);
}

int z_IUOriginPartToKeep (ODR o, Z_IUOriginPartToKeep **p, int opt,
			  const char *name)
{
    if (!odr_sequence_begin (o, p, sizeof(**p), 0))
        return opt && odr_ok (o);
    return
        odr_implicit (o, odr_integer,
		      &(*p)->action, ODR_CONTEXT, 1, 0) &&
        odr_implicit (o, z_InternationalString,
		      &(*p)->databaseName, ODR_CONTEXT, 2, 0) &&
        odr_implicit (o, odr_oid,
		      &(*p)->schema, ODR_CONTEXT, 3, 1) &&
        odr_implicit (o, z_InternationalString,
		      &(*p)->elementSetName, ODR_CONTEXT, 4, 1) &&
	odr_implicit (o, odr_external,
		      &(*p)->actionQualifier, ODR_CONTEXT, 5, 1) &&
        odr_sequence_end (o);
}

int z_IUTargetPart (ODR o, Z_IUTargetPart **p, int opt, const char *name)
{
    if (!odr_sequence_begin (o, p, sizeof(**p), 0))
        return opt && odr_ok (o);
    return
        odr_implicit (o, odr_integer,
		      &(*p)->updateStatus, ODR_CONTEXT, 1, 0) &&
        odr_implicit_settag (o, ODR_CONTEXT, 2) &&
        (odr_sequence_of(o, (Odr_fun)z_DiagRec, &(*p)->globalDiagnostics,
			 &(*p)->num_globalDiagnostics, 0) || odr_ok(o)) &&
        odr_implicit_settag (o, ODR_CONTEXT, 3) &&
        odr_sequence_of(o, (Odr_fun)z_IUTaskPackageRecordStructure,
			&(*p)->taskPackageRecords,
			&(*p)->num_taskPackageRecords, 0) &&
        odr_sequence_end (o);
}

int z_IUUpdateEsRequest (ODR o, Z_IUUpdateEsRequest **p, int opt,
			 const char *name)
{
    if (!odr_sequence_begin (o, p, sizeof(**p), 0))
        return opt && odr_ok (o);
    return
        odr_explicit (o, z_IUOriginPartToKeep,
		      &(*p)->toKeep, ODR_CONTEXT, 1, 0) &&
        odr_explicit (o, z_IUSuppliedRecords,
		      &(*p)->notToKeep, ODR_CONTEXT, 2, 0) &&
        odr_sequence_end (o);
}

int z_IUUpdateTaskPackage (ODR o, Z_IUUpdateTaskPackage **p, int opt,
			   const char *name)
{
    if (!odr_sequence_begin (o, p, sizeof(**p), 0))
        return opt && odr_ok (o);
    return
        odr_explicit (o, z_IUOriginPartToKeep,
		      &(*p)->originPart, ODR_CONTEXT, 1, 0) &&
        odr_explicit (o, z_IUTargetPart,
		      &(*p)->targetPart, ODR_CONTEXT, 2, 0) &&
        odr_sequence_end (o);
}

int z_IUUpdate (ODR o, Z_IUUpdate **p, int opt, const char *name)
{
    static Odr_arm arm[] = {
        {ODR_IMPLICIT, ODR_CONTEXT, 1, Z_IUUpdate_esRequest,
         (Odr_fun)z_IUUpdateEsRequest, 0},
        {ODR_IMPLICIT, ODR_CONTEXT, 2, Z_IUUpdate_taskPackage,
         (Odr_fun)z_IUUpdateTaskPackage, 0},
        {-1, -1, -1, -1, 0, 0}
    };
    if (!odr_initmember(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
        return 1;
    *p = 0;
    return opt && odr_ok(o);
}
