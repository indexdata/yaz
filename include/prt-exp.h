/*
 * Copyright (c) 1995, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The names of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 */

#ifndef PRT_EXP_H
#define PRT_EXP_H

#include <yconfig.h>

#define multipleDbSearch multipleDBsearch

typedef struct Z_CommonInfo
{
    char *dateAdded;           /* OPTIONAL */
    char *dateChanged;         /* OPTIONAL */
    char *expiry;              /* OPTIONAL */
    char *humanStringLanguage;    /* OPTIONAL */
    Z_OtherInformation *otherInfo;          /* OPTIONAL */
} Z_CommonInfo;

typedef struct Z_HumanStringUnit
{
    char *language;               /* OPTIONAL */
    char *text;
} Z_HumanStringUnit;

typedef struct Z_HumanString
{
    int num_strings;
    Z_HumanStringUnit **strings;
} Z_HumanString;

typedef struct Z_IconObjectUnit
{
    enum
    {
    	Z_IconObject_ianaType,
	Z_IconObject_z3950type,
	Z_IconObject_otherType
    } which;
    char *bodyType;
    Odr_oct *content;
} Z_IconObjectUnit;

typedef struct Z_IconObject
{
    int num_iconUnits;
    Z_IconObjectUnit **iconUnits;
} Z_IconObject;

typedef struct Z_ContactInfo
{
    char *name;                             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    Z_HumanString *address;                 /* OPTIONAL */
    char *email;                            /* OPTIONAL */
    char *phone;                            /* OPTIONAL */
} Z_ContactInfo;

typedef struct Z_NetworkAddressIA
{
    char *hostAddress;
    int *port;
} Z_NetworkAddressIA;

typedef struct Z_NetworkAddressOPA
{
    char *pSel;
    char *sSel;                  /* OPTIONAL */
    char *tSel;                  /* OPTIONAL */
    char *nSap;
} Z_NetworkAddressOPA;

typedef struct Z_NetworkAddressOther
{
    char *type;
    char *address;
} Z_NetworkAddressOther;

typedef struct Z_NetworkAddress
{
    enum
    {
    	Z_NetworkAddress_iA,
	Z_NetworkAddress_oPA,
	Z_NetworkAddress_other
    } which;
    union
    {
    	Z_NetworkAddressIA *internetAddress;
	Z_NetworkAddressOPA *osiPresentationAddress;
	Z_NetworkAddressOther *other;
    } u;
} Z_NetworkAddress;

typedef struct Z_PrivateCapOperator
{
    char *operator;
    Z_HumanString *description;             /* OPTIONAL */
} Z_PrivateCapOperator;
    
typedef struct Z_SearchKey
{
    char *searchKey;
    Z_HumanString *description;             /* OPTIONAL */
} Z_SearchKey;

typedef struct Z_PrivateCapabilities
{
    int num_operators;
    Z_PrivateCapOperator **operators;      /* OPTIONAL */
    int num_searchKeys;
    Z_SearchKey **searchKeys;               /* OPTIONAL */
    int num_description;
    Z_HumanString **description;            /* OPTIONAL */
} Z_PrivateCapabilities;

typedef struct Z_ProxSupportPrivate
{
    int *unit;
    Z_HumanString *description;             /* OPTIONAL */
} Z_ProxSupportPrivate;

typedef struct Z_ProxSupportUnit
{
    enum
    {
    	Z_ProxSupportUnit_known,
	Z_ProxSupportUnit_private
    } which;
    union
    {
    	int known;
	Z_ProxSupportPrivate *private;
    } u;
} Z_ProxSupportUnit;

typedef struct Z_ProximitySupport
{
    bool_t *anySupport;
    int num_unitsSupported;
    Z_ProxSupportUnit **unitsSupported;     /* OPTIONAL */
} Z_ProximitySupport;

typedef struct Z_RpnCapabilities
{
    int num_operators;
    int **operators;                        /* OPTIONAL */
    bool_t *resultSetAsOperandSupported;
    bool_t *restrictionOperandSupported;
    Z_ProximitySupport *proximity;          /* OPTIONAL */
} Z_RpnCapabilities;

