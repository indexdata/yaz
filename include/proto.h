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
 * 2. The name of Index Data or the individual authors may not be used to
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
 * $Log: proto.h,v $
 * Revision 1.39  1997-05-14 06:53:44  adam
 * C++ support.
 *
 * Revision 1.38  1997/04/30 08:52:09  quinn
 * Null
 *
 * Revision 1.37  1996/10/09  15:54:58  quinn
 * Added SearchInfoReport
 *
 * Revision 1.36  1996/10/07  15:29:20  quinn
 * Added SOIF support
 *
 * Revision 1.35  1996/06/10  08:57:38  quinn
 * Added OPAC, ResourceReport
 *
 * Revision 1.34  1996/06/10  08:55:36  quinn
 * Added Summary.
 *
 * Revision 1.33  1996/02/23  10:00:51  quinn
 * SCAN Work
 *
 * Revision 1.32  1996/02/20  12:52:39  quinn
 * Various
 *
 * Revision 1.31  1996/01/22  09:46:45  quinn
 * Added Sort PDU. Moved StringList to main protocol file.
 *
 * Revision 1.30  1996/01/10  15:21:32  quinn
 * Added links to access control PDUs
 *
 * Revision 1.29  1996/01/02  11:46:49  quinn
 * Changed 'operator' to 'roperator' to avoid C++ conflict.
 * Moved to pl3
 *
 * Revision 1.28  1996/01/02  08:57:32  quinn
 * Changed enums in the ASN.1 .h files to #defines. Changed oident.class to oclass
 *
 * Revision 1.27  1995/10/12  10:34:46  quinn
 * Added Espec-1.
 *
 * Revision 1.26  1995/09/29  17:12:08  quinn
 * Smallish
 *
 * Revision 1.25  1995/09/27  15:02:49  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.24  1995/08/29  11:17:43  quinn
 * *** empty log message ***
 *
 * Revision 1.23  1995/08/21  09:10:36  quinn
 * Smallish fixes to suppport new formats.
 *
 * Revision 1.22  1995/08/17  12:45:14  quinn
 * Fixed minor problems with GRS-1. Added support in c&s.
 *
 * Revision 1.21  1995/08/15  12:00:11  quinn
 * Updated External
 *
 * Revision 1.20  1995/08/10  08:54:35  quinn
 * Added Explain.
 *
 * Revision 1.19  1995/06/19  12:38:28  quinn
 * Reorganized include-files. Added small features.
 *
 * Revision 1.18  1995/06/16  13:16:05  quinn
 * Fixed Defaultdiagformat.
 *
 * Revision 1.17  1995/06/15  15:42:05  quinn
 * Fixed some v3 bugs
 *
 * Revision 1.16  1995/06/15  07:45:06  quinn
 * Moving to v3.
 *
 * Revision 1.15  1995/06/14  15:26:43  quinn
 * *** empty log message ***
 *
 * Revision 1.14  1995/06/07  14:42:34  quinn
 * Fixed CLOSE
 *
 * Revision 1.13  1995/06/07  14:36:47  quinn
 * Added CLOSE
 *
 * Revision 1.12  1995/06/05  10:53:13  quinn
 * Smallish.
 *
 * Revision 1.11  1995/06/02  09:49:47  quinn
 * Add access control
 *
 * Revision 1.10  1995/05/29  08:11:34  quinn
 * Moved oid from odr/asn to util.
 *
 * Revision 1.9  1995/05/22  11:31:25  quinn
 * Added PDUs
 *
 * Revision 1.8  1995/05/17  08:41:35  quinn
 * Added delete to proto & other little things.
 * Relaying auth info to backend.
 *
 * Revision 1.7  1995/05/16  08:50:37  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.6  1995/05/15  11:55:55  quinn
 * Work on asynchronous activity.
 *
 * Revision 1.5  1995/04/17  11:28:18  quinn
 * Smallish
 *
 * Revision 1.4  1995/04/10  10:22:47  quinn
 * Added SCAN
 *
 * Revision 1.3  1995/03/30  12:18:09  quinn
 * Added info.
 *
 * Revision 1.2  1995/03/30  10:26:48  quinn
 * Added Term structure
 *
 * Revision 1.1  1995/03/30  09:39:42  quinn
 * Moved .h files to include directory
 *
 * Revision 1.11  1995/03/30  09:08:44  quinn
 * Added Resource control protocol
 *
 * Revision 1.10  1995/03/29  15:39:39  quinn
 * Adding some resource control elements, and a null-check to getentbyoid
 *
 * Revision 1.9  1995/03/29  08:06:18  quinn
 * Added a few v3 elements
 *
 * Revision 1.8  1995/03/22  10:12:49  quinn
 * Added Z_PRES constants.
 *
 * Revision 1.7  1995/03/20  09:45:12  quinn
 * Working towards v3
 *
 * Revision 1.5  1995/03/07  16:29:34  quinn
 * Added authentication stuff.
 *
 * Revision 1.4  1995/03/07  10:13:00  quinn
 * Added prototype for z_APDU()
 *
 * Revision 1.3  1995/02/14  11:54:23  quinn
 * Fixing include.
 *
 * Revision 1.2  1995/02/09  15:51:40  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/06  16:44:48  quinn
 * First hack at Z/SR protocol
 *
 */

#ifndef PROTO_H
#define PROTO_H

