/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 *
 * $Log: oid.c,v $
 * Revision 1.44  2000-10-02 13:58:50  adam
 * Added some OID's.
 *
 * Revision 1.43  2000/03/14 09:21:08  ian
 * Added Admin Extended Service OID
 *
 * Revision 1.42  2000/02/29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.41  2000/01/10 15:16:53  adam
 * Added several OID's.
 *
 * Revision 1.40  2000/01/06 14:59:13  adam
 * Added oid_init/oid_exit. Changed oid_exit.
 *
 * Revision 1.39  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 * Revision 1.38  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.37  1999/09/13 12:51:15  adam
 * Added CLIENT IP OID.
 *
 * Revision 1.36  1999/05/27 13:02:20  adam
 * Assigned OID for old DB Update (VAL_DBUPDATE0).
 *
 * Revision 1.35  1999/04/20 09:56:49  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.34  1999/04/15 09:19:43  adam
 * Added COOKIE UserInfo OID.
 *
 * Revision 1.33  1999/04/09 12:16:11  adam
 * Added OtherInfo private OID proxy.
 *
 * Revision 1.32  1999/02/18 10:30:46  quinn
 * Changed ES: Update OID
 *
 * Revision 1.31  1998/12/03 11:33:05  adam
 * Added OID's for XML.
 *
 * Revision 1.30  1998/10/18 07:48:56  adam
 * Fixed oid_getentbyoid so that it returns NULL when parsed oid is NULL.
 *
 * Revision 1.29  1998/10/14 13:32:35  adam
 * Added include of string.h.
 *
 * Revision 1.28  1998/10/13 16:01:53  adam
 * Implemented support for dynamic object identifiers.
 * Function oid_getvalbyname now accepts raw OID's as well as traditional
 * names.
 *
 * Revision 1.27  1998/05/18 10:10:02  adam
 * Added Explain-schema and Explain-tagset to OID database.
 *
 * Revision 1.26  1998/03/20 14:46:06  adam
 * Added UNIverse Resource Reports.
 *
 * Revision 1.25  1998/02/10 15:32:03  adam
 * Added new Object Identifiers.
 *
 * Revision 1.24  1997/09/29 13:19:00  adam
 * Added function, oid_ent_to_oid, to replace the function
 * oid_getoidbyent, which is not thread safe.
 *
 * Revision 1.23  1997/09/09 10:10:19  adam
 * Another MSV5.0 port. Changed projects to include proper
 * library/include paths.
 * Server starts server in test-mode when no options are given.
 *
 * Revision 1.22  1997/08/29 13:34:58  quinn
 * Added thesaurus oids
 *
 * Revision 1.21  1997/08/19 08:46:05  quinn
 * Added Thesaurus OID
 *
 * Revision 1.20  1997/07/28 12:34:43  adam
 * Added new OID entries (RVDM).
 *
 * Revision 1.19  1997/05/02 08:39:41  quinn
 * Support for private OID table added. Thanks to Ronald van der Meer
 *
 * Revision 1.18  1997/04/30 08:52:12  quinn
 * Null
 *
 * Revision 1.17  1996/10/10  12:35:23  quinn
 * Added Update extended service.
 *
 * Revision 1.16  1996/10/09  15:55:02  quinn
 * Added SearchInfoReport
 *
 * Revision 1.15  1996/10/07  15:29:43  quinn
 * Added SOIF support
 *
 * Revision 1.14  1996/02/20  17:58:28  adam
 * Added const to oid_getvalbyname.
 *
 * Revision 1.13  1996/02/20  16:37:33  quinn
 * Using yaz_matchstr in oid_getvalbyname
 *
 * Revision 1.12  1996/01/02  08:57:53  quinn
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
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <yaz/oid.h>
#include <yaz/yaz-util.h>

static int z3950_prefix[] = { 1, 2, 840, 10003, -1 };
static int sr_prefix[]    = { 1, 0, 10163, -1 };

