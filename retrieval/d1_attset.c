/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_attset.c,v $
 * Revision 1.3  1995-12-13 17:14:26  quinn
 * *** empty log message ***
 *
 * Revision 1.2  1995/11/01  16:34:55  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.1  1995/11/01  11:56:07  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <xmalloc.h>
#include <log.h>
#include <d1_attset.h>
#include <data1.h>
#include <tpath.h>

data1_att *data1_getattbyname(data1_attset *s, char *name)
{
    data1_att *r;

    for (; s; s = s->next)
    {
	/* scan local set */
	for (r = s->atts; r; r = r->next)
	    if (!data1_matchstr(r->name, name))
		return r;
	/* scan included sets */
	if (s->children && (r = data1_getattbyname(s->children, name)))
	    return r;
    }
    return 0;
}

data1_attset *data1_read_attset(char *file)
{
    char line[512], *r, cmd[512], args[512];
    data1_attset *res = 0, **childp;
    data1_att **attp;
    FILE *f;

    if (!(f = yaz_path_fopen(data1_tabpath, file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }

    if (!(res = xmalloc(sizeof(*res))))
	abort();
    res->name = 0;
    res->reference = VAL_NONE;
    res->ordinal = -1;
    res->atts = 0;
    res->children = res->next = 0;
    childp = &res->children;
    attp = &res->atts;

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
	    return res;
	    fclose(f);
	}
	if (sscanf(r, "%s %[^\n]", cmd, args) < 2)
	    *args = '\0';
	if (!strcmp(cmd, "att"))
	{
	    int num, local, rr;
	    char name[512];
	    data1_att *t;

	    if ((rr = sscanf(args, "%d %s %d", &num, name, &local)) < 2)
	    {
		logf(LOG_WARN, "Not enough arguments to att in '%s' in %s",
		    args, file);
		fclose(f);
		return 0;
	    }
	    if (rr < 3)
		local = num;
	    if (!(t = *attp = xmalloc(sizeof(*t))))
		abort();
	    t->parent = res;
	    if (!(t->name = xmalloc(strlen(name)+1)))
		abort();
	    strcpy(t->name, name);
	    t->value = num;
	    t->local = local;
	    t->next = 0;
	    attp = &t->next;
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
		logf(LOG_WARN, "Unknown attset name '%s' in %s", name, file);
		fclose(f);
		return 0;
	    }
	}
	else if (!strcmp(cmd, "ordinal"))
	{
	    if (!sscanf(args, "%d", &res->ordinal))
	    {
		logf(LOG_WARN, "%s malformed ordinal directive in %s", file);
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
	    if (!(*childp = data1_read_attset(name)))
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