#include <yconfig.h>
#include <odr.h>
#include <oid.h>
#include <odr_use.h>
#include <yaz-version.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Because we didn't have time to put all of the extra v3 elements in here
 * before the first applications were written, we have to place them
 * in #ifdefs in places where they would break existing code. If you are
 * developing new stuff, we urge you to leave them in, even if you don't
 * intend to use any v3 features. When we are comfortable that the old
 * apps have been updated, we'll remove the #ifdefs.
 */

#define Z_95

/* ----------------- GLOBAL AUXILIARY DEFS ----------------*/

struct Z_External;
typedef struct Z_External Z_External;

typedef Odr_oct Z_ReferenceId;
typedef char Z_DatabaseName;
typedef char Z_ResultSetId;
typedef Odr_oct Z_ResultsetId;

typedef struct Z_InfoCategory
{
    Odr_oid *categoryTypeId;         /* OPTIONAL */
    int *categoryValue;
} Z_InfoCategory;

typedef struct Z_OtherInformationUnit
{
    Z_InfoCategory *category;        /* OPTIONAL */
    int which;
#define Z_OtherInfo_characterInfo 0
#define Z_OtherInfo_binaryInfo 1
#define Z_OtherInfo_externallyDefinedInfo 2
#define Z_OtherInfo_oid 3
    union
    {
    	char *characterInfo; 
	Odr_oct *binaryInfo;
	Z_External *externallyDefinedInfo;
	Odr_oid *oid;
    } information;
} Z_OtherInformationUnit;

typedef struct Z_OtherInformation
{
    int num_elements;
    Z_OtherInformationUnit **list;
} Z_OtherInformation;

typedef struct Z_StringOrNumeric
{
    int which;
#define Z_StringOrNumeric_string 0
#define Z_StringOrNumeric_numeric 1
    union
    {
    	char *string;
	int *numeric;
    } u;
} Z_StringOrNumeric;

typedef struct Z_Unit
{
    char *unitSystem;               /* OPTIONAL */
    Z_StringOrNumeric *unitType;    /* OPTIONAL */
    Z_StringOrNumeric *unit;        /* OPTIONAL */
    int *scaleFactor;               /* OPTIONAL */
} Z_Unit;

typedef struct Z_IntUnit
{
    int *value;
    Z_Unit *unitUsed;
} Z_IntUnit;

typedef Odr_oct Z_SUTRS;

typedef struct Z_StringList
{
    int num_strings;
    char **strings;
} Z_StringList;

/* ----------------- INIT SERVICE  ----------------*/

typedef struct
{
    char *groupId;       /* OPTIONAL */
    char *userId;         /* OPTIONAL */
    char *password;      /* OPTIONAL */
} Z_IdPass;

typedef struct Z_IdAuthentication
{
    int which;
#define Z_IdAuthentication_open 0
#define Z_IdAuthentication_idPass 1
#define Z_IdAuthentication_anonymous 2
#define Z_IdAuthentication_other 3
    union
    {
    	char *open;
    	Z_IdPass *idPass;
    	Odr_null *anonymous;
    	Z_External *other;
    } u;
} Z_IdAuthentication;

#define Z_ProtocolVersion_1               0
#define Z_ProtocolVersion_2               1
#define Z_ProtocolVersion_3               2

#define Z_Options_search                  0
#define Z_Options_present                 1
#define Z_Options_delSet                  2
#define Z_Options_resourceReport          3
#define Z_Options_triggerResourceCtrl     4
#define Z_Options_resourceCtrl            5
#define Z_Options_accessCtrl              6
#define Z_Options_scan                    7
#define Z_Options_sort                    8
#define Z_Options_reserved                9
#define Z_Options_extendedServices       10
#define Z_Options_level_1Segmentation    11
#define Z_Options_level_2Segmentation    12
#define Z_Options_concurrentOperations   13
#define Z_Options_namedResultSets        14

typedef struct Z_InitRequest
{
    Z_ReferenceId *referenceId;                   /* OPTIONAL */
    Odr_bitmask *protocolVersion;
    Odr_bitmask *options;
    int *preferredMessageSize;
    int *maximumRecordSize;
    Z_IdAuthentication* idAuthentication;        /* OPTIONAL */
    char *implementationId;                      /* OPTIONAL */
    char *implementationName;                    /* OPTIONAL */
    char *implementationVersion;                 /* OPTIONAL */
    Z_External *userInformationField;          /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;               /* OPTIONAL */
#endif
} Z_InitRequest;

typedef struct Z_InitResponse
{
    Z_ReferenceId *referenceId;    /* OPTIONAL */
    Odr_bitmask *protocolVersion;
    Odr_bitmask *options;
    int *preferredMessageSize;
    int *maximumRecordSize;
    bool_t *result;
    char *implementationId;      /* OPTIONAL */
    char *implementationName;    /* OPTIONAL */
    char *implementationVersion; /* OPTIONAL */
    Z_External *userInformationField; /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;    /* OPTIONAL */
#endif
} Z_InitResponse;

typedef struct Z_NSRAuthentication
{
    char *user;
    char *password;
    char *account;
} Z_NSRAuthentication;

int z_NSRAuthentication(ODR o, Z_NSRAuthentication **p, int opt);

int z_StrAuthentication(ODR o, char **p, int opt);

/* ------------------ SEARCH SERVICE ----------------*/

typedef struct Z_DatabaseSpecificUnit
{
    char *databaseName;
    char *elementSetName;
} Z_DatabaseSpecificUnit;

typedef struct Z_DatabaseSpecific
{
    int num_elements;
    Z_DatabaseSpecificUnit **elements;
} Z_DatabaseSpecific;

