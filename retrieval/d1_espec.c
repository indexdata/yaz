/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: d1_espec.c,v 1.22 2002-05-03 13:48:27 adam Exp $
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <yaz/odr.h>
#include <yaz/proto.h>
#include <yaz/log.h>
#include <yaz/data1.h>

static Z_Variant *read_variant(int argc, char **argv, NMEM nmem,
			       const char *file, int lineno)
{
    Z_Variant *r = (Z_Variant *)nmem_malloc(nmem, sizeof(*r));
    oident var1;
    int i;
    int oid[OID_SIZE];

    var1.proto = PROTO_Z3950;
    var1.oclass = CLASS_VARSET;
    var1.value = VAL_VAR1;
    r->globalVariantSetId = odr_oiddup_nmem(nmem, oid_ent_to_oid(&var1, oid));

    if (argc)
	r->triples = (Z_Triple **)nmem_malloc(nmem, sizeof(Z_Triple*) * argc);
    else
	r->triples = 0;
    r->num_triples = argc;
    for (i = 0; i < argc; i++)
    {
	int zclass, type;
	char value[512];
	Z_Triple *t;

	if (sscanf(argv[i], "(%d,%d,%511[^)])", &zclass, &type, value) < 3)
	{
	    yaz_log(LOG_WARN, "%s:%d: Syntax error in variant component '%s'",
		    file, lineno, argv[i]);
	    return 0;
	}
	t = r->triples[i] = (Z_Triple *)nmem_malloc(nmem, sizeof(Z_Triple));
	t->variantSetId = 0;
	t->zclass = (int *)nmem_malloc(nmem, sizeof(int));
	*t->zclass = zclass;
	t->type = (int *)nmem_malloc(nmem, sizeof(int));
	*t->type = type;
	/*
	 * This is wrong.. we gotta look up the correct type for the
	 * variant, I guess... damn this stuff.
	 */
	if (*value == '@')
	{
	    t->which = Z_Triple_null;
	    t->value.null = odr_nullval();
	}
	else if (d1_isdigit(*value))
	{
	    t->which = Z_Triple_integer;
	    t->value.integer = (int *)
		nmem_malloc(nmem, sizeof(*t->value.integer));
	    *t->value.integer = atoi(value);
	}
	else
	{
	    t->which = Z_Triple_internationalString;
	    t->value.internationalString = (char *)
		nmem_malloc(nmem, strlen(value)+1);
	    strcpy(t->value.internationalString, value);
	}
    }
    return r;
}

static Z_Occurrences *read_occurrences(char *occ, NMEM nmem,
				       const char *file, int lineno)
{
    Z_Occurrences *op = (Z_Occurrences *)nmem_malloc(nmem, sizeof(*op));
    char *p;
    
    if (!occ)
    {
	op->which = Z_Occurrences_values;
	op->u.values = (Z_OccurValues *)
	    nmem_malloc(nmem, sizeof(Z_OccurValues));
	op->u.values->start = (int *)nmem_malloc(nmem, sizeof(int));
	*op->u.values->start = 1;
	op->u.values->howMany = 0;
    }
    else if (!strcmp(occ, "all"))
    {
	op->which = Z_Occurrences_all;
	op->u.all = odr_nullval();
    }
    else if (!strcmp(occ, "last"))
    {
	op->which = Z_Occurrences_last;
	op->u.all = odr_nullval();
    }
    else
    {
	Z_OccurValues *ov = (Z_OccurValues *)nmem_malloc(nmem, sizeof(*ov));
    
	if (!d1_isdigit(*occ))
	{
	    yaz_log(LOG_WARN, "%s:%d: Bad occurrences-spec %s",
		    file, lineno, occ);
	    return 0;
	}
	op->which = Z_Occurrences_values;
	op->u.values = ov;
	ov->start = (int *)nmem_malloc(nmem, sizeof(*ov->start));
	*ov->start = atoi(occ);
	if ((p = strchr(occ, '+')))
	{
	    ov->howMany = (int *)nmem_malloc(nmem, sizeof(*ov->howMany));
	    *ov->howMany = atoi(p + 1);
	}
	else
	    ov->howMany = 0;
    }
    return op;
}


static Z_ETagUnit *read_tagunit(char *buf, NMEM nmem,
				const char *file, int lineno)
{
    Z_ETagUnit *u = (Z_ETagUnit *)nmem_malloc(nmem, sizeof(*u));
    int terms;
    int type;
    char value[512], occ[512];
    
    if (*buf == '*')
    {
	u->which = Z_ETagUnit_wildPath;
	u->u.wildPath = odr_nullval();
    }
    else if (*buf == '?')
    {
	u->which = Z_ETagUnit_wildThing;
	if (buf[1] == ':')
	    u->u.wildThing = read_occurrences(buf+2, nmem, file, lineno);
	else
	    u->u.wildThing = read_occurrences(0, nmem, file, lineno);
    }
    else if ((terms = sscanf(buf, "(%d,%511[^)]):%511[a-zA-Z0-9+]",
                             &type, value, occ)) >= 2)
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
	u->u.specificTag = t = (Z_SpecificTag *)nmem_malloc(nmem, sizeof(*t));
	t->tagType = (int *)nmem_malloc(nmem, sizeof(*t->tagType));
	*t->tagType = type;
	t->tagValue = (Z_StringOrNumeric *)
	    nmem_malloc(nmem, sizeof(*t->tagValue));
	if (!force_string && (numval = atoi(valp)))
	{
	    t->tagValue->which = Z_StringOrNumeric_numeric;
	    t->tagValue->u.numeric = (int *)nmem_malloc(nmem, sizeof(int));
	    *t->tagValue->u.numeric = numval;
	}
	else
	{
	    t->tagValue->which = Z_StringOrNumeric_string;
	    t->tagValue->u.string = (char *)nmem_malloc(nmem, strlen(valp)+1);
	    strcpy(t->tagValue->u.string, valp);
	}
	if (terms > 2) /* an occurrences-spec exists */
	    t->occurrences = read_occurrences(occ, nmem, file, lineno);
	else
	    t->occurrences = 0;
    }
    return u;
}

