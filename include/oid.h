/*
 * Copyright (c) 1995-1999, Index Data.
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
 * $Id: oid.h,v 1.32 1999-04-20 09:56:48 adam Exp $
 */

#ifndef OID_H
#define OID_H

#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OID_SIZE 20
    
typedef enum oid_proto
{
    PROTO_NOP=0,
    PROTO_Z3950,
    PROTO_SR,
    PROTO_GENERAL,
    PROTO_WAIS
} oid_proto;

typedef enum oid_class
{
    CLASS_NOP=0,
    CLASS_APPCTX,
    CLASS_ABSYN,
    CLASS_ATTSET,
    CLASS_TRANSYN,
    CLASS_DIAGSET,
    CLASS_RECSYN,
    CLASS_RESFORM,
    CLASS_ACCFORM,
    CLASS_EXTSERV,
    CLASS_USERINFO,
    CLASS_ELEMSPEC,
    CLASS_VARSET,
    CLASS_SCHEMA,
    CLASS_TAGSET,
    CLASS_GENERAL
} oid_class;

typedef enum oid_value
{
    VAL_NOP=0,
    VAL_APDU,
    VAL_BER,
    VAL_BASIC_CTX,
    VAL_BIB1,
    VAL_EXP1,
    VAL_EXT1,
    VAL_CCL1,
    VAL_GILS,
    VAL_WAIS,
    VAL_STAS,
    VAL_COLLECT1,
    VAL_CIMI1,
    VAL_GEO,
    VAL_DIAG1,
    VAL_ISO2709,
    VAL_UNIMARC,
    VAL_INTERMARC,
    VAL_CCF,
    VAL_USMARC,
    VAL_UKMARC,
    VAL_NORMARC,
    VAL_LIBRISMARC,
    VAL_DANMARC,
    VAL_FINMARC,
    VAL_MAB,
    VAL_CANMARC,
    VAL_SBN,
    VAL_PICAMARC,
    VAL_AUSMARC,
    VAL_IBERMARC,
    VAL_CATMARC,
    VAL_MALMARC,
    VAL_EXPLAIN,
    VAL_SUTRS,
    VAL_OPAC,
    VAL_SUMMARY,
    VAL_GRS0,
    VAL_GRS1,
    VAL_EXTENDED,
    VAL_FRAGMENT,
    VAL_RESOURCE1,
    VAL_RESOURCE2,
    VAL_PROMPT1,
    VAL_DES1,
    VAL_KRB1,
    VAL_PRESSET,
    VAL_PQUERY,
    VAL_PCQUERY,
    VAL_ITEMORDER,
    VAL_DBUPDATE,
    VAL_EXPORTSPEC,
    VAL_EXPORTINV,
    VAL_NONE,
    VAL_SETM,
    VAL_SETG,
    VAL_VAR1,
    VAL_ESPEC1,
    VAL_SOIF,
    VAL_SEARCHRES1,
    VAL_THESAURUS,
    VAL_CHARLANG,
    VAL_USERINFO1,
    VAL_MULTISRCH1,
    VAL_MULTISRCH2,
    VAL_DATETIME,
    VAL_SQLRS,
    VAL_PDF,
    VAL_POSTSCRIPT,
    VAL_HTML,
    VAL_TIFF,
    VAL_GIF,
    VAL_JPEG,
    VAL_PNG,
    VAL_MPEG,
    VAL_SGML,
    VAL_TIFFB,
    VAL_WAV,
    VAL_UPDATEES,
    VAL_TEXT_XML,
    VAL_APPLICATION_XML,
    VAL_UNIVERSE_REPORT,
    VAL_PROXY,
    VAL_COOKIE,
/* add new types here... */

/* VAL_DYNAMIC must have highest value */
    VAL_DYNAMIC,
    VAL_MAX = VAL_DYNAMIC+30
} oid_value;

typedef struct oident
{
    oid_proto proto;
    oid_class oclass;
    oid_value value;
    int oidsuffix[OID_SIZE];
    char *desc;
} oident;

YAZ_EXPORT int *oid_getoidbyent(struct oident *ent);
YAZ_EXPORT int *oid_ent_to_oid(struct oident *ent, int *dst);
YAZ_EXPORT struct oident *oid_getentbyoid(int *o);
YAZ_EXPORT void oid_oidcpy(int *t, int *s);
YAZ_EXPORT void oid_oidcat(int *t, int *s);
YAZ_EXPORT int oid_oidcmp(int *o1, int *o2);
YAZ_EXPORT int oid_oidlen(int *o);
YAZ_EXPORT oid_value oid_getvalbyname(const char *name);
YAZ_EXPORT void oid_setprivateoids(oident *list);
YAZ_EXPORT struct oident *oid_addent (int *oid, enum oid_proto proto,
				      enum oid_class oclass,
				      const char *desc, int value);

#ifdef __cplusplus
}
#endif

#endif
