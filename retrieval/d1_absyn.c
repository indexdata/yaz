/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_absyn.c,v $
 * Revision 1.19  1998-03-05 08:15:32  adam
 * Implemented data1_add_insert_taggeddata utility which is more flexible
 * than data1_insert_taggeddata.
 *
 * Revision 1.18  1998/02/27 14:08:04  adam
 * Added const to some char pointer arguments.
 * Reworked data1_read_node so that it doesn't create a tree with
 * pointers to original "SGML"-buffer.
 *
 * Revision 1.17  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.16  1997/12/18 10:51:30  adam
 * Implemented sub-trees feature for schemas - including forward
 * references.
 *
 * Revision 1.15  1997/12/09 16:18:16  adam
 * Work on EXPLAIN schema. First implementation of sub-schema facility
 * in the *.abs files.
 *
 * Revision 1.14  1997/10/31 12:20:09  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 * Revision 1.13  1997/10/27 13:54:18  adam
 * Changed structure field in data1 node to be simple string which
 * is "unknown" to the retrieval system itself.
 *
 * Revision 1.12  1997/09/17 12:10:34  adam
 * YAZ version 1.4.
 *
 * Revision 1.11  1997/09/05 09:50:55  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.10  1997/05/14 06:54:01  adam
 * C++ support.
 *
 * Revision 1.9  1997/02/19 14:46:15  adam
 * The "all" specifier only affects elements that are indexed (and not
 * all elements).
 *
 * Revision 1.8  1997/01/02 10:47:59  quinn
 * Added optional, physical ANY
 *
 * Revision 1.7  1996/06/10 08:56:01  quinn
 * Work on Summary.
 *
 * Revision 1.6  1996/05/31  13:52:21  quinn
 * Fixed uninitialized variable for local tags in abstract syntax.
 *
 * Revision 1.5  1996/05/09  07:27:43  quinn
 * Multiple local attributes values supported.
 *
 * Revision 1.4  1996/05/01  12:45:28  quinn
 * Support use of local tag names in abs file.
 *
 * Revision 1.3  1995/11/01  16:34:55  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.2  1995/11/01  13:54:44  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:06  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <oid.h>
#include <log.h>
#include <tpath.h>

#include <data1.h>

#define D1_MAX_NESTING  128

struct data1_absyn_cache_info 
{
    char *name;
    data1_absyn *absyn;
    data1_absyn_cache next;
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
    p->absyn = data1_read_absyn (dh, fname);
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

    if (!parent)
        r = abs->main_elements;
    else
	r = parent->children;
    assert (abs->main_elements);
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
    assert (absyn->main_elements);
    for (r = absyn->main_elements; r; r = r->next)
	if (!data1_matchstr(r->name, name))
	    return r;
    return 0;
}


void fix_element_ref (data1_handle dh, data1_absyn *absyn, data1_element *e)
{
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
		logf (LOG_WARN, "Unresolved reference to sub-elements %s",
		      e->sub_name);
	}
    }
}

