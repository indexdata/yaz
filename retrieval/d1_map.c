/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: d1_map.c,v 1.23 2002-07-03 14:09:34 adam Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/oid.h>
#include <yaz/log.h>
#include <yaz/readconf.h>
#include <yaz/tpath.h>
#include <yaz/data1.h>
#include <yaz/d1_map.h>

data1_maptab *data1_read_maptab (data1_handle dh, const char *file)
{
    NMEM mem = data1_nmem_get (dh);
    data1_maptab *res = (data1_maptab *)nmem_malloc(mem, sizeof(*res));
    FILE *f;
    int lineno = 0;
    int argc;
    char *argv[50], line[512];
    data1_mapunit **mapp;
    int local_numeric = 0;

    if (!(f = data1_path_fopen(dh, file, "r")))
    {
	yaz_log(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }

    res->name = 0;
    res->target_absyn_ref = VAL_NONE;
    res->map = 0;
    mapp = &res->map;
    res->next = 0;

    while ((argc = readconf_line(f, &lineno, line, 512, argv, 50)))
	if (!strcmp(argv[0], "targetref"))
	{
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # args for targetref",
			file, lineno);
		continue;
	    }
	    if ((res->target_absyn_ref = oid_getvalbyname(argv[1]))
		== VAL_NONE)
	    {
		yaz_log(LOG_WARN, "%s:%d: Unknown reference '%s'",
			file, lineno, argv[1]);
		continue;
	    }
	}
	else if (!strcmp(argv[0], "targetname"))
	{
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # args for targetname",
			file, lineno);
		continue;
	    }
	    res->target_absyn_name =
		(char *)nmem_malloc(mem, strlen(argv[1])+1);
	    strcpy(res->target_absyn_name, argv[1]);
	}
	else if (!yaz_matchstr(argv[0], "localnumeric"))
	    local_numeric = 1;
	else if (!strcmp(argv[0], "name"))
	{
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # args for name", file, lineno);
		continue;
	    }
	    res->name = (char *)nmem_malloc(mem, strlen(argv[1])+1);
	    strcpy(res->name, argv[1]);
	}
	else if (!strcmp(argv[0], "map"))
	{
	    data1_maptag **mtp;
	    char *ep, *path = argv[2];

	    if (argc < 3)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args for map",
			file, lineno);
		continue;
	    }
	    *mapp = (data1_mapunit *)nmem_malloc(mem, sizeof(**mapp));
	    (*mapp)->next = 0;
	    if (argc > 3 && !data1_matchstr(argv[3], "nodata"))
		(*mapp)->no_data = 1;
	    else
		(*mapp)->no_data = 0;
	    (*mapp)->source_element_name =
		(char *)nmem_malloc(mem, strlen(argv[1])+1);
	    strcpy((*mapp)->source_element_name, argv[1]);
	    mtp = &(*mapp)->target_path;
	    if (*path == '/')
		path++;
	    for (ep = strchr(path, '/'); path; (void)((path = ep) &&
		(ep = strchr(path, '/'))))
	    {
		int type, np;
		char valstr[512], parm[512];

		if (ep)
		    ep++;
		if ((np = sscanf(path, "(%d,%511[^)]):%511[^/]", &type, valstr,
		    parm)) < 2)
		{
		    yaz_log(LOG_WARN, "%s:%d: Syntax error in map "
			    "directive: %s", file, lineno, argv[2]);
		    fclose(f);
		    return 0;
		}
		*mtp = (data1_maptag *)nmem_malloc(mem, sizeof(**mtp));
		(*mtp)->next = 0;
		(*mtp)->type = type;
		if (np > 2 && !data1_matchstr(parm, "new"))
		    (*mtp)->new_field = 1;
		else
		    (*mtp)->new_field = 0;
		if ((type != 3 || local_numeric) && d1_isdigit(*valstr))
                {
		    (*mtp)->which = D1_MAPTAG_numeric;
		    (*mtp)->value.numeric = atoi(valstr);
		}
		else
		{
		    (*mtp)->which = D1_MAPTAG_string;
		    (*mtp)->value.string =
			(char *)nmem_malloc(mem, strlen(valstr)+1);
		    strcpy((*mtp)->value.string, valstr);
		}
		mtp = &(*mtp)->next;
	    }
	    mapp = &(*mapp)->next;
	}
	else 
	    yaz_log(LOG_WARN, "%s:%d: Unknown directive '%s'",
		    file, lineno, argv[0]);

    fclose(f);
    return res;
}

/*
 * Locate node with given elementname.
 * NOTE: This is stupid - we don't find repeats this way.
 */