typedef struct Z_Iso8777Capabilities
{
    int num_searchKeys;
    Z_SearchKey **searchKeys;
    Z_HumanString *restrictions;            /* OPTIONAL */
} Z_Iso8777Capabilities;

typedef struct Z_QueryTypeDetails
{
    enum
    {
    	Z_QueryTypeDetails_private,
    	Z_QueryTypeDetails_rpn,
    	Z_QueryTypeDetails_iso8777,
    	Z_QueryTypeDetails_z3958,
    	Z_QueryTypeDetails_erpn,
    	Z_QueryTypeDetails_rankedList
    } which;
    union
    {
	Z_PrivateCapabilities *private;
	Z_RpnCapabilities *rpn;
	Z_Iso8777Capabilities *iso8777;
	Z_HumanString *z3958;
	Z_RpnCapabilities *erpn;
	Z_HumanString *rankedList;
    } u;
} Z_QueryTypeDetails;

typedef struct Z_AccessRestrictionsUnit
{
    int *accessType;
#define Z_AccessRestrictions_any                 0
#define Z_AccessRestrictions_search              1
#define Z_AccessRestrictions_present             2
#define Z_AccessRestrictions_specific_elements   3
#define Z_AccessRestrictions_extended_services   4
#define Z_AccessRestrictions_by_database         5
    Z_HumanString *accessText;              /* OPTIONAL */
    int num_accessChallenges;
    Odr_oid **accessChallenges;             /* OPTIONAL */
} Z_AccessRestrictionsUnit;

typedef struct Z_AccessRestrictions
{
    int num_restrictions;
    Z_AccessRestrictionsUnit **restrictions;
} Z_AccessRestrictions;

typedef struct Z_Charge
{
    Z_IntUnit *cost;
    Z_Unit *perWhat;                        /* OPTIONAL */
    Z_HumanString *text;                    /* OPTIONAL */
} Z_Charge;

typedef struct Z_CostsOtherCharge
{
    Z_HumanString *forWhat;
    Z_Charge *charge;
} Z_CostsOtherCharge;

typedef struct Z_Costs
{
    Z_Charge *connectCharge;                /* OPTIONAL */
    Z_Charge *connectTime;                  /* OPTIONAL */
    Z_Charge *displayCharge;                /* OPTIONAL */
    Z_Charge *searchCharge;                 /* OPTIONAL */
    Z_Charge *subscriptCharge;              /* OPTIONAL */
    int num_otherCharges;
    Z_CostsOtherCharge **otherCharges;      /* OPTIONAL */
} Z_Costs;

typedef struct Z_AccessInfo
{
    int num_queryTypesSupported;
    Z_QueryTypeDetails **queryTypesSupported;  /* OPTIONAL */
    int num_diagnosticsSets;
    Odr_oid **diagnosticsSets;              /* OPTIONAL */
    int num_attributeSetIds;
    Odr_oid **attributeSetIds;     /* OPTIONAL */
    int num_schemas;
    Odr_oid **schemas;                      /* OPTIONAL */
    int num_recordSyntaxes;
    Odr_oid **recordSyntaxes;               /* OPTIONAL */
    int num_resourceChallenges;
    Odr_oid **resourceChallenges;           /* OPTIONAL */
    Z_AccessRestrictions *restrictedAccess;  /* OPTIONAL */
    Z_Costs *costInfo;                      /* OPTIONAL */
    int num_variantSets;
    Odr_oid **variantSets;                  /* OPTIONAL */
    int num_elementSetNames;
    char **elementSetNames;     /* OPTIONAL */
    int num_unitSystems;
    char **unitSystems;                     /* OPTIONAL */
} Z_AccessInfo;

typedef struct Z_DatabaseList
{
    int num_databases;
    Z_DatabaseName **databases;
} Z_DatabaseList;

typedef struct Z_AttributeValueList
{
    int num_attributes;
    Z_StringOrNumeric **attributes;
} Z_AttributeValueList;

typedef struct Z_AttributeOccurrence
{
    Odr_oid *attributeSet;         /* OPTIONAL */
    int *attributeType;
    Odr_null *mustBeSupplied;               /* OPTIONAL */
    enum
    {
    	Z_AttributeOcc_anyOrNone,
	Z_AttributeOcc_specific
    } which;
    union
    {
	Odr_null *anyOrNone;
	Z_AttributeValueList *specific;
    } *attributeValues;
} Z_AttributeOccurrence;

