/*
 * Copyright (c) 1995-1998, Index Data.
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

#ifndef PRT_EXD_H
#define PRT_EXD_H

#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Z_TaskPackage
{
    Odr_oid *packageType;                 
    char *packageName;                      /* OPTIONAL */
    char *userId;                           /* OPTIONAL */
    Z_IntUnit *retentionTime;               /* OPTIONAL */
    Z_Permissions *permissions;             /* OPTIONAL */
    char *description;                      /* OPTIONAL */
    Odr_oct *targetReference;             
    char *creationDateTime;    /* OPTIONAL */
    int *taskStatus;                      
#define Z_TaskPackage_pending             0
#define Z_TaskPackage_active              1
#define Z_TaskPackage_complete            2
#define Z_TaskPackage_aborted             3
    int num_packageDiagnostics;
    Z_DiagRec **packageDiagnostics;         /* OPTIONAL */
    Z_External *taskSpecificParameters;   
} Z_TaskPackage;

int z_TaskPackage(ODR o, Z_TaskPackage **p, int opt, const char *name);

/* ----------------------- ITEM ORDER ------------------------- */

typedef struct Z_IOTargetPart
{
    Z_External *itemRequest;                   /* OPTIONAL */
    Z_External *statusOrErrorReport;
    int *auxiliaryStatus;                      /* OPTIONAL */
#define Z_IOTargetPart_notReceived        1
#define Z_IOTargetPart_loanQueue          2
#define Z_IOTargetPart_forwarded          3
#define Z_IOTargetPart_unfilledCopyright  4
#define Z_IOTargetPart_filledCopyright    1
} Z_IOTargetPart;

typedef struct Z_IOResultSetItem
{
    char *resultSetId;
    int *item;
} Z_IOResultSetItem;

typedef struct Z_IOOriginPartNotToKeep
{
    Z_IOResultSetItem *resultSetItem;          /* OPTIONAL */
    Z_External *itemRequest;                   /* OPTIONAL */
} Z_IOOriginPartNotToKeep;

typedef struct Z_IOContact
{
    char *name;                                /* OPTIONAL */
    char *phone;                               /* OPTIONAL */
    char *email;                               /* OPTIONAL */
} Z_IOContact;

typedef struct Z_IOCreditCardInfo
{
    char *nameOnCard;
    char *expirationDate;
    char *cardNumber;
} Z_IOCreditCardInfo;

typedef struct Z_IOBilling
{
    int which;
#define Z_IOBilling_billInvoice 0
#define Z_IOBilling_prepay 1
#define Z_IOBilling_depositAccount 2
#define Z_IOBilling_creditCard 3
#define Z_IOBilling_cardInfoPreviouslySupplied 4
#define Z_IOBilling_privateKnown 5
#define Z_IOBilling_privateNotKnown 6
    union
    {
	Odr_null *noinfo;
	Z_IOCreditCardInfo *creditCard;
	Z_External *privateNotKnown;
    } paymentMethod;
    char *customerReference;                        /* OPTIONAL */
    char *customerPONumber;                         /* OPTIONAL */
} Z_IOBilling;

typedef struct Z_IOOriginPartToKeep
{
    Z_External *supplDescription;              /* OPTIONAL */
    Z_IOContact *contact;                      /* OPTIONAL */
    Z_IOBilling *addlBilling;                  /* OPTIONAL */
} Z_IOOriginPartToKeep;

typedef struct Z_IORequest
{
    Z_IOOriginPartToKeep *toKeep;              /* OPTIONAL */
    Z_IOOriginPartNotToKeep *notToKeep;
} Z_IORequest;

typedef struct Z_IOTaskPackage
{
    Z_IOOriginPartToKeep *originPart;          /* OPTIONAL */
    Z_IOTargetPart *targetPart;
} Z_IOTaskPackage;

typedef struct Z_ItemOrder
{
    int which;
#define Z_ItemOrder_esRequest 0
#define Z_ItemOrder_taskPackage 1
    union
    {
	Z_IORequest *esRequest;
	Z_IOTaskPackage *taskPackage;
    } u;
} Z_ItemOrder;

int z_ItemOrder(ODR o, Z_ItemOrder **p, int opt, const char *name);

