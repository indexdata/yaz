/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: oid_db.c,v 1.3 2007-04-13 13:58:00 adam Exp $
 */

/**
 * \brief OID Database
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <yaz/yaz-util.h>
#include <yaz/odr.h>
#include <yaz/oid_util.h>
#include <yaz/oid_db.h>

struct yaz_oid_entry {
    int oclass;
    int oid[OID_SIZE];
    char *name;
};

#define Z3950_PREFIX 1, 2, 840, 10003

static struct yaz_oid_entry standard_list[] =
{
    /* General definitions */
    {CLASS_TRANSYN, {2, 1, 1,-1}, "BER" },
    {CLASS_TRANSYN, {1, 0, 2709, 1, 1,-1},  "ISO2709"},
    {CLASS_GENERAL, {1, 0, 10161, 2, 1,-1},  OID_STR_ILL_1 },
    {CLASS_ABSYN,  {2, 1,-1}, "Z-APDU"}, 
    {CLASS_APPCTX, {1, 1,-1}, "Z-BASIC"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 1,-1}, "Bib-1"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 2,-1}, "Exp-1"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 3,-1}, "Ext-1"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 4,-1}, "CCL-1"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 5,-1}, "GILS"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 5,-1}, "GILS-attset"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 6,-1}, "STAS-attset"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 7,-1}, "Collections-attset"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 8,-1}, "CIMI-attset"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 9,-1}, "Geo-attset"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 10,-1}, "ZBIG"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 11,-1}, "Util"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 12,-1}, "XD-1"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 13,-1}, "Zthes"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 14,-1}, "Fin-1"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 15,-1}, "Dan-1"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 16,-1}, "Holdings"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 17,-1}, "MARC"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 18,-1}, "Bib-2"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3, 19,-1}, "ZeeRex"},
    /* New applications should use Zthes-1 instead of this Satan-spawn */
    {CLASS_ATTSET, {Z3950_PREFIX, 3,1000, 81,1,-1}, "Thesaurus-attset"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3,1000, 81,2,-1}, "IDXPATH"},
    {CLASS_ATTSET, {Z3950_PREFIX, 3,1000, 81,3,-1}, "EXTLITE"},
    {CLASS_DIAGSET, {Z3950_PREFIX, 4, 1,-1}, OID_STR_BIB1},
    {CLASS_DIAGSET, {Z3950_PREFIX, 4, 2,-1}, OID_STR_DIAG1},
    {CLASS_DIAGSET, {Z3950_PREFIX, 4, 3,-1}, "Diag-ES"},
    {CLASS_DIAGSET,  {Z3950_PREFIX, 4, 3,-1}, "Diag-General"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 1,-1}, "Unimarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 2,-1}, "Intermarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 3,-1}, "CCF"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 10,-1}, OID_STR_USMARC},
    /* MARC21 is just an alias for the original USmarc */
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 10,-1}, "MARC21"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 11,-1}, "UKmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 12,-1}, "Normarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 13,-1}, "Librismarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 14,-1}, "Danmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 15,-1}, "Finmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 16,-1}, "MAB"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 17,-1}, "Canmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 18,-1}, "SBN"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 19,-1}, "Picamarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 20,-1}, "Ausmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 21,-1}, "Ibermarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 22,-1}, "Carmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 23,-1}, "Malmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 24,-1}, "JPmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 25,-1}, "SWEmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 26,-1}, "SIGLEmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 27,-1}, "ISDSmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 28,-1}, "RUSmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 29,-1}, "Hunmarc"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 30,-1}, "NACSIS-CATP"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 31,-1}, "FINMARC2000"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 32,-1}, "MARC21-fin"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 100,-1}, OID_STR_EXPLAIN},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 101,-1}, OID_STR_SUTRS},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 102,-1}, OID_STR_OPAC},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 103,-1}, OID_STR_SUMMARY},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 104,-1}, "GRS-0"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 105,-1}, OID_STR_GRS1 },
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 106,-1}, OID_STR_EXTENDED},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 107,-1}, "Fragment"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,1,-1}, "pdf"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,2,-1}, OID_STR_POSTSCRIPT},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,3,-1}, OID_STR_HTML},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,4,-1}, "tiff"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,5,-1}, "gif"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,6,-1}, "jpeg"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,7,-1}, "png"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,8,-1}, "mpeg"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109,9,-1}, "sgml"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 110,1,-1}, "tiff-b"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 110,2,-1}, "wav"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 111,-1}, "SQL-RS"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 1000, 81, 2,-1}, OID_STR_SOIF},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109, 10,-1}, OID_STR_XML },
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109, 10,-1}, "application-XML"},
    {CLASS_RECSYN, {Z3950_PREFIX, 5, 109, 11,-1}, OID_STR_APPLICATION_XML },
    {CLASS_RESFORM, {Z3950_PREFIX, 7, 1,-1}, "Resource-1"},
    {CLASS_RESFORM, {Z3950_PREFIX, 7, 2,-1}, "Resource-2"},
    {CLASS_RESFORM, {Z3950_PREFIX, 7, 1000, 81, 1,-1}, "UNIverse-Resource-Report"},
    {CLASS_ACCFORM, {Z3950_PREFIX, 8, 1,-1}, "Prompt-1"},
    {CLASS_ACCFORM, {Z3950_PREFIX, 8, 2,-1}, "Des-1"},
    {CLASS_ACCFORM, {Z3950_PREFIX, 8, 3,-1}, "Krb-1"},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 1,-1}, "Pers. set"},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 2,-1}, "Pers. query"},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 3,-1}, "Per'd query"},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 4,-1},  OID_STR_ITEMORDER },
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 5,-1}, "DB. Update (first version)"},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 5,1,-1}, "DB. Update (second version)"},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 5, 1, 1,-1}, OID_STR_EXT_UPDATE},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 6,-1}, "exp. spec."},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 7,-1}, "exp. inv."},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 1000, 81, 1,-1}, OID_STR_ADMIN},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 1,-1}, "searchResult-1"},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 2,-1}, "CharSetandLanguageNegotiation"},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 3,-1},  OID_STR_USERINFO_1},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 4,-1}, "MultipleSearchTerms-1"},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 5,-1}, "MultipleSearchTerms-2"},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 6,-1}, "DateTime"},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 1000, 81, 1,-1}, OID_STR_PROXY},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 1000, 81, 2,-1}, OID_STR_COOKIE},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 1000, 81, 3,-1},  OID_STR_CLIENT_IP },
    {CLASS_ELEMSPEC, {Z3950_PREFIX, 11, 1,-1}, "Espec-1"},
    {CLASS_VARSET, {Z3950_PREFIX, 12, 1,-1}, OID_STR_VARIANT_1},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 1,-1}, "WAIS-schema"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 2,-1}, "GILS-schema"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 3,-1}, "Collections-schema"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 4,-1}, "Geo-schema"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 5,-1}, "CIMI-schema"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 6,-1}, "Update ES"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 7,-1}, "Holdings"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 8,-1}, "Zthes"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 1000, 81, 1,-1}, "thesaurus-schema"},
    {CLASS_SCHEMA, {Z3950_PREFIX, 13, 1000, 81, 2,-1}, "Explain-schema"},
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 1,-1}, "TagsetM"},
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 2,-1}, "TagsetG"},
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 3,-1}, "STAS-tagset"},
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 4,-1}, "GILS-tagset"},
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 5,-1}, "Collections-tagset"},
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 6,-1}, "CIMI-tagset"},
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 1000, 81, 1,-1}, "thesaurus-tagset"}, 
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 1000, 81, 2,-1}, "Explain-tagset"},
    {CLASS_TAGSET, {Z3950_PREFIX, 14, 8,-1}, "Zthes-tagset"},
    {CLASS_NEGOT, {Z3950_PREFIX, 15, 3,-1}, OID_STR_CHARNEG_3 },
    {CLASS_NEGOT, {Z3950_PREFIX, 15, 4,-1}, OID_STR_CHARNEG_4 },
    {CLASS_NEGOT, {Z3950_PREFIX, 15, 1000, 81, 1, -1}, OID_STR_ID_CHARSET },
    {CLASS_USERINFO, {1, 2, 840, 1003, 16,  2, -1}, "CQL"},
    {CLASS_GENERAL, {1,0,10646,1,0,2,-1}, "UCS-2"},
    {CLASS_GENERAL, {1,0,10646,1,0,4,-1}, "UCS-4"},
    {CLASS_GENERAL, {1,0,10646,1,0,5,-1}, "UTF-16"},
    {CLASS_GENERAL, {1,0,10646,1,0,8,-1}, "UTF-8"},
    {CLASS_USERINFO, {Z3950_PREFIX, 10, 1000, 17, 1, -1}, "OCLC-userInfo"},
    {CLASS_EXTSERV, {Z3950_PREFIX, 9, 1000,105,4,-1}, OID_STR_XMLES },
    {CLASS_NOP, {-1}, 0}
};

