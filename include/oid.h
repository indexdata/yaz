/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: oid.h,v $
 * Revision 1.1  1995-03-30 09:39:41  quinn
 * Moved .h files to include directory
 *
 * Revision 1.1  1995/03/27  08:32:13  quinn
 * Added OID database
 *
 *
 */

#ifndef OID_H
#define OID_H

#include <odr.h>

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
	VAL_EXPORTINV
    } value;
    Odr_oid oidsuffix[20];
    char *desc;
} oident;

typedef enum oid_proto oid_proto;
typedef enum oid_class oid_class;
typedef enum oid_value oid_value;

Odr_oid *oid_getoidbyent(struct oident *ent);
struct oident *oid_getentbyoid(Odr_oid *o);

#endif