typedef struct Z_AttributeCombination
{
    int num_occurrences;
    Z_AttributeOccurrence **occurrences;
} Z_AttributeCombination;

typedef struct Z_AttributeCombinations
{
    Odr_oid *defaultAttributeSet;
    int num_legalCombinations;
    Z_AttributeCombination **legalCombinations;
} Z_AttributeCombinations;

typedef struct Z_AttributeValue
{
    Z_StringOrNumeric *value;
    Z_HumanString *description;             /* OPTIONAL */
    int num_subAttributes;
    Z_StringOrNumeric **subAttributes;      /* OPTIONAL */
    int num_superAttributes;
    Z_StringOrNumeric **superAttributes;    /* OPTIONAL */
    Odr_null *partialSupport;               /* OPTIONAL */
} Z_AttributeValue;

typedef struct Z_TargetInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * key elements
     */
    char *name;
    /*
     * non-key brief elements
     */
    Z_HumanString *recentNews;             /* OPTIONAL */
    Z_IconObject *icon;                     /* OPTIONAL */
    bool_t *namedResultSets;
    bool_t *multipleDbSearch;
    int *maxResultSets;                     /* OPTIONAL */
    int *maxResultSize;                     /* OPTIONAL */
    int *maxTerms;                          /* OPTIONAL */
    Z_IntUnit *timeoutInterval;             /* OPTIONAL */
    Z_HumanString *welcomeMessage;          /* OPTIONAL */
    /*
     * non-brief elements
     */
    Z_ContactInfo *contactInfo;             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    int num_nicknames;
    char **nicknames;
    Z_HumanString *usageRest;              /* OPTIONAL */
    Z_HumanString *paymentAddr;             /* OPTIONAL */
    Z_HumanString *hours;                   /* OPTIONAL */
    int num_dbCombinations;
    Z_DatabaseList **dbCombinations;        /* OPTIONAL */
    int num_addresses;
    Z_NetworkAddress **addresses;           /* OPTIONAL */
    Z_AccessInfo *commonAccessInfo;         /* OPTIONAL */
} Z_TargetInfo;

typedef struct Z_DatabaseInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Z_DatabaseName *name;
    /* 
     * Non-key elements.
     */
    Odr_null *explainDatabase;              /* OPTIONAL */
    int num_nicknames;
    Z_DatabaseName **nicknames;             /* OPTIONAL */
    Z_IconObject *icon;                     /* OPTIONAL */
    bool_t *userFee;
    bool_t *available;
    Z_HumanString *titleString;             /* OPTIONAL */
    /*
     * Non-brief elements.
     */
    int num_keywords;
    Z_HumanString **keywords;               /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    Z_DatabaseList *associatedDbs;          /* OPTIONAL */
    Z_DatabaseList *subDbs;                 /* OPTIONAL */
    Z_HumanString *disclaimers;             /* OPTIONAL */
    Z_HumanString *news;                    /* OPTIONAL */
    enum
    {
    	Z_Exp_RecordCount_actualNumber,
	Z_Exp_RecordCount_approxNumber
    } recordCount_which;
    int *recordCount;                       /* OPTIONAL */
    Z_HumanString *defaultOrder;            /* OPTIONAL */
    int *avRecordSize;                      /* OPTIONAL */
    int *maxRecordSize;                     /* OPTIONAL */
    Z_HumanString *hours;                   /* OPTIONAL */
    Z_HumanString *bestTime;                /* OPTIONAL */
    char *lastUpdate;          /* OPTIONAL */
    Z_IntUnit *updateInterval;              /* OPTIONAL */
    Z_HumanString *coverage;                /* OPTIONAL */
    bool_t *proprietary;                    /* OPTIONAL */
    Z_HumanString *copyrightText;           /* OPTIONAL */
    Z_HumanString *copyrightNotice;         /* OPTIONAL */
    Z_ContactInfo *producerContactInfo;     /* OPTIONAL */
    Z_ContactInfo *supplierContactInfo;     /* OPTIONAL */
    Z_ContactInfo *submissionContactInfo;   /* OPTIONAL */
    Z_AccessInfo *accessInfo;               /* OPTIONAL */
} Z_DatabaseInfo;