typedef struct Z_ElementSetNames
{
    int which;
#define Z_ElementSetNames_generic 0
#define Z_ElementSetNames_databaseSpecific 1
    union
    {
        char *generic;
        Z_DatabaseSpecific *databaseSpecific;
    } u;
} Z_ElementSetNames;

/* ---------------------- RPN QUERY --------------------------- */

typedef struct Z_ComplexAttribute
{
    int num_list;
    Z_StringOrNumeric **list;
    int num_semanticAction;
    int **semanticAction;           /* OPTIONAL */
} Z_ComplexAttribute;

typedef struct Z_AttributeElement
{
#ifdef Z_95
    Odr_oid *attributeSet;           /* OPTIONAL - v3 only */
#endif
    int *attributeType;
#ifdef Z_95
    int which;
#define Z_AttributeValue_numeric 0
#define Z_AttributeValue_complex 1
    union
    {
    	int *numeric;
	Z_ComplexAttribute *complex;
    } value;
#else
    int *attributeValue;
#endif
} Z_AttributeElement;

typedef struct Z_Term 
{
    int which;
#define Z_Term_general 0
#define Z_Term_numeric 1
#define Z_Term_characterString 2
#define Z_Term_oid 3
#define Z_Term_dateTime 4
#define Z_Term_external 5
#define Z_Term_integerAndUnit 6
#define Z_Term_null 7
    union
    {
    	Odr_oct *general; /* this is required for v2 */
    	int *numeric;
    	char *characterString;
    	Odr_oid *oid;
    	char *dateTime;
    	Z_External *external;
    	/* Z_IntUnit *integerAndUnit; */
    	Odr_null *null;
    } u;
} Z_Term;

typedef struct Z_AttributesPlusTerm
{
    int num_attributes;
    Z_AttributeElement **attributeList;
    Z_Term *term;
} Z_AttributesPlusTerm;

typedef struct Z_ResultSetPlusAttributes
{
    char *resultSet;
    int num_attributes;
    Z_AttributeElement **attributeList;
} Z_ResultSetPlusAttributes;

typedef struct Z_ProximityOperator
{
    bool_t *exclusion;          /* OPTIONAL */
    int *distance;
    bool_t *ordered;
    int *relationType;
#define Z_Prox_lessThan           1
#define Z_Prox_lessThanOrEqual    2
#define Z_Prox_equal              3
#define Z_Prox_greaterThanOrEqual 4
#define Z_Prox_greaterThan        5
#define Z_Prox_notEqual           6
    int which;
#define Z_ProxCode_known 0
#define Z_ProxCode_private 1
    int *proximityUnitCode;
#define Z_ProxUnit_character       1
#define Z_ProxUnit_word            2
#define Z_ProxUnit_sentence        3
#define Z_ProxUnit_paragraph       4
#define Z_ProxUnit_section         5
#define Z_ProxUnit_chapter         6
#define Z_ProxUnit_document        7
#define Z_ProxUnit_element         8
#define Z_ProxUnit_subelement      9
#define Z_ProxUnit_elementType    10
#define Z_ProxUnit_byte           11   /* v3 only */
} Z_ProximityOperator;

typedef struct Z_Operator
{
    int which;
#define Z_Operator_and 0
#define Z_Operator_or 1
#define Z_Operator_and_not 2
#define Z_Operator_prox 3
    union
    {
    	Odr_null *and;          /* these guys are nulls. */
    	Odr_null *or;
    	Odr_null *and_not;
	Z_ProximityOperator *prox;
    } u;
} Z_Operator;

typedef struct Z_Operand
{
    int which;
#define Z_Operand_APT 0
#define Z_Operand_resultSetId 1
#define Z_Operand_resultAttr             /* v3 only */ 2
    union
    {
    	Z_AttributesPlusTerm *attributesPlusTerm;
    	Z_ResultSetId *resultSetId;
	Z_ResultSetPlusAttributes *resultAttr;
    } u;
} Z_Operand;

typedef struct Z_Complex
{
    struct Z_RPNStructure *s1;
    struct Z_RPNStructure *s2;
    Z_Operator *roperator;
} Z_Complex;

typedef struct Z_RPNStructure
{
    int which;
#define Z_RPNStructure_simple 0
#define Z_RPNStructure_complex 1
    union
    {
    	Z_Operand *simple;
    	Z_Complex *complex;
    } u;
} Z_RPNStructure;

typedef struct Z_RPNQuery
{
    Odr_oid *attributeSetId;
    Z_RPNStructure *RPNStructure;
} Z_RPNQuery;

/* -------------------------- SEARCHREQUEST -------------------------- */

typedef struct Z_Query
{
    int which;
#define Z_Query_type_1 1
#define Z_Query_type_2 2
#define Z_Query_type_101 3
    union
    {
    	Z_RPNQuery *type_1;
    	Odr_oct *type_2;
	Z_RPNQuery *type_101;
    } u;
} Z_Query;

typedef struct Z_SearchRequest
{
    Z_ReferenceId *referenceId;   /* OPTIONAL */
    int *smallSetUpperBound;
    int *largeSetLowerBound;
    int *mediumSetPresentNumber;
    bool_t *replaceIndicator;
    char *resultSetName;
    int num_databaseNames;
    char **databaseNames;
    Z_ElementSetNames *smallSetElementSetNames;    /* OPTIONAL */
    Z_ElementSetNames *mediumSetElementSetNames;    /* OPTIONAL */
    Odr_oid *preferredRecordSyntax;  /* OPTIONAL */
    Z_Query *query;
#ifdef Z_95
    Z_OtherInformation *additionalSearchInfo;       /* OPTIONAL */
    Z_OtherInformation *otherInfo;                  /* OPTIONAL */
#endif
} Z_SearchRequest;

