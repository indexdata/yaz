/*
 * Copyright (c) 1995-2000, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_map.c,v $
 * Revision 1.18  2000-11-29 14:22:47  adam
 * Implemented XML/SGML attributes for data1 so that d1_read reads them
 * and d1_write generates proper attributes for XML/SGML records. Added
 * register locking for threaded version.
 *
 * Revision 1.17  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.16  1999/10/21 12:06:29  adam
 * Retrieval module no longer uses ctype.h - functions.
 *
 * Revision 1.15  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.14  1998/10/13 16:09:50  adam
 * Added support for arbitrary OID's for tagsets, schemas and attribute sets.
 * Added support for multiple attribute set references and tagset references
 * from an abstract syntax file.
 * Fixed many bad logs-calls in routines that read the various
 * specifications regarding data1 (*.abs,*.att,...) and made the messages
 * consistent whenever possible.
 * Added extra 'lineno' argument to function readconf_line.
 *
 * Revision 1.13  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.12  1997/11/18 09:51:09  adam
 * Removed element num_children from data1_node. Minor changes in
 * data1 to Explain.
 *
 * Revision 1.11  1997/09/17 12:10:36  adam
 * YAZ version 1.4.
 *
 * Revision 1.10  1997/09/05 09:50:56  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.9  1996/06/10 08:56:02  quinn
 * Work on Summary.
 *
 * Revision 1.8  1996/05/01  12:45:31  quinn
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

    if (!(f = yaz_path_fopen(data1_get_tabpath(dh), file, "r")))
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
		if ((np = sscanf(path, "(%d,%[^)]):%[^/]", &type, valstr,
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
			    cur = data1_mk_node_type (dh, mem, DATA1N_tag);
			    cur->u.tag.tag = mt->value.string;

			    cur->parent = pn;
			    cur->root = pn->root;
			    if (!pn->child)
				pn->child = cur;
			    if (pn->last_child)
				pn->last_child->next = cur;
			    pn->last_child = cur;
			}
			
			if (mt->next)
			    pn = cur;
			else if (!m->no_data)
			{
			    cur->child = c->child;
			    cur->last_child = c->last_child;
			    c->child = 0;
			    c->last_child = 0;
			}
		    }
		    break;
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
    data1_node *res = data1_mk_node(dh, m);

    res->which = DATA1N_root;
    res->u.root.type = map->target_absyn_name;
    if (!(res->u.root.absyn = data1_get_absyn(dh, map->target_absyn_name)))
    {
	yaz_log(LOG_WARN, "%s: Failed to load target absyn '%s'",
		map->name, map->target_absyn_name);
    }
    res->parent = 0;
    res->root = res;

    if (map_children(dh, n, map, res, m) < 0)
    {
	data1_free_tree(dh, res);
	return 0;
    }
    return res;
}

