/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-exp.c,v $
 * Revision 1.1  1995-08-10 08:54:02  quinn
 * Added Explain
 *
 *
 */

#include <proto.h>
#include <prt-exp.h>

int z_LanguageCode(ODR o, char **p, int opt);
int z_CommonInfo(ODR o, Z_CommonInfo **p, int opt);
int z_HumanStringUnit(ODR o, Z_HumanStringUnit **p, int opt);
int z_HumanString(ODR o, Z_HumanString **p, int opt);
int z_IconObjectUnit(ODR o, Z_IconObjectUnit **p, int opt);
int z_IconObject(ODR o, Z_IconObject **p, int opt);
int z_ContactInfo(ODR o, Z_ContactInfo **p, int opt);
int z_NetworkAddressIA(ODR o, Z_NetworkAddressIA **p, int opt);
int z_NetworkAddressOPA(ODR o, Z_NetworkAddressOPA **p, int opt);
int z_NetworkAddressOther(ODR o, Z_NetworkAddressOther **p, int opt);
int z_NetworkAddress(ODR o, Z_NetworkAddress **p, int opt);
int z_AccessInfo(ODR o, Z_AccessInfo **p, int opt);
int z_QueryTypeDetails(ODR o, Z_QueryTypeDetails **p, int opt);
int z_PrivateCapOperator(ODR o, Z_PrivateCapOperator **p, int opt);
int z_PrivateCapabilities(ODR o, Z_PrivateCapabilities **p, int opt);
int z_RpnCapabilities(ODR o, Z_RpnCapabilities **p, int opt);
int z_Iso8777Capabilities(ODR o, Z_Iso8777Capabilities **p, int opt);
int z_ProxSupportPrivate(ODR o, Z_ProxSupportPrivate **p, int opt);
int z_ProxSupportUnit(ODR o, Z_ProxSupportUnit **p, int opt);
int z_ProximitySupport(ODR o, Z_ProximitySupport **p, int opt);
int z_SearchKey(ODR o, Z_SearchKey **p, int opt);
int z_AccessRestrictionsUnit(ODR o, Z_AccessRestrictionsUnit **p, int opt);
int z_AccessRestrictions(ODR o, Z_AccessRestrictions **p, int opt);
int z_CostsOtherCharge(ODR o, Z_CostsOtherCharge **p, int opt);
int z_Costs(ODR o, Z_Costs **p, int opt);
int z_Charge(ODR o, Z_Charge **p, int opt);
int z_DatabaseList(ODR o, Z_DatabaseList **p, int opt);
int z_AttributeCombinations(ODR o, Z_AttributeCombinations **p, int opt);
int z_AttributeCombination(ODR o, Z_AttributeCombination **p, int opt);
int z_AttributeValueList(ODR o, Z_AttributeValueList **p, int opt);
int z_AttributeOccurrence(ODR o, Z_AttributeOccurrence **p, int opt);
int z_AttributeValue(ODR o, Z_AttributeValue **p, int opt);
int z_TargetInfo(ODR o, Z_TargetInfo **p, int opt);
int z_DatabaseInfo(ODR o, Z_DatabaseInfo **p, int opt);
int z_TagTypeMapping(ODR o, Z_TagTypeMapping **p, int opt);
int z_SchemaInfo(ODR o, Z_SchemaInfo **p, int opt);
int z_ElementInfo(ODR o, Z_ElementInfo **p, int opt);
int z_PathUnit(ODR o, Z_PathUnit **p, int opt);
int z_Path(ODR o, Z_Path **p, int opt);
int z_ElementInfoList(ODR o, Z_Path **p, int opt);
int z_ElementDataType(ODR o, Z_ElementDataType **p, int opt);
int z_TagSetInfoElements(ODR o, Z_TagSetInfoElements **p, int opt);
int z_TagSetInfo(ODR o, Z_TagSetInfo **p, int opt);
int z_RecordSyntaxInfo(ODR o, Z_RecordSyntaxInfo **p, int opt);
int z_AttributeSetInfo(ODR o, Z_AttributeSetInfo **p, int opt);
int z_AttributeType(ODR o, Z_AttributeType **p, int opt);
int z_AttributeDescription(ODR o, Z_AttributeDescription **p, int opt);
int z_TermListElement(ODR o, Z_TermListElement **p, int opt);
int z_TermListInfo(ODR o, Z_TermListInfo **p, int opt);
int z_ExtendedServicesInfo(ODR o, Z_ExtendedServicesInfo **p, int opt);
int z_AttributeDetails(ODR o, Z_AttributeDetails **p, int opt);
int z_AttributeSetDetails(ODR o, Z_AttributeSetDetails **p, int opt);
int z_AttributeTypeDetails(ODR o, Z_AttributeTypeDetails **p, int opt);
int z_OmittedAttributeInterpretation(ODR o, Z_OmittedAttributeInterpretation **p, int opt);
int z_EScanInfo(ODR o, Z_EScanInfo **p, int opt);
int z_TermListDetails(ODR o, Z_TermListDetails **p, int opt);
int z_ElementSetDetails(ODR o, Z_ElementSetDetails **p, int opt);
int z_RetrievalRecordDetails(ODR o, Z_RetrievalRecordDetails **p, int opt);
int z_PerElementDetails(ODR o, Z_PerElementDetails **p, int opt);
int z_RecordTag(ODR o, Z_RecordTag **p, int opt);
int z_SortDetails(ODR o, Z_SortDetails **p, int opt);
int z_SortKeyDetailsSortType(ODR o, Z_SortKeyDetailsSortType **p, int opt);
int z_SortKeyDetails(ODR o, Z_SortKeyDetails **p, int opt);
int z_ProcessingInformation(ODR o, Z_ProcessingInformation **p, int opt);
int z_VariantSetInfo(ODR o, Z_VariantSetInfo **p, int opt);
int z_VariantClass(ODR o, Z_VariantClass **p, int opt);
int z_VariantType(ODR o, Z_VariantType **p, int opt);
int z_VariantValue(ODR o, Z_VariantValue **p, int opt);
int z_ValueSetEnumerated(ODR o, Z_ValueSetEnumerated **p, int opt);
int z_ValueSet(ODR o, Z_ValueSet **p, int opt);
int z_ValueRange(ODR o, Z_ValueRange **p, int opt);
int z_ValueDescription(ODR o, Z_ValueDescription **p, int opt);
int z_UnitInfo(ODR o, Z_UnitInfo **p, int opt);
int z_UnitType(ODR o, Z_UnitType **p, int opt);
int z_Units(ODR o, Z_Units **p, int opt);
int z_CategoryList(ODR o, Z_CategoryList **p, int opt);
int z_CategoryInfo(ODR o, Z_CategoryInfo **p, int opt);
int z_ExplainRecord(ODR o, Z_ExplainRecord **p, int opt);

