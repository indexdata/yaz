/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_tagset.c,v $
 * Revision 1.1  1995-11-01 11:56:09  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <xmalloc.h>
#include <log.h>

#include "data1.h"

/*
 * We'll probably want to add some sort of hashed index to these lookup-
 * functions eventually.
 */

data1_datatype data1_maptype(char *t)
{
    static struct
    {
	char *tname;
	data1_datatype type;
    } types[] =
    {
	{"structured", DATA1K_structured},
	{"string", DATA1K_string},
	{"numeric", DATA1K_numeric},
	{"oid", DATA1K_oid},
	{"bool", DATA1K_bool},
	{"generalizedtime", DATA1K_generalizedtime},
	{"intunit", DATA1K_intunit},
	{"int", DATA1K_int},
	{"octetstring", DATA1K_octetstring},
	{0, -1}
    };
    int i;

    for (i = 0; types[i].tname; i++)
	if (!data1_matchstr(types[i].tname, t))
	    return types[i].type;
    return 0;
}

data1_tag *data1_gettagbynum(data1_tagset *s, int type, int value)
{
    data1_tag *r;

    for (; s; s = s->next)
    {
	/* scan local set */
	if (type == s->type)
	    for (r = s->tags; r; r = r->next)
		if (r->which == DATA1T_numeric && r->value.numeric == value)
		    return r;
	/* scan included sets */
	if (s->children && (r = data1_gettagbynum(s->children, type, value)))
	    return r;
    }
    return 0;
}

data1_tag *data1_gettagbyname(data1_tagset *s, char *name)
{
    data1_tag *r;

    for (; s; s = s->next)
    {
	/* scan local set */
	for (r = s->tags; r; r = r->next)
	{
	    data1_name *np;

	    for (np = r->names; np; np = np->next)
		if (!data1_matchstr(np->name, name))
		    return r;
	}
	/* scan included sets */
	if (s->children && (r = data1_gettagbyname(s->children, name)))
	    return r;
    }
    return 0;
}

data1_tagset *data1_read_tagset(char *file)
{
    char line[512], *r, cmd[512], args[512];
    data1_tagset *res = 0, **childp;
    data1_tag **tagp;
    FILE *f;

    if (!(f = fopen(file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }

    if (!(res = xmalloc(sizeof(*res))))
	abort();
    res->name = 0;
    res->type = 0;
    res->tags = 0;
    res->children = 0;
    res->next = 0;
    childp = &res->children;
    tagp = &res->tags;

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
	if (!strcmp(cmd, "tag"))
	{
	    int value;
	    char names[512], type[512], *nm;
	    data1_tag *rr;
	    data1_name **npp;

	    if (sscanf(args, "%d %s %s", &value, names, type) < 3)
	    {
		logf(LOG_WARN, "Bad number of parms in '%s' in %s",
		    args, file);
		fclose(f);
		return 0;
	    }
	    if (!(rr = *tagp = xmalloc(sizeof(*rr))))
		abort();

	    rr->tagset = res;
	    rr->next = 0;
	    rr->which = DATA1T_numeric;
	    rr->value.numeric = value;
	    /*
	     * how to deal with local numeric tags?
	     */

	    if (!(rr->kind = data1_maptype(type)))
	    {
		logf(LOG_WARN, "Unknown datatype %s in %s", type, file);
		fclose(f);
		return 0;
	    }
	    
	    /* read namelist */
	    nm = names;
	    npp = &rr->names;
	    do
	    {
		char *e;

		if (!(*npp = xmalloc(sizeof(**npp))))
		    abort();
		if ((e = strchr(nm, '/')))
		    *(e++) = '\0';
		if (!((*npp)->name = xmalloc(strlen(nm)+1)))
		    abort();
		strcpy((*npp)->name, nm);
		(*npp)->next = 0;
		npp = &(*npp)->next;
		nm = e;
	    }
	    while (nm);
	    tagp = &rr->next;
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
	else if (!strcmp(cmd, "type"))
	{
	    if (!sscanf(args, "%d", &res->type))
	    {
		logf(LOG_WARN, "%s malformed type directive in %s", file);
		fclose(f);
		return 0;
	    }
	}
	else if (!strcmp(cmd, "include"))
	{
	    char name[512];

	    if (!sscanf(args, "%s", name))
	    {
		logf(LOG_WARN, "%s malformed reference directive in %s", file);
		fclose(f);
		return 0;
	    }
	    if (!(*childp = data1_read_tagset(name)))
	    {
		logf(LOG_WARN, "Inclusion failed in %s", file);
		fclose(f);
		return 0;
	    }
	    childp = &(*childp)->next;
	}
	else
	{
	    logf(LOG_WARN, "Unknown directive '%s' in %s", cmd, file);
	    fclose(f);
	    return 0;
	}
    }
}