typedef struct Z_TagTypeMapping
{
    int *tagType;
    Odr_oid *tagSet;                        /* OPTIONAL */
    Odr_null *defaultTagType;               /* OPTIONAL */
} Z_TagTypeMapping;

typedef struct Z_PathUnit
{
    int *tagType;
    Z_StringOrNumeric *tagValue;
} Z_PathUnit;

typedef struct Z_Path
{
    int num;
    Z_PathUnit **list;
} Z_Path;

struct Z_ElementDataType;
typedef struct Z_ElementDataType Z_ElementDataType;

typedef struct Z_ElementInfo
{
    char *elementName;
    Z_Path *elementTagPath;
    Z_ElementDataType *dataType;            /* OPTIONAL */
    bool_t *required;
    bool_t *repeatable;
    Z_HumanString *description;             /* OPTIONAL */
} Z_ElementInfo;

typedef struct Z_ElementInfoList
{
    int num;
    Z_ElementInfo **list;
} Z_ElementInfoList;

struct Z_ElementDataType
{
    enum
    {
    	Z_ElementDataType_primitive,
	Z_ElementDataType_structured
    } which;
    union
    {
    	int *primitive;
#define Z_PrimitiveElement_octetString         0
#define Z_PrimitiveElement_numeric             1
#define Z_PrimitiveElement_date                2
#define Z_PrimitiveElement_external            3
#define Z_PrimitiveElement_string              4
#define Z_PrimitiveElement_trueOrFalse         5
#define Z_PrimitiveElement_oid                 6
#define Z_PrimitiveElement_intUnit             7
#define Z_PrimitiveElement_empty               8
#define Z_PrimitiveElement_noneOfTheAbove      100
	Z_ElementInfoList *structured;
    } u;
};

typedef struct Z_TagSetInfoElements
{
    char *elementName;
    int num_nicknames;
    char **nicknames;                       /* OPTIONAL */
    Z_StringOrNumeric *elementTag;
    Z_HumanString *description;             /* OPTIONAL */
    int *dataType;                          /* OPTIONAL */
    /* (value as in Z_PrimitiveElement) */
    Z_OtherInformation *otherTagInfo;       /* OPTIONAL */
} Z_TagSetInfoElements;

typedef struct Z_SchemaInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Odr_oid *schema;
    /*
     * Non-key elements
     */
    char *name;
    /*
     * Non-brief elements
     */
    Z_HumanString *description;             /* OPTIONAL */
    int num_tagTypeMapping;
    Z_TagTypeMapping **tagTypeMapping;      /* OPTIONAL */
    int num_recordStructure;
    Z_ElementInfo **recordStructure;        /* OPTIONAL */
} Z_SchemaInfo;


typedef struct Z_TagSetInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Odr_oid *tagSet;
    /*
     * Non-key elements
     */
    char *name;
    /*
     * Non-brief elements
     */
    Z_HumanString *description;             /* OPTIONAL */
    int num_elements;
    Z_TagSetInfoElements **elements;        /* OPTIONAL */
} Z_TagSetInfo;

typedef struct Z_RecordSyntaxInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Odr_oid *recordSyntax;
    /*
     * Non-key elements
     */
    char *name;
    /*
     * Non-brief elements
     */
    int num_transferSyntaxes;
    Odr_oid **transferSyntaxes;             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    char *asn1Module;                       /* OPTIONAL */
    int num_abstractStructure;
    Z_ElementInfo **abstractStructure;      /* OPTIONAL */
} Z_RecordSyntaxInfo;

typedef struct Z_AttributeDescription
{
    char *name;                             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    Z_StringOrNumeric *attributeValue;
    int num_equivalentAttributes;
    Z_StringOrNumeric **equivalentAttributes; /* OPTIONAL */
} Z_AttributeDescription;

typedef struct Z_AttributeType
{
    char *name;                             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    int *attributeType;
    int num_attributeValues;
    Z_AttributeDescription **attributeValues;
} Z_AttributeType;

