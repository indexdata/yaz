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
 * Revision 1.20  1995-08-10 08:54:35  quinn
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

#include <odr.h>
#include <oid.h>
#include <odr_use.h>
#include <yaz-version.h>

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
    enum
    {
    	Z_OtherInfo_characterInfo,
	Z_OtherInfo_binaryInfo,
	Z_OtherInfo_externallyDefinedInfo,
	Z_OtherInfo_oid
    } which;
    union
    {
    	char *characterInfo; 
	Odr_oct *binaryInfo;
	Odr_external *externallyDefinedInfo;
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
    enum
    {
    	Z_StringOrNumeric_string,
	Z_StringOrNumeric_numeric
    } which;
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

/* ----------------- INIT SERVICE  ----------------*/

typedef struct
{
    char *groupId;       /* OPTIONAL */
    char *userId;         /* OPTIONAL */
    char *password;      /* OPTIONAL */
} Z_IdPass;

typedef struct Z_IdAuthentication
{
    enum
    {
    	Z_IdAuthentication_open,
	Z_IdAuthentication_idPass,
	Z_IdAuthentication_anonymous,
	Z_IdAuthentication_other
    } which;
    union
    {
    	char *open;
    	Z_IdPass *idPass;
    	Odr_null *anonymous;
    	Odr_external *other;
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
    Odr_external *userInformationField;          /* OPTIONAL */
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
    Odr_external *userInformationField; /* OPTIONAL */
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
    enum
    {
    	Z_AttributeValue_numeric,
	Z_AttributeValue_complex
    } which;
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
    enum
    {
	Z_Term_general,
	Z_Term_numeric,
	Z_Term_characterString,
	Z_Term_oid,
	Z_Term_dateTime,
	Z_Term_external,
	Z_Term_integerAndUnit,
	Z_Term_null
    } which;
    union
    {
    	Odr_oct *general; /* this is required for v2 */
    	int *numeric;
    	char *characterString;
    	Odr_oid *oid;
    	char *dateTime;
    	Odr_external *external;
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
    enum
    {
	Z_ProxCode_known,
	Z_ProxCode_private
    } which;
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
    enum
    {
	Z_Operator_and,
	Z_Operator_or,
	Z_Operator_and_not,
	Z_Operator_prox
    } which;
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
    enum
    {
	Z_Operand_APT,
	Z_Operand_resultSetId,
	Z_Operand_resultAttr             /* v3 only */
    } which;
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
    Z_Operator *operator;
} Z_Complex;

typedef struct Z_RPNStructure
{
    enum
    {
	Z_RPNStructure_simple,
	Z_RPNStructure_complex
    } which;
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
    enum
    {
	Z_Query_type_1 = 1,
	Z_Query_type_2,
	Z_Query_type_101
    }
    which;
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

typedef Odr_external Z_DatabaseRecord;

#ifdef Z_95

typedef struct Z_DefaultDiagFormat
{
    Odr_oid *diagnosticSetId; /* This is opt'l to interwork with bad targets */
    int *condition;
    /* until the whole character set issue becomes more definite,
     * you can probably ignore this on input. */
    enum  
    {
    	Z_DiagForm_v2AddInfo,
	Z_DiagForm_v3AddInfo
    } which;
    char *addinfo;
} Z_DefaultDiagFormat;

typedef struct Z_DiagRec
{
    enum
    {	
    	Z_DiagRec_defaultFormat,
	Z_DiagRec_externallyDefined
    } which;
    union
    {
    	Z_DefaultDiagFormat *defaultFormat;
	Odr_external *externallyDefined;
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
    enum
    {
	Z_NamePlusRecord_databaseRecord,
	Z_NamePlusRecord_surrogateDiagnostic
    }
    which;
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
    enum
    {
	Z_Records_DBOSD,
	Z_Records_NSD,
	Z_Records_multipleNSD
    } which;
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
    enum
    {
    	Z_ElementSpec_elementSetName,
	Z_ElementSpec_externalSpec
    } which;
    union
    {
    	char *elementSetName;
	Odr_external *externalSpec;
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
    enum
    {
    	Z_RecordComp_simple,
	Z_RecordComp_complex
    } which;
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
    Odr_external *resourceReport; /* OPTIONAL */
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
    enum
    {
    	Z_AccessRequest_simpleForm,
	Z_AccessRequest_externallyDefined
    } which;
    union
    {
    	Odr_oct *simpleForm;
	Odr_external *externallyDefined;
    } u;
#ifdef Z_95
    Z_OtherInformation *otherInfo;           /* OPTIONAL */
#endif
} Z_AccessControlRequest;

typedef struct Z_AccessControlResponse
{
    Z_ReferenceId *referenceId;              /* OPTIONAL */
    enum
    {
    	Z_AccessResponse_simpleForm,
	Z_AccessResponse_externallyDefined
    } which;
    union
    {
    	Odr_oct *simpleForm;
	Odr_external *externallyDefined;
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

typedef struct Z_OccurrenceByAttributes
{
    Z_AttributeList *attributes;
#if 0
    enum
    {
    	Z_OByAtt_global,
	Z_ObyAtt_byDatabase
    } which;
    union
    {
#endif
    	int *global;
#if 0
	/* Z_ByDatabase *byDatabase; */
    } u;
#endif
} Z_OccurrenceByAttributes;

typedef struct Z_TermInfo
{
    Z_Term *term;
    Z_AttributeList *suggestedAttributes;  /* OPTIONAL */
    Z_AlternativeTerm *alternativeTerm;    /* OPTIONAL */
    int *globalOccurrences;                /* OPTIONAL */
    Z_OccurrenceByAttributes *byAttributes; /* OPTIONAL */
} Z_TermInfo;

typedef struct Z_Entry
{
    enum
    {
    	Z_Entry_termInfo,
	Z_Entry_surrogateDiagnostic
    } which;
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
    enum
    {
    	Z_ListEntries_entries,
	Z_ListEntries_nonSurrogateDiagnostics
    } which;
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
    Z_ListStatus *deleteListStatuses;  /* OPTIONAL */
    int *numberNotDeleted;             /* OPTIONAL */
    int num_bulkStatuses;
    Z_ListStatus *bulkStatuses;        /* OPTIONAL */
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
    Odr_external *resourceReport;         /* OPTIONAL */
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


/* ------------------------ APDU ---------------------------- */

typedef struct Z_APDU
{    
    enum Z_APDU_which
    {
	Z_APDU_initRequest,
	Z_APDU_initResponse,
	Z_APDU_searchRequest,
	Z_APDU_searchResponse,
	Z_APDU_presentRequest,
	Z_APDU_presentResponse,
	Z_APDU_deleteResultSetRequest,
	Z_APDU_deleteResultSetResponse,
	Z_APDU_resourceControlRequest,
	Z_APDU_resourceControlResponse,
	Z_APDU_triggerResourceControlRequest,
	Z_APDU_scanRequest,
	Z_APDU_scanResponse,
	Z_APDU_segmentRequest,
	Z_APDU_close
    } which;
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
	Z_ResourceControlRequest *resourceControlRequest;
	Z_ResourceControlResponse *resourceControlResponse;
	Z_TriggerResourceControlRequest *triggerResourceControlRequest;
	Z_ScanRequest *scanRequest;
	Z_ScanResponse *scanResponse;
	Z_Segment *segmentRequest;
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
Z_APDU *zget_APDU(ODR o, enum Z_APDU_which which);

#include <prt-rsc.h>
#include <prt-acc.h>

#endif
