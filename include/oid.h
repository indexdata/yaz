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
 * $Log: oid.h,v $
 * Revision 1.27  1998-05-18 13:06:58  adam
 * Changed the way attribute sets are handled by the retriaval module.
 * Extended Explain conversion / schema.
 * Modified server and client to work with ASN.1 compiled protocol handlers.
 *
 * Revision 1.26  1998/03/20 14:46:06  adam
 * Added UNIverse Resource Reports.
 *
 * Revision 1.25  1998/02/10 15:31:52  adam
 * Implemented date and time structure. Changed the Update Extended
 * Service.
 *
 * Revision 1.24  1997/09/29 13:18:59  adam
 * Added function, oid_ent_to_oid, to replace the function
 * oid_getoidbyent, which is not thread safe.
 *
 * Revision 1.23  1997/09/01 08:49:50  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.22  1997/08/19 08:45:13  quinn
 * Added Thesaurus
 *
 * Revision 1.21  1997/08/19 08:43:49  quinn
 * Housekeeping
 *
 * Revision 1.19  1997/07/28 12:34:42  adam
 * Added new OID entries (RVDM).
 *
 * Revision 1.18  1997/05/14 06:53:42  adam
 * C++ support.
 *
 * Revision 1.17  1997/05/02 08:39:27  quinn
 * Support for private OID table added. Thanks to Ronald van der Meer
 *
 * Revision 1.16  1997/04/30 08:52:08  quinn
 * Null
 *
 * Revision 1.15  1996/10/09  15:54:57  quinn
 * Added SearchInfoReport
 *
 * Revision 1.14  1996/10/07  15:29:17  quinn
 * Added SOIF support
 *
 * Revision 1.13  1996/02/20  17:57:53  adam
 * Added const to oid_getvalbyname.
 *
 * Revision 1.12  1996/02/20  12:52:37  quinn
 * Various
 *
 * Revision 1.11  1996/01/02  08:57:30  quinn
 * Changed enums in the ASN.1 .h files to #defines. Changed oident.class to oclass
 *
 * Revision 1.10  1995/11/13  09:27:31  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.9  1995/10/12  10:34:45  quinn
 * Added Espec-1.
 *
 * Revision 1.8  1995/10/10  16:27:08  quinn
 * *** empty log message ***
 *
 * Revision 1.7  1995/09/29  17:12:05  quinn
 * Smallish
 *
 * Revision 1.6  1995/09/27  15:02:48  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.5  1995/09/12  11:31:46  quinn
 * Added some oids.
 *
 * Revision 1.4  1995/06/27  13:20:32  quinn
 * Added SUTRS support
 *
 * Revision 1.3  1995/05/29  08:11:33  quinn
 * Moved oid from odr/asn to util.
 *
 * Revision 1.2  1995/05/16  08:50:35  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/30  09:39:41  quinn
 * Moved .h files to include directory
 *
 * Revision 1.1  1995/03/27  08:32:13  quinn
 * Added OID database
 *
 *
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
    CLASS_TAGSET
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
    VAL_UNIVERSE_REPORT
} oid_value;

typedef struct oident
{
    oid_proto proto;
    oid_class oclass;
    oid_value value;
    int oidsuffix[20];
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

#ifdef __cplusplus
}
#endif

#endif
