/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_marc.c,v $
 * Revision 1.13  1998-10-13 16:09:52  adam
 * Added support for arbitrary OID's for tagsets, schemas and attribute sets.
 * Added support for multiple attribute set references and tagset references
 * from an abstract syntax file.
 * Fixed many bad logs-calls in routines that read the various
 * specifications regarding data1 (*.abs,*.att,...) and made the messages
 * consistent whenever possible.
 * Added extra 'lineno' argument to function readconf_line.
 *
 * Revision 1.12  1998/02/23 10:57:09  adam
 * Take care of integer data nodes as well in conversion.
 *
 * Revision 1.11  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.10  1997/09/30 11:50:04  adam
 * Added handler data1_get_map_buf that is used by data1_nodetomarc.
 *
 * Revision 1.9  1997/09/24 13:35:45  adam
 * Added two members to data1_marctab to ease reading of weird MARC records.
 *
 * Revision 1.8  1997/09/17 12:10:37  adam
 * YAZ version 1.4.
 *
 * Revision 1.7  1997/09/05 09:50:57  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.6  1997/09/04 13:51:58  adam
 * Added data1 to marc conversion with indicators.
 *
 * Revision 1.5  1997/09/04 13:48:04  adam
 * Added data1 to marc conversion.
 *
 * Revision 1.4  1996/03/25 10:18:03  quinn
 * Removed trailing whitespace from data elements
 *
 * Revision 1.3  1995/11/01  16:34:57  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.2  1995/11/01  13:54:48  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:08  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */


#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <oid.h>
#include <log.h>
#include <marcdisp.h>
#include <readconf.h>
#include <xmalloc.h>
#include <data1.h>
#include <tpath.h>

data1_marctab *data1_read_marctab (data1_handle dh, const char *file)
{
    FILE *f;
    NMEM mem = data1_nmem_get (dh);
    data1_marctab *res = (data1_marctab *)nmem_malloc(mem, sizeof(*res));
    char line[512], *argv[50];
    int lineno = 0;
    int argc;
    
    if (!(f = yaz_path_fopen(data1_get_tabpath(dh), file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }

    res->name = 0;
    res->reference = VAL_NONE;
    res->next = 0;
    res->length_data_entry = 4;
    res->length_starting = 5;
    res->length_implementation = 0;
    strcpy(res->future_use, "4");

    strcpy(res->record_status, "n");
    strcpy(res->implementation_codes, "    ");
    res->indicator_length = 2;
    res->identifier_length = 2;
    res->force_indicator_length = -1;
    res->force_identifier_length = -1;
    strcpy(res->user_systems, "z  ");
    
    while ((argc = readconf_line(f, &lineno, line, 512, argv, 50)))
	if (!strcmp(*argv, "name"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d:Missing arg for %s", file, lineno,
		     *argv);
		continue;
	    }
	    res->name = nmem_strdup(mem, argv[1]);
	}
	else if (!strcmp(*argv, "reference"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Missing arg for %s", file, lineno,
		     *argv);
		continue;
	    }
	    if ((res->reference = oid_getvalbyname(argv[1])) == VAL_NONE)
	    {
		logf(LOG_WARN, "%s:%d: Unknown tagset reference '%s'",
		     file, lineno, argv[1]);
		continue;
	    }
	}
	else if (!strcmp(*argv, "length-data-entry"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Missing arg for %s", file, lineno,
		     *argv);
		continue;
	    }
	    res->length_data_entry = atoi(argv[1]);
	}
	else if (!strcmp(*argv, "length-starting"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Missing arg for %s", file, lineno,
		     *argv);
		continue;
	    }
	    res->length_starting = atoi(argv[1]);
	}
	else if (!strcmp(*argv, "length-implementation"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Missing arg for %s", file, lineno,
		     *argv);
		continue;
	    }
	    res->length_implementation = atoi(argv[1]);
	}
	else if (!strcmp(*argv, "future-use"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Missing arg for %s", file, lineno,
		     *argv);
		continue;
	    }
	    strncpy(res->future_use, argv[1], 2);
	}
	else if (!strcmp(*argv, "force-indicator-length"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Missing arg for %s", file, lineno,
		     *argv);
		continue;
	    }
	    res->force_indicator_length = atoi(argv[1]);
	}
	else if (!strcmp(*argv, "force-identifier-length"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s:%d: Missing arg for %s", file, lineno,
		     *argv);
		continue;
	    }
	    res->force_identifier_length = atoi(argv[1]);
	}
	else
	    logf(LOG_WARN, "%s:%d: Unknown directive '%s'", file, lineno,
		 *argv);

    fclose(f);
    return res;
}

/*
 * Locate some data under this node. This routine should handle variants
 * prettily.
 */
