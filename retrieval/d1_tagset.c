/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_tagset.c,v $
 * Revision 1.9  1998-10-13 16:09:53  adam
 * Added support for arbitrary OID's for tagsets, schemas and attribute sets.
 * Added support for multiple attribute set references and tagset references
 * from an abstract syntax file.
 * Fixed many bad logs-calls in routines that read the various
 * specifications regarding data1 (*.abs,*.att,...) and made the messages
 * consistent whenever possible.
 * Added extra 'lineno' argument to function readconf_line.
 *
 * Revision 1.8  1998/05/18 13:07:07  adam
 * Changed the way attribute sets are handled by the retriaval module.
 * Extended Explain conversion / schema.
 * Modified server and client to work with ASN.1 compiled protocol handlers.
 *
 * Revision 1.7  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.6  1997/09/17 12:10:38  adam
 * YAZ version 1.4.
 *
 * Revision 1.5  1997/09/05 09:50:57  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.4  1995/11/13 09:27:38  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.3  1995/11/01  16:34:58  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.2  1995/11/01  13:54:49  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:09  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <log.h>
#include <data1.h>

/*
 * We'll probably want to add some sort of hashed index to these lookup-
 * functions eventually.
 */

data1_datatype data1_maptype (data1_handle dh, char *t)
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
	{"null", DATA1K_null},
	{NULL, (data1_datatype) -1}
    };
    int i;

    for (i = 0; types[i].tname; i++)
	if (!data1_matchstr(types[i].tname, t))
	    return types[i].type;
    return 0;
}

data1_tag *data1_gettagbynum (data1_handle dh, data1_tagset *s,
			      int type, int value)
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
	if (s->children && (r = data1_gettagbynum (dh, s->children,
						   type, value)))
	    return r;
    }
    return 0;
}

data1_tag *data1_gettagbyname (data1_handle dh, data1_tagset *s, char *name)
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
	if (s->children && (r = data1_gettagbyname (dh, s->children, name)))
	    return r;
    }
    return 0;
}

data1_tagset *data1_empty_tagset (data1_handle dh)
{
    data1_tagset *res =
	(data1_tagset *) nmem_malloc(data1_nmem_get (dh), sizeof(*res));
    res->name = 0;
    res->reference = VAL_NONE;
    res->type = 0;
    res->tags = 0;
    res->children = 0;
    res->next = 0;
    return res;
}

data1_tagset *data1_read_tagset (data1_handle dh, char *file)
{
    NMEM mem = data1_nmem_get (dh);
    data1_tagset *res = 0, **childp;
    data1_tag **tagp;
    FILE *f;
    int lineno = 0;
    int argc;
    char *argv[50], line[512];

    if (!(f = yaz_path_fopen(data1_get_tabpath(dh), file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }
    res = data1_empty_tagset (dh);
    childp = &res->children;
    tagp = &res->tags;

    while ((argc = readconf_line(f, &lineno, line, 512, argv, 50)))
    {
	char *cmd = *argv;
	if (!strcmp(cmd, "tag"))
	{
	    int value;
	    char *names, *type, *nm;
	    data1_tag *rr;
	    data1_name **npp;

	    if (argc != 4)
	    {
		logf(LOG_WARN, "%s:%d: Bad # args to tag", file, lineno);
		continue;
	    }
	    value = atoi(argv[1]);
	    names = argv[2];
	    type = argv[3];

	    rr = *tagp = (data1_tag *)nmem_malloc(mem, sizeof(*rr));
	    rr->tagset = res;
	    rr->next = 0;
	    rr->which = DATA1T_numeric;
	    rr->value.numeric = value;
	    /*
	     * how to deal with local numeric tags?
	     */

	    if (!(rr->kind = data1_maptype(dh, type)))
	    {
		logf(LOG_WARN, "%s:%d: Unknown datatype %s",
		     file, lineno, type);
		fclose(f);
		return 0;
	    }
	    
	    /* read namelist */
	    nm = names;
	    npp = &rr->names;
	    do
	    {
		char *e;

		*npp = (data1_name *)nmem_malloc(mem, sizeof(**npp));
		if ((e = strchr(nm, '/')))
		    *(e++) = '\0';
		(*npp)->name = nmem_strdup(mem, nm);
		(*npp)->next = 0;
		npp = &(*npp)->next;
		nm = e;
	    }
	    while (nm);
	    tagp = &rr->next;
	}
	else if (!strcmp(cmd, "name"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Bad # args to name", file, lineno);
		continue;
	    }
	    res->name = nmem_strdup(mem, argv[1]);
	}
	else if (!strcmp(cmd, "reference"))
	{
	    char *name;
	    
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Bad # args to reference", file, lineno);
		continue;
	    }
	    name = argv[1];
	    if ((res->reference = oid_getvalbyname(name)) == VAL_NONE)
	    {
		logf(LOG_WARN, "%s:%d: Unknown tagset ref '%s'",
		     file, lineno, name);
		continue;
	    }
	}
	else if (!strcmp(cmd, "type"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Bad # args to type", file, lineno);
		continue;
	    }
	    res->type = atoi(argv[1]);
	}
	else if (!strcmp(cmd, "include"))
	{
	    char *name;

	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Bad # args to include",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    if (!(*childp = data1_read_tagset (dh, name)))
	    {
		logf(LOG_WARN, "%s:%d: Inclusion failed for tagset %s",
		     file, lineno, name);
		continue;
	    }
	    childp = &(*childp)->next;
	}
	else
	{
	    logf(LOG_WARN, "%s:%d: Unknown directive '%s'",
		 file, lineno, cmd);
	}
    }
    fclose(f);
    return res;
}
