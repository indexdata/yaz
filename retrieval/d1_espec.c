/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_espec.c,v $
 * Revision 1.8  1997-09-05 09:50:56  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.7  1997/05/14 06:54:02  adam
 * C++ support.
 *
 * Revision 1.6  1996/07/06 19:58:34  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.5  1996/01/02  08:57:44  quinn
 * Changed enums in the ASN.1 .h files to #defines. Changed oident.class to oclass
 *
 * Revision 1.4  1995/12/05  11:16:10  quinn
 * Fixed malloc of 0.
 *
 * Revision 1.3  1995/11/13  09:27:34  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.2  1995/11/01  16:34:56  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.1  1995/11/01  11:56:07  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <xmalloc.h>
#include <odr.h>
#include <proto.h>
#include <log.h>
#include <readconf.h>
#include <tpath.h>
#include <data1.h>

static Z_Variant *read_variant(int argc, char **argv, ODR o)
{
    Z_Variant *r = odr_malloc(o, sizeof(*r));
    oident var1;
    int i;

    var1.proto = PROTO_Z3950;
    var1.oclass = CLASS_VARSET;
    var1.value = VAL_VAR1;
    r->globalVariantSetId = odr_oiddup(o, oid_getoidbyent(&var1));

    if (argc)
	r->triples = odr_malloc(o, sizeof(Z_Triple*) * argc);
    else
	r->triples = 0;
    r->num_triples = argc;
    for (i = 0; i < argc; i++)
    {
	int zclass, type;
	char value[512];
	Z_Triple *t;

	if (sscanf(argv[i], "(%d,%d,%[^)])", &zclass, &type, value) < 3)
	{
	    logf(LOG_WARN, "Syntax error in variant component '%s'",
	    	argv[i]);
	    return 0;
	}
	t = r->triples[i] = odr_malloc(o, sizeof(Z_Triple));
	t->variantSetId = 0;
	t->zclass = odr_malloc(o, sizeof(int));
	*t->zclass = zclass;
	t->type = odr_malloc(o, sizeof(int));
	*t->type = type;
	/*
	 * This is wrong.. we gotta look up the correct type for the
	 * variant, I guess... damn this stuff.
	 */
	if (*value == '@')
	{
	    t->which = Z_Triple_null;
	    t->value.null = ODR_NULLVAL;
	}
	else if (isdigit(*value))
	{
	    t->which = Z_Triple_integer;
	    t->value.integer = odr_malloc(o, sizeof(*t->value.integer));
	    *t->value.integer = atoi(value);
	}
	else
	{
	    t->which = Z_Triple_internationalString;
	    t->value.internationalString = odr_malloc(o, strlen(value)+1);
	    strcpy(t->value.internationalString, value);
	}
    }
    return r;
}

static Z_Occurrences *read_occurrences(char *occ, ODR o)
{
    Z_Occurrences *op = odr_malloc(o, sizeof(*op));
    char *p;

    if (!occ)
    {
	op->which = Z_Occurrences_values;
	op->u.values = odr_malloc(o, sizeof(Z_OccurValues));
	op->u.values->start = odr_malloc(o, sizeof(int));
	*op->u.values->start = 1;
	op->u.values->howMany = 0;
    }
    else if (!strcmp(occ, "all"))
    {
	op->which = Z_Occurrences_all;
	op->u.all = ODR_NULLVAL;
    }
    else if (!strcmp(occ, "last"))
    {
	op->which = Z_Occurrences_last;
	op->u.all = ODR_NULLVAL;
    }
    else
    {
	Z_OccurValues *ov = odr_malloc(o, sizeof(*ov));

	if (!isdigit(*occ))
	{
	    logf(LOG_WARN, "Bad occurrences-spec in %s", occ);
	    return 0;
	}
	op->which = Z_Occurrences_values;
	op->u.values = ov;
	ov->start = odr_malloc(o, sizeof(*ov->start));
	*ov->start = atoi(occ);
	if ((p = strchr(occ, '+')))
	{
	    ov->howMany = odr_malloc(o, sizeof(*ov->howMany));
	    *ov->howMany = atoi(p + 1);
	}
	else
	    ov->howMany = 0;
    }
    return op;
}


static Z_ETagUnit *read_tagunit(char *buf, ODR o)
{
    Z_ETagUnit *u = odr_malloc(o, sizeof(*u));
    int terms;
    int type;
    char value[512], occ[512];

    if (*buf == '*')
    {
	u->which = Z_ETagUnit_wildPath;
	u->u.wildPath = ODR_NULLVAL;
    }
    else if (*buf == '?')
    {
	u->which = Z_ETagUnit_wildThing;
	if (buf[1] == ':')
	    u->u.wildThing = read_occurrences(buf+2, o);
	else
	    u->u.wildThing = read_occurrences(0, o);
    }
    else if ((terms = sscanf(buf, "(%d,%[^)]):%[a-z0-9+]", &type, value,
	occ)) >= 2)
    {
	int numval;
	Z_SpecificTag *t;
	char *valp = value;
	int force_string = 0;

	if (*valp == '\'')
	{
	    valp++;
	    force_string = 1;
	}
	u->which = Z_ETagUnit_specificTag;
	u->u.specificTag = t = odr_malloc(o, sizeof(*t));
	t->tagType = odr_malloc(o, sizeof(*t->tagType));
	*t->tagType = type;
	t->tagValue = odr_malloc(o, sizeof(*t->tagValue));
	if (!force_string && (numval = atoi(valp)))
	{
	    t->tagValue->which = Z_StringOrNumeric_numeric;
	    t->tagValue->u.numeric = odr_malloc(o, sizeof(int));
	    *t->tagValue->u.numeric = numval;
	}
	else
	{
	    t->tagValue->which = Z_StringOrNumeric_string;
	    t->tagValue->u.string = odr_malloc(o, strlen(valp)+1);
	    strcpy(t->tagValue->u.string, valp);
	}
	if (terms > 2) /* an occurrences-spec exists */
	    t->occurrences = read_occurrences(occ, o);
	else
	    t->occurrences = 0;
    }
    return u;
}