int z_LanguageCode(ODR o, char **p, int opt)
{
    return z_InternationalString(o, p, opt);
}

int z_CommonInfo(ODR o, Z_CommonInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_generalizedtime, &(*p)->dateAdded, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_generalizedtime, &(*p)->dateChanged, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, odr_generalizedtime, &(*p)->expiry, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_LanguageCode, &(*p)->humanStringLanguage, ODR_CONTEXT, 3, 1) &&
        z_OtherInformation(o, &(*p)->otherInfo, 1) &&
        odr_sequence_end(o);
}

int z_HumanStringUnit(ODR o, Z_HumanStringUnit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_LanguageCode, &(*p)->language, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->text, ODR_CONTEXT,
	    1, 0) &&
        odr_sequence_end(o);
}

int z_HumanString(ODR o, Z_HumanString **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt && odr_ok(o);
    if (odr_sequence_of(o, z_HumanStringUnit, &(*p)->strings,
	&(*p)->num_strings))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_IconObjectUnit(ODR o, Z_IconObjectUnit **p, int opt)
{
    static Odr_arm arm[] = 
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_IconObject_ianaType,
	    z_InternationalString},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_IconObject_z3950type,
	    z_InternationalString},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_IconObject_otherType,
	    z_InternationalString},
	{-1, -1, -1, -1, 0}
    };

    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
    	odr_constructed_begin(o, &(*p)->bodyType, ODR_CONTEXT, 1) &&
	odr_choice(o, arm, &(*p)->bodyType, &(*p)->which) &&
	odr_constructed_end(o) &&
        odr_implicit(o, odr_octetstring, &(*p)->content, ODR_CONTEXT, 2, 0) &&
        odr_sequence_end(o);
}

int z_IconObject(ODR o, Z_IconObject **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_IconObjectUnit, &(*p)->iconUnits,
    	&(*p)->num_iconUnits))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ContactInfo(ODR o, Z_ContactInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->address, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->email, ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->phone, ODR_CONTEXT, 4, 1) &&
	odr_sequence_end(o);
}

int z_NetworkAddressIA(ODR o, Z_NetworkAddressIA **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
        odr_implicit(o, z_InternationalString, &(*p)->hostAddress, ODR_CONTEXT, 0, 0) &&
        odr_implicit(o, odr_integer, &(*p)->port, ODR_CONTEXT, 1, 0) &&
	odr_sequence_end(o);
}

int z_NetworkAddressOPA(ODR o, Z_NetworkAddressOPA **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
        odr_implicit(o, z_InternationalString, &(*p)->pSel, ODR_CONTEXT, 0, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->sSel, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->tSel, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->nSap, ODR_CONTEXT, 3, 0) &&
	odr_sequence_end(o);
}

int z_NetworkAddressOther(ODR o, Z_NetworkAddressOther **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
        odr_implicit(o, z_InternationalString, &(*p)->type, ODR_CONTEXT, 0, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->address, ODR_CONTEXT, 1, 0) &&
	odr_sequence_end(o);
}

int z_NetworkAddress(ODR o, Z_NetworkAddress **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_NetworkAddress_iA,
	    z_NetworkAddressIA},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_NetworkAddress_oPA,
	    z_NetworkAddressOPA},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_NetworkAddress_other,
	    z_NetworkAddressOther},
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

