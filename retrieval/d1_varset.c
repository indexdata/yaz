/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_varset.c,v $
 * Revision 1.2  1995-11-01 13:54:50  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:09  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <string.h>
#include <stdlib.h>

#include <readconf.h>
#include <oid.h>
#include <log.h>

#include <data1.h>

data1_vartype *data1_getvartypebyct(data1_varset *set, char *class, char *type)
{
    data1_varclass *c;
    data1_vartype *t;

    for (c = set->classes; c; c = c->next)
	if (!data1_matchstr(c->name, class))
	{
	    for (t = c->types; t; t = t->next)
		if (!data1_matchstr(t->name, type))
		    return t;
	    logf(LOG_WARN, "Unknown variant type %s in class %s", type, class);
	    return 0;
	}
    logf(LOG_WARN, "Unknown variant class %s", class);
    return 0;
}

data1_varset *data1_read_varset(char *file)
{
    data1_varset *res = xmalloc(sizeof(*res));
    data1_varclass **classp = &res->classes, *class = 0;
    data1_vartype **typep = 0;
    FILE *f;
    int argc;
    char *argv[50],line[512];

    res->name = 0;
    res->reference = VAL_NONE;
    res->classes = 0;

    if (!(f = fopen(file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }
    while ((argc = readconf_line(f, line, 512, argv, 50)))
	if (!strcmp(argv[0], "class"))
	{
	    data1_varclass *r;

	    if (argc != 3)
	    {
		logf(LOG_FATAL, "%s: malformed class directive", file);
		fclose(f);
		return 0;
	    }
	    *classp = r = class = xmalloc(sizeof(*r));
	    r->set = res;
	    r->class = atoi(argv[1]);
	    r->name = xmalloc(strlen(argv[2])+1);
	    strcpy(r->name, argv[2]);
	    r->types = 0;
	    typep = &r->types;
	    r->next = 0;
	    classp = &r->next;
	}
	else if (!strcmp(argv[0], "type"))
	{
	    data1_vartype *r;

	    if (!typep)
	    {
		logf(LOG_WARN, "%s: class directive must precede type", file);
		fclose(f);
		return 0;
	    }
	    if (argc != 4)
	    {
		logf(LOG_WARN, "%s: Malformed type directive", file);
		fclose(f);
		return 0;
	    }
	    *typep = r = xmalloc(sizeof(*r));
	    r->name = xmalloc(strlen(argv[2])+1);
	    strcpy(r->name, argv[2]);
	    r->class = class;
	    r->type = atoi(argv[1]);
	    if (!(r->datatype = data1_maptype(argv[3])))
	    {
		logf(LOG_WARN, "%s: Unknown datatype '%s'", file, argv[3]);
		fclose(f);
		return 0;
	    }
	    r->next = 0;
	    typep = &r->next;
	}
	else if (!strcmp(argv[0], "name"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s name: Expected 1 argument", file);
		fclose(f);
		return 0;
	    }
	    res->name = xmalloc(strlen(argv[1])+1);
	    strcpy(res->name, argv[1]);
	}
	else if (!strcmp(argv[0], "reference"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s: reference: Expected 1 argument", file);
		fclose(f);
		return 0;
	    }
	    if ((res->reference = oid_getvalbyname(argv[1])) == VAL_NONE)
	    {
		logf(LOG_WARN, "Unknown reference '%s' in %s", argv[1], file);
		fclose(f);
		return 0;
	    }
	}
	else 
	    logf(LOG_WARN, "varset: Unknown directive '%s' in %s", argv[0],
		file);

    fclose(f);
    return res;
}
