/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: d1_absyn.c,v 1.34 2002-08-29 21:26:08 mike Exp $
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/oid.h>
#include <yaz/log.h>
#include <yaz/data1.h>

#define D1_MAX_NESTING  128

struct data1_absyn_cache_info 
{
    char *name;
    data1_absyn *absyn;
    data1_absyn_cache next;
};

struct data1_attset_cache_info 
{
    char *name;
    data1_attset *attset;
    data1_attset_cache next;
};

data1_absyn *data1_absyn_search (data1_handle dh, const char *name)
{
    data1_absyn_cache p = *data1_absyn_cache_get (dh);

    while (p)
    {
	if (!strcmp (name, p->name))
	    return p->absyn;
	p = p->next;
    }
    return NULL;
}

void data1_absyn_trav (data1_handle dh, void *handle,
		       void (*fh)(data1_handle dh, void *h, data1_absyn *a))
{
    data1_absyn_cache p = *data1_absyn_cache_get (dh);

    while (p)
    {
	(*fh)(dh, handle, p->absyn);
	p = p->next;
    }
}

data1_absyn *data1_absyn_add (data1_handle dh, const char *name)
{
    char fname[512];
    NMEM mem = data1_nmem_get (dh);

    data1_absyn_cache p = (data1_absyn_cache)nmem_malloc (mem, sizeof(*p));
    data1_absyn_cache *pp = data1_absyn_cache_get (dh);

    sprintf(fname, "%s.abs", name);
    p->absyn = data1_read_absyn (dh, fname, 0);
    p->name = nmem_strdup (mem, name);
    p->next = *pp;
    *pp = p;
    return p->absyn;
}

data1_absyn *data1_get_absyn (data1_handle dh, const char *name)
{
    data1_absyn *absyn;

    if (!(absyn = data1_absyn_search (dh, name)))
	absyn = data1_absyn_add (dh, name);
    return absyn;
}

data1_attset *data1_attset_search_name (data1_handle dh, const char *name)
{
    data1_attset_cache p = *data1_attset_cache_get (dh);

    while (p)
    {
	if (!strcmp (name, p->name))
	    return p->attset;
	p = p->next;
    }
    return NULL;
}

data1_attset *data1_attset_search_id (data1_handle dh, int id)
{
    data1_attset_cache p = *data1_attset_cache_get (dh);

    while (p)
    {
	if (id == p->attset->reference)
	    return p->attset;
	p = p->next;
    }
    return NULL;
}

data1_attset *data1_attset_add (data1_handle dh, const char *name)
{
    char fname[512], aname[512];
    NMEM mem = data1_nmem_get (dh);
    data1_attset *attset;

    strcpy (aname, name);
    sprintf(fname, "%s.att", name);
    attset = data1_read_attset (dh, fname);
    if (!attset)
    {
	char *cp;
	attset = data1_read_attset (dh, name);
	if (attset && (cp = strrchr (aname, '.')))
	    *cp = '\0';
    }
    if (!attset)
	yaz_log (LOG_WARN|LOG_ERRNO, "Couldn't load attribute set %s", name);
    else
    {
	data1_attset_cache p = (data1_attset_cache)
	    nmem_malloc (mem, sizeof(*p));
	data1_attset_cache *pp = data1_attset_cache_get (dh);
	
	attset->name = p->name = nmem_strdup (mem, aname);
	p->attset = attset;
	p->next = *pp;
	*pp = p;
    }
    return attset;
}

data1_attset *data1_get_attset (data1_handle dh, const char *name)
{
    data1_attset *attset;

    if (!(attset = data1_attset_search_name (dh, name)))
	attset = data1_attset_add (dh, name);
    return attset;
}

data1_esetname *data1_getesetbyname(data1_handle dh, data1_absyn *a,
				    const char *name)
{
    data1_esetname *r;

    for (r = a->esetnames; r; r = r->next)
	if (!data1_matchstr(r->name, name))
	    return r;
    return 0;
}

data1_element *data1_getelementbytagname (data1_handle dh, data1_absyn *abs,
					  data1_element *parent,
					  const char *tagname)
{
    data1_element *r;

    /* It's now possible to have a data1 tree with no abstract syntax */
    if ( !abs )
        return 0;

    if (!parent)
        r = abs->main_elements;
    else
	r = parent->children;

    for (; r; r = r->next)
    {
	data1_name *n;

	for (n = r->tag->names; n; n = n->next)
	    if (!data1_matchstr(tagname, n->name))
		return r;
    }
    return 0;
}

