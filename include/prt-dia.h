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

#ifndef PRT_DIA_H
#define PRT_DIA_H

#include <yconfig.h>

typedef struct Z_TooMany
{
    int *tooManyWhat;
#define Z_TooMany_argumentWords        1
#define Z_TooMany_truncatedWords       2
#define Z_TooMany_booleanOperators     3
#define Z_TooMany_incompleteSubfields  4
#define Z_TooMany_characters           5
#define Z_TooMany_recordsRetrieved     6
#define Z_TooMany_databasesSpecified   7
#define Z_TooMany_resultSetsCreated    8
#define Z_TooMany_indexTermsProcessed  9
    int *max;                    /* OPTIONAL */
} Z_TooMany;

typedef struct Z_BadSpec
{
    Z_Specification *spec;
    char *db;                    /* OPTIONAL */
    int num_goodOnes;
    Z_Specification **goodOnes;  /* OPTIONAL */
} Z_BadSpec;

typedef struct Z_DbUnavailWhy
{
    int *reasonCode;             /* OPTIONAL */
#define Z_DbUnavail_doesNotExist         0
#define Z_DbUnavail_existsButUnavail     1
#define Z_DbUnavail_locked               2
#define Z_DbUnavail_accessDenied         3
    char *message;               /* OPTIONAL */
} Z_DbUnavailWhy;

typedef struct Z_DbUnavail
{
    char *db;
    Z_DbUnavailWhy *why;         /* OPTIONAL */
} Z_DbUnavail;

typedef struct Z_Attribute
{
    Odr_oid *id;
    int *type;                   /* OPTIONAL */
    int *value;                  /* OPTIONAL */
    Z_Term *term;                /* OPTIONAL */
} Z_Attribute;

typedef struct Z_AttCombo
{
    Z_AttributeList *unsupportedCombination;
    int num_alternatives;
    Z_AttributeList **alternatives;    /* OPTIONAL */
} Z_AttCombo;

typedef struct Z_DiagTerm 
{
    int *problem;                /* OPTIONAL */
#define Z_DiagtermProb_codedValue     1
#define Z_DiagtermProb_unparsable     2
#define Z_DiagtermProb_tooShort       3
#define Z_DiagtermProb_type           4
    Z_Term *term;
} Z_DiagTerm;

typedef struct Z_Proximity
{
    enum
    {
	Z_Proximity_resultSets,
	Z_Proximity_badSet,
	Z_Proximity_relation,
	Z_Proximity_unit,
	Z_Proximity_distance,
	Z_Proximity_attributes,
	Z_Proximity_ordered,
	Z_Proximity_exclusion
    } which;
    union
    {
	Odr_null *resultSets;
	char *badSet;
	int *relation;
	int *unit;
	int *distance;
	Z_AttributeList *attributes;
	Odr_null *ordered;
	Odr_null *exclusion;
    } u;
} Z_Proximity;

typedef struct Z_AttrListList
{
    int num_lists;
    Z_AttributeList *lists;
} Z_AttrListList;

typedef struct Z_Scan
{
    enum
    {
	Z_ScanD_nonZeroStepSize,
	Z_ScanD_specifiedStepSize,
	Z_ScanD_termList1,
	Z_ScanD_termList2,
	Z_ScanD_posInResponse,
	Z_ScanD_resources,
	Z_ScanD_endOfList
    } which;
    union
    {
	Odr_null *nonZeroStepSize;
	Odr_null *specifiedStepSize;
	Odr_null *termList1;
	Z_AttrListList *termList2;
	int *posInResponse;
#define Z_ScanPosInRsp_mustBeOne         1
#define Z_ScanPosInRsp_mustBePositive    2
#define Z_ScanPosInRsp_mustBeNonNegative 3
#define Z_ScanPosInRsp_other             4
	Odr_null *resources;
	Odr_null *endOfList;
    } u;
} Z_Scan;

typedef struct Z_StringList
{
    int num_strings;
    char **strings;
} Z_StringList;

typedef struct Z_Sort
{
    enum
    {
	Z_SortD_sequence,
	Z_SortD_noRsName,
	Z_SortD_tooMany,
	Z_SortD_incompatible,
	Z_SortD_generic,
	Z_SortD_dbSpecific,
	Z_SortD_sortElement,
	Z_SortD_key,
	Z_SortD_action,
	Z_SortD_illegal,
	Z_SortD_inputTooLarge,
	Z_SortD_aggregateTooLarge
    } which;
    union
    {
	Odr_null *sequence;
	Odr_null *noRsName;
	int *tooMany;
	Odr_null *incompatible;
	Odr_null *generic;
	Odr_null *dbSpecific;
#if 0
	Z_SortElement *sortElement;
#endif
	int *key;
#define Z_SortKey_tooMany       1
#define Z_SortKey_duplicate     2
	Odr_null *action;
	int *illegal;
#define Z_SortIllegal_relation  1
#define Z_SortIllegal_case      2
#define Z_SortIllegal_action    3
#define Z_SortIllegal_sort      4
	Z_StringList *inputTooLarge;
	Odr_null *aggregateTooLarge;
    } u;
} Z_Sort;