typedef struct Z_AttributeSetInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Odr_oid *attributeSet;
    /*
     * Non-key elements
     */
    char *name;
    /*
     * Non-brief elements
     */
    int num_attributes;
    Z_AttributeType **attributes;           /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
} Z_AttributeSetInfo;

typedef struct Z_TermListElement
{
    char *name;
    Z_HumanString *title;                   /* OPTIONAL */
    int *searchCost;                        /* OPTIONAL */
#define Z_TermListInfo_optimized           0
#define Z_TermListInfo_normal              1
#define Z_TermListInfo_expensive           2
#define Z_TermListInfo_filter              3
    bool_t *scanable;
    int num_broader;
    char **broader;                         /* OPTIONAL */
    int num_narrower;
    char **narrower;                        /* OPTIONAL */
} Z_TermListElement;

typedef struct Z_TermListInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Z_DatabaseName *databaseName;
    /*
     * Non-key elements
     */
    int num_termLists;
    Z_TermListElement **termLists;
} Z_TermListInfo;

typedef struct Z_ExtendedServicesInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Odr_oid *type;
    /*
     * Non-key elements
     */
    char *name;                             /* OPTIONAL */
    bool_t *privateType;
    bool_t *restrictionsApply;
    bool_t *feeApply;
    bool_t *available;
    bool_t *retentionSupported;
    int *waitAction;
#define Z_ExtendedServicesInfo_waitSupported       1
#define Z_ExtendedServicesInfo_waitAlways          2
#define Z_ExtendedServicesInfo_waitNotSupported    3
#define Z_ExtendedServicesInfo_depends             4
#define Z_ExtendedServicesInfo_notSaying           5
    /*
     * Non-brief elements
     */
    Z_HumanString *description;             /* OPTIONAL */
    Z_External *specificExplain;          /* OPTIONAL */
    char *esASN;                            /* OPTIONAL */
} Z_ExtendedServicesInfo;

typedef struct Z_OmittedAttributeInterpretation
{
    Z_StringOrNumeric *defaultValue;        /* OPTIONAL */
    Z_HumanString *defaultDescription;      /* OPTIONAL */
} Z_OmittedAttributeInterpretation;

typedef struct Z_AttributeTypeDetails
{
    int *attributeType;
    Z_OmittedAttributeInterpretation *optionalType;  /* OPTIONAL */
    int num_attributeValues;
    Z_AttributeValue **attributeValues;     /* OPTIONAL */
} Z_AttributeTypeDetails;

typedef struct Z_AttributeSetDetails
{
    Odr_oid *attributeSet;
    int num_attributesByType;
    Z_AttributeTypeDetails **attributesByType;
} Z_AttributeSetDetails;

typedef struct Z_AttributeDetails
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key details
     */
    Z_DatabaseName *databaseName;
    /*
     * Non-brief elements
     */
    int num_attributesBySet;
    Z_AttributeSetDetails **attributesBySet;  /* OPTIONAL */
    Z_AttributeCombinations *attributeCombinations;  /* OPTIONAL */
} Z_AttributeDetails;

typedef struct Z_EScanInfo
{
    int *maxStepSize;                       /* OPTIONAL */
    Z_HumanString *collatingSequence;       /* OPTIONAL */
    bool_t *increasing;                     /* OPTIONAL */
} Z_EScanInfo;

typedef struct Z_TermListDetails
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    char *termListName;
    /*
     * Non-brief elements
     */
    Z_HumanString *description;             /* OPTIONAL */
    Z_AttributeCombinations *attributes;    /* OPTIONAL */
    Z_EScanInfo *scanInfo;                  /* OPTIONAL */
    int *estNumberTerms;                    /* OPTIONAL */
    int num_sampleTerms;
    Z_Term **sampleTerms;                   /* OPTIONAL */
} Z_TermListDetails;

typedef struct Z_RecordTag
{
    Z_StringOrNumeric *qualifier;           /* OPTIONAL */
    Z_StringOrNumeric *tagValue;
} Z_RecordTag;