data1_element *data1_getelementbyname (data1_handle dh, data1_absyn *absyn,
				       const char *name)
{
    data1_element *r;

    /* It's now possible to have a data1 tree with no abstract syntax */
    if ( !absyn )
        return 0;
    for (r = absyn->main_elements; r; r = r->next)
	if (!data1_matchstr(r->name, name))
	    return r;
    return 0;
}


void fix_element_ref (data1_handle dh, data1_absyn *absyn, data1_element *e)
{
    /* It's now possible to have a data1 tree with no abstract syntax */
    if ( !absyn )
        return;

    for (; e; e = e->next)
    {
	if (!e->sub_name)
	{
	    if (e->children)
		fix_element_ref (dh, absyn, e->children);
	}
	else
	{
	    data1_sub_elements *sub_e = absyn->sub_elements;
	    while (sub_e && strcmp (e->sub_name, sub_e->name))
		sub_e = sub_e->next;
	    if (sub_e)
		e->children = sub_e->elements;
	    else
		yaz_log (LOG_WARN, "Unresolved reference to sub-elements %s",
		      e->sub_name);
	}
    }
}


static int parse_termlists (data1_handle dh, data1_termlist ***tpp,
			    char *p, const char *file, int lineno,
			    const char *element_name, data1_absyn *res)
{
    data1_termlist **tp = *tpp;
    do
    {
	char attname[512], structure[512];
	char *source;
	int r;
	
	if (!(r = sscanf(p, "%511[^:,]:%511[^,]", attname,
			 structure)))
	{
	    yaz_log(LOG_WARN,
		    "%s:%d: Syntax error in termlistspec '%s'",
		    file, lineno, p);
	    return -1;
	}
	if (*attname == '!')
	    strcpy(attname, element_name);
	*tp = (data1_termlist *)
	    nmem_malloc(data1_nmem_get(dh), sizeof(**tp));
	(*tp)->next = 0;
	if (!((*tp)->att = data1_getattbyname(dh, res->attset,
					      attname)))
	{
	    yaz_log(LOG_WARN,
		    "%s:%d: Couldn't find att '%s' in attset",
		    file, lineno, attname);
	    return -1;
	}
	if (r == 2 && (source = strchr(structure, ':')))
	    *source++ = '\0';   /* cut off structure .. */
	else
	    source = "data";    /* ok: default is leaf data */
	(*tp)->source = (char *)
	    nmem_strdup (data1_nmem_get (dh), source);
	
	if (r < 2) /* is the structure qualified? */
	    (*tp)->structure = "w";
	else 
	    (*tp)->structure = (char *)
		nmem_strdup (data1_nmem_get (dh), structure);
	tp = &(*tp)->next;
    }
    while ((p = strchr(p, ',')) && *(++p));
    *tpp = tp;
    return 0;
}