struct yaz_oid_db {
    struct yaz_oid_entry *entries;
    struct yaz_oid_db *next;
    int xmalloced;
};

struct yaz_oid_db standard_db_l = {
    standard_list, 0, 0
};

yaz_oid_db_t standard_db = &standard_db_l;

yaz_oid_db_t yaz_oid_std(void)
{
    return standard_db;
}

const int *yaz_string_to_oid(yaz_oid_db_t oid_db,
			     int oclass, const char *name)
{
    for (; oid_db; oid_db = oid_db->next)
    {
	struct yaz_oid_entry *e = oid_db->entries;
	for (; e->name; e++)
	{
	    if (!yaz_matchstr(e->name, name)
		&& (oclass == CLASS_GENERAL || oclass == e->oclass))
		return e->oid;
	}
    }
    return 0;
}

int *yaz_string_to_oid_nmem(yaz_oid_db_t oid_list,
			    int oclass, const char *name, NMEM nmem)
{
    const int *oid = yaz_string_to_oid(oid_list, oclass, name);
    if (oid)
	return odr_oiddup_nmem(nmem, oid);
    return odr_getoidbystr_nmem(nmem, name);
}

int *yaz_string_to_oid_odr(yaz_oid_db_t oid_list,
			   int oclass, const char *name, ODR o)
{
    return yaz_string_to_oid_nmem(oid_list, oclass, name, odr_getmem(o));
}