int z_AccessInfo(ODR o, Z_AccessInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
	odr_implicit_settag(o, ODR_CONTEXT, 0) &&
	(odr_sequence_of(o, z_QueryTypeDetails, &(*p)->queryTypesSupported,
	    &(*p)->num_queryTypesSupported) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	(odr_sequence_of(o, odr_oid, &(*p)->diagnosticsSets,
	    &(*p)->num_diagnosticsSets) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, odr_oid, &(*p)->attributeSetIds,
	    &(*p)->num_attributeSetIds) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, odr_oid, &(*p)->schemas, &(*p)->num_schemas) ||
	    odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 4) &&
	(odr_sequence_of(o, odr_oid, &(*p)->recordSyntaxes,
	    &(*p)->num_recordSyntaxes) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 5) &&
	(odr_sequence_of(o, odr_oid, &(*p)->resourceChallenges,
	    &(*p)->num_resourceChallenges) || odr_ok(o)) &&
        odr_implicit(o, z_AccessRestrictions, &(*p)->restrictedAccess,
	    ODR_CONTEXT, 6, 1) &&
        odr_implicit(o, z_Costs, &(*p)->costInfo, ODR_CONTEXT, 8, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 9) &&
	(odr_sequence_of(o, odr_oid, &(*p)->variantSets,
	    &(*p)->num_variantSets) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 10) &&
	(odr_sequence_of(o, z_ElementSetName, &(*p)->elementSetNames,
	    &(*p)->num_elementSetNames) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 11) &&
	(odr_sequence_of(o, z_InternationalString, &(*p)->unitSystems,
	    &(*p)->num_unitSystems) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_QueryTypeDetails(ODR o, Z_QueryTypeDetails **p, int opt)
{
	static Odr_arm arm[] =
	{
	    {ODR_IMPLICIT, ODR_CONTEXT, 0, Z_QueryTypeDetails_private,
	    	z_PrivateCapabilities},
	    {ODR_IMPLICIT, ODR_CONTEXT, 1, Z_QueryTypeDetails_rpn,
	    	z_RpnCapabilities},
	    {ODR_IMPLICIT, ODR_CONTEXT, 2, Z_QueryTypeDetails_iso8777,
	    	z_Iso8777Capabilities},
	    {ODR_IMPLICIT, ODR_CONTEXT, 3, Z_QueryTypeDetails_z3958,
	    	z_HumanString},
	    {ODR_IMPLICIT, ODR_CONTEXT, 4, Z_QueryTypeDetails_erpn,
	    	z_RpnCapabilities},
	    {ODR_IMPLICIT, ODR_CONTEXT, 5, Z_QueryTypeDetails_rankedList,
	    	z_HumanString},
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

int z_PrivateCapOperator(ODR o, Z_PrivateCapOperator **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->operator, ODR_CONTEXT, 0, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
	odr_sequence_end(o);
}

int z_PrivateCapabilities(ODR o, Z_PrivateCapabilities **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit_settag(o, ODR_CONTEXT, 0) &&
	(odr_sequence_of(o, z_PrivateCapOperator, &(*p)->operators,
	    &(*p)->num_operators) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	(odr_sequence_of(o, z_SearchKey, &(*p)->searchKeys,
	    &(*p)->num_searchKeys) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_HumanString, &(*p)->description,
	    &(*p)->num_description) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_RpnCapabilities(ODR o, Z_RpnCapabilities **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
	odr_implicit_settag(o, ODR_CONTEXT, 0) &&
	(odr_sequence_of(o, odr_integer, &(*p)->operators, &(*p)->num_operators) || odr_ok(o)) &&
        odr_implicit(o, odr_bool, &(*p)->resultSetAsOperandSupported,
	    ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, odr_bool, &(*p)->restrictionOperandSupported,
	    ODR_CONTEXT, 2, 0) &&
        odr_implicit(o, z_ProximitySupport, &(*p)->proximity, ODR_CONTEXT,
	    3, 1) &&
        odr_sequence_end(o);
}

int z_Iso8777Capabilities(ODR o, Z_Iso8777Capabilities **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
	odr_implicit_settag(o, ODR_CONTEXT, 0) &&
	odr_sequence_of(o, z_SearchKey, &(*p)->searchKeys,
	    &(*p)->num_searchKeys) &&
        odr_implicit(o, z_HumanString, &(*p)->restrictions, ODR_CONTEXT,
	    1, 1) &&
        odr_sequence_end(o);
}

int z_ProxSupportPrivate(ODR o, Z_ProxSupportPrivate **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_integer, &(*p)->unit, ODR_CONTEXT, 0, 0) &&
	odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
	odr_sequence_end(o);
}

int z_ProxSupportUnit(ODR o, Z_ProxSupportUnit **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ProxSupportUnit_known,
	    odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ProxSupportUnit_private,
	    z_ProxSupportPrivate},
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

int z_ProximitySupport(ODR o, Z_ProximitySupport **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_bool, &(*p)->anySupport, ODR_CONTEXT, 0, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	(odr_sequence_of(o, z_ProxSupportUnit, &(*p)->unitsSupported,
	    &(*p)->num_unitsSupported) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_SearchKey(ODR o, Z_SearchKey **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->searchKey, ODR_CONTEXT, 0, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
        odr_sequence_end(o);
}

int z_AccessRestrictionsUnit(ODR o, Z_AccessRestrictionsUnit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_explicit(o, odr_integer, &(*p)->accessType, ODR_CONTEXT, 0, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->accessText, ODR_CONTEXT, 1, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, odr_oid, &(*p)->accessChallenges,
	    &(*p)->num_accessChallenges) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_AccessRestrictions(ODR o, Z_AccessRestrictions **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_AccessRestrictionsUnit, &(*p)->restrictions,
    	&(*p)->num_restrictions))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_CostsOtherCharge(ODR o, Z_CostsOtherCharge **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
        odr_implicit(o, z_HumanString, &(*p)->forWhat, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_Charge, &(*p)->charge, ODR_CONTEXT, 2, 1) &&
        odr_sequence_end(o);
}
    	
int z_Costs(ODR o, Z_Costs **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_Charge, &(*p)->connectCharge, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_Charge, &(*p)->connectTime, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, z_Charge, &(*p)->displayCharge, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_Charge, &(*p)->searchCharge, ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, z_Charge, &(*p)->subscriptCharge, ODR_CONTEXT, 4, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 5) &&
	(odr_sequence_of(o, z_CostsOtherCharge, &(*p)->otherCharges,
	    &(*p)->num_otherCharges) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_Charge(ODR o, Z_Charge **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_IntUnit, &(*p)->cost, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_Unit, &(*p)->perWhat, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->text, ODR_CONTEXT, 3, 1) &&
        odr_sequence_end(o);
}

int z_DatabaseList(ODR o, Z_DatabaseList **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_DatabaseName, &(*p)->databases,
	&(*p)->num_databases))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_AttributeCombinations(ODR o, Z_AttributeCombinations **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_oid, &(*p)->defaultAttributeSet,
	    ODR_CONTEXT, 0, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	odr_sequence_of(o, z_AttributeCombination, &(*p)->legalCombinations,
	    &(*p)->num_legalCombinations) &&
        odr_sequence_end(o);
}

int z_AttributeCombination(ODR o, Z_AttributeCombination **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_AttributeOccurrence, &(*p)->occurrences,
	&(*p)->num_occurrences))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_AttributeValueList(ODR o, Z_AttributeValueList **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_StringOrNumeric, &(*p)->attributes,
	&(*p)->num_attributes))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_AttributeOccurrence(ODR o, Z_AttributeOccurrence **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_AttributeOcc_anyOrNone, odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_AttributeOcc_specific,
	    z_AttributeValueList},
	{-1, -1, -1, -1, 0}
    };

    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_oid, &(*p)->attributeSet, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_integer, &(*p)->attributeType, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, odr_null, &(*p)->mustBeSupplied, ODR_CONTEXT, 2, 1) &&
	odr_choice(o, arm, &(*p)->attributeValues, &(*p)->which) &&
        odr_sequence_end(o);
}