typedef struct Z_PerElementDetails
{
    char *name;                             /* OPTIONAL */
    Z_RecordTag *recordTag;                 /* OPTIONAL */
    int num_schemaTags;
    Z_Path **schemaTags;                    /* OPTIONAL */
    int *maxSize;                           /* OPTIONAL */
    int *minSize;                           /* OPTIONAL */
    int *avgSize;                           /* OPTIONAL */
    int *fixedSize;                         /* OPTIONAL */
    bool_t *repeatable;
    bool_t *required;
    Z_HumanString *description;             /* OPTIONAL */
    Z_HumanString *contents;                /* OPTIONAL */
    Z_HumanString *billingInfo;             /* OPTIONAL */
    Z_HumanString *restrictions;            /* OPTIONAL */
    int num_alternateNames;
    char **alternateNames;                  /* OPTIONAL */
    int num_genericNames;
    char **genericNames;                    /* OPTIONAL */
    Z_AttributeCombinations *searchAccess;  /* OPTIONAL */
} Z_PerElementDetails;

typedef struct Z_ElementSetDetails
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Z_DatabaseName *databaseName;
    char *elementSetName;
    Odr_oid *recordSyntax;
    /*
     * Brief elements
     */
    Odr_oid *schema;
    /*
     * Non-brief elements
     */
    Z_HumanString *description;             /* OPTIONAL */
    int num_detailsPerElement;
    Z_PerElementDetails **detailsPerElement;  /* OPTIONAL */
} Z_ElementSetDetails;

typedef struct Z_RetrievalRecordDetails
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Z_DatabaseName *databaseName;
    Odr_oid *schema;
    Odr_oid *recordSyntax;
    /*
     * Non-brief elements
     */
    Z_HumanString *description;             /* OPTIONAL */
    int num_detailsPerElement;
    Z_PerElementDetails **detailsPerElement;  /* OPTIONAL */
} Z_RetrievalRecordDetails;

typedef struct Z_SortKeyDetailsSortType
{
    enum
    {
    	Z_SortKeyDetailsSortType_character,
    	Z_SortKeyDetailsSortType_numeric,
    	Z_SortKeyDetailsSortType_structured
    } which;
    union
    {
	Odr_null *character;
	Odr_null *numeric;
	Z_HumanString *structured;
    } u;
} Z_SortKeyDetailsSortType;

typedef struct Z_SortKeyDetails
{
    Z_HumanString *description;                        /* OPTIONAL */
    int num_elementSpecifications;
    Z_Specification **elementSpecifications;           /* OPTIONAL */
    Z_AttributeCombinations *attributeSpecifications;  /* OPTIONAL */
    Z_SortKeyDetailsSortType *sortType;                /* OPTIONAL */
    int *caseSensitivity;                              /* OPTIONAL */
#define Z_SortKeyDetails_always              0
#define Z_SortKeyDetails_never               1
#define Z_SortKeyDetails_defaultYes          2
#define Z_SortKeyDetails_defaultNo           3
} Z_SortKeyDetails;

typedef struct Z_SortDetails
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Z_DatabaseName *databaseName;
    /* 
     * Non-brief elements
     */
    int num_sortKeys;
    Z_SortKeyDetails **sortKeys;            /* OPTIONAL */
} Z_SortDetails;

typedef struct Z_ProcessingInformation
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Z_DatabaseName *databaseName;
    int *processingContext;
#define Z_ProcessingInformation_access              0
#define Z_ProcessingInformation_search              1
#define Z_ProcessingInformation_retrieval           2
#define Z_ProcessingInformation_recordPresentation  3
#define Z_ProcessingInformation_recordHandling      4
    char *name;
    Odr_oid *oid;
    /*
     * Non-brief elements
     */
    Z_HumanString *description;             /* OPTIONAL */
    Z_External *instructions;             /* OPTIONAL */
} Z_ProcessingInformation;

typedef struct Z_ValueDescription
{
    enum
    {
    	Z_ValueDescription_integer,
    	Z_ValueDescription_string,
    	Z_ValueDescription_octets,
    	Z_ValueDescription_oid,
    	Z_ValueDescription_unit,
    	Z_ValueDescription_valueAndUnit
    } which;
    union
    {
	int *integer;
	char *string;
	Odr_oct *octets;
	Odr_oid *oid;
	Z_Unit *unit;
	Z_IntUnit *valueAndUnit;
    } u;
} Z_ValueDescription;