static data1_node *find_node(data1_node *p, char *elementname)
{
    data1_node *c, *r;

    for (c = p->child; c; c = c->next)
	if (c->which == DATA1N_tag && c->u.tag.element &&
	    !data1_matchstr(c->u.tag.element->name, elementname))
	    return c;
	else if ((r = find_node(c, elementname)))
	    return r;
    return 0;
}

/*
 * See if the node n is equivalent to the tag t.
 */
static int tagmatch(data1_node *n, data1_maptag *t)
{
    if (n->which != DATA1N_tag)
	return 0;
    if (n->u.tag.element)
    {
	if (n->u.tag.element->tag->tagset)
	{
	    if (n->u.tag.element->tag->tagset->type != t->type)
		return 0;
	}
	else if (t->type != 3)
	    return 0;
	if (n->u.tag.element->tag->which == DATA1T_numeric)
	{
	    if (t->which != D1_MAPTAG_numeric)
		return 0;
	    if (n->u.tag.element->tag->value.numeric != t->value.numeric)
		return 0;
	}
	else
	{
	    if (t->which != D1_MAPTAG_string)
		return 0;
	    if (data1_matchstr(n->u.tag.element->tag->value.string,
		t->value.string))
		return 0;
	}
    }
    else /* local tag */
    {
	char str[10];

	if (t->type != 3)
	    return 0;
	if (t->which == D1_MAPTAG_numeric)
	    sprintf(str, "%d", t->value.numeric);
	else
	    strcpy(str, t->value.string);
	if (data1_matchstr(n->u.tag.tag, str))
	    return 0;
    }
    return 1;
}

static data1_node *dup_child (data1_handle dh, data1_node *n,
                              data1_node **last, NMEM mem,
                              data1_node *parent)
{
    data1_node *first = 0;
    data1_node **m = &first;

    for (; n; n = n->next)
    {
        *last = *m = nmem_malloc (mem, sizeof(**m));
        memcpy (*m, n, sizeof(**m));
        
        (*m)->parent = parent;
        (*m)->root = parent->root;
        (*m)->child = dup_child(dh, n->child, &(*m)->last_child, mem, *m);
        m = &(*m)->next;
    }
    *m = 0;
    return first;
}

static int map_children(data1_handle dh, data1_node *n, data1_maptab *map,
			data1_node *res, NMEM mem)
{
    data1_node *c;
    data1_mapunit *m;
    /*
     * locate each source element in turn.
     */
    for (c = n->child; c; c = c->next)
	if (c->which == DATA1N_tag && c->u.tag.element)
	{
	    for (m = map->map; m; m = m->next)
	    {
		if (!data1_matchstr(m->source_element_name,
		    c->u.tag.element->name))
		{
		    data1_node *pn = res;
		    data1_node *cur = pn->last_child;
		    data1_maptag *mt;

		    /*
		     * process the target path specification.
		     */
		    for (mt = m->target_path; mt; mt = mt->next)
		    {
			if (!cur || mt->new_field || !tagmatch(cur, mt))
			{
			    cur = data1_mk_node2 (dh, mem, DATA1N_tag, pn);
			    cur->u.tag.tag = mt->value.string;
			}
			
			if (mt->next)
			    pn = cur;
			else if (!m->no_data)
			{
                            cur->child =
                                dup_child (dh, c->child,
                                           &cur->last_child, mem, cur);
			}
		    }
		}
	    }
	    if (map_children(dh, c, map, res, mem) < 0)
		return -1;
	}
    return 0;
}

/*
 * Create a (possibly lossy) copy of the given record based on the
 * table. The new copy will refer back to the data of the original record,
 * which should not be discarded during the lifetime of the copy.
 */
data1_node *data1_map_record (data1_handle dh, data1_node *n,
			      data1_maptab *map, NMEM m)
{
    data1_node *res1, *res = data1_mk_node2 (dh, m, DATA1N_root, 0);

    res->which = DATA1N_root;
    res->u.root.type = map->target_absyn_name;
    if (!(res->u.root.absyn = data1_get_absyn(dh, map->target_absyn_name)))
    {
	yaz_log(LOG_WARN, "%s: Failed to load target absyn '%s'",
		map->name, map->target_absyn_name);
    }
    n = n->child;
    if (!n)
        return 0;
    res1 = data1_mk_tag (dh, m, map->target_absyn_name, 0, res);

    if (map_children(dh, n, map, res1, m) < 0)
    {
	data1_free_tree(dh, res);
	return 0;
    }
    return res;
}