/*
 * Read an element-set specification from a file.
 * NOTE: If !o, memory is allocated directly from the heap by nmem_malloc().
 */
Z_Espec1 *data1_read_espec1 (data1_handle dh, const char *file)
{
    FILE *f;
    NMEM nmem = data1_nmem_get (dh);
    int lineno = 0;
    int argc, size_esn = 0;
    char *argv[50], line[512];
    Z_Espec1 *res = (Z_Espec1 *)nmem_malloc(nmem, sizeof(*res));
    
    if (!(f = data1_path_fopen(dh, file, "r")))
    {
	yaz_log(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }
    
    res->num_elementSetNames = 0;
    res->elementSetNames = 0;
    res->defaultVariantSetId = 0;
    res->defaultVariantRequest = 0;
    res->defaultTagType = 0;
    res->num_elements = 0;
    res->elements = 0;
    
    while ((argc = readconf_line(f, &lineno, line, 512, argv, 50)))
	if (!strcmp(argv[0], "elementsetnames"))
	{
	    int nnames = argc-1, i;
	    
	    if (!nnames)
	    {
		yaz_log(LOG_WARN, "%s:%d: Empty elementsetnames directive",
			file, lineno);
		continue;
	    }
	    
	    res->elementSetNames =
		(char **)nmem_malloc(nmem, sizeof(char**)*nnames);
	    for (i = 0; i < nnames; i++)
	    {
		res->elementSetNames[i] = (char *)
		    nmem_malloc(nmem, strlen(argv[i+1])+1);
		strcpy(res->elementSetNames[i], argv[i+1]);
	    }
	    res->num_elementSetNames = nnames;
	}
	else if (!strcmp(argv[0], "defaultvariantsetid"))
	{
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args for %s",
			file, lineno, argv[0]);
		continue;
	    }
	    if (!(res->defaultVariantSetId =
		  odr_getoidbystr_nmem(nmem, argv[1])))
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad defaultvariantsetid",
			file, lineno);
		continue;
	    }
	}
	else if (!strcmp(argv[0], "defaulttagtype"))
	{
	    if (argc != 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args for %s",
			file, lineno, argv[0]);
		continue;
	    }
	    res->defaultTagType = (int *)nmem_malloc(nmem, sizeof(int));
	    *res->defaultTagType = atoi(argv[1]);
	}
	else if (!strcmp(argv[0], "defaultvariantrequest"))
	{
	    if (!(res->defaultVariantRequest =
		  read_variant(argc-1, argv+1, nmem, file, lineno)))
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad defaultvariantrequest",
			file, lineno);
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
		res->elements = (Z_ElementRequest **)
		    nmem_malloc(nmem, size_esn = 24*sizeof(er));
	    else if (res->num_elements >= (int) (size_esn/sizeof(er)))
	    {
		Z_ElementRequest **oe = res->elements;
		size_esn *= 2;
		res->elements = (Z_ElementRequest **)
		    nmem_malloc (nmem, size_esn*sizeof(er));
		memcpy (res->elements, oe, size_esn/2);
	    }
	    if (argc < 2)
	    {
		yaz_log(LOG_WARN, "%s:%d: Bad # of args for %s",
			file, lineno, argv[0]);
		continue;
	    }
	    
	    res->elements[res->num_elements++] = er =
		(Z_ElementRequest *)nmem_malloc(nmem, sizeof(*er));
	    er->which = Z_ERequest_simpleElement;
	    er->u.simpleElement = se = (Z_SimpleElement *)
		nmem_malloc(nmem, sizeof(*se));
	    se->variantRequest = 0;
	    se->path = tp = (Z_ETagPath *)nmem_malloc(nmem, sizeof(*tp));
	    tp->num_tags = 0;
	    /*
	     * Parse the element selector.
	     */
	    for (num = 1, ep = path; (ep = strchr(ep, '/')); num++, ep++)
		;
	    tp->tags = (Z_ETagUnit **)
		nmem_malloc(nmem, sizeof(Z_ETagUnit*)*num);
	    
	    for ((ep = strchr(path, '/')) ; path ;
		 (void)((path = ep) && (ep = strchr(path, '/'))))
	    {
		Z_ETagUnit *tagunit;
		if (ep)
		    ep++;
		
		assert(i<num);
		tagunit = read_tagunit(path, nmem, file, lineno);
		if (!tagunit)
		{
		    yaz_log (LOG_WARN, "%s%d: Bad tag unit at %s",
			     file, lineno, path);
		    break;
		}
		tp->tags[tp->num_tags++] = tagunit;
	    }
	    
	    if (argc > 2 && !strcmp(argv[2], "variant"))
		se->variantRequest=
		    read_variant(argc-3, argv+3, nmem, file, lineno);
	}
	else
	    yaz_log(LOG_WARN, "%s:%d: Unknown directive '%s'",
		    file, lineno, argv[0]);
    fclose (f);
    return res;
}
