/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: proto.h,v $
 * Revision 1.1  1995-03-30 09:39:42  quinn
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
#include <odr_use.h>

/* ----------------- GLOBAL AUXILIARY DEFS ----------------*/

typedef Odr_oct Z_ReferenceId;
typedef char Z_DatabaseName;
typedef char Z_ResultSetId;
typedef Odr_oct Z_ResultsetId;
typedef Odr_external Z_UserInformationField;

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
    	void *anonymous;         /* NULL */
    	Odr_external *other;
    } u;
} Z_IdAuthentication;

#define Z_ProtocolVersion_1            0
#define Z_ProtocolVersion_2            1
#define Z_ProtocolVersion_3            2

#define Z_Options_search               0
#define Z_Options_present              1
#define Z_Options_delSet               2
#define Z_Options_resourceReport       3
#define Z_Options_triggerResourceCtrl  4
#define Z_Options_resourceCtrl         5
#define Z_Options_accessCtrl           6
#define Z_Options_scan                 7
#define Z_Options_sort                 8
#define Z_Options_reserved             9
#define Z_Options_extendedServices    10
#define Z_Options_level_1Segmentation 11
#define Z_Options_level_2Segmentation 12
#define Z_Options_concurrentOperations 13
#define Z_Options_namedResultSets     14

typedef struct Z_InitRequest
{
    Z_ReferenceId *referenceId;                   /* OPTIONAL */
    Odr_bitmask *options;
    Odr_bitmask *protocolVersion;
    int *preferredMessageSize;
    int *maximumRecordSize;
    Z_IdAuthentication* idAuthentication;        /* OPTIONAL */
    char *implementationId;                      /* OPTIONAL */
    char *implementationName;                    /* OPTIONAL */
    char *implementationVersion;                 /* OPTIONAL */
    Z_UserInformationField *userInformationField; /* OPTIONAL */
} Z_InitRequest;

typedef struct Z_InitResponse
{
    Z_ReferenceId *referenceId;    /* OPTIONAL */
    Odr_bitmask *options;
    Odr_bitmask *protocolVersion;
    int *preferredMessageSize;
    int *maximumRecordSize;
    bool_t *result;
    char *implementationId;      /* OPTIONAL */
    char *implementationName;    /* OPTIONAL */
    char *implementationVersion; /* OPTIONAL */
    Z_UserInformationField *userInformationField; /* OPTIONAL */
} Z_InitResponse;

typedef struct Z_NSRAuthentication
{
    char *user;
    char *password;
    char *account;
} Z_NSRAuthentication;

int z_NSRAuthentication(ODR o, Z_NSRAuthentication **p, int opt);

int z_StrAuthentication(ODR o, char **p, int opt);


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
} Z_ResourceControlRequest;

typedef struct Z_ResourceControlResponse
{
    Z_ReferenceId *referenceId;    /* OPTIONAL */
    bool_t *continueFlag;
    bool_t *resultSetWanted;       /* OPTIONAL */
} Z_ResourceControlResponse;

/* ------------------ SEARCH SERVICE ----------------*/

typedef Odr_oid Z_PreferredRecordSyntax;

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

typedef struct Z_AttributeElement
{
    int *attributeType;
    int *attributeValue;
} Z_AttributeElement;

#ifdef Z_V3

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
    	Odr_oct *general;
    	int *numeric;
    	char *characterString;
    	Odr_oid *oid;
    	char *dateTime;
    	Odr_external *external;
    	Z_IntUnit *integerAndUnit;
    	void *null;
    } u;
} Z_Term;

#endif

typedef struct Z_AttributesPlusTerm
{
    int num_attributes;
    Z_AttributeElement **attributeList;
#ifdef Z_V3
    Z_Term *term;
#else
    Odr_oct *term;
#endif
} Z_AttributesPlusTerm;

typedef struct Z_ProximityOperator
{
    bool_t *exclusion;          /* OPTIONAL */
    int *distance;
    bool_t *ordered;
    int *relationType;
    enum
    {
	Z_ProximityOperator_known,
	Z_ProximityOperator_private
    } which;
    union
    {
    	int *known;
    	int *private;
    } u;
} Z_ProximityOperator;

typedef struct Z_Operator
{
    enum
    {
	Z_Operator_and,
	Z_Operator_or,
	Z_Operator_and_not,
	Z_Operator_proximity
    } which;
    union
    {
    	void *and;          /* these guys are nulls. */
    	void *or;
    	void *and_not;
	Z_ProximityOperator *proximity;
    } u;
} Z_Operator;

typedef struct Z_Operand
{
    int which;
#define Z_Operand_APT 0
#define Z_Operand_resultSetId 1
    union
    {
    	Z_AttributesPlusTerm *attributesPlusTerm;
    	Z_ResultSetId *resultSetId;
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
    union
    {
    	Z_RPNQuery *type_1;
    	Odr_oct *type_2;
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
    Z_PreferredRecordSyntax *preferredRecordSyntax;  /* OPTIONAL */
    Z_Query *query;
} Z_SearchRequest;

/* ------------------------ RECORD -------------------------- */

typedef Odr_external Z_DatabaseRecord;

typedef struct Z_DiagRec
{
    Odr_oid *diagnosticSetId;
    int *condition;
    char *addinfo;
} Z_DiagRec;

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
    union
    {
    	Z_NamePlusRecordList *databaseOrSurDiagnostics;
    	Z_DiagRec *nonSurrogateDiagnostic;
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
    int *presentStatus;                /* OPTIONAL */
#define Z_PRES_SUCCESS      0
#define Z_PRES_PARTIAL_1    1
#define Z_PRES_PARTIAL_2    2
#define Z_PRES_PARTIAL_3    3
#define Z_PRES_PARTIAL_4    4
#define Z_PRES_FAILURE      5
    Z_Records *records;                  /* OPTIONAL */
} Z_SearchResponse;

/* ------------------------- PRESENT SERVICE -----------------*/

typedef struct Z_PresentRequest
{
    Z_ReferenceId *referenceId;          /* OPTIONAL */
    Z_ResultSetId *resultSetId;
    int *resultSetStartPoint;
    int *numberOfRecordsRequested;
    Z_ElementSetNames *elementSetNames;  /* OPTIONAL */
    Z_PreferredRecordSyntax *preferredRecordSyntax;  /* OPTIONAL */
} Z_PresentRequest;

typedef struct Z_PresentResponse
{
    Z_ReferenceId *referenceId;        /* OPTIONAL */
    int *numberOfRecordsReturned;
    int *nextResultSetPosition;
    int *presentStatus;
    Z_Records *records;
} Z_PresentResponse;

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
    union
    {
	Z_InitRequest  *initRequest;
    	Z_InitResponse *initResponse;
    	Z_SearchRequest *searchRequest;
    	Z_SearchResponse *searchResponse;
    	Z_PresentRequest *presentRequest;
    	Z_PresentResponse *presentResponse;
    } u;
} Z_APDU;

int z_APDU(ODR o, Z_APDU **p, int opt);

#endif