/* ------------------------ RECORD -------------------------- */

typedef Z_External Z_DatabaseRecord;

#ifdef Z_95

typedef struct Z_DefaultDiagFormat
{
    Odr_oid *diagnosticSetId; /* This is opt'l to interwork with bad targets */
    int *condition;
    /* until the whole character set issue becomes more definite,
     * you can probably ignore this on input. */
    int which;
#define Z_DiagForm_v2AddInfo 0
#define Z_DiagForm_v3AddInfo 1
    char *addinfo;
} Z_DefaultDiagFormat;

typedef struct Z_DiagRec
{
    int which;
#define Z_DiagRec_defaultFormat 0
#define Z_DiagRec_externallyDefined 1
    union
    {
    	Z_DefaultDiagFormat *defaultFormat;
	Z_External *externallyDefined;
    } u;
} Z_DiagRec;

#else

typedef struct Z_DiagRec
{
    Odr_oid *diagnosticSetId; /* This is opt'l to interwork with bad targets */
    int *condition;
    char *addinfo;
} Z_DiagRec;

#endif

typedef struct Z_DiagRecs
{
    int num_diagRecs;
    Z_DiagRec **diagRecs;
} Z_DiagRecs;

typedef struct Z_NamePlusRecord
{
    char *databaseName;      /* OPTIONAL */
    int which;
#define Z_NamePlusRecord_databaseRecord 0
#define Z_NamePlusRecord_surrogateDiagnostic 1
    union
    {
    	Z_DatabaseRecord *databaseRecord;
    	Z_DiagRec *surrogateDiagnostic;
    } u;
} Z_NamePlusRecord;

typedef struct Z_NamePlusRecordList
{
    int num_records;
    Z_NamePlusRecord **records;
} Z_NamePlusRecordList;

typedef struct Z_Records
{
    int which;
#define Z_Records_DBOSD 0
#define Z_Records_NSD 1
#define Z_Records_multipleNSD 2
    union
    {
    	Z_NamePlusRecordList *databaseOrSurDiagnostics;
    	Z_DiagRec *nonSurrogateDiagnostic;
	Z_DiagRecs *multipleNonSurDiagnostics;
    } u;
} Z_Records;

/* ------------------------ SEARCHRESPONSE ------------------ */

typedef struct Z_SearchResponse
{
    Z_ReferenceId *referenceId;       /* OPTIONAL */
    int *resultCount;
    int *numberOfRecordsReturned;
    int *nextResultSetPosition;
    bool_t *searchStatus;
    int *resultSetStatus;              /* OPTIONAL */
#define Z_RES_SUBSET        1
#define Z_RES_INTERIM       2
#define Z_RES_NONE          3
    int *presentStatus;                /* OPTIONAL */
#define Z_PRES_SUCCESS      0
#define Z_PRES_PARTIAL_1    1
#define Z_PRES_PARTIAL_2    2
#define Z_PRES_PARTIAL_3    3
#define Z_PRES_PARTIAL_4    4
#define Z_PRES_FAILURE      5
    Z_Records *records;                  /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *additionalSearchInfo;
    Z_OtherInformation *otherInfo;
#endif
} Z_SearchResponse;

/* ------------------------- PRESENT SERVICE -----------------*/

typedef struct Z_ElementSpec
{
    int which;
#define Z_ElementSpec_elementSetName 0
#define Z_ElementSpec_externalSpec 1
    union
    {
    	char *elementSetName;
	Z_External *externalSpec;
    } u;
} Z_ElementSpec;

typedef struct Z_Specification
{
    Odr_oid *schema;                  /* OPTIONAL */
    Z_ElementSpec *elementSpec;       /* OPTIONAL */
} Z_Specification;

typedef struct Z_DbSpecific
{
    char *databaseName;
    Z_Specification *spec;
} Z_DbSpecific;

typedef struct Z_CompSpec
{
    bool_t *selectAlternativeSyntax;
    Z_Specification *generic;            /* OPTIONAL */
    int num_dbSpecific;
    Z_DbSpecific **dbSpecific;           /* OPTIONAL */
    int num_recordSyntax;
    Odr_oid **recordSyntax;              /* OPTIONAL */
} Z_CompSpec;

typedef struct Z_RecordComposition
{
    int which;
#define Z_RecordComp_simple 0
#define Z_RecordComp_complex 1
    union
    {
    	Z_ElementSetNames *simple;
	Z_CompSpec *complex;
    } u;
} Z_RecordComposition;

typedef struct Z_Range
{
    int *startingPosition;
    int *numberOfRecords;
} Z_Range;

typedef struct Z_PresentRequest
{
    Z_ReferenceId *referenceId;              /* OPTIONAL */
    Z_ResultSetId *resultSetId;
    int *resultSetStartPoint;
    int *numberOfRecordsRequested;
#ifdef Z_95
    int num_ranges;
    Z_Range **additionalRanges;              /* OPTIONAL */
    Z_RecordComposition *recordComposition;  /* OPTIONAL */
#else
    Z_ElementSetNames *elementSetNames;  /* OPTIONAL */
#endif
    Odr_oid *preferredRecordSyntax;  /* OPTIONAL */
#ifdef Z_95
    int *maxSegmentCount;                 /* OPTIONAL */
    int *maxRecordSize;                   /* OPTIONAL */
    int *maxSegmentSize;                  /* OPTIONAL */
    Z_OtherInformation *otherInfo;        /* OPTIONAL */
#endif
} Z_PresentRequest;

