/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: d1_handle.c,v 1.8 2002-04-05 12:46:07 adam Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <yaz/log.h>
#include <yaz/data1.h>

struct data1_handle_info {
    WRBUF wrbuf;
    char *tab_path;
    char *tab_root;

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
    p->tab_root = NULL;
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
    if (dh->tab_root)
	xfree (dh->tab_root);
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
    xfree (dp->tab_path);
    dp->tab_path = NULL;
    if (p)
        dp->tab_path = xstrdup (p);
}

void data1_set_tabroot (data1_handle dp, const char *p)
{
    xfree (dp->tab_root);
    dp->tab_root = NULL;
    if (p)
        dp->tab_root = xstrdup (p);
}

const char *data1_get_tabpath (data1_handle dp)
{
    return dp->tab_path;
}

const char *data1_get_tabroot (data1_handle dp)
{
    return dp->tab_root;
}

FILE *data1_path_fopen (data1_handle dh, const char *file, const char *mode)
{
    const char *path = data1_get_tabpath(dh);
    const char *root = data1_get_tabroot(dh);
    return yaz_fopen (path, file, "r", root);
}
