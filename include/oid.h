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
 * Revision 1.6  1995-09-27 15:02:48  quinn
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

#define OID_SIZE 100

typedef struct oident
{
    enum oid_proto
    {
    	PROTO_Z3950,
	PROTO_SR,
	PROTO_GENERAL
    } proto;
    enum oid_class
    {
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
	CLASS_DBSCHEMA,
	CLASS_TAGSET
    } class;
    enum oid_value
    {
    	VAL_APDU,
	VAL_BER,
	VAL_BASIC_CTX,
	VAL_BIB1,
	VAL_EXP1,
	VAL_EXT1,
	VAL_CCL1,
	VAL_GILS,
	VAL_STAS,
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
	VAL_EXPLAIN,
	VAL_SUTRS,
	VAL_OPAC,
	VAL_SUMMARY,
	VAL_GRS0,
	VAL_GRS1,
	VAL_EXTENDED,
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
	VAL_SETG
    } value;
    int oidsuffix[20];
    char *desc;
} oident;

typedef enum oid_proto oid_proto;
typedef enum oid_class oid_class;
typedef enum oid_value oid_value;

int MDF *oid_getoidbyent(struct oident *ent);
struct oident MDF *oid_getentbyoid(int *o);
void MDF oid_oidcpy(int *t, int *s);
void MDF oid_oidcat(int *t, int *s);
int MDF oid_oidcmp(int *o1, int *o2);
int MDF oid_oidlen(int *o);
oid_value MDF oid_getvalbyname(char *name);

#endif
