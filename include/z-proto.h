/*
 * Copyright (c) 1998-1999, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: z-proto.h,v $
 * Revision 1.2  1999-06-09 10:52:11  adam
 * Added YAZ_EXPORT.
 *
 * Revision 1.1  1999/06/08 13:11:55  adam
 * Fixed problem with proto.h.
 *
 * Revision 1.2  1999/04/20 10:37:04  adam
 * Updated for ODR - added name parameter.
 *
 * Revision 1.1  1998/03/31 16:04:05  adam
 * First version of proto.h which is almost compatible with the old one.
 *
 */
#ifndef Z_PROTO_H
#define Z_PROTO_H

#define Z_95 1
#define ASN_COMPILED 1

#include <yaz-version.h>
#include <z-accdes1-p.h>
#include <z-accform1-p.h>
#include <z-acckrb1-p.h>
#include <z-core-p.h>
#include <z-diag1-p.h>
#include <z-espec1-p.h>
#include <z-estask-p.h>
#include <z-exp-p.h>
#include <z-grs-p.h>
#include <z-opac-p.h>
#include <z-rrf1-p.h>
#include <z-rrf2-p.h>
#include <z-sum-p.h>
#include <z-sutrs-p.h>
#include <z-uifr1-p.h>
#include <zes-expi-p.h>
#include <zes-exps-p.h>
#include <zes-order-p.h>
#include <zes-pquery-p.h>
#include <zes-psched-p.h>
#include <zes-pset-p.h>
#include <zes-update-p.h>
#include <z-date-p.h>
#include <z-univ-p.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Z_PRES_SUCCESS   Z_PresentStatus_success
#define Z_PRES_PARTIAL_1 Z_PresentStatus_partial_1
#define Z_PRES_PARTIAL_2 Z_PresentStatus_partial_2
#define Z_PRES_PARTIAL_3 Z_PresentStatus_partial_3
#define Z_PRES_PARTIAL_4 Z_PresentStatus_partial_4
#define Z_PRES_FAILURE   Z_PresentStatus_failure

#define Z_RES_SUBSET  Z_SearchResponse_subset
#define Z_RES_INTERIM Z_SearchResponse_interim
#define Z_RES_NONE    Z_SearchResponse_none

#define Z_SortStatus_success Z_SortResponse_success 
#define Z_SortStatus_partial_1 Z_SortResponse_partial_1
#define Z_SortStatus_failure Z_SortResponse_failure

#define Z_SortRelation_ascending            Z_SortKeySpec_ascending 
#define Z_SortRelation_descending           Z_SortKeySpec_descending
#define Z_SortRelation_ascendingByFreq      Z_SortKeySpec_ascendingByFrequency
#define Z_SortRelation_descendingByFreq     Z_SortKeySpec_descendingByfrequency 

#define Z_SortCase_caseSensitive            Z_SortKeySpec_caseSensitive
#define Z_SortCase_caseInsensitive          Z_SortKeySpec_descendingByfrequency

#define Z_TriggerResourceCtrl_resourceReport Z_TriggerResourceControlRequest_resourceReport
#define Z_TriggerResourceCtrl_resourceControl  Z_TriggerResourceControlRequest_resourceControl
#define Z_TriggerResourceCtrl_cancel Z_TriggerResourceControlRequest_cancel

#define Z_DeleteRequest_list    Z_DeleteResultSetRequest_list
#define Z_DeleteRequest_all     Z_DeleteResultSetRequest_all

#define Z_AccessRequest_simpleForm Z_AccessControlRequest_simpleForm
#define Z_AccessRequest_externallyDefined Z_AccessControlRequest_externallyDefined

#define Z_AccessResponse_simpleForm Z_AccessControlResponse_simpleForm
#define Z_AccessResponse_externallyDefined Z_AccessControlResponse_externallyDefined

#define Z_ResourceReportStatus_success   Z_ResourceReportResponse_success
#define Z_ResourceReportStatus_partial   Z_ResourceReportResponse_partial
#define Z_ResourceReportStatus_failure_1 Z_ResourceReportResponse_failure_1
#define Z_ResourceReportStatus_failure_2 Z_ResourceReportResponse_failure_2
#define Z_ResourceReportStatus_failure_3 Z_ResourceReportResponse_failure_3
#define Z_ResourceReportStatus_failure_4 Z_ResourceReportResponse_failure_4
#define Z_ResourceReportStatus_failure_5 Z_ResourceReportResponse_failure_5
#define Z_ResourceReportStatus_failure_6 Z_ResourceReportResponse_failure_6

#define Z_SortResultSetStatus_empty       Z_SortResponse_empty
#define Z_SortResultSetStatus_interim     Z_SortResponse_interim
#define Z_SortResultSetStatus_unchanged   Z_SortResponse_unchanged
#define Z_SortResultSetStatus_none        Z_SortResponse_none

typedef Z_External Z_DatabaseRecord;
typedef struct Z_IOItemOrder Z_ItemOrder;

YAZ_EXPORT Z_APDU *zget_APDU(ODR o, int which);
YAZ_EXPORT Z_Close *zget_Close (ODR o);

#ifdef __cplusplus
}
#endif

#include <prt-ext.h>

#endif
