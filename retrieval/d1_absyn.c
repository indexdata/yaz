/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_absyn.c,v $
 * Revision 1.3  1995-11-01 16:34:55  quinn
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

#include <xmalloc.h>
#include <oid.h>
#include <log.h>
#include <tpath.h>

#include <data1.h>

#define D1_MAX_NESTING  128
#define DATA1_MAX_SYNTAXES 30 /* max no of syntaxes to handle in one session */

static struct /* cache of abstract syntaxes */
{
    char *name;
    data1_absyn *absyn;
} syntaxes[DATA1_MAX_SYNTAXES] = {{0,0}};

data1_absyn *data1_get_absyn(char *name)
{
    char fname[512];
    int i;

    for (i = 0; syntaxes[i].name; i++)
	if (!strcmp(name, syntaxes[i].name))
	    return syntaxes[i].absyn;

    if (i >= DATA1_MAX_SYNTAXES - 1)
    {
	logf(LOG_WARN, "Too many abstract syntaxes loaded");
	return 0;
    }
    sprintf(fname, "%s.abs", name);
    if (!(syntaxes[i].absyn = data1_read_absyn(fname)))
	return 0;
    if (!(syntaxes[i].name = xmalloc(strlen(name)+1)))
	abort();
    strcpy(syntaxes[i].name, name);
    syntaxes[i+1].name = 0;
    return syntaxes[i].absyn;
}

data1_esetname *data1_getesetbyname(data1_absyn *a, char *name)
{
    data1_esetname *r;

    for (r = a->esetnames; r; r = r->next)
	if (!data1_matchstr(r->name, name))
	    return r;
    return 0;
}

data1_element *data1_getelementbytagname(data1_absyn *abs,
    data1_element *parent, char *tagname)
{
    data1_element *r;

    if (!parent)
	r = abs->elements;
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

data1_element *data1_getelementbyname(data1_absyn *absyn, char *name)
{
    data1_element *r;

    for (r = absyn->elements; r; r = r->next)
	if (!data1_matchstr(r->name, name))
	    return r;
    return 0;
}

data1_absyn *data1_read_absyn(char *file)
{
    char line[512], *r, cmd[512], args[512];
    data1_absyn *res = 0;
    FILE *f;
    data1_element **ppl[D1_MAX_NESTING];
    data1_esetname **esetpp;
    data1_maptab **maptabp;
    data1_marctab **marcp;
    int level = 0;

    if (!(f = yaz_path_fopen(data1_tabpath, file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }

    if (!(res = xmalloc(sizeof(*res))))
	abort();
    res->name = 0;
    res->reference = VAL_NONE;
    res->tagset = 0;
    res->attset = 0;
    res->varset = 0;
    res->esetnames = 0;
    res->maptabs = 0;
    maptabp = &res->maptabs;
    res->marc = 0;
    marcp = &res->marc;
    res->elements = 0;
    ppl[0] = &res->elements;
    esetpp = &res->esetnames;

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
	{
	    fclose(f);
	    return res;
	}
	if (sscanf(r, "%s %[^\n]", cmd, args) < 2)
	    *args = '\0';
	if (!strcmp(cmd, "elm"))
	{
	    data1_element *new;
	    int i;
	    char path[512], name[512], att[512], *p;
	    int type, value;

	    if (sscanf(args, "%s %s %s", path, name, att) < 3)
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
		logf(LOG_WARN, "Bad level inc in %s in '%'", file, args);
		fclose(f);
		return 0;
	    }
	    level = i;
	    if (!(new = *ppl[level] = xmalloc(sizeof(*new))))
		abort;
	    new ->next = new->children = 0;
	    ppl[level] = &new->next;
	    ppl[level+1] = &new->children;

	    if (sscanf(p, "(%d,%d)", &type, &value) < 2)
	    {
		logf(LOG_WARN, "Malformed element '%s' in %s", p, file);
		fclose(f);
		return 0;
	    }
	    if (!res->tagset)
	    {
		logf(LOG_WARN, "No tagset loaded in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(new->tag = data1_gettagbynum(res->tagset, type, value)))
	    {
		logf(LOG_WARN, "Couldn't find tag %s in tagset in %s",
		    p, file);
		fclose(f);
		return 0;
	    }
	    if (*att == '!')
		strcpy(att, name);
	    if (*att == '-')
		new->att = 0;
	    else
	    {
		if (!res->attset)
		{
		    logf(LOG_WARN, "No attset loaded in %s", file);
		    fclose(f);
		    return 0;
		}
		if (!(new->att = data1_getattbyname(res->attset, att)))
		{
		    logf(LOG_WARN, "Couldn't find att '%s' in attset", att);
		    fclose(f);
		    return 0;
		}
	    }
	    if (!(new->name = xmalloc(strlen(name)+1)))
		abort();
	    strcpy(new->name, name);
	}
	else if (!strcmp(cmd, "name"))
	{
	    char name[512];

	    if (!sscanf(args, "%s", name))
	    {
		logf(LOG_WARN, "%s malformed name directive in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(res->name = xmalloc(strlen(args)+1)))
		abort();
	    strcpy(res->name, name);
	}
	else if (!strcmp(cmd, "reference"))
	{
	    char name[512];

	    if (!sscanf(args, "%s", name))
	    {
		logf(LOG_WARN, "%s malformed reference directive in %s", file);
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
		logf(LOG_WARN, "%s malformed attset directive in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(res->attset = data1_read_attset(name)))
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
		logf(LOG_WARN, "%s malformed tagset directive in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(res->tagset = data1_read_tagset(name)))
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
		logf(LOG_WARN, "%s malformed varset directive in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(res->varset = data1_read_varset(name)))
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
		logf(LOG_WARN, "%s: Two arg's required for esetname directive");
		fclose(f);
		return 0;
	    }
	    *esetpp = xmalloc(sizeof(**esetpp));
	    (*esetpp)->name = xmalloc(strlen(name)+1);
	    strcpy((*esetpp)->name, name);
	    if (*fname == '@')
		(*esetpp)->spec = 0;
	    else if (!((*esetpp)->spec = data1_read_espec1(fname, 0)))
	    {
		logf(LOG_WARN, "%s: Espec-1 read failed", file);
		fclose(f);
		return 0;
	    }
	    (*esetpp)->next = 0;
	    esetpp = &(*esetpp)->next;
	}
	else if (!strcmp(cmd, "maptab"))
	{
	    char name[512];

	    if (sscanf(args, "%s", name) != 1)
	    {
		logf(LOG_WARN, "%s: One argument required for maptab directive",
		    file);
		continue;
	    }
	    if (!(*maptabp = data1_read_maptab(name)))
	    {
		logf(LOG_WARN, "%s: Failed to read maptab.");
		continue;
	    }
	    maptabp = &(*maptabp)->next;
	}
	else if (!strcmp(cmd, "marc"))
	{
	    char name[512];

	    if (sscanf(args, "%s", name) != 1)
	    {
		logf(LOG_WARN, "%s: One argument required for marc directive",
		    file);
		continue;
	    }
	    if (!(*marcp = data1_read_marctab(name)))
	    {
		logf(LOG_WARN, "%s: Failed to read marctab.");
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
}