int z_AttributeValue(ODR o, Z_AttributeValue **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_explicit(o, z_StringOrNumeric, &(*p)->value, ODR_CONTEXT, 0, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_StringOrNumeric, &(*p)->subAttributes,
	    &(*p)->num_subAttributes) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, z_StringOrNumeric, &(*p)->superAttributes,
	    &(*p)->num_superAttributes) || odr_ok(o)) &&
        odr_implicit(o, odr_null, &(*p)->partialSupport, ODR_CONTEXT, 4, 1) &&
	odr_sequence_end(o);
}



int z_TargetInfo(ODR o, Z_TargetInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    1, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->recentNews, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_IconObject, &(*p)->icon, ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, odr_bool, &(*p)->namedResultSets, ODR_CONTEXT, 4, 0) &&
        odr_implicit(o, odr_bool, &(*p)->multipleDBsearch, ODR_CONTEXT, 5, 0) &&
        odr_implicit(o, odr_integer, &(*p)->maxResultSets, ODR_CONTEXT, 6, 1) &&
        odr_implicit(o, odr_integer, &(*p)->maxResultSize, ODR_CONTEXT, 7, 1) &&
        odr_implicit(o, odr_integer, &(*p)->maxTerms, ODR_CONTEXT, 8, 1) &&
        odr_implicit(o, z_IntUnit, &(*p)->timeoutInterval, ODR_CONTEXT, 9, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->welcomeMessage, ODR_CONTEXT,
	    10, 1) &&
        odr_implicit(o, z_ContactInfo, &(*p)->contactInfo, ODR_CONTEXT,
	    11, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT,
	    12, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 13) &&
	(odr_sequence_of(o, z_InternationalString, &(*p)->nicknames,
	    &(*p)->num_nicknames) || odr_ok(o)) &&
        odr_implicit(o, z_HumanString, &(*p)->usageRest, ODR_CONTEXT, 14, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->paymentAddr, ODR_CONTEXT,
	    15, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->hours, ODR_CONTEXT, 16, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 17) &&
	(odr_sequence_of(o, z_DatabaseList, &(*p)->dbCombinations,
	    &(*p)->num_dbCombinations) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 18) &&
	(odr_sequence_of(o, z_NetworkAddress, &(*p)->addresses,
	    &(*p)->num_addresses) || odr_ok(o)) &&
        odr_implicit(o, z_AccessInfo, &(*p)->commonAccessInfo, ODR_CONTEXT,
	    19, 1) &&
        odr_sequence_end(o);
}

