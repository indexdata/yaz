/*
 * Copyright (c) 1998-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: z-proto.h,v $
 * Revision 1.6  2001-09-19 10:19:07  adam
 * Z_SortCase_caseInsensitive was set to wrong value.
 *
 * Revision 1.5  2001/05/17 14:16:15  adam
 * Added EXTERNAL handling for item update0 (1.0).
 *
 * Revision 1.4  2000/03/14 09:19:49  ian
 * Added admin extended service encoders & decoders.
 *
 * Revision 1.3  2000/02/28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.2  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.2  1999/06/09 10:52:11  adam
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

#include <yaz/yaz-version.h>
#include <yaz/z-accdes1.h>
#include <yaz/z-accform1.h>
#include <yaz/z-acckrb1.h>
#include <yaz/z-core.h>
#include <yaz/z-diag1.h>
#include <yaz/z-espec1.h>
#include <yaz/z-estask.h>
#include <yaz/z-exp.h>
#include <yaz/z-grs.h>
#include <yaz/z-opac.h>
#include <yaz/z-rrf1.h>
#include <yaz/z-rrf2.h>
#include <yaz/z-sum.h>
#include <yaz/z-sutrs.h>
#include <yaz/z-uifr1.h>
#include <yaz/zes-expi.h>
#include <yaz/zes-exps.h>
#include <yaz/zes-order.h>
#include <yaz/zes-pquery.h>
#include <yaz/zes-psched.h>
#include <yaz/zes-pset.h>
#include <yaz/zes-update.h>
#include <yaz/zes-admin.h>
#include <yaz/z-date.h>
#include <yaz/z-univ.h>
#include <yaz/zes-update0.h>

YAZ_BEGIN_CDECL

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
#define Z_SortCase_caseInsensitive          Z_SortKeySpec_caseInsensitive

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

YAZ_END_CDECL

#include <yaz/prt-ext.h>

#endif
