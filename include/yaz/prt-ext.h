/*
 * Copyright (C) 1995-2005, Index Data ApS
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
 * $Id: prt-ext.h,v 1.12 2005-01-27 09:08:42 adam Exp $
 */

/**
 * \file prt-ext.h
 * \brief Header for utilities that handles Z39.50 EXTERNALs
 */

/*
 * Biased-choice External for Z39.50.
 */

#ifndef PRT_EXT_H
#define PRT_EXT_H

#include <yaz/yconfig.h>
#include <yaz/oid.h>


YAZ_BEGIN_CDECL

/**
 * Used to keep track of known External definitions (a loose approach
 * to DEFINED_BY).
 */
typedef struct Z_ext_typeent
{
    oid_value dref;    /* the direct-reference OID value. */
    int what;          /* discriminator value for the external CHOICE */
    Odr_fun fun;       /* decoder function */
} Z_ext_typeent;

/** \brief structure for all known EXTERNALs */
struct Z_External
{
    Odr_oid *direct_reference;
    int *indirect_reference;
    char *descriptor;
    int which;
/* Generic types */
#define Z_External_single 0
#define Z_External_octet 1
#define Z_External_arbitrary 2
/* Specific types */
#define Z_External_sutrs 3
#define Z_External_explainRecord 4
#define Z_External_resourceReport1 5
#define Z_External_resourceReport2 6
#define Z_External_promptObject1 7
#define Z_External_grs1 8
#define Z_External_extendedService 9
#define Z_External_itemOrder 10
#define Z_External_diag1 11
#define Z_External_espec1 12
#define Z_External_summary 13
#define Z_External_OPAC 14
#define Z_External_searchResult1 15
#define Z_External_update 16
#define Z_External_dateTime 17
#define Z_External_universeReport 18
#define Z_External_ESAdmin 19
#define Z_External_update0 20
#define Z_External_userInfo1 21
#define Z_External_charSetandLanguageNegotiation 22
#define Z_External_acfPrompt1 23
#define Z_External_acfDes1 24
#define Z_External_acfKrb1 25
#define Z_External_multisrch2 26
#define Z_External_CQL 27
#define Z_External_OCLCUserInfo 28
    union
    {
	/* Generic types */
	Odr_any *single_ASN1_type;
	Odr_oct *octet_aligned;
	Odr_bitmask *arbitrary;

	/* Specific types */
	Z_SUTRS *sutrs;
	Z_ExplainRecord *explainRecord;

	Z_ResourceReport1 *resourceReport1;
	Z_ResourceReport2 *resourceReport2;
	Z_PromptObject1 *promptObject1;
	Z_GenericRecord *grs1;
	Z_TaskPackage *extendedService;

	Z_ItemOrder *itemOrder;
	Z_DiagnosticFormat *diag1;
	Z_Espec1 *espec1;
	Z_BriefBib *summary;
        Z_OPACRecord *opac;

	Z_SearchInfoReport *searchResult1;
	Z_IUUpdate *update;
	Z_DateTime *dateTime;
        Z_UniverseReport *universeReport;
        Z_Admin *adminService;

	Z_IU0Update *update0;
        Z_OtherInformation *userInfo1;
        Z_CharSetandLanguageNegotiation *charNeg3;
        Z_PromptObject1 *acfPrompt1;
        Z_DES_RN_Object *acfDes1;

        Z_KRBObject *acfKrb1;
        Z_MultipleSearchTerms_2 *multipleSearchTerms_2;
        Z_InternationalString *cql;
	Z_OCLC_UserInformation *oclc;
    } u;
};


/** \brief codec for BER EXTERNAL */
YAZ_EXPORT int z_External(ODR o, Z_External **p, int opt, const char *name);
/** \brief returns type information for OID (NULL if not known) */
YAZ_EXPORT Z_ext_typeent *z_ext_getentbyref(oid_value val);
/** \brief encodes EXTERNAL record based on OID (NULL if knot known) */
YAZ_EXPORT Z_External *z_ext_record(ODR o, int format, const char *buf,
				    int len);

YAZ_END_CDECL

#endif