typedef struct Z_PresentResponse
{
    Z_ReferenceId *referenceId;        /* OPTIONAL */
    int *numberOfRecordsReturned;
    int *nextResultSetPosition;
    int *presentStatus;
    Z_Records *records;
#ifdef Z_95
    Z_OtherInformation *otherInfo;     /* OPTIONAL */
#endif
} Z_PresentResponse;

/* ------------------ RESOURCE CONTROL ----------------*/

typedef struct Z_TriggerResourceControlRequest
{
    Z_ReferenceId *referenceId;    /* OPTIONAL */
    int *requestedAction;
#define Z_TriggerResourceCtrl_resourceReport  1
#define Z_TriggerResourceCtrl_resourceControl 2
#define Z_TriggerResourceCtrl_cancel          3
    Odr_oid *prefResourceReportFormat;  /* OPTIONAL */
    bool_t *resultSetWanted;            /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;
#endif
} Z_TriggerResourceControlRequest;

typedef struct Z_ResourceControlRequest
{
    Z_ReferenceId *referenceId;    /* OPTIONAL */
    bool_t *suspendedFlag;         /* OPTIONAL */
    Z_External *resourceReport; /* OPTIONAL */
    int *partialResultsAvailable;  /* OPTIONAL */
#define Z_ResourceControlRequest_subset    1
#define Z_ResourceControlRequest_interim   2
#define Z_ResourceControlRequest_none      3
    bool_t *responseRequired;
    bool_t *triggeredRequestFlag;  /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;
#endif
} Z_ResourceControlRequest;

typedef struct Z_ResourceControlResponse
{
    Z_ReferenceId *referenceId;    /* OPTIONAL */
    bool_t *continueFlag;
    bool_t *resultSetWanted;       /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;
#endif
} Z_ResourceControlResponse;


/* ------------------ ACCESS CTRL SERVICE ----------------*/

typedef struct Z_AccessControlRequest
{
    Z_ReferenceId *referenceId;           /* OPTIONAL */
    int which;
#define Z_AccessRequest_simpleForm 0
#define Z_AccessRequest_externallyDefined 1
    union
    {
    	Odr_oct *simpleForm;
	Z_External *externallyDefined;
    } u;
#ifdef Z_95
    Z_OtherInformation *otherInfo;           /* OPTIONAL */
#endif
} Z_AccessControlRequest;

typedef struct Z_AccessControlResponse
{
    Z_ReferenceId *referenceId;              /* OPTIONAL */
    int which;
#define Z_AccessResponse_simpleForm 0
#define Z_AccessResponse_externallyDefined 1
    union
    {
    	Odr_oct *simpleForm;
	Z_External *externallyDefined;
    } u;
    Z_DiagRec *diagnostic;                   /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;           /* OPTIONAL */
#endif
} Z_AccessControlResponse;

/* ------------------------ SCAN SERVICE -------------------- */

typedef struct Z_AttributeList
{
    int num_attributes;
    Z_AttributeElement **attributes;
} Z_AttributeList;

typedef struct Z_AlternativeTerm
{
    int num_terms;
    Z_AttributesPlusTerm **terms;
} Z_AlternativeTerm;

#if 1

typedef struct Z_ByDatabase
{
    char *db;
    int *num;                           /* OPTIONAL */
    Z_OtherInformation *otherDbInfo;    /* OPTIONAL */
} Z_ByDatabase;

typedef struct Z_ByDatabaseList
{
    int num_elements;
    Z_ByDatabase **elements;
} Z_ByDatabaseList;

typedef struct Z_ScanOccurrences
{
    int which;
#define Z_ScanOccurrences_global         0
#define Z_ScanOccurrences_byDatabase     1
    union
    {
	int *global;
	Z_ByDatabaseList *byDatabase;
    } u;

} Z_ScanOccurrences;

typedef struct Z_OccurrenceByAttributesElem
{
    Z_AttributeList *attributes;
    Z_ScanOccurrences *occurrences;         /* OPTIONAL */
    Z_OtherInformation *otherOccurInfo;      /* OPTIONAL */
} Z_OccurrenceByAttributesElem;

typedef struct Z_OccurrenceByAttributes
{
    int num_elements;
    Z_OccurrenceByAttributesElem **elements;
} Z_OccurrenceByAttributes;

#else

/*
 * Old, incomplete definition of OccurrenceByAttributes.
 */

typedef struct Z_OccurrenceByAttributes
{
    Z_AttributeList *attributes;
#if 0
    int which;
#define Z_OByAtt_global 0
#define Z_ObyAtt_byDatabase 1
    union
    {
#endif
    	int *global;
#if 0
	/* Z_ByDatabase *byDatabase; */
    } u;
#endif
} Z_OccurrenceByAttributes;

#endif

typedef struct Z_TermInfo
{
    Z_Term *term;
    char *displayTerm;                     /* OPTIONAL */
    Z_AttributeList *suggestedAttributes;  /* OPTIONAL */
    Z_AlternativeTerm *alternativeTerm;    /* OPTIONAL */
    int *globalOccurrences;                /* OPTIONAL */
    Z_OccurrenceByAttributes *byAttributes; /* OPTIONAL */
    Z_OtherInformation *otherTermInfo;      /* OPTIONAL */
} Z_TermInfo;

