/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: oid.c,v $
 * Revision 1.12  1996-01-02 08:57:53  quinn
 * Changed enums in the ASN.1 .h files to #defines. Changed oident.class to oclass
 *
 * Revision 1.11  1995/12/13  16:03:35  quinn
 * *** empty log message ***
 *
 * Revision 1.10  1995/11/28  09:30:44  quinn
 * Work.
 *
 * Revision 1.9  1995/11/13  09:27:53  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.8  1995/10/12  10:34:56  quinn
 * Added Espec-1.
 *
 * Revision 1.7  1995/10/10  16:27:12  quinn
 * *** empty log message ***
 *
 * Revision 1.6  1995/09/29  17:12:35  quinn
 * Smallish
 *
 * Revision 1.5  1995/09/29  17:01:51  quinn
 * More Windows work
 *
 * Revision 1.4  1995/09/27  15:03:03  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.3  1995/09/12  11:32:06  quinn
 * Added a looker-upper by name.
 *
 * Revision 1.2  1995/08/21  09:11:16  quinn
 * Smallish fixes to suppport new formats.
 *
 * Revision 1.1  1995/05/29  08:17:13  quinn
 * iMoved oid to util to support comstack.
 *
 * Revision 1.5  1995/05/22  11:30:16  quinn
 * Adding Z39.50-1992 stuff to proto.c. Adding zget.c
 *
 * Revision 1.4  1995/05/16  08:50:22  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.3  1995/04/11  11:52:02  quinn
 * Fixed possible buf in proto.c
 *
 * Revision 1.2  1995/03/29  15:39:38  quinn
 * Adding some resource control elements, and a null-check to getentbyoid
 *
 * Revision 1.1  1995/03/27  08:32:12  quinn
 * Added OID database
 *
 *
 */

/*
 * More or less protocol-transparent OID database.
 * We could (and should?) extend this so that the user app can add new
 * entries to the list at initialization.
 */

#include <oid.h>

static int z3950_prefix[] = { 1, 2, 840, 10003, -1 };
static int sr_prefix[]    = { 1, 0, 10163, -1 };

/*
 * OID database
 */
static oident oids[] =
{
    /* General definitions */
    {PROTO_GENERAL, CLASS_TRANSYN, VAL_BER,       {2,1,1,-1},  "BER"         },
    {PROTO_GENERAL, CLASS_TRANSYN, VAL_ISO2709,   {1,0,2709,1,1,-1},"ISO2709"},

    /* Z39.50v3 definitions */
    {PROTO_Z3950,   CLASS_ABSYN,   VAL_APDU,      {2,1,-1},    "Z-APDU"      },
    {PROTO_Z3950,   CLASS_APPCTX,  VAL_BASIC_CTX, {1,1,-1},    "Z-BASIC"     },
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_BIB1,      {3,1,-1},    "Bib-1"       },
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_EXP1,      {3,2,-1},    "Exp-1"       },
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_EXT1,      {3,3,-1},    "Ext-1"       },
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_CCL1,      {3,4,-1},    "CCL-1"       },
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_GILS,      {3,5,-1},    "GILS-attset" },
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_STAS,      {3,6,-1},    "STAS-attset" },
    {PROTO_Z3950,   CLASS_DIAGSET, VAL_BIB1,      {4,1,-1},    "Bib-1"       },
    {PROTO_Z3950,   CLASS_DIAGSET, VAL_DIAG1,     {4,2,-1},    "Diag-1"      },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_UNIMARC,   {5,1,-1},    "Unimarc"     },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_INTERMARC, {5,2,-1},    "Intermarc"   },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_CCF,       {5,3,-1},    "CCF"         },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_USMARC,    {5,10,-1},   "USmarc"      },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_UKMARC,    {5,11,-1},   "UKmarc"      },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_NORMARC,   {5,12,-1},   "Normarc"     },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_LIBRISMARC,{5,13,-1},   "Librismarc"  },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_DANMARC,   {5,14,-1},   "Danmarc"     },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_FINMARC,   {5,15,-1},   "Finmarc"     },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_MAB,       {5,16,-1},   "MAB"         },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_CANMARC,   {5,17,-1},   "Canmarc"     },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SBN,       {5,18,-1},   "SBN"         },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_PICAMARC,  {5,19,-1},   "Picamarc"    },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_AUSMARC,   {5,20,-1},   "Ausmarc"     },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_IBERMARC,  {5,21,-1},   "Ibermarc"    },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_EXPLAIN,   {5,100,-1},  "Explain"     },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SUTRS,     {5,101,-1},  "SUTRS"       },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_OPAC,      {5,102,-1},  "OPAC"        },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SUMMARY,   {5,103,-1},  "Summary"     },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_GRS0,      {5,104,-1},  "GRS-0"       },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_GRS1,      {5,105,-1},  "GRS-1"       },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_EXTENDED,  {5,106,-1},  "Extended"    },
