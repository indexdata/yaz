/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
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
    0, 0, 0
};
yaz_oid_db_t standard_db = &standard_db_l;

yaz_oid_db_t yaz_oid_std(void)
{
    return standard_db;
}

#define get_entries(db) (db->xmalloced==0 ? yaz_oid_standard_entries : db->entries)

const Odr_oid *yaz_string_to_oid(yaz_oid_db_t oid_db,
                                 oid_class oclass, const char *name)
{
    for (; oid_db; oid_db = oid_db->next)
    {
        struct yaz_oid_entry *e;
        if (oclass != CLASS_GENERAL)
        {
            for (e = get_entries(oid_db); e->name; e++)
            {
                if (!yaz_matchstr(e->name, name) && oclass == e->oclass)
                    return e->oid;
            }
        }
        for (e = get_entries(oid_db); e->name; e++)
        {
            if (!yaz_matchstr(e->name, name))
                return e->oid;
        }
    }
    return 0;
}

Odr_oid *yaz_string_to_oid_nmem(yaz_oid_db_t oid_list,
                                oid_class oclass, const char *name, NMEM nmem)
{
    const Odr_oid *oid = yaz_string_to_oid(oid_list, oclass, name);
    if (oid)
	return odr_oiddup_nmem(nmem, oid);
    return odr_getoidbystr_nmem(nmem, name);
}

Odr_oid *yaz_string_to_oid_odr(yaz_oid_db_t oid_list,
                               oid_class oclass, const char *name, ODR o)
{
    return yaz_string_to_oid_nmem(oid_list, oclass, name, odr_getmem(o));
}

const char *yaz_oid_to_string(yaz_oid_db_t oid_db,
			      const Odr_oid *oid, oid_class *oclass)
{
    if (!oid)
	return 0;
    for (; oid_db; oid_db = oid_db->next)
    {
	struct yaz_oid_entry *e = get_entries(oid_db);
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

const char *yaz_oid_to_string_buf(const Odr_oid *oid, oid_class *oclass, char *buf)
{
    const char *p = yaz_oid_to_string(yaz_oid_std(), oid, oclass);
    if (p)
	return p;
    if (oclass)
	*oclass = CLASS_GENERAL;
    return oid_oid_to_dotstring(oid, buf);
}


char *oid_name_to_dotstring(oid_class oclass, const char *name, char *oid_buf)
{
    const Odr_oid *oid = yaz_string_to_oid(yaz_oid_std(), oclass, name);
    if (oid)
        return oid_oid_to_dotstring(oid, oid_buf);
    return 0;
}


int yaz_oid_is_iso2709(const Odr_oid *oid)
{
    if (oid_oidlen(oid) == 6 && oid[0] == 1 && oid[1] == 2
	&& oid[2] == 840 && oid[3] == 10003 && oid[4] == 5 
	&& oid[5] <= 29 && oid[5] != 16)
	return 1;
    return 0;
}

int yaz_oid_add(yaz_oid_db_t oid_db, oid_class oclass, const char *name,
		const Odr_oid *new_oid)
{
    const Odr_oid *oid = yaz_string_to_oid(oid_db, oclass, name);
    if (!oid)
    {
	struct yaz_oid_entry *ent;
        Odr_oid *alloc_oid;

	while (oid_db->next)
	    oid_db = oid_db->next;
	oid_db->next = (struct yaz_oid_db *) xmalloc(sizeof(*oid_db->next));
	oid_db = oid_db->next;

	oid_db->next = 0;
	oid_db->xmalloced = 1;
	oid_db->entries = ent = (struct yaz_oid_entry *) xmalloc(2 * sizeof(*ent));

        alloc_oid = (Odr_oid *)
            xmalloc(sizeof(*alloc_oid) * (oid_oidlen(new_oid)+1));
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
    yaz_oid_db_t p = (yaz_oid_db_t) xmalloc(sizeof(*p));
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
		  void (*func)(const Odr_oid *oid,
			       oid_class oclass, const char *name,
			       void *client_data),
		  void *client_data)
{
    for (; oid_db; oid_db = oid_db->next)
    {
	struct yaz_oid_entry *e = get_entries(oid_db);
	
	for (; e->name; e++)
	    func(e->oid, e->oclass, e->name, client_data);
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