int z_DatabaseInfo(ODR o, Z_DatabaseInfo **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_Exp_RecordCount_actualNumber,
	    odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Exp_RecordCount_approxNumber,
	    odr_integer},
	{-1, -1, -1, -1, 0}
    };

    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_DatabaseName, &(*p)->name, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, odr_null, &(*p)->explainDatabase, ODR_CONTEXT, 2, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, z_DatabaseName, &(*p)->nicknames,
	    &(*p)->num_nicknames) || odr_ok(o)) &&
        odr_implicit(o, z_IconObject, &(*p)->icon, ODR_CONTEXT, 4, 1) &&
        odr_implicit(o, odr_bool, &(*p)->userFee, ODR_CONTEXT, 5, 0) &&
        odr_implicit(o, odr_bool, &(*p)->available, ODR_CONTEXT, 6, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->titleString, ODR_CONTEXT, 7, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 8) &&
	(odr_sequence_of(o, z_HumanString, &(*p)->keywords,
	    &(*p)->num_keywords) || odr_ok(o)) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 9, 1) &&
        odr_implicit(o, z_DatabaseList, &(*p)->associatedDbs, ODR_CONTEXT,
	    10, 1) &&
        odr_implicit(o, z_DatabaseList, &(*p)->subDbs, ODR_CONTEXT, 11, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->disclaimers, ODR_CONTEXT,
	    12, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->news, ODR_CONTEXT, 13, 1) &&
	((odr_constructed_begin(o, p, ODR_CONTEXT, 14) &&
	    odr_choice(o, arm, &(*p)->recordCount, &(*p)->recordCount_which) &&
	    odr_constructed_end(o)) || odr_ok(o)) &&
        odr_implicit(o, z_HumanString, &(*p)->defaultOrder, ODR_CONTEXT,
	    15, 1) &&
        odr_implicit(o, odr_integer, &(*p)->avRecordSize, ODR_CONTEXT, 16, 1) &&
        odr_implicit(o, odr_integer, &(*p)->maxRecordSize, ODR_CONTEXT,
	    17, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->hours, ODR_CONTEXT, 18, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->bestTime, ODR_CONTEXT, 19, 1) &&
        odr_implicit(o, odr_generalizedtime, &(*p)->lastUpdate, ODR_CONTEXT,
	    20, 1) &&
        odr_implicit(o, z_IntUnit, &(*p)->updateInterval, ODR_CONTEXT, 21, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->coverage, ODR_CONTEXT, 22, 1) &&
        odr_implicit(o, odr_bool, &(*p)->proprietary, ODR_CONTEXT, 23, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->copyrightText, ODR_CONTEXT,
	    24, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->copyrightNotice, ODR_CONTEXT,
	    25, 1) &&
        odr_implicit(o, z_ContactInfo, &(*p)->producerContactInfo, ODR_CONTEXT,
	    26, 1) &&
        odr_implicit(o, z_ContactInfo, &(*p)->supplierContactInfo, ODR_CONTEXT,
	    27, 1) &&
        odr_implicit(o, z_ContactInfo, &(*p)->submissionContactInfo,
	    ODR_CONTEXT, 28, 1) &&
        odr_implicit(o, z_AccessInfo, &(*p)->accessInfo, ODR_CONTEXT, 29, 1) &&
        odr_sequence_end(o);
}

int z_TagTypeMapping(ODR o, Z_TagTypeMapping **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	odr_implicit(o, odr_integer, &(*p)->tagType, ODR_CONTEXT, 0, 0) &&
	odr_implicit(o, odr_oid, &(*p)->tagSet, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, odr_null, &(*p)->defaultTagType, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_SchemaInfo(ODR o, Z_SchemaInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_oid, &(*p)->schema, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    2, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 3, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 4) &&
	(odr_sequence_of(o, z_TagTypeMapping, &(*p)->tagTypeMapping,
	    &(*p)->num_tagTypeMapping) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 5) &&
	(odr_sequence_of(o, z_ElementInfo, &(*p)->recordStructure,
	    &(*p)->num_recordStructure) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_ElementInfo(ODR o, Z_ElementInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->elementName, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_Path, &(*p)->elementTagPath, ODR_CONTEXT, 2, 0) &&
        odr_explicit(o, z_ElementDataType, &(*p)->dataType, ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, odr_bool, &(*p)->required, ODR_CONTEXT, 4, 0) &&
        odr_implicit(o, odr_bool, &(*p)->repeatable, ODR_CONTEXT, 5, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 6, 1) &&
        odr_sequence_end(o);
}

int z_PathUnit(ODR o, Z_PathUnit **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	odr_implicit(o, odr_integer, &(*p)->tagType, ODR_CONTEXT, 1, 0) &&
	odr_explicit(o, z_StringOrNumeric, &(*p)->tagValue, ODR_CONTEXT,
	    2, 0) &&
	odr_sequence_end(o);
}

int z_Path(ODR o, Z_Path **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    odr_implicit_settag(o, ODR_CONTEXT, 201);
    if (odr_sequence_of(o, z_OtherInformationUnit, &(*p)->list,
	&(*p)->num))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ElementInfoList(ODR o, Z_Path **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    odr_implicit_settag(o, ODR_CONTEXT, 201);
    if (odr_sequence_of(o, z_OtherInformationUnit, &(*p)->list,
	&(*p)->num))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ElementDataType(ODR o, Z_ElementDataType **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_ElementDataType_primitive,
	    odr_integer},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ElementDataType_structured,
	    z_ElementInfoList},
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

int z_TagSetInfoElements(ODR o, Z_TagSetInfoElements **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    return
    	odr_implicit(o, z_InternationalString, &(*p)->elementName,
	    ODR_CONTEXT, 1, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_InternationalString, &(*p)->nicknames,
	    &(*p)->num_nicknames) || odr_ok(o)) &&
	odr_explicit(o, z_StringOrNumeric, &(*p)->elementTag, ODR_CONTEXT,
	    3, 0) &&
	odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT,
	    4, 1) &&
	odr_implicit(o, odr_integer, &(*p)->dataType, ODR_CONTEXT, 5, 1) &&
	z_OtherInformation(o, &(*p)->otherTagInfo, 1) &&
   	odr_sequence_end(o);
}

int z_TagSetInfo(ODR o, Z_TagSetInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_oid, &(*p)->tagSet, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    2, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT,
	    3, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 4) &&
	(odr_sequence_of(o, z_TagSetInfoElements, &(*p)->elements,
	    &(*p)->num_elements) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_RecordSyntaxInfo(ODR o, Z_RecordSyntaxInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_oid, &(*p)->recordSyntax, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    2, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, odr_oid, &(*p)->transferSyntaxes,
	    &(*p)->num_transferSyntaxes) || odr_ok(o)) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 4, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->asn1Module, ODR_CONTEXT,
	    5, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 6) &&
	(odr_sequence_of(o, z_ElementInfo, &(*p)->abstractStructure,
	    &(*p)->num_abstractStructure) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_AttributeSetInfo(ODR o, Z_AttributeSetInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_oid, &(*p)->attributeSet, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 2, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, z_AttributeType, &(*p)->attributes,
	    &(*p)->num_attributes) || odr_ok(o)) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 4, 1) &&
        odr_sequence_end(o);
}

