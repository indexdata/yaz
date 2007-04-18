/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: oid_db.c,v 1.6 2007-04-18 08:08:02 adam Exp $
 */

/**
 * \file oid_db.c
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

struct yaz_oid_db {
     struct yaz_oid_entry *entries;
     struct yaz_oid_db *next;
     int xmalloced;
};

struct yaz_oid_db standard_db_l = {
    yaz_oid_standard_entries, 0, 0
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
        struct yaz_oid_entry *e;
        if (oclass != CLASS_GENERAL)
        {
            for (e = oid_db->entries; e->name; e++)
            {
                if (!yaz_matchstr(e->name, name) && oclass == e->oclass)
                    return e->oid;
            }
        }
        for (e = oid_db->entries; e->name; e++)
        {
            if (!yaz_matchstr(e->name, name))
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
        int *alloc_oid;

	while (oid_db->next)
	    oid_db = oid_db->next;
	oid_db->next = xmalloc(sizeof(*oid_db->next));
	oid_db = oid_db->next;

	oid_db->next = 0;
	oid_db->xmalloced = 1;
	oid_db->entries = ent = xmalloc(2 * sizeof(*ent));

        alloc_oid = xmalloc(sizeof(*alloc_oid) * (oid_oidlen(new_oid)+1));
	oid_oidcpy(alloc_oid, new_oid);
        ent[0].oid = alloc_oid;
	ent[0].name = xstrdup(name);
	ent[0].oclass = oclass;

	ent[1].oid = 0;
	ent[1].name = 0;
	ent[1].oclass = CLASS_NOP;
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

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