typedef struct Z_Entry
{
    int which;
#define Z_Entry_termInfo 0
#define Z_Entry_surrogateDiagnostic 1
    union
    {
    	Z_TermInfo *termInfo;
	Z_DiagRec *surrogateDiagnostic;
    } u;
} Z_Entry;

typedef struct Z_Entries
{
    int num_entries;
    Z_Entry **entries;
} Z_Entries;

typedef struct Z_ListEntries
{
    int which;
#define Z_ListEntries_entries 0
#define Z_ListEntries_nonSurrogateDiagnostics 1
    union
    {
    	Z_Entries *entries;
	Z_DiagRecs *nonSurrogateDiagnostics;
    } u;
} Z_ListEntries;

typedef struct Z_ScanRequest
{
    Z_ReferenceId *referenceId;       /* OPTIONAL */
    int num_databaseNames;
    char **databaseNames;
    Odr_oid *attributeSet;          /* OPTIONAL */
    Z_AttributesPlusTerm *termListAndStartPoint;
    int *stepSize;                    /* OPTIONAL */
    int *numberOfTermsRequested;
    int *preferredPositionInResponse;   /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;
#endif
} Z_ScanRequest;

typedef struct Z_ScanResponse
{
    Z_ReferenceId *referenceId;       /* OPTIONAL */
    int *stepSize;                    /* OPTIONAL */
    int *scanStatus;
#define Z_Scan_success      0
#define Z_Scan_partial_1    1
#define Z_Scan_partial_2    2
#define Z_Scan_partial_3    3
#define Z_Scan_partial_4    4
#define Z_Scan_partial_5    5
#define Z_Scan_failure      6
    int *numberOfEntriesReturned;
    int *positionOfTerm;              /* OPTIONAL */
    Z_ListEntries *entries;           /* OPTIONAL */
    Odr_oid *attributeSet;            /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;
#endif
} Z_ScanResponse; 


/* ------------------------ DELETE -------------------------- */

#define Z_DeleteStatus_success                          0
#define Z_DeleteStatus_resultSetDidNotExist             1
#define Z_DeleteStatus_previouslyDeletedByTarget        2
#define Z_DeleteStatus_systemProblemAtTarget            3
#define Z_DeleteStatus_accessNotAllowed                 4
#define Z_DeleteStatus_resourceControlAtOrigin          5
#define Z_DeleteStatus_resourceControlAtTarget          6
#define Z_DeleteStatus_bulkDeleteNotSupported           7
#define Z_DeleteStatus_notAllRsltSetsDeletedOnBulkDlte  8
#define Z_DeleteStatus_notAllRequestedResultSetsDeleted 9
#define Z_DeleteStatus_resultSetInUse                  10

typedef struct Z_ListStatus
{
    Z_ResultSetId *id;
    int *status;
} Z_ListStatus;

typedef struct Z_DeleteResultSetRequest
{
    Z_ReferenceId *referenceId;        /* OPTIONAL */
    int *deleteFunction;
#define Z_DeleteRequest_list    0
#define Z_DeleteRequest_all     1
    int num_ids;
    Z_ResultSetId **resultSetList;      /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;
#endif
} Z_DeleteResultSetRequest;

typedef struct Z_DeleteResultSetResponse
{
    Z_ReferenceId *referenceId;        /* OPTIONAL */
    int *deleteOperationStatus;
    int num_statuses;
    Z_ListStatus **deleteListStatuses; /* OPTIONAL */
    int *numberNotDeleted;             /* OPTIONAL */
    int num_bulkStatuses;
    Z_ListStatus **bulkStatuses;       /* OPTIONAL */
    char *deleteMessage;               /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;
#endif
} Z_DeleteResultSetResponse;

/* ------------------------ CLOSE SERVICE ------------------- */

typedef struct Z_Close
{
    Z_ReferenceId *referenceId;         /* OPTIONAL */
    int *closeReason;
#define Z_Close_finished           0
#define Z_Close_shutdown           1
#define Z_Close_systemProblem      2
#define Z_Close_costLimit          3
#define Z_Close_resources          4
#define Z_Close_securityViolation  5
#define Z_Close_protocolError      6
#define Z_Close_lackOfActivity     7
#define Z_Close_peerAbort          8
#define Z_Close_unspecified        9
    char *diagnosticInformation;          /* OPTIONAL */
    Odr_oid *resourceReportFormat;        /* OPTIONAL */
    Z_External *resourceReport;         /* OPTIONAL */
#ifdef Z_95
    Z_OtherInformation *otherInfo;        /* OPTIONAL */
#endif
} Z_Close;

/* ------------------------ SEGMENTATION -------------------- */

typedef struct Z_Segment
{
    Z_ReferenceId *referenceId;   /* OPTIONAL */
    int *numberOfRecordsReturned;
    int num_segmentRecords;
    Z_NamePlusRecord **segmentRecords;
    Z_OtherInformation *otherInfo;  /* OPTIONAL */
} Z_Segment;

/* ----------------------- Extended Services ---------------- */

typedef struct Z_Permissions
{
    char *userId;                         
    int num_allowableFunctions;
    int **allowableFunctions;             
#define Z_Permissions_delete              1
#define Z_Permissions_modifyContents      2
#define Z_Permissions_modifyPermissions   3
#define Z_Permissions_present             4
#define Z_Permissions_invoke              5
} Z_Permissions;