const char *yaz_oid_to_string(yaz_oid_db_t oid_db,
			      const int *oid, int *oclass)
{
    if (!oid)
	return 0;
    for (; oid_db; oid_db = oid_db->next)
    {
	struct yaz_oid_entry *e = oid_db->entries;
	for (; e->name; e++)
	{
	    if (!oid_oidcmp(e->oid, oid))
	    {
		if (oclass)
		    *oclass = e->oclass;
		return e->name;
	    }
	}
    }
    return 0;
}

const char *yaz_oid_to_string_buf(const int *oid, int *oclass, char *buf)
{
    const char *p = yaz_oid_to_string(standard_db, oid, oclass);
    if (p)
	return p;
    if (oclass)
	*oclass = CLASS_GENERAL;
    return oid_oid_to_dotstring(oid, buf);
}

int yaz_oid_is_iso2709(const int *oid)
{
    if (oid_oidlen(oid) == 6 && oid[0] == 1 && oid[1] == 2
	&& oid[2] == 840 && oid[3] == 10003 && oid[4] == 5 
	&& oid[5] <= 29 && oid[5] != 16)
	return 1;
    return 0;
}

int yaz_oid_add(yaz_oid_db_t oid_db, int oclass, const char *name,
		const int *new_oid)
{
    const int *oid = yaz_string_to_oid(oid_db, oclass, name);
    if (!oid)
    {
	struct yaz_oid_entry *ent;

	while (oid_db->next)
	    oid_db = oid_db->next;
	oid_db->next = xmalloc(sizeof(*oid_db->next));
	oid_db = oid_db->next;

	oid_db->next = 0;
	oid_db->xmalloced = 1;
	oid_db->entries = ent = xmalloc(2 * sizeof(*ent));
	ent[0].oclass = oclass;
	oid_oidcpy(ent[0].oid, new_oid);
	ent[0].name = xstrdup(name);
	ent[1].oclass = CLASS_NOP;
	ent[1].oid[0] = -1;
	ent[1].name = 0;
	return 0;
    }
    return -1;
}

yaz_oid_db_t yaz_oid_db_new(void)
{
    yaz_oid_db_t p = xmalloc(sizeof(*p));
    p->entries = 0;
    p->next = 0;
    p->xmalloced = 1;
    return p;
}

void yaz_oid_db_destroy(yaz_oid_db_t oid_db)
{
    while (oid_db)
    {
	yaz_oid_db_t p = oid_db;

	oid_db = oid_db->next;
	if (p->xmalloced)
	{
	    struct yaz_oid_entry *e = p->entries;
	    for (; e->name; e++)
		xfree (e->name);
	    xfree(p->entries);
	    xfree(p);
	}
    }
}

void yaz_oid_trav(yaz_oid_db_t oid_db,
		  void (*func)(const int *oid,
			       int oclass, const char *name,
			       void *client_data),
		  void *client_data)
{
    for (; oid_db; oid_db = oid_db->next)
    {
	struct yaz_oid_entry *e = oid_db->entries;
	
	for (; e->name; e++)
	    func(e->oid, e->oclass, e->name, client_data);
    }
}