data1_absyn *data1_read_absyn (data1_handle dh, const char *file,
                               int file_must_exist)
{
    data1_sub_elements *cur_elements = NULL;
    data1_absyn *res = 0;
    FILE *f;
    data1_element **ppl[D1_MAX_NESTING];
    data1_esetname **esetpp;
    data1_maptab **maptabp;
    data1_marctab **marcp;
    data1_termlist *all = 0;
    data1_attset_child **attset_childp;
    data1_tagset **tagset_childp;
    int level = 0;
    int lineno = 0;
    int argc;
    char *argv[50], line[512];

    if (!(f = data1_path_fopen(dh, file, "r")))
    {
	yaz_log(LOG_WARN|LOG_ERRNO, "Couldn't open %s", file);
        if (file_must_exist)
            return 0;
    }
    
    res = (data1_absyn *) nmem_malloc(data1_nmem_get(dh), sizeof(*res));
    res->name = 0;
    res->reference = VAL_NONE;
    res->tagset = 0;
    res->encoding = 0;
    res->enable_xpath_indexing = (f ? 0 : 1);
    tagset_childp = &res->tagset;

    res->attset = data1_empty_attset (dh);
    attset_childp =  &res->attset->children;

    res->varset = 0;
    res->esetnames = 0;
    esetpp = &res->esetnames;
    res->maptabs = 0;
    maptabp = &res->maptabs;
    res->marc = 0;
    marcp = &res->marc;

    res->sub_elements = NULL;
    res->main_elements = NULL;
    
    while (f && (argc = readconf_line(f, &lineno, line, 512, argv, 50)))
    {
	char *cmd = *argv;
	if (!strcmp(cmd, "elm") || !strcmp(cmd, "element"))
	{
	    data1_element *new_element;
	    int i;
	    char *p, *sub_p, *path, *name, *termlists;
	    int type, value;
	    data1_termlist **tp;

	    if (argc < 4)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args to elm", file, lineno);
		continue;
	    }
	    path = argv[1];
	    name = argv[2];
	    termlists = argv[3];

	    if (!cur_elements)
	    {
                cur_elements = (data1_sub_elements *)
		    nmem_malloc(data1_nmem_get(dh), sizeof(*cur_elements));
	        cur_elements->next = res->sub_elements;
		cur_elements->elements = NULL;
		cur_elements->name = "main";
		res->sub_elements = cur_elements;
		
		level = 0;
    		ppl[level] = &cur_elements->elements;
            }
	    p = path;
	    for (i = 1;; i++)
	    {
		char *e;

		if ((e = strchr(p, '/')))
		    p = e+1;
		else
		    break;
	    }
	    if (i > level+1)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad level increase", file, lineno);
		fclose(f);
		return 0;
	    }
	    level = i;
	    new_element = *ppl[level-1] = (data1_element *)
		nmem_malloc(data1_nmem_get(dh), sizeof(*new_element));
	    new_element->next = new_element->children = 0;
	    new_element->tag = 0;
	    new_element->termlists = 0;
	    new_element->sub_name = 0;
	    
	    tp = &new_element->termlists;
	    ppl[level-1] = &new_element->next;
	    ppl[level] = &new_element->children;
	    
	    /* consider subtree (if any) ... */
	    if ((sub_p = strchr (p, ':')) && sub_p[1])
	    {
		*sub_p++ = '\0';
		new_element->sub_name =
		    nmem_strdup (data1_nmem_get(dh), sub_p);		
	    }
	    /* well-defined tag */
	    if (sscanf(p, "(%d,%d)", &type, &value) == 2)
	    {
		if (!res->tagset)
		{
		    yaz_log(LOG_WARN, "%s:%d: No tagset loaded", file, lineno);
		    fclose(f);
		    return 0;
		}
		if (!(new_element->tag = data1_gettagbynum (dh, res->tagset,
							    type, value)))
		{
		    yaz_log(LOG_WARN, "%s:%d: Couldn't find tag %s in tagset",
			 file, lineno, p);
		    fclose(f);
		    return 0;
		}
	    }
	    /* private tag */
	    else if (*p)
	    {
		data1_tag *nt =
		    new_element->tag = (data1_tag *)
		    nmem_malloc(data1_nmem_get (dh),
				sizeof(*new_element->tag));
		nt->which = DATA1T_string;
		nt->value.string = nmem_strdup(data1_nmem_get (dh), p);
		nt->names = (data1_name *)
		    nmem_malloc(data1_nmem_get(dh), 
				sizeof(*new_element->tag->names));
		nt->names->name = nt->value.string;
		nt->names->next = 0;
		nt->kind = DATA1K_string;
		nt->next = 0;
		nt->tagset = 0;
	    }
	    else
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad element", file, lineno);
		fclose(f);
		return 0;
	    }
	    /* parse termList definitions */
	    p = termlists;
	    if (*p != '-')
	    {
		assert (res->attset);
		
		if (parse_termlists (dh, &tp, p, file, lineno, name, res))
		{
		    fclose (f);
		    return 0;
		}
	        *tp = all; /* append any ALL entries to the list */
	    }
	    new_element->name = nmem_strdup(data1_nmem_get (dh), name);
	}
 	else if (!strcmp(cmd, "section"))
	{
	    char *name;
	    
	    if (argc < 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args to section",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    
            cur_elements = (data1_sub_elements *)
		nmem_malloc(data1_nmem_get(dh), sizeof(*cur_elements));
	    cur_elements->next = res->sub_elements;
	    cur_elements->elements = NULL;
	    cur_elements->name = nmem_strdup (data1_nmem_get(dh), name);
	    res->sub_elements = cur_elements;
	    
	    level = 0;
    	    ppl[level] = &cur_elements->elements;
	}
        else if (!strcmp(cmd, "xpath"))
        {
            if (argc != 2)
            {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args to 'xpath' directive",
		     file, lineno);
		continue;
            }
            if (!strcmp(argv[1], "enable"))
                res->enable_xpath_indexing = 1;
            else if (!strcmp (argv[1], "disable"))
                res->enable_xpath_indexing = 0;
            else
            {
		yaz_log(LOG_WARN, "%s:%d: Expecting disable/enable "
                        "after 'xpath' directive", file, lineno);
            }
        }
	else if (!strcmp(cmd, "all"))
	{
	    data1_termlist **tp = &all;
	    if (all)
	    {
		yaz_log(LOG_WARN, "%s:%d: Too many 'all' directives - ignored",
		     file, lineno);
		continue;
	    }
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args to 'all' directive",
		     file, lineno);
		continue;
	    }
	    if (parse_termlists (dh, &tp, argv[1], file, lineno, 0, res))
	    {
		fclose (f);
		return 0;
	    }
	}
	else if (!strcmp(cmd, "name"))
	{
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args to name directive",
		     file, lineno);
		continue;
	    }
	    res->name = nmem_strdup(data1_nmem_get(dh), argv[1]);
	}
	else if (!strcmp(cmd, "reference"))
	{
	    char *name;
	    
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args to reference",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    if ((res->reference = oid_getvalbyname(name)) == VAL_NONE)
	    {
		yaz_log(LOG_WARN, "%s:%d: Unknown tagset ref '%s'", 
		     file, lineno, name);
		continue;
	    }
	}
	else if (!strcmp(cmd, "attset"))
	{
	    char *name;
	    data1_attset *attset;
	    
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args to attset",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    if (!(attset = data1_get_attset (dh, name)))
	    {
		yaz_log(LOG_WARN, "%s:%d: Couldn't find attset  %s",
		     file, lineno, name);
		continue;
	    }
	    *attset_childp = (data1_attset_child *)
		nmem_malloc (data1_nmem_get(dh), sizeof(**attset_childp));
	    (*attset_childp)->child = attset;
	    (*attset_childp)->next = 0;
	    attset_childp = &(*attset_childp)->next;
	}
	else if (!strcmp(cmd, "tagset"))
	{
	    char *name;
	    int type = 0;
	    if (argc < 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args to tagset",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    if (argc == 3)
		type = atoi(argv[2]);
	    *tagset_childp = data1_read_tagset (dh, name, type);
	    if (!(*tagset_childp))
	    {
		yaz_log(LOG_WARN, "%s:%d: Couldn't load tagset %s",
		     file, lineno, name);
		continue;
	    }
	    tagset_childp = &(*tagset_childp)->next;
	}
	else if (!strcmp(cmd, "varset"))
	{
	    char *name;

	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args in varset",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    if (!(res->varset = data1_read_varset (dh, name)))
	    {
		yaz_log(LOG_WARN, "%s:%d: Couldn't load Varset %s",
		     file, lineno, name);
		continue;
	    }
	}
	else if (!strcmp(cmd, "esetname"))
	{
	    char *name, *fname;

	    if (argc != 3)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args in esetname",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    fname = argv[2];
	    
	    *esetpp = (data1_esetname *)
		nmem_malloc(data1_nmem_get(dh), sizeof(**esetpp));
	    (*esetpp)->name = nmem_strdup(data1_nmem_get(dh), name);
	    (*esetpp)->next = 0;
	    if (*fname == '@')
		(*esetpp)->spec = 0;
	    else if (!((*esetpp)->spec = data1_read_espec1 (dh, fname)))
	    {
		yaz_log(LOG_WARN, "%s:%d: Espec-1 read failed for %s",
		     file, lineno, fname);
		continue;
	    }
	    esetpp = &(*esetpp)->next;
	}
	else if (!strcmp(cmd, "maptab"))
	{
	    char *name;
	    
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args for maptab",
                     file, lineno);
		continue;
	    }
	    name = argv[1];
	    if (!(*maptabp = data1_read_maptab (dh, name)))
	    {
		yaz_log(LOG_WARN, "%s:%d: Couldn't load maptab %s",
                     file, lineno, name);
		continue;
	    }
	    maptabp = &(*maptabp)->next;
	}
	else if (!strcmp(cmd, "marc"))
	{
	    char *name;
	    
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # or args for marc",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    if (!(*marcp = data1_read_marctab (dh, name)))
	    {
		yaz_log(LOG_WARN, "%s:%d: Couldn't read marctab %s",
                     file, lineno, name);
		continue;
	    }
	    marcp = &(*marcp)->next;
	}
	else if (!strcmp(cmd, "encoding"))
	{
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # or args for encoding",
		     file, lineno);
		continue;
	    }
            res->encoding = nmem_strdup (data1_nmem_get(dh), argv[1]);
	}
	else
	{
	    yaz_log(LOG_WARN, "%s:%d: Unknown directive '%s'", file, 
                    lineno, cmd);
	    continue;
	}
    }
    if (f)
        fclose(f);
    
    for (cur_elements = res->sub_elements; cur_elements;
	 cur_elements = cur_elements->next)
    {
	if (!strcmp (cur_elements->name, "main"))
	    res->main_elements = cur_elements->elements;
	fix_element_ref (dh, res, cur_elements->elements);
    }
    yaz_log (LOG_DEBUG, "%s: data1_read_absyn end", file);
    return res;
}