typedef struct Z_ExtendedServicesRequest
{
    Z_ReferenceId *referenceId;             /* OPTIONAL */
    int *function;                        
#define Z_ExtendedServicesRequest_create              1
#define Z_ExtendedServicesRequest_delete              2
#define Z_ExtendedServicesRequest_modify              3
    Odr_oid *packageType;                 
    char *packageName;                      /* OPTIONAL */
    char *userId;                           /* OPTIONAL */
    Z_IntUnit *retentionTime;               /* OPTIONAL */
    Z_Permissions *permissions;             /* OPTIONAL */
    char *description;                      /* OPTIONAL */
    Z_External *taskSpecificParameters;     /* OPTIONAL */
    int *waitAction;                      
#define Z_ExtendedServicesRequest_wait                1
#define Z_ExtendedServicesRequest_waitIfPossible      2
#define Z_ExtendedServicesRequest_dontWait            3
#define Z_ExtendedServicesRequest_dontReturnPackage   4
    char *elements;             /* OPTIONAL */
    Z_OtherInformation *otherInfo;          /* OPTIONAL */
} Z_ExtendedServicesRequest;

typedef struct Z_ExtendedServicesResponse
{
    Z_ReferenceId *referenceId;             /* OPTIONAL */
    int *operationStatus;                 
#define Z_ExtendedServicesResponse_done                1
#define Z_ExtendedServicesResponse_accepted            2
#define Z_ExtendedServicesResponse_failure             3
    int num_diagnostics;
    Z_DiagRec **diagnostics;                /* OPTIONAL */
    Z_External *taskPackage;                /* OPTIONAL */
    Z_OtherInformation *otherInfo;          /* OPTIONAL */
} Z_ExtendedServicesResponse;

/* ------------------------ Sort --------------------------- */

typedef struct Z_SortAttributes
{
    Odr_oid *id;
    Z_AttributeList *list;
} Z_SortAttributes;

typedef struct Z_SortKey
{
    int which;
#define Z_SortKey_sortField             0
#define Z_SortKey_elementSpec           1
#define Z_SortKey_sortAttributes        2
    union
    {
	char *sortField;
	Z_Specification *elementSpec;
	Z_SortAttributes *sortAttributes;
    } u;
} Z_SortKey;

typedef struct Z_SortDbSpecific
{
    char *databaseName;
    Z_SortKey *dbSort;
} Z_SortDbSpecific;

typedef struct Z_SortDbSpecificList
{
    int num_dbSpecific;
    Z_SortDbSpecific **dbSpecific;
} Z_SortDbSpecificList;

typedef struct Z_SortElement
{
    int which;
#define Z_SortElement_generic               0
#define Z_SortElement_databaseSpecific      1
    union
    {
	Z_SortKey *generic;
	Z_SortDbSpecificList *databaseSpecific;
    } u;
} Z_SortElement;

typedef struct Z_SortMissingValueAction
{
    int which;
#define Z_SortMissingValAct_abort           0
#define Z_SortMissingValAct_null            1
#define Z_SortMissingValAct_valData         2
    union
    {
	Odr_null *abort;
	Odr_null *null;
	Odr_oct *valData;
    } u;
} Z_SortMissingValueAction;

typedef struct Z_SortKeySpec
{
    Z_SortElement *sortElement;
    int *sortRelation;
#define Z_SortRelation_ascending            0
#define Z_SortRelation_descending           1
#define Z_SortRelation_ascendingByFreq      3
#define Z_SortRelation_descendingByFreq     4
    int *caseSensitivity;
#define Z_SortCase_caseSensitive            0
#define Z_SortCase_caseInsensitive          1
    Z_SortMissingValueAction *missingValueAction;  /* OPTIONAL */
} Z_SortKeySpec;

typedef struct Z_SortResponse
{
    Z_ReferenceId *referenceId;             /* OPTIONAL */
    int *sortStatus;
#define Z_SortStatus_success              0
#define Z_SortStatus_partial_1            1
#define Z_SortStatus_failure              2
    int *resultSetStatus;                   /* OPTIONAL */
#define Z_SortResultSetStatus_empty       1
#define Z_SortResultSetStatus_interim     2
#define Z_SortResultSetStatus_unchanged   3
#define Z_SortResultSetStatus_none        4
    Z_DiagRecs *diagnostics;                /* OPTIONAL */
    Z_OtherInformation *otherInfo;          /* OPTIONAL */
} Z_SortResponse;

typedef struct Z_SortKeySpecList
{
    int num_specs;
    Z_SortKeySpec **specs;
} Z_SortKeySpecList;

typedef struct Z_SortRequest
{
    Z_ReferenceId *referenceId;             /* OPTIONAL */
    Z_StringList *inputResultSetNames;
    char *sortedResultSetName;
    Z_SortKeySpecList *sortSequence;
    Z_OtherInformation *otherInfo;          /* OPTIONAL */
} Z_SortRequest;

/* ----------------------- Resource Report ------------------ */

typedef struct Z_ResourceReportRequest
{
    Z_ReferenceId *referenceId;             /* OPTIONAL */
    Z_ReferenceId *opId;                    /* OPTIONAL */
    Odr_oid *prefResourceReportFormat;      /* OPTIONAL */
    Z_OtherInformation *otherInfo;          /* OPTIONAL */
} Z_ResourceReportRequest;

typedef struct Z_ResourceReportResponse
{
    Z_ReferenceId *referenceId;             /* OPTIONAL */
    int *resourceReportStatus;
#define Z_ResourceReportStatus_success   0
#define Z_ResourceReportStatus_partial   1
#define Z_ResourceReportStatus_failure_1 2
#define Z_ResourceReportStatus_failure_2 3
#define Z_ResourceReportStatus_failure_3 4
#define Z_ResourceReportStatus_failure_4 5
#define Z_ResourceReportStatus_failure_5 6
#define Z_ResourceReportStatus_failure_6 7
    Z_External *resourceReport;             /* OPTIONAL */
    Z_OtherInformation *otherInfo;          /* OPTIONAL */
} Z_ResourceReportResponse;