int z_AttributeType(ODR o, Z_AttributeType **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, odr_integer, &(*p)->attributeType, ODR_CONTEXT, 2, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	odr_sequence_of(o, z_AttributeDescription, &(*p)->attributeValues,
	    &(*p)->num_attributeValues) &&
        odr_sequence_end(o);
}

int z_AttributeDescription(ODR o, Z_AttributeDescription **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
        odr_explicit(o, z_StringOrNumeric, &(*p)->attributeValue, ODR_CONTEXT, 2, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, z_StringOrNumeric, &(*p)->equivalentAttributes,
	    &(*p)->num_equivalentAttributes) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_TermListElement(ODR o, Z_TermListElement **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->title, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, odr_integer, &(*p)->searchCost, ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, odr_bool, &(*p)->scanable, ODR_CONTEXT, 4, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 5) &&
	(odr_sequence_of(o, z_InternationalString, &(*p)->broader,
	    &(*p)->num_broader) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 6) &&
	(odr_sequence_of(o, z_InternationalString, &(*p)->narrower,
	    &(*p)->num_narrower) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_TermListInfo(ODR o, Z_TermListInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_DatabaseName, &(*p)->databaseName, ODR_CONTEXT,
	    1, 0) &&
 	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_TermListElement, &(*p)->termLists,
	    &(*p)->num_termLists) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_ExtendedServicesInfo(ODR o, Z_ExtendedServicesInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_oid, &(*p)->type, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, odr_bool, &(*p)->privateType, ODR_CONTEXT, 3, 0) &&
        odr_implicit(o, odr_bool, &(*p)->restrictionsApply, ODR_CONTEXT, 5, 0) &&
        odr_implicit(o, odr_bool, &(*p)->feeApply, ODR_CONTEXT, 6, 0) &&
        odr_implicit(o, odr_bool, &(*p)->available, ODR_CONTEXT, 7, 0) &&
        odr_implicit(o, odr_bool, &(*p)->retentionSupported, ODR_CONTEXT, 8, 0) &&
        odr_implicit(o, odr_integer, &(*p)->waitAction, ODR_CONTEXT, 9, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 10, 1) &&
        odr_implicit(o, odr_external, &(*p)->specificExplain, ODR_CONTEXT, 11, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->esASN, ODR_CONTEXT, 12, 1) &&
        odr_sequence_end(o);
}

int z_AttributeDetails(ODR o, Z_AttributeDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_DatabaseName, &(*p)->databaseName, ODR_CONTEXT, 1, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_AttributeSetDetails, &(*p)->attributesBySet,
	    &(*p)->num_attributesBySet) && odr_ok(o)) &&
        odr_implicit(o, z_AttributeCombinations, &(*p)->attributeCombinations, ODR_CONTEXT, 3, 1) &&
        odr_sequence_end(o);
}

int z_AttributeSetDetails(ODR o, Z_AttributeSetDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_oid, &(*p)->attributeSet, ODR_CONTEXT,
	    0, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	odr_sequence_of(o, z_AttributeTypeDetails, &(*p)->attributesByType,
	    &(*p)->num_attributesByType) &&
        odr_sequence_end(o);
}

int z_AttributeTypeDetails(ODR o, Z_AttributeTypeDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_integer, &(*p)->attributeType, ODR_CONTEXT, 0, 0) &&
        odr_implicit(o, z_OmittedAttributeInterpretation, &(*p)->optionalType, ODR_CONTEXT, 1, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_AttributeValue, &(*p)->attributeValues,
	    &(*p)->num_attributeValues) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_OmittedAttributeInterpretation(ODR o, Z_OmittedAttributeInterpretation **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_explicit(o, z_StringOrNumeric, &(*p)->defaultValue, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->defaultDescription, ODR_CONTEXT, 1, 1) &&
        odr_sequence_end(o);
}

