/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_map.c,v $
 * Revision 1.8  1996-05-01 12:45:31  quinn
 * Support use of local tag names in abs file.
 *
 * Revision 1.7  1995/12/13  13:44:31  quinn
 * Modified Data1-system to use nmem
 *
 * Revision 1.6  1995/12/12  16:37:08  quinn
 * Added destroy element to data1_node.
 *
 * Revision 1.5  1995/12/12  14:11:31  quinn
 * More work on the large-record problem.
 *
 * Revision 1.4  1995/12/11  15:22:37  quinn
 * Added last_child field to the node.
 * Rewrote schema-mapping.
 *
 * Revision 1.3  1995/11/01  16:34:56  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.2  1995/11/01  13:54:46  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:08  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oid.h>
#include <xmalloc.h>
#include <log.h>
#include <readconf.h>

#include <tpath.h>
#include <data1.h>
#include "d1_map.h"

data1_maptab *data1_read_maptab(char *file)
{
    data1_maptab *res = xmalloc(sizeof(*res));
    FILE *f;
    int argc;
    char *argv[50], line[512];
    data1_mapunit **mapp;

    if (!(f = yaz_path_fopen(data1_tabpath, file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }

    res->name = 0;
    res->target_absyn_ref = ODR_NONE;
    res->map = 0;
    mapp = &res->map;
    res->next = 0;

    while ((argc = readconf_line(f, line, 512, argv, 50)))
	if (!strcmp(argv[0], "targetref"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s: one argument required for targetref",
		    file);
		continue;
	    }
	    if ((res->target_absyn_ref = oid_getvalbyname(argv[1])) == ODR_NONE)
	    {
		logf(LOG_WARN, "%s: Unknown reference '%s'", file, argv[1]);
		continue;
	    }
	}
	else if (!strcmp(argv[0], "targetname"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s: one argument required for targetref",
		    file);
		continue;
	    }
	    res->target_absyn_name = xmalloc(strlen(argv[1])+1);
	    strcpy(res->target_absyn_name, argv[1]);
	}
	else if (!strcmp(argv[0], "name"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s: one argument required for name",
		    file);
		continue;
	    }
	    res->name = xmalloc(strlen(argv[1])+1);
	    strcpy(res->name, argv[1]);
	}
	else if (!strcmp(argv[0], "map"))
	{
	    data1_maptag **mtp;
	    char *ep, *path = argv[2];

	    if (argc < 3)
	    {
		logf(LOG_WARN, "%s: At least 2 arguments required for map",
		    file);
		continue;
	    }
	    *mapp = xmalloc(sizeof(**mapp));
	    (*mapp)->next = 0;
	    if (argc > 3 && !data1_matchstr(argv[3], "nodata"))
		(*mapp)->no_data = 1;
	    else
		(*mapp)->no_data = 0;
	    (*mapp)->source_element_name = xmalloc(strlen(argv[1])+1);
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
		if ((np = sscanf(path, "(%d,%[^)]):%[^/]", &type, valstr,
		    parm)) < 2)
		{
		    logf(LOG_WARN, "%s: Syntax error in map directive: %s",
		        file, argv[2]);
		    fclose(f);
		    return 0;
		}
		*mtp = xmalloc(sizeof(**mtp));
		(*mtp)->next = 0;
		(*mtp)->type = type;
		if (np > 2 && !data1_matchstr(parm, "new"))
		    (*mtp)->new_field = 1;
		else
		    (*mtp)->new_field = 0;
#if 0
		if ((numval = atoi(valstr)))
		{
		    (*mtp)->which = D1_MAPTAG_numeric;
		    (*mtp)->value.numeric = numval;
		}
		else
		{
#endif
		    (*mtp)->which = D1_MAPTAG_string;
		    (*mtp)->value.string = xmalloc(strlen(valstr)+1);
		    strcpy((*mtp)->value.string, valstr);
#if 0
		}
#endif 
		mtp = &(*mtp)->next;
	    }
	    mapp = &(*mapp)->next;
	}
	else 
	    logf(LOG_WARN, "%s: Unknown directive '%s'", argv[0]);

    fclose(f);
    return res;
}