/*
 * Read an element-set specification from a file.
 * NOTE: If !o, memory is allocated directly from the heap by odr_malloc().
 */
Z_Espec1 *data1_read_espec1(char *file, ODR o)
{
    FILE *f;
    int argc, size_esn = 0;
    char *argv[50], line[512];
    Z_Espec1 *res = odr_malloc(o, sizeof(*res));

    if (!(f = yaz_path_fopen(data1_get_tabpath(), file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }

    res->num_elementSetNames = 0;
    res->elementSetNames = 0;
    res->defaultVariantSetId = 0;
    res->defaultVariantRequest = 0;
    res->defaultTagType = 0;
    res->num_elements = 0;
    res->elements = 0;

    while ((argc = readconf_line(f, line, 512, argv, 50)))
	if (!strcmp(argv[0], "elementsetnames"))
	{
	    int nnames = argc-1, i;

	    if (!nnames)
	    {
		logf(LOG_WARN, "%s: Empty elementsetnames directive",
		    file);
		continue;
	    }

	    res->elementSetNames = odr_malloc(o, sizeof(char**)*nnames);
	    for (i = 0; i < nnames; i++)
	    {
		res->elementSetNames[i] = odr_malloc(o, strlen(argv[i+1])+1);
		strcpy(res->elementSetNames[i], argv[i+1]);
	    }
	    res->num_elementSetNames = nnames;
	}
	else if (!strcmp(argv[0], "defaultvariantsetid"))
	{
	    if (argc != 2 || !(res->defaultVariantSetId =
		odr_getoidbystr(o, argv[1])))
	    {
		logf(LOG_WARN, "%s: Bad defaultvariantsetid directive", file);
		continue;
	    }
	}
	else if (!strcmp(argv[0], "defaulttagtype"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s: Bad defaulttagtype directive", file);
		continue;
	    }
	    res->defaultTagType = odr_malloc(o, sizeof(int));
	    *res->defaultTagType = atoi(argv[1]);
	}
	else if (!strcmp(argv[0], "defaultvariantrequest"))
	{
	    if (!(res->defaultVariantRequest = read_variant(argc-1, argv+1, o)))
	    {
		logf(LOG_WARN, "%s: Bad defaultvariantrequest", file);
		continue;
	    }
	}
	else if (!strcmp(argv[0], "simpleelement"))
	{
	    Z_ElementRequest *er;
	    Z_SimpleElement *se;
	    Z_ETagPath *tp;
	    char *path = argv[1];
	    char *ep;
	    int num, i = 0;
	    
	    if (!res->elements)
		res->elements = odr_malloc(o, size_esn = 24*sizeof(er));
	    else if (res->num_elements >= size_esn/sizeof(er))
	    {
		size_esn *= 2;
		if (o)
		{
		    Z_ElementRequest **oe = res->elements;
		    
		    res->elements = odr_malloc (o, size_esn*sizeof(er));
		    memcpy (res->elements, oe, size_esn/2);
		}
		else
		    res->elements =
			xrealloc(res->elements, size_esn*sizeof(er));
	    }
	    if (argc < 2)
	    {
		logf(LOG_WARN, "%s: Empty simpleelement directive", file);
		continue;
	    }
	    
	    res->elements[res->num_elements++] = er =
		odr_malloc(o, sizeof(*er));
	    er->which = Z_ERequest_simpleElement;
	    er->u.simpleElement = se = odr_malloc(o, sizeof(*se));
	    se->variantRequest = 0;
	    se->path = tp = odr_malloc(o, sizeof(*tp));
	    tp->num_tags = 0;
	    /*
	     * Parse the element selector.
	     */
	    for (num = 1, ep = path; (ep = strchr(ep, '/')); num++, ep++)
		;
	    tp->tags = odr_malloc(o, sizeof(Z_ETagUnit*)*num);
	    
	    for ((ep = strchr(path, '/')) ; path ;
		 (void)((path = ep) && (ep = strchr(path, '/'))))
	    {
		if (ep)
		    ep++;
		
		assert(i<num);
		tp->tags[tp->num_tags++] = read_tagunit(path, o);
	    }
	    
	    if (argc > 2 && !strcmp(argv[2], "variant"))
		se->variantRequest= read_variant(argc-3, argv+3, o);
	}
	else
	{
	    logf(LOG_WARN, "%s: Unknown directive %s", file, argv[0]);
	    fclose(f);
	    return 0;
	}

    return res;
}