struct oident_list {
    struct oident oident;
    struct oident_list *next;
};

static struct oident_list *oident_table = NULL;
static int oid_value_dynamic = VAL_DYNAMIC;
static int oid_init_flag = 0;

/*
 * OID database
 */
static oident oids[] =
{
    /* General definitions */
    {PROTO_GENERAL, CLASS_TRANSYN, VAL_BER,          {2,1,1,-1},
     "BER" },
    {PROTO_GENERAL, CLASS_TRANSYN, VAL_ISO2709,      {1,0,2709,1,1,-1},
     "ISO2709"},
    {PROTO_GENERAL, CLASS_GENERAL, VAL_ISO_ILL_1,    {1,2,10161,2,1,-1},
     "ISOILL-1"},
    /* Z39.50v3 definitions */
    {PROTO_Z3950,   CLASS_ABSYN,   VAL_APDU,         {2,1,-1},
     "Z-APDU"},    
    {PROTO_Z3950,   CLASS_APPCTX,  VAL_BASIC_CTX,    {1,1,-1},
     "Z-BASIC"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_BIB1,         {3,1,-1},
     "Bib-1"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_EXP1,         {3,2,-1},
     "Exp-1"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_EXT1,         {3,3,-1},
     "Ext-1"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_CCL1,         {3,4,-1},
     "CCL-1"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_GILS,         {3,5,-1},
     "GILS-attset"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_STAS,         {3,6,-1},
     "STAS-attset"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_COLLECT1,     {3,7,-1},
     "Collections-attset"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_CIMI1,        {3,8,-1},
     "CIMI-attset"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_GEO,          {3,9,-1},
     "Geo-attset"},

    {PROTO_Z3950,   CLASS_ATTSET,  VAL_ZBIG,         {3,10,-1},
     "ZBIG"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_UTIL,         {3,11,-1},
     "Util"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_XD1,          {3,12,-1},
     "XD-1"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_ZTHES,        {3,13,-1},
     "Zthes"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_FIN1,         {3,14,-1},
     "Fin-1"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_DAN1,         {3,15,-1},
     "Dan-1"},
    {PROTO_Z3950,   CLASS_ATTSET,  VAL_HOLDINGS,     {3,16,-1},
     "Holdings"},

    {PROTO_Z3950,   CLASS_ATTSET,  VAL_THESAURUS,    {3,1000,81,1,-1},
     "Thesaurus-attset"},
    {PROTO_Z3950,   CLASS_DIAGSET, VAL_BIB1,         {4,1,-1},
     "Bib-1"},
    {PROTO_Z3950,   CLASS_DIAGSET, VAL_DIAG1,        {4,2,-1},
     "Diag-1"},
    {PROTO_Z3950,   CLASS_DIAGSET, VAL_DIAG_ES,      {4,3,-1},
     "Diag-ES"},
    {PROTO_Z3950,   CLASS_DIAGSET, VAL_DIAG_GENERAL, {4,3,-1},
     "Diag-General"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_UNIMARC,      {5,1,-1},
     "Unimarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_INTERMARC,    {5,2,-1},
     "Intermarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_CCF,          {5,3,-1},
     "CCF"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_USMARC,       {5,10,-1},
     "USmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_UKMARC,       {5,11,-1},
     "UKmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_NORMARC,      {5,12,-1},
     "Normarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_LIBRISMARC,   {5,13,-1},
     "Librismarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_DANMARC,      {5,14,-1},
     "Danmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_FINMARC,      {5,15,-1},
     "Finmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_MAB,          {5,16,-1},
     "MAB"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_CANMARC,      {5,17,-1},
     "Canmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SBN,          {5,18,-1},
     "SBN"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_PICAMARC,     {5,19,-1},
     "Picamarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_AUSMARC,      {5,20,-1},
     "Ausmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_IBERMARC,     {5,21,-1},
     "Ibermarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_CATMARC,      {5,22,-1},
     "Carmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_MALMARC,      {5,23,-1},
     "Malmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_JPMARC,       {5,24,-1},
     "JPmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SWEMARC,      {5,25,-1},
     "SWEmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SIGLEMARC,    {5,26,-1},
     "SIGLEmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_ISDSMARC,     {5,27,-1},
     "ISDSmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_RUSMARC,      {5,28,-1},
     "RUSmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_HUNMARC,      {5,29,-1},
     "Hunmarc"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_EXPLAIN,      {5,100,-1},
     "Explain"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SUTRS,        {5,101,-1},
     "SUTRS"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_OPAC,         {5,102,-1},
     "OPAC"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SUMMARY,      {5,103,-1},
     "Summary"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_GRS0,         {5,104,-1},
     "GRS-0"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_GRS1,         {5,105,-1},
     "GRS-1"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_EXTENDED,     {5,106,-1},
     "Extended"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_FRAGMENT,     {5,107,-1},
     "Fragment"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_PDF,          {5,109,1,-1},
     "pdf"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_POSTSCRIPT,   {5,109,2,-1},
     "postscript"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_HTML,         {5,109,3,-1},
     "html"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_TIFF,         {5,109,4,-1},
     "tiff"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_GIF,          {5,109,5,-1},
     "gif"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_JPEG,         {5,109,6,-1},
     "jpeg"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_PNG,          {5,109,7,-1},
     "png"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_MPEG,         {5,109,8,-1},
     "mpeg"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SGML,         {5,109,9,-1},
     "sgml"},

    {PROTO_Z3950,   CLASS_RECSYN,  VAL_TIFFB,        {5,110,1,-1},
     "tiff-b"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_WAV,          {5,110,2,-1},
     "wav"},

    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SQLRS,        {5,111,-1},
     "SQL-RS"},
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_SOIF,         {5,1000,81,2,-1},
     "SOIF" },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_TEXT_XML,     {5,109,10,-1},
     "text-XML" },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_TEXT_XML,     {5,109,10,-1},
     "XML" },
    {PROTO_Z3950,   CLASS_RECSYN,  VAL_APPLICATION_XML, {5,109,11,-1},
     "application-XML" },
    {PROTO_Z3950,   CLASS_RESFORM, VAL_RESOURCE1,    {7,1,-1},
     "Resource-1"},
    {PROTO_Z3950,   CLASS_RESFORM, VAL_RESOURCE2,    {7,2,-1},
     "Resource-2"},
    {PROTO_Z3950,   CLASS_RESFORM, VAL_UNIVERSE_REPORT, {7,1000,81,1,-1},
     "UNIverse-Resource-Report"},

    {PROTO_Z3950,   CLASS_ACCFORM, VAL_PROMPT1,      {8,1,-1},
     "Prompt-1"},
    {PROTO_Z3950,   CLASS_ACCFORM, VAL_DES1,         {8,2,-1},
     "Des-1"},
    {PROTO_Z3950,   CLASS_ACCFORM, VAL_KRB1,         {8,3,-1},
     "Krb-1"},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_PRESSET,      {9,1,-1},
     "Pers. set"},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_PQUERY,       {9,2,-1},
     "Pers. query"},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_PCQUERY,      {9,3,-1},
     "Per'd query"},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_ITEMORDER,    {9,4,-1},
     "Item order"},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_DBUPDATE0,    {9,5,1,-1},
     "DB. Update (old version)"},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_DBUPDATE,     {9,5,1,1,-1},
     "DB. Update"},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_EXPORTSPEC,   {9,6,-1},
     "exp. spec."},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_EXPORTINV,    {9,7,-1},
     "exp. inv."},
    {PROTO_Z3950,   CLASS_EXTSERV, VAL_ADMINSERVICE, {9,81,1,-1},
     "Admin"},
    {PROTO_Z3950,   CLASS_USERINFO,VAL_SEARCHRES1,   {10,1,-1},
     "searchResult-1"},
    {PROTO_Z3950,   CLASS_USERINFO,VAL_CHARLANG,     {10,2,-1},
     "CharSetandLanguageNegotiation"},
    {PROTO_Z3950,   CLASS_USERINFO,VAL_USERINFO1,    {10,3,-1},
     "UserInfo-1"},
    {PROTO_Z3950,   CLASS_USERINFO,VAL_MULTISRCH1,   {10,4,-1},
     "MultipleSearchTerms-1"},
    {PROTO_Z3950,   CLASS_USERINFO,VAL_MULTISRCH2,   {10,5,-1},
     "MultipleSearchTerms-2"},
    {PROTO_Z3950,   CLASS_USERINFO,VAL_DATETIME,     {10,6,-1},
     "DateTime"},
    {PROTO_Z3950,   CLASS_USERINFO,VAL_PROXY,        {10,1000,81,1,-1},
     "Proxy" },
    {PROTO_Z3950,   CLASS_USERINFO,VAL_COOKIE,       {10,1000,81,2,-1},
     "Cookie" },
    {PROTO_Z3950,   CLASS_USERINFO,VAL_CLIENT_IP,    {10,1000,81,3,-1},
     "Client-IP" },
    {PROTO_Z3950,   CLASS_ELEMSPEC,VAL_ESPEC1,       {11,1,-1},
     "Espec-1"},
    {PROTO_Z3950,   CLASS_VARSET,  VAL_VAR1,         {12,1,-1},
     "Variant-1"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_WAIS,         {13,1,-1},
     "WAIS-schema"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_GILS,         {13,2,-1},
     "GILS-schema"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_COLLECT1,     {13,3,-1},
     "Collections-schema"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_GEO,          {13,4,-1},
     "Geo-schema"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_CIMI1,        {13,5,-1},
     "CIMI-schema"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_UPDATEES,     {13,6,-1},
     "Update ES"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_HOLDINGS,     {13,7,-1},
     "Holdings"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_ZTHES,        {13,8,-1},
     "Zthes"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_THESAURUS,    {13,1000,81,1,-1},
     "thesaurus-schema"},
    {PROTO_Z3950,   CLASS_SCHEMA,  VAL_EXPLAIN,      {13,1000,81,2,-1},
     "Explain-schema"},
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_SETM,         {14,1,-1},
     "TagsetM"},
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_SETG,         {14,2,-1},
     "TagsetG"},
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_STAS,         {14,3,-1},
     "STAS-tagset"},
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_GILS,         {14,4,-1},
     "GILS-tagset"},
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_COLLECT1,     {14,5,-1},
     "Collections-tagset"},
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_CIMI1,        {14,6,-1},
     "CIMI-tagset"},
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_THESAURUS,    {14,1000,81,1,-1},
     "thesaurus-tagset"},
    {PROTO_Z3950,   CLASS_TAGSET,  VAL_EXPLAIN,      {14,1000,81,2,-1},
     "Explain-tagset"},
    

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
    {PROTO_SR,      CLASS_ATTSET,  VAL_COLLECT1,  {3,7,-1},    "Collections-attset"},
    {PROTO_SR,      CLASS_ATTSET,  VAL_CIMI1,     {3,8,-1},    "CIMI-attset"},
    {PROTO_SR,      CLASS_ATTSET,  VAL_GEO,       {3,9,-1},    "Geo-attset"},

    {PROTO_SR,      CLASS_DIAGSET, VAL_BIB1,      {4,1,-1},    "Bib-1"       },
    {PROTO_SR,      CLASS_DIAGSET, VAL_DIAG1,     {4,2,-1},    "Diag-1"      },

    {PROTO_SR,      CLASS_RECSYN,  VAL_UNIMARC,   {5,1,-1},    "Unimarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_INTERMARC, {5,2,-1},    "Intermarc"   },
    {PROTO_SR,      CLASS_RECSYN,  VAL_CCF,       {5,3,-1},    "CCF"        },
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
    {PROTO_SR,      CLASS_RECSYN,  VAL_CATMARC,   {5,22,-1},   "Catmarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_MALMARC,   {5,23,-1},   "Malmarc"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_EXPLAIN,   {5,100,-1},  "Explain"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_SUTRS,     {5,101,-1},  "SUTRS"       },
    {PROTO_SR,      CLASS_RECSYN,  VAL_OPAC,      {5,102,-1},  "OPAC"        },
    {PROTO_SR,      CLASS_RECSYN,  VAL_SUMMARY,   {5,103,-1},  "Summary"     },
    {PROTO_SR,      CLASS_RECSYN,  VAL_GRS0,      {5,104,-1},  "GRS-0"       },
    {PROTO_SR,      CLASS_RECSYN,  VAL_GRS1,      {5,105,-1},  "GRS-1"       },
    {PROTO_SR,      CLASS_RECSYN,  VAL_EXTENDED,  {5,106,-1},  "Extended"    },
    {PROTO_SR,      CLASS_RECSYN,  VAL_FRAGMENT,  {5,107,-1},  "Fragment"    },

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
    {PROTO_SR,      CLASS_SCHEMA,  VAL_COLLECT1,  {13,3,-1},   "Collections-schema" },
    {PROTO_SR,      CLASS_SCHEMA,  VAL_GEO,       {13,4,-1},   "Geo-schema" },
    {PROTO_SR,      CLASS_SCHEMA,  VAL_CIMI1,     {13,5,-1},   "CIMI-schema" },

    {PROTO_SR,      CLASS_TAGSET,  VAL_SETM,      {14,1,-1},   "TagsetM"     },
    {PROTO_SR,      CLASS_TAGSET,  VAL_SETG,      {14,2,-1},   "TagsetG"     },

    {PROTO_SR,      CLASS_TAGSET,  VAL_STAS,      {14,3,-1},   "STAS-tagset" },
    {PROTO_SR,      CLASS_TAGSET,  VAL_GILS,      {14,4,-1},   "GILS-tagset" },
    {PROTO_SR,      CLASS_TAGSET,  VAL_COLLECT1,  {14,5,-1},   "Collections-tagset"},
    {PROTO_SR,      CLASS_TAGSET,  VAL_CIMI1,     {14,6,-1},   "CIMI-tagset" },

    {PROTO_NOP,     CLASS_NOP,     VAL_NOP,       {-1},        0          }
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

void oid_transfer (struct oident *oident)
{
    while (*oident->oidsuffix >= 0)
    {
	oid_addent (oident->oidsuffix, oident->proto,
		    oident->oclass,
		    oident->desc, oident->value);
	oident++;
    }
}

void oid_init (void)
{
    if (oid_init_flag)
	return;
    /* oid_transfer is thread safe, so there's nothing wrong in having
       two threads calling it simultaniously. On the other hand
       no thread may exit oid_init before all OID's bave been
       transferred - which is why checked is set after oid_transfer... 
    */
    oid_transfer (oids);
    oid_init_flag = 1;
}

void oid_exit (void)
{
    while (oident_table)
    {
	struct oident_list *this_p = oident_table;
	oident_table = oident_table->next;

	xfree (this_p->oident.desc);
	xfree (this_p);
    }
    oid_init_flag = 0;
}

static struct oident *oid_getentbyoid_x(int *o)
{
    enum oid_proto proto;
    int prelen;
    struct oident_list *ol;
    
    /* determine protocol type */
    if ((prelen = match_prefix(o, z3950_prefix)) != 0)
        proto = PROTO_Z3950;
    else if ((prelen = match_prefix(o, sr_prefix)) != 0)
        proto = PROTO_SR;
    else
        proto = PROTO_GENERAL;
    for (ol = oident_table; ol; ol = ol->next)
    {
	struct oident *p = &ol->oident;
        if (p->proto == proto && !oid_oidcmp(o + prelen, p->oidsuffix))
            return p;
	if (p->proto == PROTO_GENERAL && !oid_oidcmp (o, p->oidsuffix))
	    return p;
    }
    return 0;
}

/*
 * To query, fill out proto, class, and value of the ent parameter.
 */
int *oid_ent_to_oid(struct oident *ent, int *ret)
{
    struct oident_list *ol;
    
    oid_init ();
    for (ol = oident_table; ol; ol = ol->next)
    {
	struct oident *p = &ol->oident;
	if ((ent->proto == p->proto || p->proto == PROTO_GENERAL) &&
	    (ent->oclass == p->oclass || p->oclass == CLASS_GENERAL) &&
	    ent->value == p->value)
	{
	    if (p->proto == PROTO_Z3950)
		oid_oidcpy(ret, z3950_prefix);
	    else if (p->proto == PROTO_SR)
		oid_oidcpy(ret, sr_prefix);
	    else
		ret[0] = -1;
	    oid_oidcat(ret, p->oidsuffix);
	    ent->desc = p->desc;
	    return ret;
	}
    }
    ret[0] = -1;
    return 0;
}

/*
 * To query, fill out proto, class, and value of the ent parameter.
 */
int *oid_getoidbyent(struct oident *ent)
{
    static int ret[OID_SIZE];

    return oid_ent_to_oid (ent, ret);
}

struct oident *oid_addent (int *oid, enum oid_proto proto,
			   enum oid_class oclass,
			   const char *desc, int value)
{
    struct oident *oident;

    nmem_critical_enter ();
    oident = oid_getentbyoid_x (oid);
    if (!oident)
    {
	char desc_str[200];
	struct oident_list *oident_list;
	oident_list = (struct oident_list *) xmalloc (sizeof(*oident_list));
	oident = &oident_list->oident;
	oident->proto = proto;
	oident->oclass = oclass;

	if (!desc)
	{
	    int i;

	    sprintf (desc_str, "%d", *oid);
	    for (i = 1; oid[i] >= 0; i++)
		sprintf (desc_str+strlen(desc_str), ".%d", oid[i]);
	    desc = desc_str;
	}
	oident->desc = (char *) xmalloc (strlen(desc)+1);
	strcpy (oident->desc, desc);
	if (value == VAL_DYNAMIC)
	    oident->value = (enum oid_value) (++oid_value_dynamic);
	else
	    oident->value = (enum oid_value) value;
	oid_oidcpy (oident->oidsuffix, oid);
	oident_list->next = oident_table;
	oident_table = oident_list;
    }
    nmem_critical_leave ();
    return oident;
}

struct oident *oid_getentbyoid(int *oid)
{
    struct oident *oident;

    if (!oid)
	return 0;
    oid_init ();
    oident = oid_getentbyoid_x (oid);
    if (!oident)
	oident = oid_addent (oid, PROTO_GENERAL, CLASS_GENERAL,
			     NULL, VAL_DYNAMIC);
    return oident;
}

static oid_value oid_getval_raw(const char *name)
{
    int val = 0, i = 0, oid[OID_SIZE];
    struct oident *oident;
    
    while (isdigit (*name))
    {
        val = val*10 + (*name - '0');
        name++;
        if (*name == '.')
        {
            if (i < OID_SIZE-1)
                oid[i++] = val;
	    val = 0;
            name++;
        }
    }
    oid[i] = val;
    oid[i+1] = -1;
    oident = oid_addent (oid, PROTO_GENERAL, CLASS_GENERAL, NULL,
			 VAL_DYNAMIC);
    return oident->value;
}

oid_value oid_getvalbyname(const char *name)
{
    struct oident_list *ol;

    oid_init ();
    if (isdigit (*name))
        return oid_getval_raw (name);
    for (ol = oident_table; ol; ol = ol->next)
	if (!yaz_matchstr(ol->oident.desc, name))
	{
	    return ol->oident.value;
	}
    return VAL_NONE;
}

void oid_setprivateoids(oident *list)
{
    oid_transfer (list);
}