#if 1

/*
 * Locate node with givel elementname.
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

static int map_children(data1_node *n, data1_maptab *map, data1_node *res,
    NMEM mem)
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
			    cur = data1_mk_node(mem);
			    cur->which = DATA1N_tag;
			    cur->u.tag.element = 0;
			    cur->u.tag.tag = mt->value.string;
			    cur->u.tag.node_selected = 0;
			    cur->parent = pn;
			    cur->root = pn->root;
			    if (!pn->child)
				pn->child = cur;
			    if (pn->last_child)
				pn->last_child->next = cur;
			    pn->last_child = cur;
			    pn->num_children++;
			}
			
			if (mt->next)
			    pn = cur;
			else if (!m->no_data)
			{
			    cur->child = c->child;
			    cur->last_child = c->last_child;
			    cur->num_children = c->num_children;
			    c->child = 0;
			    c->last_child = 0;
			    c->num_children = 0;
			}
		    }
		    break;
		}
	    }
	    if (map_children(c, map, res, mem) < 0)
		return -1;
	}
    return 0;
}


#else

/*
 * See if the node n is equivalent to the tag t.
 */
static int tagmatch(data1_node *n, data1_maptag *t)
{
    if (n->which != DATA1N_tag)
	return 0;
    if (n->u.tag.element)
    {
	if (n->u.tag.element->tag->tagset->type != t->type)
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

static int map_elements(data1_node *res, data1_node *n, data1_mapunit *m)
{
    data1_node *c;

    for (c = n->child; c; c = c->next)
    {
	if (c->which == DATA1N_tag)
	{
	    if (c->u.tag.element && !data1_matchstr(c->u.tag.element->name,
		m->source_element_name))
	    {
		/* Process target path specification */
		data1_maptag *mt;
		data1_node *pn = res, *cur = pn->last_child;

		for (mt = m->target_path; mt; mt = mt->next)
		{
		    if (!cur || !tagmatch(cur, mt))
		    {
			cur = data1_mk_node();
			cur->which = DATA1N_tag;
			cur->u.tag.element = 0;
			cur->u.tag.tag = mt->value.string;
			cur->u.tag.node_selected = 0;
			cur->parent = pn;
			cur->root = pn->root;
			if (!pn->child)
			    pn->child = cur;
			if (pn->last_child)
			    pn->last_child->next = cur;
			pn->last_child = cur;
			pn->num_children++;
		    }
		    if (mt->next)
			pn = cur;
		    else if (!m->no_data)
		    {
			cur->child = c->child;
			cur->num_children = c->num_children;
			c->child = 0;
			c->num_children = 0;
		    }
		}
	    }
	    else if (map_elements(res, c, m) < 0)
		return -1;
	}
    }
    return 0;
}

static int map_record(data1_node *res, data1_node *n, data1_maptab *map)
{
    data1_mapunit *m;

    for (m = map->map; m; m = m->next)
	if (map_elements(res, n, m) < 0)
	    return -1;
    return 0;
}

#endif

/*
 * Create a (possibly lossy) copy of the given record based on the
 * table. The new copy will refer back to the data of the original record,
 * which should not be discarded during the lifetime of the copy.
 */
data1_node *data1_map_record(data1_node *n, data1_maptab *map, NMEM m)
{
    data1_node *res = data1_mk_node(m);

    res->which = DATA1N_root;
    res->u.root.type = map->target_absyn_name;
    if (!(res->u.root.absyn = data1_get_absyn(map->target_absyn_name)))
    {
	logf(LOG_WARN, "%s: Failed to load target absyn '%s'",
	    map->name, map->target_absyn_name);
    }
    res->parent = 0;
    res->root = res;

    if (map_children(n, map, res, m) < 0)
    {
	data1_free_tree(res);
	return 0;
    }
    return res;
}

