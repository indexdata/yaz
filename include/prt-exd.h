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

#ifndef PRT_EXD_H
#define PRT_EXD_H

#include <yconfig.h>

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

int MDF z_TaskPackage(ODR o, Z_TaskPackage **p, int opt);

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
    enum
    {
	Z_IOBilling_billInvoice,
	Z_IOBilling_prepay,
	Z_IOBilling_depositAccount,
	Z_IOBilling_creditCard,
	Z_IOBilling_cardInfoPreviouslySupplied,
	Z_IOBilling_privateKnown,
	Z_IOBilling_privateNotKnown
    } which;
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
    enum
    {
	Z_ItemOrder_esRequest,
	Z_ItemOrder_taskPackage
    } which;
    union
    {
	Z_IORequest *esRequest;
	Z_IOTaskPackage *taskPackage;
    } u;
} Z_ItemOrder;

int MDF z_ItemOrder(ODR o, Z_ItemOrder **p, int opt);

#endif
