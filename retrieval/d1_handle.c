/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_handle.c,v $
 * Revision 1.5  1999-08-27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.4  1998/05/18 13:07:05  adam
 * Changed the way attribute sets are handled by the retriaval module.
 * Extended Explain conversion / schema.
 * Modified server and client to work with ASN.1 compiled protocol handlers.
 *
 * Revision 1.3  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.2  1997/09/30 11:50:04  adam
 * Added handler data1_get_map_buf that is used by data1_nodetomarc.
 *
 * Revision 1.1  1997/09/17 12:28:24  adam
 * Introduced new 'global' data1 handle.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <log.h>
#include <data1.h>

struct data1_handle_info {
    WRBUF wrbuf;
    char *tab_path;

    char *read_buf;
    int read_len;

    data1_absyn_cache absyn_cache;
    data1_attset_cache attset_cache;

    char *map_buf;
    int map_len;

    NMEM mem;
};

data1_handle data1_create (void)
{
    data1_handle p = (data1_handle)xmalloc (sizeof(*p));
    if (!p)
	return NULL;
    p->tab_path = NULL;
    p->wrbuf = wrbuf_alloc();
    p->read_buf = NULL;
    p->read_len = 0;
    p->map_buf = NULL;
    p->map_len = 0;
    p->absyn_cache = NULL;
    p->attset_cache = NULL;
    p->mem = nmem_create ();
    return p;
}

NMEM data1_nmem_get (data1_handle dh)
{
    return dh->mem;
}

data1_absyn_cache *data1_absyn_cache_get (data1_handle dh)
{
    return &dh->absyn_cache;
}

data1_attset_cache *data1_attset_cache_get (data1_handle dh)
{
    return &dh->attset_cache;
}

void data1_destroy (data1_handle dh)
{
    if (!dh)
	return;
    wrbuf_free (dh->wrbuf, 1);
    if (dh->tab_path)
	xfree (dh->tab_path);
    if (dh->read_buf)
	xfree (dh->read_buf);
    if (dh->map_buf)
        xfree (dh->map_buf);
    nmem_destroy (dh->mem);
    
    xfree (dh);
}

WRBUF data1_get_wrbuf (data1_handle dp)
{
    return dp->wrbuf;
}

char **data1_get_read_buf (data1_handle dp, int **lenp)
{
    *lenp = &dp->read_len;
    yaz_log (LOG_DEBUG, "data1_get_read_buf lenp=%u", **lenp);
    return &dp->read_buf;
}

char **data1_get_map_buf (data1_handle dp, int **lenp)
{
    *lenp = &dp->map_len;
    yaz_log (LOG_DEBUG, "data1_get_map_buf lenp=%u", **lenp);
    return &dp->map_buf;
}

void data1_set_tabpath (data1_handle dp, const char *p)
{
    if (dp->tab_path)
    {
        xfree (dp->tab_path);
        dp->tab_path = NULL;
    }
    if (p)
    {
        dp->tab_path = (char *)xmalloc (strlen(p)+1);
        strcpy (dp->tab_path, p);
    }
}

const char *data1_get_tabpath (data1_handle dp)
{
    return dp->tab_path;
}