static char *get_data(data1_node *n, int *len)
{
    char *r;

    while (n->which != DATA1N_data && n->child)
	n = n->child;
    if (n->which != DATA1N_data || 
	(n->u.data.what != DATA1I_text && n->u.data.what != DATA1I_num))
    {
	r = "[Structured/included data]";
	*len = strlen(r);
	return r;
    }

    *len = n->u.data.len;
    while (*len && isspace(n->u.data.data[*len - 1]))
	(*len)--;
    return n->u.data.data;
}

static void memint (char *p, int val, int len)
{
    char buf[10];

    if (len == 1)
        *p = val + '0';
    else
    {
        sprintf (buf, "%08d", val);
        memcpy (p, buf+8-len, len);
    }
}

static int is_indicator (data1_marctab *p, data1_node *subf)
{
#if 1
    if (p->indicator_length != 2 ||
	(subf->which == DATA1N_tag && strlen(subf->u.tag.tag) == 2))
	return 1;
#else
    if (subf->which == DATA1N_tag && subf->child->which == DATA1N_tag)
	return 1;
#endif
    return 0;
}

static int nodetomarc(data1_marctab *p, data1_node *n, int selected,
    char **buf, int *size)
{
    int len = 26;
    int dlen;
    int base_address = 25;
    int entry_p, data_p;
    char *op;
    data1_node *field, *subf;

    logf (LOG_DEBUG, "nodetomarc");
    for (field = n->child; field; field = field->next)
    {
	if (field->which != DATA1N_tag)
	{
	    logf(LOG_WARN, "Malformed field composition for marc output.");
	    return -1;
	}
	if (selected && !field->u.tag.node_selected)
	    continue;
        len += 4 + p->length_data_entry + p->length_starting
            + p->length_implementation;
        base_address += 3 + p->length_data_entry + p->length_starting
            + p->length_implementation;
	if (strncmp(field->u.tag.tag, "00", 2))
            len += p->indicator_length;      /* this is fairly bogus */
	subf = field->child;
	
	/*  we'll allow no indicator if length is not 2 */
	if (is_indicator (p, subf))
	    subf = subf->child;

        for (; subf; subf = subf->next)
        {
	    if (subf->which != DATA1N_tag)
	    {
		logf(LOG_WARN,
		    "Malformed subfield composition for marc output.");
		return -1;
	    }
            if (strncmp(field->u.tag.tag, "00", 2))
                len += p->identifier_length;
	    get_data(subf, &dlen);
            len += dlen;
        }
    }

    if (!*buf)
	*buf = (char *)xmalloc(*size = len);
    else if (*size <= len)
	*buf = (char *)xrealloc(*buf, *size = len);
	
    op = *buf;
    memint (op, len, 5);
    memcpy (op+5, p->record_status, 1);
    memcpy (op+6, p->implementation_codes, 4);
    memint (op+10, p->indicator_length, 1);
    memint (op+11, p->identifier_length, 1);
    memint (op+12, base_address, 5);
    memcpy (op+17, p->user_systems, 3);
    memint (op+20, p->length_data_entry, 1);
    memint (op+21, p->length_starting, 1);
    memint (op+22, p->length_implementation, 1);
    memcpy (op+23, p->future_use, 1);
    
    entry_p = 24;
    data_p = base_address;

    for (field = n->child; field; field = field->next)
    {
        int data_0 = data_p;
	char *indicator_data = "    ";
	if (selected && !field->u.tag.node_selected)
	    continue;

	subf = field->child;

	if (is_indicator (p, subf))
	{
            indicator_data = subf->u.tag.tag;
	    subf = subf->child;
	}
        if (strncmp(field->u.tag.tag, "00", 2))   /* bogus */
        {
            memcpy (op + data_p, indicator_data, p->indicator_length);
            data_p += p->indicator_length;
        }
        for (; subf; subf = subf->next)
        {
	    char *data;

            if (strncmp(field->u.tag.tag, "00", 2))
            {
                op[data_p] = ISO2709_IDFS;
                memcpy (op + data_p+1, subf->u.tag.tag, p->identifier_length-1);
                data_p += p->identifier_length;
            }
	    data = get_data(subf, &dlen);
            memcpy (op + data_p, data, dlen);
            data_p += dlen;
        }
        op[data_p++] = ISO2709_FS;

        memcpy (op + entry_p, field->u.tag.tag, 3);
        entry_p += 3;
        memint (op + entry_p, data_p - data_0, p->length_data_entry);
        entry_p += p->length_data_entry;
        memint (op + entry_p, data_0 - base_address, p->length_starting);
        entry_p += p->length_starting;
        entry_p += p->length_implementation;
    }
    op[entry_p++] = ISO2709_FS;
    assert (entry_p == base_address);
    op[data_p++] = ISO2709_RS;
    assert (data_p == len);
    return len;
}

char *data1_nodetomarc(data1_handle dh, data1_marctab *p, data1_node *n,
		       int selected, int *len)
{
    int *size;
    char **buf = data1_get_map_buf (dh, &size);

    *len = nodetomarc(p, n, selected, buf, size);
    return *buf;
}