/* ------------------------ APDU ---------------------------- */

typedef struct Z_APDU
{    
    int which;
#define Z_APDU_initRequest 0
#define Z_APDU_initResponse 1
#define Z_APDU_searchRequest 2
#define Z_APDU_searchResponse 3
#define Z_APDU_presentRequest 4
#define Z_APDU_presentResponse 5
#define Z_APDU_deleteResultSetRequest 6
#define Z_APDU_deleteResultSetResponse 7
#define Z_APDU_resourceControlRequest 8
#define Z_APDU_resourceControlResponse 9
#define Z_APDU_triggerResourceControlRequest 10
#define Z_APDU_scanRequest 11
#define Z_APDU_scanResponse 12
#define Z_APDU_segmentRequest 13
#define Z_APDU_extendedServicesRequest 14
#define Z_APDU_extendedServicesResponse 15
#define Z_APDU_close 16
#define Z_APDU_accessControlRequest 17
#define Z_APDU_accessControlResponse 18
#define Z_APDU_sortRequest 20
#define Z_APDU_sortResponse 21
#define Z_APDU_resourceReportRequest 22
#define Z_APDU_resourceReportResponse 23
    union
    {
	Z_InitRequest  *initRequest;
    	Z_InitResponse *initResponse;
    	Z_SearchRequest *searchRequest;
    	Z_SearchResponse *searchResponse;
    	Z_PresentRequest *presentRequest;
    	Z_PresentResponse *presentResponse;
	Z_DeleteResultSetRequest *deleteResultSetRequest;
	Z_DeleteResultSetResponse *deleteResultSetResponse;
	Z_AccessControlRequest *accessControlRequest;
	Z_AccessControlResponse *accessControlResponse;
	Z_ResourceControlRequest *resourceControlRequest;
	Z_ResourceControlResponse *resourceControlResponse;
	Z_TriggerResourceControlRequest *triggerResourceControlRequest;
	Z_ResourceReportRequest *resourceReportRequest;
	Z_ResourceReportResponse *resourceReportResponse;
	Z_ScanRequest *scanRequest;
	Z_ScanResponse *scanResponse;
	Z_SortRequest *sortRequest;
	Z_SortResponse *sortResponse;
	Z_Segment *segmentRequest;
	Z_ExtendedServicesRequest *extendedServicesRequest;
	Z_ExtendedServicesResponse *extendedServicesResponse;
	Z_Close *close;
    } u;
} Z_APDU;

int z_APDU(ODR o, Z_APDU **p, int opt);
int z_SUTRS(ODR o, Odr_oct **p, int opt);

Z_InitRequest *zget_InitRequest(ODR o);
Z_InitResponse *zget_InitResponse(ODR o);
Z_SearchRequest *zget_SearchRequest(ODR o);
Z_SearchResponse *zget_SearchResponse(ODR o);
Z_PresentRequest *zget_PresentRequest(ODR o);
Z_PresentResponse *zget_PresentResponse(ODR o);
Z_DeleteResultSetRequest *zget_DeleteResultSetRequest(ODR o);
Z_DeleteResultSetResponse *zget_DeleteResultSetResponse(ODR o);
Z_ScanRequest *zget_ScanRequest(ODR o);
Z_ScanResponse *zget_ScanResponse(ODR o);
Z_TriggerResourceControlRequest *zget_TriggerResourceControlRequest(ODR o);
Z_ResourceControlRequest *zget_ResourceControlRequest(ODR o);
Z_ResourceControlResponse *zget_ResourceControlResponse(ODR o);
Z_Close *zget_Close(ODR o);
int z_StringList(ODR o, Z_StringList **p, int opt);
int z_InternationalString(ODR o, char **p, int opt);
int z_OtherInformation(ODR o, Z_OtherInformation **p, int opt);
int z_ElementSetName(ODR o, char **p, int opt);
int z_IntUnit(ODR o, Z_IntUnit **p, int opt);
int z_Unit(ODR o, Z_Unit **p, int opt);
int z_DatabaseName(ODR o, Z_DatabaseName **p, int opt);
int z_StringOrNumeric(ODR o, Z_StringOrNumeric **p, int opt);
int z_OtherInformationUnit(ODR o, Z_OtherInformationUnit **p, int opt);
int z_Term(ODR o, Z_Term **p, int opt);
int z_Specification(ODR o, Z_Specification **p, int opt);
int z_Permissions(ODR o, Z_Permissions **p, int opt);
int z_DiagRec(ODR o, Z_DiagRec **p, int opt);
int z_AttributeList(ODR o, Z_AttributeList **p, int opt);
int z_DefaultDiagFormat(ODR o, Z_DefaultDiagFormat **p, int opt);
Z_APDU *zget_APDU(ODR o, int which);
int z_Query(ODR o, Z_Query **p, int opt);

#ifdef __cplusplus
}
#endif

#include <prt-rsc.h>
#include <prt-acc.h>
#include <prt-exp.h>
#include <prt-grs.h>
#include <prt-arc.h>
#include <prt-exd.h>
#include <prt-dia.h>
#include <prt-esp.h>
#include <prt-add.h>

#include <prt-ext.h>

#endif
