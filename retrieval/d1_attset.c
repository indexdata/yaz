/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_attset.c,v $
 * Revision 1.11  1998-10-14 13:31:56  adam
 * Bug fix. Bug introduced by previous commit.
 *
 * Revision 1.10  1998/10/13 16:09:48  adam
 * Added support for arbitrary OID's for tagsets, schemas and attribute sets.
 * Added support for multiple attribute set references and tagset references
 * from an abstract syntax file.
 * Fixed many bad logs-calls in routines that read the various
 * specifications regarding data1 (*.abs,*.att,...) and made the messages
 * consistent whenever possible.
 * Added extra 'lineno' argument to function readconf_line.
 *
 * Revision 1.9  1998/05/18 13:07:03  adam
 * Changed the way attribute sets are handled by the retriaval module.
 * Extended Explain conversion / schema.
 * Modified server and client to work with ASN.1 compiled protocol handlers.
 *
 * Revision 1.8  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.7  1997/09/17 12:10:34  adam
 * YAZ version 1.4.
 *
 * Revision 1.6  1997/09/05 09:50:56  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.5  1996/05/09 07:27:43  quinn
 * Multiple local attributes values supported.
 *
 * Revision 1.4  1996/02/21  15:23:36  quinn
 * Reversed fclose and return;
 *
 * Revision 1.3  1995/12/13  17:14:26  quinn
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

#include <log.h>
#include <d1_attset.h>
#include <data1.h>

data1_att *data1_getattbyname(data1_handle dh, data1_attset *s, char *name)
{
    data1_att *r;
    data1_attset_child *c;
    
    /* scan local set */
    for (r = s->atts; r; r = r->next)
	if (!data1_matchstr(r->name, name))
	    return r;
    for (c = s->children; c; c = c->next)
    {
	assert (c->child);
	/* scan included sets */
	if ((r = data1_getattbyname (dh, c->child, name)))
	    return r;
    }
    return 0;
}

data1_attset *data1_empty_attset(data1_handle dh)
{
    NMEM mem = data1_nmem_get (dh);
    data1_attset *res = (data1_attset*) nmem_malloc(mem,sizeof(*res));

    res->name = 0;
    res->reference = VAL_NONE;
    res->atts = 0;
    res->children = 0;
    res->next = 0;
    return res;
}

data1_attset *data1_read_attset(data1_handle dh, const char *file)
{
    data1_attset *res = 0;
    data1_attset_child **childp;
    data1_att **attp;
    FILE *f;
    NMEM mem = data1_nmem_get (dh);
    int lineno = 0;
    int argc;
    char *argv[50], line[512];

    if (!(f = yaz_path_fopen(data1_get_tabpath(dh), file, "r")))
	return NULL;
    res = data1_empty_attset (dh);

    childp = &res->children;
    attp = &res->atts;

    while ((argc = readconf_line(f, &lineno, line, 512, argv, 50)))
    {
	char *cmd = argv[0];
	if (!strcmp(cmd, "att"))
	{
	    int num;
	    char *name;
	    data1_att *t;
	    data1_local_attribute *locals;
	    
	    if (argc < 3)
	    {
		logf(LOG_WARN, "%s:%d: Bad # of args to att", file, lineno);
		continue;
	    }
	    num = atoi (argv[1]);
	    name = argv[2];
	    
	    if (argc == 3) /* no local attributes given */
	    {
		locals = (data1_local_attribute *)
		    nmem_malloc(mem, sizeof(*locals));
		locals->local = num;
		locals->next = 0;
	    }
	    else /* parse the string "local{,local}" */
	    {
		char *p = argv[3];
		data1_local_attribute **ap = &locals;
		do
		{
		    *ap = (data1_local_attribute *)
			nmem_malloc(mem, sizeof(**ap));
		    (*ap)->local = atoi(p);
		    (*ap)->next = 0;
		    ap = &(*ap)->next;
		}
		while ((p = strchr(p, ',')) && *(++p));
	    }
	    t = *attp = (data1_att *)nmem_malloc(mem, sizeof(*t));
	    t->parent = res;
	    t->name = nmem_strdup(mem, name);
	    t->value = num;
	    t->locals = locals;
	    t->next = 0;
	    attp = &t->next;
	}
	else if (!strcmp(cmd, "name"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Bad # of args to name", file, lineno);
		continue;
	    }
	}
	else if (!strcmp(cmd, "reference"))
	{
	    char *name;

	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Bad # of args to reference",
		     file, lineno);
		continue;
	    }
	    name = argv[1];
	    if ((res->reference = oid_getvalbyname(name)) == VAL_NONE)
	    {
		logf(LOG_WARN, "%s:%d: Unknown reference oid '%s'",
		     file, lineno, name);
		fclose(f);
		return 0;
	    }
	}
	else if (!strcmp(cmd, "ordinal"))
	{
	    logf (LOG_WARN, "%s:%d: Directive ordinal ignored",
		  file, lineno);
	}
	else if (!strcmp(cmd, "include"))
	{
	    char *name;
	    data1_attset *attset;

	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Bad # of args to include",
		     file, lineno);
		continue;
	    }
	    name = argv[1];

	    if (!(attset = data1_get_attset (dh, name)))
	    {
		logf(LOG_WARN, "%s:%d: Include of attset %s failed",
		     file, lineno, name);
		continue;
		
	    }
	    *childp = (data1_attset_child *)
		nmem_malloc (mem, sizeof(**childp));
	    (*childp)->child = attset;
	    (*childp)->next = 0;
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
