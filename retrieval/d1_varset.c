/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_varset.c,v $
 * Revision 1.8  1998-10-13 16:09:54  adam
 * Added support for arbitrary OID's for tagsets, schemas and attribute sets.
 * Added support for multiple attribute set references and tagset references
 * from an abstract syntax file.
 * Fixed many bad logs-calls in routines that read the various
 * specifications regarding data1 (*.abs,*.att,...) and made the messages
 * consistent whenever possible.
 * Added extra 'lineno' argument to function readconf_line.
 *
 * Revision 1.7  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.6  1997/09/17 12:10:39  adam
 * YAZ version 1.4.
 *
 * Revision 1.5  1997/09/05 09:50:58  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.4  1997/05/14 06:54:04  adam
 * C++ support.
 *
 * Revision 1.3  1995/11/01 16:34:58  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.2  1995/11/01  13:54:50  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:09  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <string.h>
#include <stdlib.h>

#include <oid.h>
#include <log.h>

#include <data1.h>

data1_vartype *data1_getvartypebyct (data1_handle dh, data1_varset *set,
				     char *zclass, char *type)
{
    data1_varclass *c;
    data1_vartype *t;

    for (c = set->classes; c; c = c->next)
	if (!data1_matchstr(c->name, zclass))
	{
	    for (t = c->types; t; t = t->next)
		if (!data1_matchstr(t->name, type))
		    return t;
	    logf(LOG_WARN, "Unknown variant type %s in class %s",
		 type, zclass);
	    return 0;
	}
    logf(LOG_WARN, "Unknown variant class %s", zclass);
    return 0;
}

data1_varset *data1_read_varset (data1_handle dh, const char *file)
{
    NMEM mem = data1_nmem_get (dh);
    data1_varset *res = (data1_varset *)nmem_malloc(mem, sizeof(*res));
    data1_varclass **classp = &res->classes, *zclass = 0;
    data1_vartype **typep = 0;
    FILE *f;
    int lineno = 0;
    int argc;
    char *argv[50],line[512];

    res->name = 0;
    res->reference = VAL_NONE;
    res->classes = 0;

    if (!(f = yaz_path_fopen(data1_get_tabpath(dh), file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }
    while ((argc = readconf_line(f, &lineno, line, 512, argv, 50)))
	if (!strcmp(argv[0], "class"))
	{
	    data1_varclass *r;
	    
	    if (argc != 3)
	    {
		logf(LOG_WARN, "%s:%d: Bad # or args to class",
		     file, lineno);
		continue;
	    }
	    *classp = r = zclass = (data1_varclass *)
		nmem_malloc(mem, sizeof(*r));
	    r->set = res;
	    r->zclass = atoi(argv[1]);
	    r->name = nmem_strdup(mem, argv[2]);
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
		logf(LOG_WARN, "%s:%d: Directive class must precede type",
		     file, lineno);
		continue;
	    }
	    if (argc != 4)
	    {
		logf(LOG_WARN, "%s:%d: Bad # or args to type", file, lineno);
		continue;
	    }
	    *typep = r = (data1_vartype *)nmem_malloc(mem, sizeof(*r));
	    r->name = nmem_strdup(mem, argv[2]);
	    r->zclass = zclass;
	    r->type = atoi(argv[1]);
	    if (!(r->datatype = data1_maptype (dh, argv[3])))
	    {
		logf(LOG_WARN, "%s:%d: Unknown datatype '%s'",
		     file, lineno, argv[3]);
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
		logf(LOG_WARN, "%s:%d: Bad # args for name",
		     file, lineno);
		continue;
	    }
	    res->name = nmem_strdup(mem, argv[1]);
	}
	else if (!strcmp(argv[0], "reference"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Bad # args for reference",
		     file, lineno);
		continue;
	    }
	    if ((res->reference = oid_getvalbyname(argv[1])) == VAL_NONE)
	    {
		logf(LOG_WARN, "%s:%d: Unknown reference '%s'",
		     file, lineno, argv[1]);
		continue;
	    }
	}
	else 
	    logf(LOG_WARN, "%s:%d: Unknown directive '%s'",
		 file, lineno, argv[0]);
    
    fclose(f);
    return res;
}