#if 0
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_ID_SGML,   {5,1000,81,1,-1},"ID-SGML" },
#endif
    {PROTO_Z3950,   CLASS_RESFORM, VAL_RESOURCE1, {7,1,-1},    "Resource-1"  },
    {PROTO_Z3950,   CLASS_RESFORM, VAL_RESOURCE2, {7,2,-1},    "Resource-2"  },
    {PROTO_Z3950,   CLASS_ACCFORM, VAL_PROMPT1,   {8,1,-1},    "Prompt-1"    },
    {PROTO_Z3950,   CLASS_ACCFORM, VAL_DES1,      {8,2,-1},    "Des-1"       },
    {PROTO_Z3950,   CLASS_ACCFORM, VAL_KRB1,      {8,3,-1},    "Krb-1"       },
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_PRESSET,   {9,1,-1},    "Pers. set"   },
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_PQUERY,    {9,2,-1},    "Pers. query" },
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_PCQUERY,   {9,3,-1},    "Per'd query" },
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_ITEMORDER, {9,4,-1},    "Item order"  },
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_DBUPDATE,  {9,5,-1},    "DB. Update"  },
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_EXPORTSPEC,{9,6,-1},    "exp. spec."  },
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_EXPORTINV, {9,7,-1},    "exp. inv."   },
    {PROTO_Z3950,   CLASS_ELEMSPEC,VAL_ESPEC1,    {11,1,-1},   "Espec-1"     },
    {PROTO_Z3950,   CLASS_VARSET,  VAL_VAR1,      {12,1,-1},   "Variant-1"   },

    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_WAIS,      {13,1,-1},   "WAIS-schema" },
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_GILS,      {13,2,-1},   "GILS-schema" },

    {PROTO_Z3950,   CLASS_TAGSET,  VAL_SETM,      {14,1,-1},   "TagsetM"     },
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_SETG,      {14,2,-1},   "TagsetG"     },
#if 0
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_GILS,      {14,3,-1},   "GILS-tagset" },
#endif

    /* SR definitions. Note that some of them aren't defined by the
        standard (yet), but are borrowed from Z3950v3 */
    {PROTO_SR,      CLASS_ABSYN,   VAL_APDU,      {2,1,-1},    "SR-APDU"     },
    {PROTO_SR,      CLASS_APPCTX,  VAL_BASIC_CTX, {1,1,-1},    "SR-BASIC"    },
    {PROTO_SR,      CLASS_ATTSET,  VAL_BIB1,      {3,1,-1},    "Bib-1"       },
    {PROTO_SR,      CLASS_ATTSET,  VAL_EXP1,      {3,2,-1},    "Exp-1"       },
    {PROTO_SR,      CLASS_ATTSET,  VAL_EXT1,      {3,3,-1},    "Ext-1"       },
    {PROTO_SR,      CLASS_ATTSET,  VAL_CCL1,      {3,4,-1},    "CCL-1"       },
    {PROTO_SR,      CLASS_ATTSET,  VAL_GILS,      {3,5,-1},    "GILS"        },
    {PROTO_SR,      CLASS_ATTSET,  VAL_STAS,      {3,6,-1},    "STAS",       },
    {PROTO_SR,      CLASS_DIAGSET, VAL_BIB1,      {4,1,-1},    "Bib-1"       },
    {PROTO_SR,      CLASS_DIAGSET, VAL_DIAG1,     {4,2,-1},    "Diag-1"      },
    {PROTO_SR,      CLASS_RECSYN,  VAL_UNIMARC,   {5,1,-1},    "Unimarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_INTERMARC, {5,2,-1},    "Intermarc"   },
    {PROTO_SR,      CLASS_RECSYN,  VAL_CCF,       {5,3,-1},     "CCF"        },
    {PROTO_SR,      CLASS_RECSYN,  VAL_USMARC,    {5,10,-1},   "USmarc"      },
    {PROTO_SR,      CLASS_RECSYN,  VAL_UKMARC,    {5,11,-1},   "UKmarc"      },
    {PROTO_SR,      CLASS_RECSYN,  VAL_NORMARC,   {5,12,-1},   "Normarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_LIBRISMARC,{5,13,-1},   "Librismarc"  },
    {PROTO_SR,      CLASS_RECSYN,  VAL_DANMARC,   {5,14,-1},   "Danmarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_FINMARC,   {5,15,-1},   "Finmarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_MAB,       {5,16,-1},   "MAB"         },
    {PROTO_SR,      CLASS_RECSYN,  VAL_CANMARC,   {5,17,-1},   "Canmarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_MAB,       {5,16,-1},   "MAB"         },
    {PROTO_SR,      CLASS_RECSYN,  VAL_CANMARC,   {5,17,-1},   "Canmarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_SBN,       {5,18,-1},   "SBN"         },
    {PROTO_SR,      CLASS_RECSYN,  VAL_PICAMARC,  {5,19,-1},   "Picamarc"    },
    {PROTO_SR,      CLASS_RECSYN,  VAL_AUSMARC,   {5,20,-1},   "Ausmarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_IBERMARC,  {5,21,-1},   "Ibermarc"    },
    {PROTO_SR,      CLASS_RECSYN,  VAL_EXPLAIN,   {5,100,-1},  "Explain"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_SUTRS,     {5,101,-1},  "SUTRS"       },
    {PROTO_SR,      CLASS_RECSYN,  VAL_OPAC,      {5,102,-1},  "OPAC"        },
    {PROTO_SR,      CLASS_RECSYN,  VAL_SUMMARY,   {5,103,-1},  "Summary"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_GRS0,      {5,104,-1},  "GRS-0"       },
    {PROTO_SR,      CLASS_RECSYN,  VAL_GRS1,      {5,105,-1},  "GRS-1"       },
    {PROTO_SR,      CLASS_RECSYN,  VAL_EXTENDED,  {5,106,-1},  "Extended"    },
    {PROTO_SR,      CLASS_RESFORM, VAL_RESOURCE1, {7,1,-1},    "Resource-1"  },
    {PROTO_SR,      CLASS_RESFORM, VAL_RESOURCE2, {7,2,-1},    "Resource-2"  },
    {PROTO_SR,      CLASS_ACCFORM, VAL_PROMPT1,   {8,1,-1},    "Prompt-1"    },
    {PROTO_SR,      CLASS_ACCFORM, VAL_DES1,      {8,2,-1},    "Des-1"       },
    {PROTO_SR,      CLASS_ACCFORM, VAL_KRB1,      {8,3,-1},    "Krb-1"       },
    {PROTO_SR,      CLASS_EXTSERV, VAL_PRESSET,   {9,1,-1},    "Pers. set"   },
    {PROTO_SR,      CLASS_EXTSERV, VAL_PQUERY,    {9,2,-1},    "Pers. query" },
    {PROTO_SR,      CLASS_EXTSERV, VAL_PCQUERY,   {9,3,-1},    "Per'd query" },
    {PROTO_SR,      CLASS_EXTSERV, VAL_ITEMORDER, {9,4,-1},    "Item order"  },
    {PROTO_SR,      CLASS_EXTSERV, VAL_DBUPDATE,  {9,5,-1},    "DB. Update"  },
    {PROTO_SR,      CLASS_EXTSERV, VAL_EXPORTSPEC,{9,6,-1},    "exp. spec."  },
    {PROTO_SR,      CLASS_EXTSERV, VAL_EXPORTINV, {9,7,-1},    "exp. inv."   },
    {PROTO_SR,      CLASS_ELEMSPEC,VAL_ESPEC1,    {11,1,-1},   "Espec-1"     },
    {PROTO_SR,      CLASS_VARSET,  VAL_VAR1,      {12,1,-1},   "Variant-1"   },

    {PROTO_SR,      CLASS_SCHEMA,  VAL_WAIS,      {13,1,-1},   "WAIS-schema" },
    {PROTO_SR,      CLASS_SCHEMA,  VAL_GILS,      {13,2,-1},   "GILS-schema" },

    {PROTO_SR,      CLASS_TAGSET,  VAL_SETM,      {14,1,-1},   "TagsetM"     },
    {PROTO_SR,      CLASS_TAGSET,  VAL_SETG,      {14,2,-1},   "TagsetG"     },
#if 0
    {PROTO_SR,      CLASS_TAGSET,  VAL_GILS,      {14,3,-1},   "GILS-tagset" },
#endif

    {0,             0,             0,             {-1},        0          }
};

/* OID utilities */

void oid_oidcpy(int *t, int *s)
{
    while ((*(t++) = *(s++)) > -1);
}

void oid_oidcat(int *t, int *s)
{
    while (*t > -1)
        t++;
    while ((*(t++) = *(s++)) > -1);
}

int oid_oidcmp(int *o1, int *o2)
{
    while (*o1 == *o2 && *o1 > -1)
    {
        o1++;
        o2++;
    }
    if (*o1 == *o2)
        return 0;
    else if (*o1 > *o2)
        return 1;
    else
        return -1;
}

int oid_oidlen(int *o)
{
    int len = 0;

    while (*(o++) >= 0)
        len++;
    return len;
}


static int match_prefix(int *look, int *prefix)
{
    int len;

    for (len = 0; *look == *prefix; look++, prefix++, len++);
    if (*prefix < 0) /* did we reach the end of the prefix? */
        return len;
    return 0;
}

struct oident *oid_getentbyoid(int *o)
{
    enum oid_proto proto;
    int prelen;
    oident *p;

    /* determine protocol type */
    if (!o)
        return 0;
    if ((prelen = match_prefix(o, z3950_prefix)))
        proto = PROTO_Z3950;
    else if ((prelen = match_prefix(o, sr_prefix)))
        proto = PROTO_SR;
    else
        proto = PROTO_GENERAL;
    for (p = oids; *p->oidsuffix >= 0; p++)
        if (p->proto == proto && !oid_oidcmp(o + prelen, p->oidsuffix))
            return p;
    return 0;
}

/*
 * To query, fill out proto, class, and value of the ent parameter.
 */
int *oid_getoidbyent(struct oident *ent)
{
    struct oident *p;
    static int ret[OID_SIZE];

    for (p = oids; *p->oidsuffix >= 0; p++)
        if (ent->proto == p->proto &&
            ent->oclass == p->oclass &&
            ent->value == p->value)
        {
            if (ent->proto == PROTO_Z3950)
                oid_oidcpy(ret, z3950_prefix);
            else if (ent->proto == PROTO_SR)
                oid_oidcpy(ret, sr_prefix);
            else
                ret[0] = -1;
            oid_oidcat(ret, p->oidsuffix);
            return ret;
        }
    return 0;
}

oid_value oid_getvalbyname(char *name)
{
    struct oident *p;

    for (p = oids; *p->oidsuffix >= 0; p++)
        if (!strcmp(p->desc, name))
            return p->value;
    return VAL_NONE;
}
