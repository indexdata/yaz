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

/*
 * Biased-choice External for Z39.50.
 */

#ifndef PRT_EXT_H
#define PRT_EXT_H

#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Used to keep track of known External definitions (a loose approach
 * to DEFINED_BY).
 */

typedef struct Z_ext_typeent
{
    oid_value dref;    /* the direct-reference OID value. */
    int what;          /* discriminator value for the external CHOICE */
    Odr_fun fun;       /* decoder function */
} Z_ext_typeent;

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
	Z_SearchInfoReport *searchResult1;
	Z_IUUpdate *update;
	Z_DateTime *dateTime;
    Z_UniverseReport *universeReport;
    } u;
};

YAZ_EXPORT int z_External(ODR o, Z_External **p, int opt);
YAZ_EXPORT Z_ext_typeent *z_ext_getentbyref(oid_value val);

#ifdef __cplusplus
}
#endif

#endif