typedef struct Z_ValueRange
{
    Z_ValueDescription *lower;              /* OPTIONAL */
    Z_ValueDescription *upper;              /* OPTIONAL */
} Z_ValueRange;

typedef struct Z_ValueSetEnumerated
{
    int num_enumerated;
    Z_ValueDescription **enumerated;
} Z_ValueSetEnumerated;

typedef struct Z_ValueSet
{
    enum
    {
    	Z_ValueSet_range,
	Z_ValueSet_enumerated
    } which;
    union
    {
	Z_ValueRange *range;
	Z_ValueSetEnumerated *enumerated;
    } u;
} Z_ValueSet;

typedef struct Z_VariantValue
{
    int *dataType;
    Z_ValueSet *values;                     /* OPTIONAL */
} Z_VariantValue;

typedef struct Z_VariantType
{
    char *name;                             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    int *variantType;
    Z_VariantValue *variantValue;           /* OPTIONAL */
} Z_VariantType;

typedef struct Z_VariantClass
{
    char *name;                             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    int *variantClass;
    int num_variantTypes;
    Z_VariantType **variantTypes;
} Z_VariantClass;

typedef struct Z_VariantSetInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    Odr_oid *variantSet;
    /*
     * Brief elements
     */
    char *name;
    /*
     * Non-brief elements
     */
    int num_variants;
    Z_VariantClass **variants;              /* OPTIONAL */
} Z_VariantSetInfo;

typedef struct Z_Units
{
    char *name;                             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    Z_StringOrNumeric *unit;
} Z_Units;

typedef struct Z_UnitType
{
    char *name;                             /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    Z_StringOrNumeric *unitType;
    int num_units;
    Z_Units **units;
} Z_UnitType;

typedef struct Z_UnitInfo
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    /*
     * Key elements
     */
    char *unitSystem;
    /*
     * Non-brief elements
     */
    Z_HumanString *description;             /* OPTIONAL */
    int num_units;
    Z_UnitType **units;                     /* OPTIONAL */
} Z_UnitInfo;

typedef struct Z_CategoryInfo
{
    char *category;
    char *originalCategory;                 /* OPTIONAL */
    Z_HumanString *description;             /* OPTIONAL */
    char *asn1Module;                       /* OPTIONAL */
} Z_CategoryInfo;

typedef struct Z_CategoryList
{
    Z_CommonInfo *commonInfo;               /* OPTIONAL */
    int num_categories;
    Z_CategoryInfo **categories;
} Z_CategoryList;

typedef struct Z_ExplainRecord
{
    enum
    {
    	Z_Explain_targetInfo,
	Z_Explain_databaseInfo,
	Z_Explain_schemaInfo,
	Z_Explain_tagSetInfo,
	Z_Explain_recordSyntaxInfo,
	Z_Explain_attributeSetInfo,
	Z_Explain_termListInfo,
	Z_Explain_extendedServicesInfo,
	Z_Explain_attributeDetails,
	Z_Explain_termListDetails,
	Z_Explain_elementSetDetails,
	Z_Explain_retrievalRecordDetails,
	Z_Explain_sortDetails,
	Z_Explain_processing,
	Z_Explain_variants,
	Z_Explain_units,
	Z_Explain_categoryList
    } which;
    union
    {
    	Z_TargetInfo *targetInfo;
	Z_DatabaseInfo *databaseInfo;
	Z_SchemaInfo *schemaInfo;
	Z_TagSetInfo *tagSetInfo;
	Z_RecordSyntaxInfo *recordSyntaxInfo;
	Z_AttributeSetInfo *attributeSetInfo;
	Z_TermListInfo *termListInfo;
	Z_ExtendedServicesInfo *extendedServicesInfo;
	Z_AttributeDetails *attributeDetails;
	Z_TermListDetails *termListDetails;
	Z_ElementSetDetails *elementSetDetails;
	Z_RetrievalRecordDetails *retrievalRecordDetails;
	Z_SortDetails *sortDetails;
	Z_ProcessingInformation *processing;
	Z_VariantSetInfo *variants;
	Z_UnitInfo *units;
	Z_CategoryList *categoryList;
    } u;
} Z_ExplainRecord;

int z_ExplainRecord(ODR o, Z_ExplainRecord **p, int opt);

#endif