typedef struct Z_Segmentation
{
    enum
    {
	Z_SegmentationD_segments
    } which;
    union
    {
	Odr_null *segments;
    } u;
} Z_Segmentation;

typedef struct Z_ExtServices
{
    enum
    {
	Z_ExtServicesD_req,
	Z_ExtServicesD_permission,
	Z_ExtServicesD_immediate
    } which;
    union
    {
	int *req;
#define Z_ExtSrvReq_nameInUse           1
#define Z_ExtSrvReq_noSuchname          2
#define Z_ExtSrvReq_quota               3
#define Z_ExtSrvReq_type                4
	int *permission;
#define Z_ExtSrvPerm_id                 1
#define Z_ExtSrvPerm_modifyDelete       2
	int *immediate;
#define Z_ExtSrvImm_failed              1
#define Z_ExtSrvImm_service             2
#define Z_ExtSrvImm_parameters          3
    } u;
} Z_ExtServices;

typedef struct Z_OidList
{
    int num_oids;
    Odr_oid **oids;
} Z_OidList;

typedef struct Z_AccessCtrl
{
    enum
    {
	Z_AccessCtrlD_noUser,
	Z_AccessCtrlD_refused,
	Z_AccessCtrlD_simple,
	Z_AccessCtrlD_oid,
	Z_AccessCtrlD_alternative,
	Z_AccessCtrlD_pwdInv,
	Z_AccessCtrlD_pwdExp
    } which;
    union
    {
	Odr_null *noUser;
	Odr_null *refused;
	Odr_null *simple;
	Z_OidList *oid;
	Z_OidList *alternative;
	Odr_null *pwdInv;
	Odr_null *pwdExp;
    } u;
} Z_AccessCtrl;

typedef struct Z_RecordSyntax
{
    Odr_oid *unsupportedSyntax;
    int num_suggestedAlternatives;           /* OPTIONAL */
    Odr_oid **suggestedAlternatives;           /* OPTIONAL */
} Z_RecordSyntax;

typedef struct Z_DiagFormat
{
    enum
    {
	Z_DiagFormat_tooMany,
	Z_DiagFormat_badSpec,
	Z_DiagFormat_dbUnavail,
	Z_DiagFormat_unSupOp,
	Z_DiagFormat_attribute,
	Z_DiagFormat_attCombo,
	Z_DiagFormat_term,
	Z_DiagFormat_proximity,
	Z_DiagFormat_scan,
	Z_DiagFormat_sort,
	Z_DiagFormat_segmentation,
	Z_DiagFormat_extServices,
	Z_DiagFormat_accessCtrl,
	Z_DiagFormat_recordSyntax
    } which;
    union
    {
	Z_TooMany *tooMany;
	Z_BadSpec *badSpec;
	Z_DbUnavail *dbUnavail;
	int *unSupOp;
#define Z_UnSupOp_and             0
#define Z_UnSupOp_or              1
#define Z_UnSupOp_and_not         2
#define Z_UnSupOp_prox            3
	Z_Attribute *attribute;
	Z_AttributeList *attCombo;
	Z_DiagTerm *term;
	Z_Proximity *proximity;
	Z_Scan *scan;
	Z_Sort *sort;
	Z_Segmentation *segmentation;
	Z_ExtServices *extServices;
	Z_AccessCtrl *accessCtrl;
	Z_RecordSyntax *recordSyntax;
    } u;
} Z_DiagFormat;

typedef struct Z_Diagnostic
{
    enum
    {
	Z_Diagnostic_defaultDiagRec,
	Z_Diagnostic_explicitDiagnostic
    } which;
    union 
    {
	Z_DefaultDiagFormat *defaultDiagRec;
	Z_DiagFormat *explicitDiagnostic;
    } u;
} Z_Diagnostic;

typedef struct Z_DiagnosticUnit
{
    Z_Diagnostic *diagnostic;                    /* OPTIONAL */
    char *message;                               /* OPTIONAL */
} Z_DiagnosticUnit;

typedef struct Z_DiagnosticFormat
{
    int num_diagnostics;
    Z_DiagnosticUnit **diagnostics;
} Z_DiagnosticFormat;

int z_DiagnosticFormat(ODR o, Z_DiagnosticFormat **p, int opt);

#endif