int z_EScanInfo(ODR o, Z_EScanInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, odr_integer, &(*p)->maxStepSize, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->collatingSequence, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, odr_bool, &(*p)->increasing, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_TermListDetails(ODR o, Z_TermListDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->termListName, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_AttributeCombinations, &(*p)->attributes, ODR_CONTEXT, 3, 1) &&
	odr_implicit(o, z_EScanInfo, &(*p)->scanInfo, ODR_CONTEXT, 4, 1) &&
        odr_implicit(o, odr_integer, &(*p)->estNumberTerms, ODR_CONTEXT, 5, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 6) &&
	(odr_sequence_of(o, z_Term, &(*p)->sampleTerms,
	    &(*p)->num_sampleTerms) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_ElementSetDetails(ODR o, Z_ElementSetDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_DatabaseName, &(*p)->databaseName, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_ElementSetName, &(*p)->elementSetName, ODR_CONTEXT, 2, 0) &&
        odr_implicit(o, odr_oid, &(*p)->recordSyntax, ODR_CONTEXT, 3, 0) &&
        odr_implicit(o, odr_oid, &(*p)->schema, ODR_CONTEXT, 4, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 5, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 6) &&
	(odr_sequence_of(o, z_PerElementDetails, &(*p)->detailsPerElement, &(*p)->num_detailsPerElement) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_RetrievalRecordDetails(ODR o, Z_RetrievalRecordDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_DatabaseName, &(*p)->databaseName, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, odr_oid, &(*p)->schema, ODR_CONTEXT, 2, 0) &&
        odr_implicit(o, odr_oid, &(*p)->recordSyntax, ODR_CONTEXT, 3, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 4, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 5) &&
	(odr_sequence_of(o, z_PerElementDetails, &(*p)->detailsPerElement,
	    &(*p)->num_detailsPerElement) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_PerElementDetails(ODR o, Z_PerElementDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_RecordTag, &(*p)->recordTag, ODR_CONTEXT, 1, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_Path, &(*p)->schemaTags, &(*p)->num_schemaTags) ||
	odr_ok(o)) &&
        odr_implicit(o, odr_integer, &(*p)->maxSize, ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, odr_integer, &(*p)->minSize, ODR_CONTEXT, 4, 1) &&
        odr_implicit(o, odr_integer, &(*p)->avgSize, ODR_CONTEXT, 5, 1) &&
        odr_implicit(o, odr_integer, &(*p)->fixedSize, ODR_CONTEXT, 6, 1) &&
        odr_implicit(o, odr_bool, &(*p)->repeatable, ODR_CONTEXT, 8, 0) &&
        odr_implicit(o, odr_bool, &(*p)->required, ODR_CONTEXT, 9, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 12, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->contents, ODR_CONTEXT, 13, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->billingInfo, ODR_CONTEXT, 14, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->restrictions, ODR_CONTEXT, 15, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 16) &&
	(odr_sequence_of(o, z_InternationalString, &(*p)->alternateNames,
	    &(*p)->num_alternateNames) || odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 17) &&
	(odr_sequence_of(o, z_InternationalString, &(*p)->genericNames,
	    &(*p)->num_genericNames) || odr_ok(o)) &&
        odr_implicit(o, z_AttributeCombinations, &(*p)->searchAccess,
	    ODR_CONTEXT, 18, 1) &&
        odr_sequence_end(o);
}

int z_RecordTag(ODR o, Z_RecordTag **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_explicit(o, z_StringOrNumeric, &(*p)->qualifier, ODR_CONTEXT,
	    0, 1) &&
        odr_explicit(o, z_StringOrNumeric, &(*p)->tagValue, ODR_CONTEXT,
	    1, 0) &&
        odr_sequence_end(o);
}

int z_SortDetails(ODR o, Z_SortDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_DatabaseName, &(*p)->databaseName, ODR_CONTEXT,
	    1, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, z_SortKeyDetails, &(*p)->sortKeys,
	    &(*p)->num_sortKeys) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_SortKeyDetailsSortType(ODR o, Z_SortKeyDetailsSortType **p, int opt)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_SortKeyDetailsSortType_character,
	    odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_SortKeyDetailsSortType_numeric,
	    odr_null},
	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_SortKeyDetailsSortType_structured,
	    z_HumanString},
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
    	
int z_SortKeyDetails(ODR o, Z_SortKeyDetails **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 0, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	(odr_sequence_of(o, z_Specification, &(*p)->elementSpecifications,
	    &(*p)->num_elementSpecifications) || odr_ok(o)) &&
        odr_implicit(o, z_AttributeCombinations, &(*p)->attributeSpecifications,
	    ODR_CONTEXT, 2, 1) &&
	odr_explicit(o, z_SortKeyDetailsSortType, &(*p)->sortType, ODR_CONTEXT,
	    3, 1) &&
        odr_implicit(o, odr_integer, &(*p)->caseSensitivity, ODR_CONTEXT,
	    4, 1) &&
        odr_sequence_end(o);
}

int z_ProcessingInformation(ODR o, Z_ProcessingInformation **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_DatabaseName, &(*p)->databaseName, ODR_CONTEXT,
	    1, 0) &&
        odr_implicit(o, odr_integer, &(*p)->processingContext, ODR_CONTEXT,
	    2, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    3, 0) &&
        odr_implicit(o, odr_oid, &(*p)->oid, ODR_CONTEXT, 4, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 5, 1) &&
        odr_implicit(o, odr_external, &(*p)->instructions, ODR_CONTEXT, 6, 1) &&
        odr_sequence_end(o);
}

int z_VariantSetInfo(ODR o, Z_VariantSetInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, odr_oid, &(*p)->variantSet, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    2, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o, z_VariantClass, &(*p)->variants,
	    &(*p)->num_variants) || odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_VariantClass(ODR o, Z_VariantClass **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, odr_integer, &(*p)->variantClass, ODR_CONTEXT, 2, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	odr_sequence_of(o, z_VariantType, &(*p)->variantTypes,
	    &(*p)->num_variantTypes) &&
        odr_sequence_end(o);
}