data1_absyn *data1_read_absyn (data1_handle dh, const char *file)
{
    char line[512], *r, cmd[512], args[512];
    data1_sub_elements *cur_elements = NULL;
    data1_absyn *res = 0;
    FILE *f;
    data1_element **ppl[D1_MAX_NESTING];
    data1_esetname **esetpp;
    data1_maptab **maptabp;
    data1_marctab **marcp;
    data1_termlist *all = 0;
    int level;

    logf (LOG_DEBUG, "begin data1_read_absyn file=%s", file);
    if (!(f = yaz_path_fopen(data1_get_tabpath (dh), file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "Couldn't open %s", file);
	return 0;
    }

    res = (data1_absyn *)nmem_malloc(data1_nmem_get(dh), sizeof(*res));
    res->name = 0;
    res->reference = VAL_NONE;
    res->tagset = 0;
    res->attset = 0;
    res->varset = 0;
    res->esetnames = 0;
    esetpp = &res->esetnames;
    res->maptabs = 0;
    maptabp = &res->maptabs;
    res->marc = 0;
    marcp = &res->marc;

    res->sub_elements = NULL;
    res->main_elements = NULL;

    for (;;)
    {
	while ((r = fgets(line, 512, f)))
	{
	    while (*r && isspace(*r))
		r++;
	    if (*r && *r != '#')
		break;
	}
	if (!r)
            break;
	if (sscanf(r, "%s %[^\n]", cmd, args) < 2)
	    *args = '\0';
	if (!strcmp(cmd, "elm"))
	{
	    data1_element *new_element;
	    int i;
	    char path[512], name[512], termlists[512], *p, *sub_p;
	    int type, value;
	    data1_termlist **tp;

	    if (!cur_elements)
	    {
                cur_elements = (data1_sub_elements *)nmem_malloc(data1_nmem_get(dh),
				    	   sizeof(*cur_elements));
	        cur_elements->next = res->sub_elements;
		cur_elements->elements = NULL;
		cur_elements->name = "main";
		res->sub_elements = cur_elements;

		level = 0;
    		ppl[level] = &cur_elements->elements;
            }
	    if (sscanf(args, "%511s %511s %511s", path, name, termlists) < 3)
	    {
		logf(LOG_WARN, "Bad # of args to elm in %s: '%s'", 
		    file, args);
		fclose(f);
		return 0;
	    }
	    p = path;
	    for (i = 0;; i++)
	    {
		char *e;

		if ((e = strchr(p, '/')))
		    p = e+1;
		else
		    break;
	    }
	    if (i > level + 1)
	    {
		logf(LOG_WARN, "Bad level inc in %s in '%s'", file, args);
		fclose(f);
		return 0;
	    }
	    level = i;
	    new_element = *ppl[level] =
		(data1_element *)nmem_malloc(data1_nmem_get(dh), sizeof(*new_element));
	    new_element->next = new_element->children = 0;
	    new_element->tag = 0;
	    new_element->termlists = 0;
	    new_element->sub_name = 0;

	    tp = &new_element->termlists;
	    ppl[level] = &new_element->next;
	    ppl[level+1] = &new_element->children;

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
		    logf(LOG_WARN, "No tagset loaded in %s", file);
		    fclose(f);
		    return 0;
		}
		if (!(new_element->tag = data1_gettagbynum (dh, res->tagset,
							    type, value)))
		{
		    logf(LOG_WARN, "Couldn't find tag %s in tagset in %s",
			p, file);
		    fclose(f);
		    return 0;
		}
	    }
	    /* private tag */
	    else if (*p)
	    {
		data1_tag *nt =
		    new_element->tag = (data1_tag *)nmem_malloc(data1_nmem_get (dh),
						   sizeof(*new_element->tag));
		nt->which = DATA1T_string;
		nt->value.string = nmem_strdup(data1_nmem_get (dh), p);
		nt->names = (data1_name *)nmem_malloc(data1_nmem_get(dh), 
					sizeof(*new_element->tag->names));
		nt->names->name = nt->value.string;
		nt->names->next = 0;
		nt->kind = DATA1K_string;
		nt->next = 0;
		nt->tagset = 0;
	    }
	    else
	    {
		logf(LOG_WARN, "Bad element is %s", file);
		fclose(f);
		return 0;
	    }
	    /* parse termList definitions */
	    p = termlists;
	    if (*p == '-')
		new_element->termlists = 0;
	    else
	    {
		if (!res->attset)
		{
		    logf(LOG_WARN, "No attset loaded in %s", file);
		    fclose(f);
		    return 0;
		}
		do
		{
		    char attname[512], structure[512];
		    int r;

		    if (!(r = sscanf(p, "%511[^:,]:%511[^,]", attname,
			structure)))
		    {
			logf(LOG_WARN, "Syntax error in termlistspec in %s",
			    file);
			fclose(f);
			return 0;
		    }
		    if (*attname == '!')
			strcpy(attname, name);
		    *tp = (data1_termlist *)nmem_malloc(data1_nmem_get(dh), sizeof(**tp));
		    (*tp)->next = 0;
		    if (!((*tp)->att = data1_getattbyname(dh, res->attset,
							  attname)))
		    {
			logf(LOG_WARN, "Couldn't find att '%s' in attset",
			     attname);
			fclose(f);
			return 0;
		    }
		    if (r < 2) /* is the structure qualified? */
			(*tp)->structure = "w";
		    else 
		    {
			(*tp)->structure = (char *)nmem_malloc (data1_nmem_get (dh),
							strlen(structure)+1);
			strcpy ((*tp)->structure, structure);
		    }
		    tp = &(*tp)->next;
		}
		while ((p = strchr(p, ',')) && *(++p));
	        *tp = all; /* append any ALL entries to the list */
	    }
	    new_element->name = nmem_strdup(data1_nmem_get (dh), name);
	}
 	else if (!strcmp(cmd, "section"))
	{
	    char name[512];
	    if (sscanf(args, "%511s", name) < 1)
	    {
		logf(LOG_WARN, "Bad # of args to sub in %s: '%s'",
                                file, args);
		continue;
	    }
            cur_elements = (data1_sub_elements *)nmem_malloc(data1_nmem_get(dh),
				    	   sizeof(*cur_elements));
	    cur_elements->next = res->sub_elements;
	    cur_elements->elements = NULL;
	    cur_elements->name = nmem_strdup (data1_nmem_get(dh), name);
	    res->sub_elements = cur_elements;

	    level = 0;
    	    ppl[level] = &cur_elements->elements;
	}
	else if (!strcmp(cmd, "all"))
	{
	    char *p;
	    data1_termlist **tp = &all;

	    if (all)
	    {
		logf(LOG_WARN, "Too many ALL declarations in %s - ignored",
		    file);
		continue;
	    }

	    p = args;
	    if (!res->attset)
	    {
		logf(LOG_WARN, "No attset loaded in %s", file);
		fclose(f);
		return 0;
	    }
	    do
	    {
		char attname[512], structure[512];
		int r;

		if (!(r = sscanf(p, "%511[^:,]:%511[^,]", attname,
		    structure)))
		{
		    logf(LOG_WARN, "Syntax error in termlistspec in %s",
			file);
		    fclose(f);
		    return 0;
		}
		*tp = (data1_termlist *)nmem_malloc(data1_nmem_get(dh), sizeof(**tp));
		if (!((*tp)->att = data1_getattbyname (dh, res->attset,
						       attname)))
		{
		    logf(LOG_WARN, "Couldn't find att '%s' in attset",
			 attname);
		    fclose(f);
		    return 0;
		}
		if (r < 2) /* is the structure qualified? */
		    (*tp)->structure = "w";
		else 
		{
		    (*tp)->structure = (char *)nmem_malloc (data1_nmem_get (dh),
						    strlen(structure)+1);
		    strcpy ((*tp)->structure, structure);
		}
		(*tp)->next = 0;
		tp = &(*tp)->next;
	    }
	    while ((p = strchr(p, ',')) && *(++p));
	}
	else if (!strcmp(cmd, "name"))
	{
	    char name[512];

	    if (!sscanf(args, "%511s", name))
	    {
		logf(LOG_WARN, "Malformed name directive in %s", file);
		fclose(f);
		return 0;
	    }
	    res->name = nmem_strdup(data1_nmem_get(dh), name);
	}
	else if (!strcmp(cmd, "reference"))
	{
	    char name[512];

	    if (!sscanf(args, "%s", name))
	    {
		logf(LOG_WARN, "Malformed reference in %s", file);
		fclose(f);
		return 0;
	    }
	    if ((res->reference = oid_getvalbyname(name)) == VAL_NONE)
	    {
		logf(LOG_WARN, "Unknown tagset ref '%s' in %s", name, file);
		fclose(f);
		return 0;
	    }
	}
	else if (!strcmp(cmd, "attset"))
	{
	    char name[512];

	    if (!sscanf(args, "%s", name))
	    {
		logf(LOG_WARN, "Malformed attset directive in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(res->attset = data1_read_attset (dh, name)))
	    {
		logf(LOG_WARN, "Attset failed in %s", file);
		fclose(f);
		return 0;
	    }
	}
	else if (!strcmp(cmd, "tagset"))
	{
	    char name[512];

	    if (!sscanf(args, "%s", name))
	    {
		logf(LOG_WARN, "Malformed tagset directive in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(res->tagset = data1_read_tagset (dh, name)))
	    {
		logf(LOG_WARN, "Tagset failed in %s", file);
		fclose(f);
		return 0;
	    }
	}
	else if (!strcmp(cmd, "varset"))
	{
	    char name[512];

	    if (!sscanf(args, "%s", name))
	    {
		logf(LOG_WARN, "Malformed varset directive in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(res->varset = data1_read_varset (dh, name)))
	    {
		logf(LOG_WARN, "Varset failed in %s", file);
		fclose(f);
		return 0;
	    }
	}
	else if (!strcmp(cmd, "esetname"))
	{
	    char name[512], fname[512];

	    if (sscanf(args, "%s %s", name, fname) != 2)
	    {
		logf(LOG_WARN, "Two arg's required for esetname in %s",
                     file);
		fclose(f);
		return 0;
	    }
	    *esetpp = (data1_esetname *)nmem_malloc(data1_nmem_get(dh), sizeof(**esetpp));
	    (*esetpp)->name = nmem_strdup(data1_nmem_get(dh), name);
	    (*esetpp)->next = 0;
	    if (*fname == '@')
		(*esetpp)->spec = 0;
	    else if (!((*esetpp)->spec = data1_read_espec1 (dh, fname)))
	    {
		logf(LOG_WARN, "%s: Espec-1 read failed", file);
		fclose(f);
		return 0;
	    }
	    esetpp = &(*esetpp)->next;
	}
	else if (!strcmp(cmd, "maptab"))
	{
	    char name[512];

	    if (sscanf(args, "%s", name) != 1)
	    {
		logf(LOG_WARN, "One argument for maptab directive in %s",
                     file);
		continue;
	    }
	    if (!(*maptabp = data1_read_maptab (dh, name)))
	    {
		logf(LOG_WARN, "Failed to read maptab %s in %s",
                     name, file);
		continue;
	    }
	    maptabp = &(*maptabp)->next;
	}
	else if (!strcmp(cmd, "marc"))
	{
	    char name[512];

	    if (sscanf(args, "%s", name) != 1)
	    {
		logf(LOG_WARN, "One argument for marc directive in %s",
		    file);
		continue;
	    }
	    if (!(*marcp = data1_read_marctab (dh, name)))
	    {
		logf(LOG_WARN, "%Failed to read marctab %s in %s",
                     name, file);
		continue;
	    }
	    marcp = &(*marcp)->next;
	}
	else
	{
	    logf(LOG_WARN, "Unknown directive '%s' in %s", cmd, file);
	    fclose(f);
	    return 0;
	}
    }
    fclose(f);
    
    for (cur_elements = res->sub_elements; cur_elements;
	 cur_elements = cur_elements->next)
    {
	if (!strcmp (cur_elements->name, "main"))
	    res->main_elements = cur_elements->elements;
	fix_element_ref (dh, res, cur_elements->elements);
    }
    logf (LOG_DEBUG, "end data1_read_absyn file=%s", file);
    return res;
}