/* ----------------------- ITEM UPDATE ------------------------ */

typedef struct Z_IUSuppliedRecordsId
{
    int which;
#define Z_IUSuppliedRecordsId_timeStamp 1
#define Z_IUSuppliedRecordsId_versionNumber 2
#define Z_IUSuppliedRecordsId_previousVersion 3
    union {
        char *timeStamp;
        char *versionNumber;
        Odr_external *previousVersion;
    } u;
} Z_IUSuppliedRecordsId;

typedef struct Z_IUCorrelationInfo
{
    char *note; /* OPTIONAL */
    int *id; /* OPTIONAL */
} Z_IUCorrelationInfo;

typedef struct Z_IUSuppliedRecords_elem
{
    int which;
#define Z_IUSuppliedRecords_number 1
#define Z_IUSuppliedRecords_string 2
#define Z_IUSuppliedRecords_opaque 3
    union {
        int *number;
        char *string;
        Odr_oct *opaque;
    } u; /* OPTIONAL */
    Z_IUSuppliedRecordsId *supplementalId; /* OPTIONAL */
    Z_IUCorrelationInfo *correlationInfo;    /* OPTIONAL */
    Odr_external *record;
} Z_IUSuppliedRecords_elem;

typedef struct Z_IUSuppliedRecords
{
    int num;
    Z_IUSuppliedRecords_elem **elements;
} Z_IUSuppliedRecords;

typedef struct Z_IUOriginPartToKeep
{
    int *action;
#define Z_IUOriginPartToKeep_recordInsert 1
#define Z_IUOriginPartToKeep_recordReplace 2
#define Z_IUOriginPartToKeep_recordDelete 3
#define Z_IUOriginPartToKeep_elementUpdate 4
#define Z_IUOriginPartToKeep_specialUpdate 5
    char *databaseName;
    Odr_oid *schema;               /* OPTIONAL */
    char *elementSetName;          /* OPTIONAL */
    Odr_external *actionQualifier; /* OPTIONAL */
} Z_IUOriginPartToKeep;

typedef struct Z_IUTaskPackageRecordStructure
{
    int which;
#define Z_IUTaskPackageRecordStructure_record 1
#define Z_IUTaskPackageRecordStructure_surrogateDiagnostics 2
    union {
        Odr_external *record;
        Z_DiagRecs *surrogateDiagnostics;
    } u; /* OPTIONAL */
    Z_IUCorrelationInfo *correlationInfo; /* OPTIONAL */
    int *recordStatus;
#define Z_IUTaskPackageRecordStructureS_success 1
#define Z_IUTaskPackageRecordStructureS_queued 2
#define Z_IUTaskPackageRecordStructureS_inProcess 3
#define Z_IUTaskPackageRecordStructureS_failure 4
    Z_DiagRecs *supplementalDiagnostics;  /* OPTIONAL */
} Z_IUTaskPackageRecordStructure;

typedef struct Z_IUTargetPart
{
    int *updateStatus;
#define Z_IUTargetPart_success 1
#define Z_IUTargetPart_partial 2
#define Z_IUTargetPart_failure 3
    int num_globalDiagnostics;
    Z_DiagRec **globalDiagnostics; /* OPTIONAL */
    int num_taskPackageRecords;
    Z_IUTaskPackageRecordStructure **taskPackageRecords;
} Z_IUTargetPart;

typedef struct Z_IUUpdateEsRequest
{
    Z_IUOriginPartToKeep *toKeep;
    Z_IUSuppliedRecords *notToKeep;
} Z_IUUpdateEsRequest;

typedef struct Z_IUUpdateTaskPackage
{
    Z_IUOriginPartToKeep *originPart;
    Z_IUTargetPart *targetPart;
} Z_IUUpdateTaskPackage;

typedef struct Z_IUUpdate
{
    int which;
#define Z_IUUpdate_esRequest 1
#define Z_IUUpdate_taskPackage 2
    union {
        Z_IUUpdateEsRequest *esRequest;
        Z_IUUpdateTaskPackage *taskPackage;
    } u;
} Z_IUUpdate;

YAZ_EXPORT int z_IUUpdate(ODR o, Z_IUUpdate **p, int opt, const char *name);

#ifdef __cplusplus
}
#endif

#endif