int z_VariantType(ODR o, Z_VariantType **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, odr_integer, &(*p)->variantType, ODR_CONTEXT, 2, 0) &&
        odr_implicit(o, z_VariantValue, &(*p)->variantValue, ODR_CONTEXT, 3, 1) &&
        odr_sequence_end(o);
}

int z_VariantValue(ODR o, Z_VariantValue **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_explicit(o, odr_integer, &(*p)->dataType, ODR_CONTEXT, 0, 0) &&
        odr_explicit(o, z_ValueSet, &(*p)->values, ODR_CONTEXT, 1, 1) &&
	odr_sequence_end(o);
}

int z_ValueSetEnumerated(ODR o, Z_ValueSetEnumerated **p, int opt)
{
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_ValueDescription, &(*p)->enumerated,
    	&(*p)->num_enumerated))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}
    
int z_ValueSet(ODR o, Z_ValueSet **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_ValueSet_range, z_ValueRange},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ValueSet_enumerated,
	    z_ValueSetEnumerated},
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

int z_ValueRange(ODR o, Z_ValueRange **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_explicit(o, z_ValueDescription, &(*p)->lower, ODR_CONTEXT, 0, 1) &&
        odr_explicit(o, z_ValueDescription, &(*p)->upper, ODR_CONTEXT, 1, 1) &&
	odr_sequence_end(o);
}

int z_ValueDescription(ODR o, Z_ValueDescription **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_NONE, -1, -1, Z_ValueDescription_integer, odr_integer},
	{ODR_NONE, -1, -1, Z_ValueDescription_string, z_InternationalString},
	{ODR_NONE, -1, -1, Z_ValueDescription_octets, odr_octetstring},
	{ODR_NONE, -1, -1, Z_ValueDescription_oid, odr_oid},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ValueDescription_unit, z_Unit},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ValueDescription_valueAndUnit, z_IntUnit},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_UnitInfo(ODR o, Z_UnitInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->unitSystem, ODR_CONTEXT,
	    1, 0) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 2, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	(odr_sequence_of(o,  z_UnitType, &(*p)->units, &(*p)->num_units) ||
	odr_ok(o)) &&
        odr_sequence_end(o);
}

int z_UnitType(ODR o, Z_UnitType **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT,
	    0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
        odr_explicit(o, z_StringOrNumeric, &(*p)->unitType, ODR_CONTEXT,
	    2, 0) &&
	odr_implicit_settag(o, ODR_CONTEXT, 3) &&
	odr_sequence_of(o, z_Units, &(*p)->units, &(*p)->num_units) &&
        odr_sequence_end(o);
}

int z_Units(ODR o, Z_Units **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->name, ODR_CONTEXT, 0, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 1, 1) &&
        odr_explicit(o, z_StringOrNumeric, &(*p)->unit, ODR_CONTEXT, 2, 0) &&
        odr_sequence_end(o);
}

int z_CategoryList(ODR o, Z_CategoryList **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_CommonInfo, &(*p)->commonInfo, ODR_CONTEXT, 0, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 1) &&
	odr_sequence_of(o, z_CategoryInfo, &(*p)->categories,
	    &(*p)->num_categories) &&
        odr_sequence_end(o);
}

int z_CategoryInfo(ODR o, Z_CategoryInfo **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->category, ODR_CONTEXT,
	    1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->originalCategory,
	    ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_HumanString, &(*p)->description, ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->asn1Module, ODR_CONTEXT,
	    4, 1) &&
        odr_sequence_end(o);
}

int z_ExplainRecord(ODR o, Z_ExplainRecord **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_Explain_targetInfo, z_TargetInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Explain_databaseInfo, z_DatabaseInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Explain_schemaInfo, z_SchemaInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Explain_tagSetInfo, z_TagSetInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_Explain_recordSyntaxInfo,
	    z_RecordSyntaxInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_Explain_attributeSetInfo,
	    z_AttributeSetInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 6, Z_Explain_termListInfo,
	    z_TermListInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 7, Z_Explain_extendedServicesInfo,
	    z_ExtendedServicesInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 8, Z_Explain_attributeDetails,
	    z_AttributeDetails},
	{ODR_IMPLICIT, ODR_CONTEXT, 9, Z_Explain_termListDetails,
	    z_TermListDetails},
	{ODR_IMPLICIT, ODR_CONTEXT, 10, Z_Explain_elementSetDetails,
	    z_ElementSetDetails},
	{ODR_IMPLICIT, ODR_CONTEXT, 11, Z_Explain_retrievalRecordDetails,
	    z_RetrievalRecordDetails},
	{ODR_IMPLICIT, ODR_CONTEXT, 12, Z_Explain_sortDetails,
	    z_SortDetails},
	{ODR_IMPLICIT, ODR_CONTEXT, 13, Z_Explain_processing,
	    z_ProcessingInformation},
	{ODR_IMPLICIT, ODR_CONTEXT, 14, Z_Explain_variants,
	    z_VariantSetInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 15, Z_Explain_units, z_UnitInfo},
	{ODR_IMPLICIT, ODR_CONTEXT, 100, Z_Explain_categoryList,
	    z_CategoryList},
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
